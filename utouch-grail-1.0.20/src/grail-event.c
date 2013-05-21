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
#include "grail-impl.h"
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <math.h>

static void compute_bbox(struct grail_coord *min, struct grail_coord *max,
			 const struct utouch_frame *frame)
{
	float x, y;
	int i;
	if (frame->num_active < 1)
		return;
	x = frame->active[0]->x;
	y = frame->active[0]->y;
	min->x = max->x = x;
	min->y = max->y = y;
	for (i = 1; i < frame->num_active; i++) {
		x = frame->active[i]->x;
		y = frame->active[i]->y;
		if (x < min->x)
			min->x = x;
		if (y < min->y)
			min->y = y;
		if (x > max->x)
			max->x = x;
		if (y > max->y)
			max->y = y;
	}
}

int gin_add_contact_props(const struct gesture_inserter *gin,
			  grail_prop_t *prop, const struct utouch_frame *frame)
{
	struct grail_coord min, max;
	int i, n = 0, ntouch = frame->num_active;
	if (!ntouch)
		return n;
	if (ntouch > 5)
		ntouch = 5;
	compute_bbox(&min, &max, frame);
	prop[n++] = gin_prop_x(gin, min.x);
	prop[n++] = gin_prop_y(gin, min.y);
	prop[n++] = gin_prop_x(gin, max.x);
	prop[n++] = gin_prop_y(gin, max.y);
	for (i = 0; i < ntouch; i++) {
		const struct utouch_contact *ct = frame->active[i];
		prop[n++] = ct->id;
		prop[n++] = gin_prop_x(gin, ct->x);
		prop[n++] = gin_prop_y(gin, ct->y);
	}
	return n;
}

int gin_get_clients(struct grail *ge,
		    struct grail_client_info *info, int maxinfo,
		    const grail_mask_t* types, int btypes,
		    const grail_mask_t* span, int bspan,
		    const struct utouch_frame *frame)
{
	struct grail_coord pos[DIM_TOUCH];
	int i, npos = 0;
	if (!ge->get_clients)
		return 0;
	grail_mask_foreach(i, span, bspan) {
		pos[npos].x = gin_prop_x(ge->gin, frame->slots[i]->x);
		pos[npos].y = gin_prop_y(ge->gin, frame->slots[i]->y);
		npos++;
	}
	return ge->get_clients(ge, info, maxinfo, pos, npos, types, btypes);
}

void gin_send_event(struct grail *ge, struct slot_state *s,
		    const struct gesture_event *ev,
		    const struct utouch_frame *frame)
{
	struct grail_impl *impl = ge->impl;
	const struct gesture_inserter *gin = ge->gin;
	struct grail_event gev;
	int i;
	if (!ge->gesture)
		return;
	gev.type = s->type;
	gev.id = s->id;
	gev.status = ev->status;
	gev.ntouch = ev->ntouch;
	gev.nprop = ev->nprop;
	gev.time = ev->time;
	gev.pos.x = gin_prop_x(gin, ev->pos.x);
	gev.pos.y = gin_prop_y(gin, ev->pos.y);
	memcpy(gev.prop, ev->prop, ev->nprop * sizeof(grail_prop_t));
	for (i = 0; i < s->nclient; i++) {
		gev.client_id = s->client_id[i];
		grailbuf_put(&impl->gbuf, &gev);
	}
}

int grail_get_contacts(const struct grail *ge,
		       struct grail_contact *touch, int max_touch)
{
	const struct gesture_inserter *gin = ge->gin;
	const struct utouch_frame *frame = ge->impl->frame;
	int i;
	if (frame->num_active < max_touch)
		max_touch = frame->num_active;
	for (i = 0; i < max_touch; i++) {
		struct grail_contact *t = &touch[i];
		const struct utouch_contact *ct = frame->active[i];
		t->id = ct->id;
		t->tool_type = ct->tool_type;
		t->pos.x = gin_prop_x(gin, ct->x);
		t->pos.y = gin_prop_y(gin, ct->y);
		t->touch_major = gin->scale_r * ct->touch_major;
		t->touch_minor = gin->scale_r * ct->touch_minor;
		t->width_major = gin->scale_r * ct->width_major;
		t->width_minor = gin->scale_r * ct->width_minor;
		t->angle = ct->orientation;
		t->pressure = ct->pressure;
	}
	return max_touch;
}
