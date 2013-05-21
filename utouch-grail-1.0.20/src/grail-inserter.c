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

static const int MAX_GESTURE_ID = 0xfff;

static int find_gslot(const struct gesture_inserter *gin, int gid)
{
	int i;
	grail_mask_foreach(i, gin->used, sizeof(gin->used))
		if (gin->state[i].id == gid)
			return i;
	return -1;
}

// todo: spanning tree for multi-user case
static void setup_new_gestures(struct grail *ge,
			       const struct utouch_frame *frame)
{
	struct gesture_inserter *gin = ge->gin;
	grail_mask_t types[DIM_GRAIL_TYPE_BYTES];
	grail_mask_t span[DIM_TOUCH_BYTES];
	struct grail_client_info info[DIM_CLIENT];
	int i, j, nclient = 0;
	int nfresh = grail_mask_count(gin->fresh, sizeof(gin->fresh));
	if (!nfresh)
		return;

	memset(types, 0, sizeof(types));
	memset(span, 0, sizeof(span));

	grail_mask_foreach(i, gin->fresh, sizeof(gin->fresh)) {
		struct slot_state *s = &gin->state[i];
		grail_mask_set(types, s->type);
		grail_mask_set_mask(span, s->span, sizeof(span));
	}

	nclient = gin_get_clients(ge, info, DIM_CLIENT, types, sizeof(types),
				  span, sizeof(span), frame);

	grail_mask_foreach(i, gin->fresh, sizeof(gin->fresh)) {
		struct slot_state *s = &gin->state[i];
		s->nclient = 0;
		for (j = 0; j < nclient; j++) {
			if (!grail_mask_get(info[j].mask, s->type))
				continue;
			if (gin->grab_active &&
			    info[j].id.client != gin->grab_client)
				continue;
			if (grail_mask_get(info[j].mask, GRAIL_TYPE_SYSFLAG1)) {
				gin->grab_active = 1;
				gin->grab_client = info[j].id.client;
			}
			s->client_id[s->nclient++] = info[j].id;
		}
	}

	memset(gin->fresh, 0, sizeof(gin->fresh));
}

int gin_init(struct grail *ge)
{
	struct gesture_inserter *gin;
	int i;
	gin = calloc(1, sizeof(*gin));
	if (!gin)
		return -ENOMEM;
	for (i = 0; i < DIM_INSTANCE; i++)
		grail_mask_set(gin->unused, i);
	gin->scale_x = 1;
	gin->scale_y = 1;
	gin->scale_r = 1;
	gin->trans_x = 1;
	gin->trans_y = 1;
	ge->gin = gin;
	return 0;
}

void gin_destroy(struct grail *ge)
{
	free(ge->gin);
	ge->gin = NULL;
}

void gin_frame_begin(struct grail *ge, const struct utouch_frame *frame)
{
	struct gesture_inserter *gin = ge->gin;
	memset(gin->types, 0, sizeof(gin->types));
	gin->time = frame->time;
	if (frame->num_active && !frame->prev->num_active)
		gin->grab_active = 0;
}

void gin_frame_end(struct grail *ge, const struct utouch_frame *frame)
{
	struct gesture_inserter *gin = ge->gin;
	int i, hold = 0, discard = 0;

	setup_new_gestures(ge, frame);

	grail_mask_foreach(i, gin->used, sizeof(gin->used)) {
		struct slot_state *s = &gin->state[i];
		if (!s->nclient)
			continue;
		if (s->priority > hold)
			hold = s->priority;
		if (s->status != GRAIL_STATUS_UPDATE)
			continue;
		if (s->priority > discard)
			discard = s->priority;
	}

	grail_mask_foreach(i, gin->used, sizeof(gin->used)) {
		struct slot_state *s = &gin->state[i];
		if (!s->nclient || s->priority < discard)
			gin_gid_discard(ge, s->id);
	}

	grail_mask_foreach(i, gin->used, sizeof(gin->used)) {
		struct slot_state *s = &gin->state[i];
		struct gesture_event ev;
		grail_mask_set(gin->types, s->type);
		if (s->priority < hold)
			continue;
		while (!gebuf_empty(&s->buf)) {
			gebuf_get(&s->buf, &ev);
			gin_send_event(ge, s, &ev, frame);
		}
	}

	grail_mask_foreach(i, gin->used, sizeof(gin->used)) {
		struct slot_state *s = &gin->state[i];
		if (s->status == GRAIL_STATUS_END)
			gin_gid_discard(ge, s->id);
	}
}

