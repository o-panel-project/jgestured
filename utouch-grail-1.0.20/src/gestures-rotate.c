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
#include <stdio.h>

static const int getype[DIM_TOUCH + 1] = {
	0,
	0,
	GRAIL_TYPE_ROTATE2,
	GRAIL_TYPE_ROTATE3,
	GRAIL_TYPE_ROTATE4,
	GRAIL_TYPE_ROTATE5,
};

static const int fm_mask = 0x08;

static void set_props(const struct gesture_inserter *gin,
		      struct combo_model *s, const struct move_model *m,
		      const struct utouch_frame *frame)
{
	s->prop[GRAIL_PROP_ROTATE_DA] = m->fm[FM_A].action_delta;
	s->prop[GRAIL_PROP_ROTATE_VA] = m->fm[FM_A].velocity;
	s->prop[GRAIL_PROP_ROTATE_A] = m->fm[FM_A].value;
	s->nprop = 3;
	s->nprop += gin_add_contact_props(gin, s->prop + s->nprop, frame);
}

int gru_rotate(struct grail *ge,
	       const struct utouch_frame *frame)
{
	struct gesture_recognizer *gru = ge->gru;
	struct combo_model *state = &gru->rotate;
	struct move_model *move = &gru->move;
	int mask = state->active ? (move->active & fm_mask) : fm_mask;
	if (!move->multi && !move->single) {
		if (state->active) {
			gru_end(ge, state->gid, move,
				state->prop, state->nprop);
			state->active = 0;
		}
		return 0;
	}
	if ((move->timeout & fm_mask) == fm_mask) {
		if (state->active) {
			gin_gid_discard(ge, state->gid);
			state->active = 0;
		}
		return 0;
	}
	if (!(move->tickle & mask))
		return 0;
	if (!state->active) {
		int type = getype[move->ntouch];
		if (!type)
			return 0;
		state->gid = gin_gid_begin(ge, type, PRIO_GESTURE, frame);
		state->active = 1;
	}
	if (!(move->active & fm_mask))
		return 0;
	set_props(ge->gin, state, move, frame);
	gru_event(ge, state->gid, move, state->prop, state->nprop);
	return 1;
}

int gru_winrotate(struct grail *ge,
		  const struct utouch_frame *frame)
{
	struct gesture_recognizer *gru = ge->gru;
	struct combo_model *state = &gru->winrotate;
	struct move_model *move = &gru->move;
	int mask = state->active ? (move->active & fm_mask) : fm_mask;
	if (!move->multi) {
		if (state->active && out_of_bounds(state, move)) {
			gru_end(ge, state->gid, move,
				state->prop, state->nprop);
			state->active = 0;
		}
		return 0;
	}
	if ((move->timeout & fm_mask) == fm_mask) {
		if (state->active) {
			gin_gid_discard(ge, state->gid);
			state->active = 0;
		}
		return 0;
	}
	if (!(move->tickle & mask))
		return 0;
	if (!state->active) {
		if (move->ntouch == 4) {
			state->gid = gin_gid_begin(ge, GRAIL_TYPE_MROTATE,
						   PRIO_META, frame);
			state->mintouch = 2;
			state->maxtouch = 4;
			state->active = 1;
		} else if (move->ntouch == 3) {
			state->gid = gin_gid_begin(ge, GRAIL_TYPE_EROTATE,
						   PRIO_ENV, frame);
			state->mintouch = 2;
			state->maxtouch = 3;
			state->active = 1;
		} else {
			return 0;
		}
	}
	if (!(move->active & fm_mask))
		return 0;
	set_props(ge->gin, state, move, frame);
	gru_event(ge, state->gid, move, state->prop, state->nprop);
	return 1;
}
