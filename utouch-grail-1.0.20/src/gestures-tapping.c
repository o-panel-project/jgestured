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

#include "grail-recognizer.h"
#include <math.h>

static const int fm_mask = 0x07;

static void set_props(const struct gesture_inserter *gin,
		      struct tapping_model *s, const struct move_model *m,
		      const struct utouch_frame *frame)
{
	s->prop[GRAIL_PROP_TAP_DT] = m->time - s->start;
	s->prop[GRAIL_PROP_TAP_X] = gin_prop_x(gin, m->fm[FM_X].value);
	s->prop[GRAIL_PROP_TAP_Y] = gin_prop_y(gin, m->fm[FM_Y].value);
	s->nprop = 3;
	s->nprop += gin_add_contact_props(gin, s->prop + s->nprop, frame);
}

int gru_tapping(struct grail *ge,
		const struct utouch_frame *frame)
{
	struct gesture_recognizer *gru = ge->gru;
	struct tapping_model *state = &gru->tapping;
	struct move_model *move = &gru->move;
	state->tap = 0;
	if (frame->num_active && !frame->prev->num_active) {
		state->mintouch = 0;
		state->maxtouch = 0;
	}
	if (move->ntouch > state->maxtouch) {
		if (state->active) {
			gin_gid_discard(ge, state->gid);
			state->active = 0;
		}
		state->start = move->time;
		state->maxtouch = move->ntouch;
		set_props(ge->gin, state, move, frame);
		if (state->maxtouch <= 5) {
			int type = GRAIL_TYPE_TAP1 + state->maxtouch - 1;
			state->gid = gin_gid_begin(ge, type, PRIO_TAP, frame);
			state->active = 1;
		}
		return 0;
	}
	if (!state->active) {
		state->mintouch = move->ntouch;
		state->maxtouch = move->ntouch;
		return 0;
	}
	if (move->ntouch <= state->mintouch) {
		int x = state->prop[GRAIL_PROP_TAP_X];
		int y = state->prop[GRAIL_PROP_TAP_Y];
		int t = move->time - state->start;
		if (t > move->fm[FM_X].bar_ms) {
			gin_gid_discard(ge, state->gid);
			state->mintouch = move->ntouch;
			state->maxtouch = move->ntouch;
			state->active = 0;
			return 0;
		}
		state->tap = state->maxtouch;
		state->prop[GRAIL_PROP_TAP_DT] = t;
		gin_gid_event(ge, state->gid, x, y, state->maxtouch,
			      state->prop, state->nprop, 1);
		state->mintouch = move->ntouch;
		state->maxtouch = move->ntouch;
		state->active = 0;
		return 1;
	}
	if (!move->ntouch)
		return 0;
	set_props(ge->gin, state, move, frame);
	if ((move->active & fm_mask) ||
	    move->time - state->start > move->fm[FM_X].bar_ms) {
		gin_gid_discard(ge, state->gid);
		state->mintouch = move->ntouch;
		state->maxtouch = move->ntouch;
		state->active = 0;
	}
	return 0;
}
