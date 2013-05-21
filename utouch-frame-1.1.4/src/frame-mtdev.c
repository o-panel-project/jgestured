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

#include <utouch/frame-mtdev.h>
#include "frame-impl.h"
#include <linux/input.h>
#include <errno.h>
#include <math.h>

static int is_pointer(const struct evemu_device *dev)
{
	return evemu_has_event(dev, EV_REL, REL_X) ||
		evemu_has_event(dev, EV_KEY, BTN_TOOL_FINGER) ||
		evemu_has_event(dev, EV_KEY, BTN_TOOL_PEN) ||
		evemu_has_event(dev, EV_KEY, BTN_STYLUS) ||
		evemu_has_event(dev, EV_KEY, BTN_MOUSE) ||
		evemu_has_event(dev, EV_KEY, BTN_LEFT);
}

int utouch_frame_is_supported_mtdev(const struct evemu_device *dev)
{

	/* only support pure absolute devices at this time */
	if (evemu_has_event(dev, EV_REL, REL_X) ||
	    evemu_has_event(dev, EV_REL, REL_Y))
		return 0;

	if (evemu_has_event(dev, EV_ABS, ABS_MT_POSITION_X) &&
	    evemu_has_event(dev, EV_ABS, ABS_MT_POSITION_Y))
		return 1;

	/* do not support multi-finger non-mt for now */
	return 0;

	return evemu_has_event(dev, EV_ABS, ABS_X) &&
		evemu_has_event(dev, EV_ABS, ABS_Y) &&
		evemu_has_event(dev, EV_KEY, BTN_TOUCH) &&
		evemu_has_event(dev, EV_KEY, BTN_TOOL_DOUBLETAP);
}

#if defined(JPANEL_TOUCHSCREEN)
static void print_surface(struct utouch_surface *s)
{
	printf("utouch surface:needs_pointer=%d\n",s->needs_pointer);
	printf("utouch surface:is_direct=%d\n",s->is_direct);
	printf("utouch surface:is_buttonpad=%d\n",s->is_buttonpad);
	printf("utouch surface:is_semi_mt=%d\n",s->is_semi_mt);
	printf("utouch surface:use_touch_major=%d\n",s->use_touch_major);
	printf("utouch surface:use_touch_minor=%d\n",s->use_touch_minor);
	printf("utouch surface:use_width_major=%d\n",s->use_width_major);
	printf("utouch surface:use_width_minor=%d\n",s->use_width_minor);
	printf("utouch surface:use_orientation=%d\n",s->use_orientation);
	printf("utouch surface:use_pressure=%d\n",s->use_pressure);
	printf("utouch surface:use_distance=%d\n",s->use_distance);
	printf("utouch surface:phys_width=%f\n",s->phys_width);
	printf("utouch surface:phys_height=%f\n",s->phys_height);
	printf("utouch surface:phys_pressure=%f\n",s->phys_pressure);
	printf("utouch surface:min_x=%f\n",s->min_x);
	printf("utouch surface:min_y=%f\n",s->min_y);
	printf("utouch surface:max_x=%f\n",s->max_x);
	printf("utouch surface:max_y=%f\n",s->max_y);
	printf("utouch surface:max_pressure=%f\n",s->max_pressure);
	printf("utouch surface:max_orient=%f\n",s->max_orient);
	printf("utouch surface:mapped_min_x=%f\n",s->mapped_min_x);
	printf("utouch surface:mapped_min_y=%f\n",s->mapped_min_y);
	printf("utouch surface:mapped_max_x=%f\n",s->mapped_max_x);
	printf("utouch surface:mapped_max_y=%f\n",s->mapped_max_y);
	printf("utouch surface:mapped_max_pressure=%f\n",s->mapped_max_pressure);
	printf("utouch surface:min_id=%d\n",s->min_id);
	printf("utouch surface:max_id=%d\n",s->max_id);
}
#endif

