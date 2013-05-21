/*****************************************************************************
 *
 * utouch-frame - Touch Frame Library
 *
 * Copyright (C) 2010 Canonical Ltd.
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

#include "frame-impl.h"
#include <errno.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

unsigned int utouch_frame_get_version(void)
{
	return UTOUCH_FRAME_VERSION;
}

unsigned int utouch_frame_get_num_frames(utouch_frame_handle fh)
{
	return fh->num_frames;
}

unsigned int utouch_frame_get_num_slots(utouch_frame_handle fh)
{
	return fh->num_slots;
}

static void destroy_slots(struct utouch_contact **slots, int nslot)
{
	int i;

	if (slots) {
		for (i = nslot - 1; i >= 0; i--)
			free(slots[i]);
		free(slots);
	}
}

static void destroy_frame(struct utouch_frame *frame, int nslot)
{
	if (frame) {
		destroy_slots(frame->slots, nslot);
		free(frame->active);
		free(frame);
	}
}

static void destroy_frames(struct utouch_frame **frames, int nframe, int nslot)
{
	int i;

	if (frames) {
		for (i = nframe - 1; i >= 0; i--)
			destroy_frame(frames[i], nslot);
		free(frames);
	}
}

static struct utouch_contact **create_slots(int nslot, int size)
{
	struct utouch_contact **slots;
	struct utouch_contact *s;
	int i;

	slots = calloc(nslot, sizeof(slots[0]));
	if (!slots)
		return 0;

	for (i = 0; i < nslot; i++) {
		s = calloc(1, size);
		if (!s)
			goto out;
		slots[i] = s;
		s->slot = i;
		s->active = 0;
	}

	return slots;
out:
	destroy_slots(slots, nslot);
	return 0;
}

static struct utouch_frame *create_frame(int nslot,
					 int frame_size, int slot_size)
{
	struct utouch_frame *frame;

	frame = calloc(1, frame_size);
	if (!frame)
		return 0;

	frame->active = calloc(nslot, sizeof(frame->active[0]));
	frame->slots = create_slots(nslot, slot_size);
	if (!frame->active || !frame->slots)
		goto out;

	return frame;
out:
	destroy_frame(frame, nslot);
	return 0;
}

static struct utouch_frame **create_frames(int nframe, int nslot,
					   int frame_size, int slot_size)
{
	struct utouch_frame **frames;
	struct utouch_frame *f;
	int i;

	frames = calloc(nframe, sizeof(frames[0]));
	if (!frames)
		return 0;

	for (i = 0; i < nframe; i++) {
		f = create_frame(nslot, frame_size, slot_size);
		if (!f)
			goto out;
		frames[i] = f;
	}

	return frames;
out:
	destroy_frames(frames, nframe, nslot);
	return 0;
}

utouch_frame_handle utouch_frame_new_engine_raw(unsigned int nframe,
						unsigned int nslot,
						unsigned int rate,
						unsigned int version,
						unsigned int surface_size,
						unsigned int frame_size,
						unsigned int slot_size)
{
	struct utouch_frame *f, *pf;
	utouch_frame_handle fh;
	int i, j;

	fh = calloc(1, sizeof(struct utouch_frame_engine));
	if (!fh)
		return 0;

	fh->num_frames = nframe;
	fh->num_slots = nslot;
	fh->hold_ms = 1000 / rate;

	surface_size = MAX(surface_size, sizeof(struct utouch_surface));
	frame_size = MAX(frame_size, sizeof(struct utouch_frame));
	slot_size = MAX(slot_size, sizeof(struct utouch_contact));

	fh->surface = calloc(1, surface_size);
	fh->frames = create_frames(nframe, nslot, frame_size, slot_size);
	fh->next = create_frame(nslot, frame_size, slot_size);
	if (!fh->surface || !fh->frames || !fh->next)
		goto out;

	pf = fh->frames[nframe - 1];
	for (i = 0; i < nframe; i++) {
		f = fh->frames[i];
		for (j = 0; j < nslot; j++)
			f->slots[j]->prev = pf->slots[j];
		f->prev = pf;
		pf = f;
	}

	return fh;
out:
	utouch_frame_delete_engine(fh);
	return 0;
}

void utouch_frame_delete_engine(utouch_frame_handle fh)
{
	free(fh->evmap);
	destroy_frame(fh->next, fh->num_slots);
	destroy_frames(fh->frames, fh->num_frames, fh->num_slots);
	free(fh->surface);
	free(fh);
}

struct utouch_surface *utouch_frame_get_surface(utouch_frame_handle fh)
{
	return fh->surface;
}

struct utouch_contact *utouch_frame_get_current_slot(utouch_frame_handle fh)
{
	return fh->next->slots[fh->slot];
}

int utouch_frame_set_current_slot(utouch_frame_handle fh, int slot)
{
	if (slot < 0 || slot >= fh->num_slots)
		return -EINVAL;
	fh->slot = slot;
	return 0;
}

int utouch_frame_set_current_id(utouch_frame_handle fh, int id)
{
	struct utouch_contact *t;
	int i;

	for (i = 0; i < fh->num_slots; i++) {
		t = fh->next->slots[i];
		if (t->active && t->id == id)
			return utouch_frame_set_current_slot(fh, i);
	}
	for (i = 0; i < fh->num_slots; i++) {
		t = fh->next->slots[i];
		if (!t->active) {
			t->active = 1;
			t->id = id;
			return utouch_frame_set_current_slot(fh, i);
		}
	}
	return -ENOMEM;
}

static void transform_slot(struct utouch_contact *slot,
			   const struct utouch_surface *s)
{
	float fx = (s->mapped_max_x - s->mapped_min_x) / (s->max_x - s->min_x);
	float fy = (s->mapped_max_y - s->mapped_min_y) / (s->max_y - s->min_y);
	/* assume clipped view for asymmetrical scaling */
	float f = MAX(fx, fy);