int gin_gid_begin(struct grail *ge, int type, int priority,
		  const struct utouch_frame *frame)
{
	struct gesture_inserter *gin = ge->gin;
	struct slot_state *s;
	int slot;
	int i = grail_mask_get_first(gin->unused, sizeof(gin->unused));
	if (i < 0)
		return -1;
	s = &gin->state[i];
	s->type = type;
	s->priority = priority;
	s->id = gin->gestureid++ & MAX_GESTURE_ID;
	s->status = GRAIL_STATUS_BEGIN;
	s->nclient = 0;
	for (slot = 0; slot < DIM_TOUCH; slot++)
		grail_mask_modify(s->span, slot, frame->slots[slot]->active);
	gebuf_clear(&s->buf);
	grail_mask_clear(gin->unused, i);
	grail_mask_set(gin->fresh, i);
	grail_mask_set(gin->used, i);
	return s->id;
}

void gin_gid_discard(struct grail *ge, int gid)
{
	struct gesture_inserter *gin = ge->gin;
	struct slot_state *s;
	int i = find_gslot(gin, gid);
	if (i < 0)
		return;
	s = &gin->state[i];
	gebuf_clear(&s->buf);
	s->status = GRAIL_STATUS_END;
	grail_mask_clear(gin->used, i);
	grail_mask_set(gin->unused, i);
}

void gin_gid_event(struct grail *ge, int gid,
		   float x, float y, int ntouch,
		   const grail_prop_t *prop, int nprop,
		   int transient)
{
	struct gesture_inserter *gin = ge->gin;
	struct gesture_event ev;
	struct slot_state *s;
	int i = find_gslot(gin, gid);
	if (i < 0)
		return;
	s = &gin->state[i];
	ev.status = transient ? GRAIL_STATUS_UPDATE : s->status;
	ev.ntouch = ntouch;
	ev.nprop = nprop;
	ev.time = gin->time;
	ev.pos.x = x;
	ev.pos.y = y;
	memcpy(ev.prop, prop, nprop * sizeof(grail_prop_t));
	gebuf_put(&s->buf, &ev);
	if (transient)
		s->status = GRAIL_STATUS_END;
	else if (s->status == GRAIL_STATUS_BEGIN)
		s->status = GRAIL_STATUS_UPDATE;
}

void gin_gid_end(struct grail *ge, int gid,
		 float x, float y, int ntouch,
		 const grail_prop_t *prop, int nprop)
{
	struct gesture_inserter *gin = ge->gin;
	struct gesture_event ev;
	struct slot_state *s;
	int i = find_gslot(gin, gid);
	if (i < 0)
		return;
	s = &gin->state[i];
	if (s->status != GRAIL_STATUS_BEGIN) {
		ev.status = GRAIL_STATUS_END;
		ev.ntouch = ntouch;
		ev.nprop = nprop;
		ev.time = gin->time;
		ev.pos.x = x;
		ev.pos.y = y;
		memcpy(ev.prop, prop, nprop * sizeof(grail_prop_t));
		gebuf_put(&s->buf, &ev);
	}
	s->status = GRAIL_STATUS_END;
}

void grail_set_bbox(struct grail *ge,
		    const struct grail_coord *tmin,
#if defined(JPANEL_TOUCHSCREEN)
		    const struct grail_coord *tmax,
			const struct grail_coord *pbox)
#else
		    const struct grail_coord *tmax)
#endif
{
	struct utouch_surface *s = utouch_frame_get_surface(ge->impl->fh);
	struct gesture_inserter *gin = ge->gin;
	double tx, ty, dx, dy, sx, sy;
	tx = tmax->x - tmin->x;
	ty = tmax->y - tmin->y;
	dx = s->max_x - s->min_x;
	dy = s->max_y - s->min_y;
	sx = tx / dx;
	sy = ty / dy;
	gin->scale_x = sx;
	gin->scale_y = sy;
	gin->scale_r = sqrt((tx * tx + ty * ty) / (dx * dx + dy * dy));
	gin->trans_x = tmin->x - s->min_x * sx;
	gin->trans_y = tmin->y - s->min_y * sy;
#if defined(JPANEL_TOUCHSCREEN)
	s->mapped_min_x = tmin->x;
	s->mapped_min_y = tmin->y;
	s->mapped_max_x = tmax->x;
	s->mapped_max_y = tmax->y;
	s->phys_width = pbox->x;
	s->phys_height = pbox->y;
	gin->scale_x = 1;
	gin->scale_y = 1;
	gin->trans_x = 0;
	gin->trans_y = 0;
#endif
}