int utouch_frame_init_mtdev(utouch_frame_handle fh,
			     const struct evemu_device *dev)
{
	struct utouch_surface *s = fh->surface;
	float tmp;

	if (!utouch_frame_is_supported_mtdev(dev))
		return -ENODEV;

	if (is_pointer(dev)) {
		s->needs_pointer = 1;
		s->is_direct = 0;
	} else {
		s->needs_pointer = 0;
		s->is_direct = 1;
	}

	s->is_buttonpad = 0;
	s->is_semi_mt = 0;

#ifdef INPUT_PROP_POINTER
	s->needs_pointer |= evemu_has_prop(dev, INPUT_PROP_POINTER);
	s->is_direct |= evemu_has_prop(dev, INPUT_PROP_DIRECT);
	s->is_buttonpad |= evemu_has_prop(dev, INPUT_PROP_BUTTONPAD);
	s->is_semi_mt |= evemu_has_prop(dev, INPUT_PROP_SEMI_MT);
#endif
	s->use_touch_major = evemu_has_event(dev, EV_ABS, ABS_MT_TOUCH_MAJOR);
	s->use_touch_minor = evemu_has_event(dev, EV_ABS, ABS_MT_TOUCH_MINOR);
	s->use_width_major = evemu_has_event(dev, EV_ABS, ABS_MT_WIDTH_MAJOR);
	s->use_width_minor = evemu_has_event(dev, EV_ABS, ABS_MT_WIDTH_MINOR);
	s->use_orientation = evemu_has_event(dev, EV_ABS, ABS_MT_ORIENTATION);
	s->use_pressure = evemu_has_event(dev, EV_ABS, ABS_MT_PRESSURE);
#ifdef ABS_MT_DISTANCE
	s->use_distance = evemu_has_event(dev, EV_ABS, ABS_MT_DISTANCE);
#endif

	if (evemu_has_event(dev, EV_ABS, ABS_MT_TRACKING_ID)) {
		s->min_id = evemu_get_abs_minimum(dev, ABS_MT_TRACKING_ID);
		s->max_id = evemu_get_abs_maximum(dev, ABS_MT_TRACKING_ID);
	} else {
		s->min_id = MT_ID_MIN;
		s->max_id = MT_ID_MAX;
	}

	s->min_x = evemu_get_abs_minimum(dev, ABS_MT_POSITION_X);
	s->min_y = evemu_get_abs_minimum(dev, ABS_MT_POSITION_Y);
	s->max_x = evemu_get_abs_maximum(dev, ABS_MT_POSITION_X);
	s->max_y = evemu_get_abs_maximum(dev, ABS_MT_POSITION_Y);
#if defined(JPANEL_TOUCHSCREEN)
	printf("%s() (%f,%f)(%f,%f)\n",
			__func__, s->min_x, s->min_y, s->max_x, s->max_y);
#endif
	if (s->min_x == s->max_x) {
		s->min_x = 0;
		s->max_x = 1024;
	}
	if (s->min_y == s->max_y) {
		s->min_y = 0;
		s->max_y = 768;
	}

	s->max_pressure = evemu_get_abs_maximum(dev, ABS_MT_PRESSURE);
	if (s->max_pressure == 0)
		s->max_pressure = 256;

	s->max_orient = evemu_get_abs_maximum(dev, ABS_MT_ORIENTATION);
	if (s->max_orient == 0)
		s->max_orient = 1;

	tmp = evemu_get_abs_resolution(dev, ABS_MT_POSITION_X);
	if (tmp == 0)
		tmp = evemu_get_abs_resolution(dev, ABS_X);
	if (tmp > 0)
		s->phys_width = (s->max_x - s->min_x) / tmp;
	else if (s->needs_pointer)
		s->phys_width = 100;
	else
		s->phys_width = 250;

	tmp = evemu_get_abs_resolution(dev, ABS_MT_POSITION_Y);
	if (tmp == 0)
		tmp = evemu_get_abs_resolution(dev, ABS_Y);
	if (tmp > 0)
		s->phys_height = (s->max_y - s->min_y) / tmp;
	else if (s->needs_pointer)
		s->phys_height = 65;
	else
		s->phys_height = 160;

	tmp = evemu_get_abs_resolution(dev, ABS_MT_PRESSURE);
	if (tmp > 0)
		s->phys_pressure = s->max_pressure / tmp;
	else
		s->phys_pressure = 10;

	/* defaults expected by initial frame version */
	s->mapped_min_x = s->min_x;
	s->mapped_min_y = s->min_y;
	s->mapped_max_x = s->max_x;
	s->mapped_max_y = s->max_y;
	s->mapped_max_pressure = s->max_pressure;

#if defined(JPANEL_TOUCHSCREEN)
//	print_surface(s);
#endif

	return 0;
}

