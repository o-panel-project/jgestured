/*****************************************************************************
 *
 * grail - Gesture Recognition And Instantiation Library
 *
 * Copyright (C) 2010 Canonical Ltd.
 * Copyright (C) 2010 Henrik Rydberg <rydberg@bitmath.org>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

#include "grail-inserter.h"
#include "grail-recognizer.h"
#include "grail-impl.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>

#define DIM_FRAMES 100
#define FRAME_RATE 100

void grail_filter_abs_events(struct grail *ge, int usage)
{
	struct grail_impl *x = ge->impl;
	x->filter_abs = usage;
}

int grail_open(struct grail *ge, int fd)
{
	struct grail_impl *x;
	int ret;
	x = calloc(1, sizeof(*x));
	if (!x)
		return -ENOMEM;

	x->evemu = evemu_new(0);
	if (!x->evemu) {
		ret = -ENOMEM;
		goto freemem;
	}
	ret = evemu_extract(x->evemu, fd);
	if (ret)
		goto freemem;
	if (!utouch_frame_is_supported_mtdev(x->evemu)) {
		ret = -ENODEV;
		goto freemem;
	}
	x->mtdev = mtdev_new_open(fd);
	if (!x->mtdev) {
		ret = -ENOMEM;
		goto freemem;
	}

	x->fh = utouch_frame_new_engine(DIM_FRAMES, DIM_TOUCH, FRAME_RATE);
	if (!x->fh) {
		ret = -ENOMEM;
		goto freedev;
	}
	ret = utouch_frame_init_mtdev(x->fh, x->evemu);
	if (ret)
		goto freeframe;

	ge->impl = x;

	ret = gin_init(ge);
	if (ret)
		goto freeframe;

	ret = gru_init(ge);
	if (ret)
		goto freegin;

	return 0;
 freegin:
	gin_destroy(ge);
 freeframe:
	utouch_frame_delete_engine(x->fh);
 freedev:
	mtdev_close_delete(x->mtdev);
 freemem:
	evemu_delete(x->evemu);
	free(x);
	ge->impl = 0;
	return ret;
}

void grail_close(struct grail *ge, int fd)
{
	struct grail_impl *x = ge->impl;
	gru_destroy(ge);
	gin_destroy(ge);
	utouch_frame_delete_engine(x->fh);
	mtdev_close_delete(x->mtdev);
	evemu_delete(x->evemu);
	free(x);
	ge->impl = 0;
}

int grail_idle(struct grail *ge, int fd, int ms)
{
	struct grail_impl *x = ge->impl;
	return mtdev_idle(x->mtdev, fd, ms);
}

void grail_get_units(const struct grail *ge,
		     struct grail_coord *min, struct grail_coord *max)
{
	struct utouch_surface *s = utouch_frame_get_surface(ge->impl->fh);
	min->x = s->min_x;
	min->y = s->min_y;
	max->x = s->max_x;
	max->y = s->max_y;
}

#if defined(JPANEL_TOUCHSCREEN)
void grail_get_mapped_units(const struct grail *ge,
		     struct grail_coord *min, struct grail_coord *max)
{
	struct utouch_surface *s = utouch_frame_get_surface(ge->impl->fh);
	min->x = s->mapped_min_x;
	min->y = s->mapped_min_y;
	max->x = s->mapped_max_x;
	max->y = s->mapped_max_y;
}
void grail_get_phys_units(const struct grail *ge,
		     struct grail_coord *box)
{
	struct utouch_surface *s = utouch_frame_get_surface(ge->impl->fh);
	box->x = s->phys_width;
	box->y = s->phys_height;
}
#endif

static void flush_events(struct grail *ge)
{
	struct grail_impl *impl = ge->impl;
	struct input_event iev;

	grailbuf_clear(&impl->gbuf);
	while (!evbuf_empty(&impl->evbuf)) {
		evbuf_get(&impl->evbuf, &iev);
		if (ge->event)
			ge->event(ge, &iev);
	}
}

static int skip_event(const struct input_event *ev, int count)
{
	switch (ev->type) {
	case EV_ABS:
		return 1;
	case EV_KEY:
		return ev->code >= BTN_DIGI && ev->code < BTN_WHEEL;
	case EV_SYN:
		switch (ev->code) {
		case SYN_MT_REPORT:
			return 1;
		case SYN_REPORT:
			return count == 0;
		default:
			return 0;
		}
	default:
		return 0;
	}
}

static void flush_gestures(struct grail *ge)
{
	struct grail_impl *impl = ge->impl;
	struct input_event iev;
	struct grail_event gev;
	int count = 0;

	while (!evbuf_empty(&impl->evbuf)) {
		evbuf_get(&impl->evbuf, &iev);
		if (skip_event(&iev, count))
			continue;
		if (ge->event)
			ge->event(ge, &iev);
		if (iev.type == EV_SYN && iev.code == SYN_REPORT)
			count = 0;
		else
			count++;
	}
	while (!grailbuf_empty(&impl->gbuf)) {
		grailbuf_get(&impl->gbuf, &gev);
		if (ge->gesture)
			ge->gesture(ge, &gev);
	}
}

static int gesture_timeout(struct grail *ge, const struct utouch_frame *frame)
{
	struct gesture_recognizer *gru = ge->gru;
	struct gesture_inserter *gin = ge->gin;

	return grail_mask_count(gin->used, sizeof(gin->used)) == 0 &&
		frame->time - frame->mod_time > gru->move.fm[FM_X].hold_ms;
}

static void report_frame(struct grail *ge,
			 const struct utouch_frame *frame,
			 const struct input_event *syn)
{
	struct grail_impl *impl = ge->impl;
	struct grail_event gev;

	ge->impl->frame = frame;

	if (frame->num_active && !frame->prev->num_active) {
		impl->ongoing = 1;
		impl->gesture = 0;
	}

	if (!impl->ongoing)
		return;

	gin_frame_begin(ge, frame);
	gru_recognize(ge, frame);
	gin_frame_end(ge, frame);

	if (!grailbuf_empty(&impl->gbuf))
		impl->gesture = 1;

	if (frame->num_active == 0 || gesture_timeout(ge, frame))
		impl->ongoing &= impl->gesture;
}

static void report_frame_raw(const struct utouch_frame *frame)
{
	int i;

	for (i = 0; i < frame->num_active; i++) {
		const struct utouch_contact *t = frame->active[i];

		fprintf(stderr, "touch %d\n", i);
		fprintf(stderr, "  slot:         %d\n", t->slot);
		fprintf(stderr, "  id:           %d\n", t->id);
		fprintf(stderr, "  tool_type:    %d\n", t->tool_type);
		fprintf(stderr, "  x:            %f\n", t->x);
		fprintf(stderr, "  y:            %f\n", t->y);
		fprintf(stderr, "  touch_major:  %f\n", t->touch_major);
		fprintf(stderr, "  touch_minor:  %f\n", t->touch_minor);
		fprintf(stderr, "  width_major:  %f\n", t->width_major);
		fprintf(stderr, "  width_minor:  %f\n", t->width_minor);
		fprintf(stderr, "  angle:        %f\n", t->orientation);
		fprintf(stderr, "  pressure:     %f\n", t->pressure);
		fprintf(stderr, "  distance:     %f\n", t->distance);
		fprintf(stderr, "  vx:           %f\n", t->vx);
		fprintf(stderr, "  vy:           %f\n", t->vy);
	}

	fprintf(stderr, "sync %d %ld %d %d %d\n",
					frame->num_active,
					frame->time,
					frame->sequence_id,
					frame->revision,
					frame->slot_revision);
}

static void grail_pump_mtdev(struct grail *ge, const struct input_event *ev)
{
	struct grail_impl *impl = ge->impl;
	const struct utouch_frame *frame;

	evbuf_put(&impl->evbuf, ev);

	if (ev->type == EV_SYN || ev->type == EV_ABS) {
		frame = utouch_frame_pump_mtdev(impl->fh, ev);
		if (frame) {
			report_frame(ge, frame, ev);
#if !defined(JPANEL_TOUCHSCREEN)
			report_frame_raw(frame);
#endif
		}
	}

	if (ev->type == EV_SYN) {
		if (!impl->ongoing)
			flush_events(ge);
		if (impl->gesture)
			flush_gestures(ge);
	}
}

int grail_pull(struct grail *ge, int fd)
{
	struct grail_impl *impl = ge->impl;
	struct input_event ev;
        int ret, count = 0;

        while ((ret = mtdev_get(impl->mtdev, fd, &ev, 1)) > 0) {
		grail_pump_mtdev(ge, &ev);
                count++;
        }

        return count > 0 ? count : ret;
}