#if defined(GOODIX_TOUCHSCREEN) && defined(GOODIX_YRES)
	/* SwapAxes, InvertY */
	float tmp_x = fx * (slot->x - s->min_x) + s->mapped_min_x;
	float tmp_y = fy * (slot->y - s->min_y) + s->mapped_min_y;
	slot->x = tmp_y;
	slot->y = s->mapped_max_y - tmp_x;
#else
	slot->x = fx * (slot->x - s->min_x) + s->mapped_min_x;
	slot->y = fy * (slot->y - s->min_y) + s->mapped_min_y;
#endif
	slot->touch_major *= f;
	slot->touch_minor *= f;
	slot->width_major *= f;
	slot->width_minor *= f;
	slot->orientation *= M_PI_2 / s->max_orient;
	slot->pressure *= s->mapped_max_pressure / s->max_pressure;
	slot->distance *= f;
}

static void set_contact(utouch_frame_handle fh,
			struct utouch_contact *a,
			struct utouch_contact *b,
			utouch_frame_time_t dt)
{
	static const float D = 0.333;
	struct utouch_surface *s = fh->surface;
	const struct utouch_contact *ap = a->prev;

	a->active = b->active;
	a->id = b->id;
	a->tool_type = b->tool_type;
	a->x = b->x;
	a->y = b->y;
	a->touch_major = b->touch_major;
	a->touch_minor = s->use_touch_minor && b->touch_minor > 0 ?
		b->touch_minor : b->touch_major;
	a->width_major = b->width_major;
	a->width_minor = s->use_width_minor && b->width_minor > 0 ?
		b->width_minor : b->width_major;
	a->orientation = b->orientation;
	a->pressure = b->pressure;
	a->distance = b->distance;

	transform_slot(a, s);

	if (a->active && ap->active && a->id == ap->id) {
		a->x += b->vx * dt;
		a->y += b->vy * dt;
		if (dt > 0) {
			a->vx = (1 - D) * ap->vx + D * (a->x - ap->x) / dt;
			a->vy = (1 - D) * ap->vy + D * (a->y - ap->y) / dt;
		}
	} else {
		a->vx = 0;
		a->vy = 0;
	}

	b->vx = a->vx;
	b->vy = a->vy;
}

static int detect_addrem(const struct utouch_contact *a,
			 const struct utouch_contact *b)
{
	return a->id != b->id || a->tool_type != b->tool_type ||
	       a->active != b->active;
}

static int detect_mod(const struct utouch_contact *a,
		      const struct utouch_contact *b)
{
	return a->x != b->x || a->y != b->y ||
		a->touch_major != b->touch_major ||
		a->touch_minor != b->touch_minor ||
		a->width_major != b->width_major ||
		a->width_minor != b->width_minor ||
		a->orientation != b->orientation ||
		a->pressure != b->pressure ||
		a->distance != b->distance;
}

static utouch_frame_time_t get_time_ms()
{
	static const utouch_frame_time_t ms = 1000;
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_usec / ms + tv.tv_sec * ms;
}

static void set_semi_mt_touches(utouch_frame_handle fh)
{
	struct utouch_frame *next = fh->next;
	struct utouch_contact *a = next->slots[0];
	struct utouch_contact *b = next->slots[1];
	struct utouch_surface *s = fh->surface;
	/* use touch ids half way around the ID range */
	unsigned int id = a->id + (s->max_id - s->min_id) / 2;
	float x = (a->x + b->x) / 2;
	float y = (a->y + b->y) / 2;
	int i;

	for (i = 2; i < fh->semi_mt_num_active; i++) {
		struct utouch_contact *c = next->slots[i];

		memset(c, 0, sizeof(struct utouch_contact));
		c->active = 1;
		if (id > s->max_id)
			id -= (s->max_id - s->min_id + 1);
		c->id = id++;
		c->x = x;
		c->y = y;
	}

	for (i = fh->semi_mt_num_active; i < fh->num_slots; i++)
		next->slots[i]->active = 0;
}

const struct utouch_frame *utouch_frame_sync(utouch_frame_handle fh,
					     utouch_frame_time_t time)
{
	struct utouch_frame *frame = fh->frames[fh->frame];
	const struct utouch_frame *prev = frame->prev;
	struct utouch_frame *next = fh->next;
	int naddrem = 0, nmod = 0;
	utouch_frame_time_t dt;
	int i;

	frame->time = time ? time : get_time_ms();
	frame->num_active = 0;
	dt = frame->time - prev->time;

	if (fh->surface->is_semi_mt)
		set_semi_mt_touches(fh);

	for (i = 0; i < fh->num_slots; i++) {
		struct utouch_contact *p = frame->slots[i];
		struct utouch_contact *q = next->slots[i];

		set_contact(fh, p, q, dt);
		if (p->active)
			frame->active[frame->num_active++] = p;

		naddrem += detect_addrem(p->prev, p);
		nmod += detect_mod(p->prev, p);
	}
	if (naddrem + nmod == 0)
		return 0;

	if (naddrem == 0 && frame->time < prev->time + fh->hold_ms)
		return 0;

	if (frame->num_active != prev->num_active) {
		next->mod_time = frame->time;
		next->revision++;
	}
	if (naddrem) {
		next->slot_mod_time = frame->time;
		next->slot_revision++;
	}

	frame->revision = next->revision;
	frame->slot_revision = next->slot_revision;
	frame->mod_time = next->mod_time;
	frame->slot_mod_time = next->slot_mod_time;
	frame->sequence_id = next->sequence_id++;

	fh->frame = (fh->frame + 1) % fh->num_frames;

	return frame;
}