static int handle_abs_event(utouch_frame_handle fh,
			    const struct input_event *ev)
{
	struct utouch_contact *t = utouch_frame_get_current_slot(fh);

	switch (ev->code) {
	case ABS_MT_SLOT:
		utouch_frame_set_current_slot(fh, ev->value);
		return 1;
	case ABS_MT_POSITION_X:
		t->x = ev->value;
		t->vx = 0;
		return 1;
	case ABS_MT_POSITION_Y:
		t->y = ev->value;
		t->vy = 0;
		return 1;
	case ABS_MT_TOUCH_MAJOR:
		t->touch_major = ev->value;
		return 1;
	case ABS_MT_TOUCH_MINOR:
		t->touch_minor = ev->value;
		return 1;
	case ABS_MT_WIDTH_MAJOR:
		t->width_major = ev->value;
		return 1;
	case ABS_MT_WIDTH_MINOR:
		t->width_minor = ev->value;
		return 1;
	case ABS_MT_ORIENTATION:
		t->orientation = ev->value;
		return 1;
	case ABS_MT_PRESSURE:
		t->pressure = ev->value;
		return 1;
#ifdef ABS_MT_DISTANCE
	case ABS_MT_DISTANCE:
		t->distance = ev->value;
		return 1;
#endif
	case ABS_MT_TOOL_TYPE:
		t->tool_type = ev->value;
		return 1;
	case ABS_MT_TRACKING_ID:
		if (ev->value == -1)
			t->active = 0;
		else {
			t->id = ev->value;
			t->active = 1;
		}
		return 1;
	default:
		return 0;
	}
}

static int handle_key_event(utouch_frame_handle fh,
			    const struct input_event *ev)
{
	/* EV_KEY events are only used for semi-mt touch count */
	if (!fh->surface->is_semi_mt)
		return 0;

	if (ev->code == BTN_TOUCH && ev->value == 0) {
		fh->semi_mt_num_active = 0;
		return 1;
	}

	if (ev->value == 0)
		return 0;

	switch (ev->code) {
	case BTN_TOOL_FINGER:
		fh->semi_mt_num_active = 1;
		return 1;
	case BTN_TOOL_DOUBLETAP:
		fh->semi_mt_num_active = 2;
		return 1;
	case BTN_TOOL_TRIPLETAP:
		fh->semi_mt_num_active = 3;
		return 1;
	case BTN_TOOL_QUADTAP:
		fh->semi_mt_num_active = 4;
		return 1;
	default:
		return 0;
	}
}

static utouch_frame_time_t get_evtime_ms(const struct input_event *syn)
{
	static const utouch_frame_time_t ms = 1000;
	return syn->time.tv_usec / ms + syn->time.tv_sec * ms;
}

const struct utouch_frame *
utouch_frame_pump_mtdev(utouch_frame_handle fh, const struct input_event *ev)
{
	const struct utouch_frame *f = 0;

	if (ev->type == EV_SYN && ev->code == SYN_REPORT)
		f = utouch_frame_sync(fh, get_evtime_ms(ev));
	else if (ev->type == EV_ABS)
		handle_abs_event(fh, ev);
	else if (ev->type == EV_KEY)
		handle_key_event(fh, ev);

	return f;
}
