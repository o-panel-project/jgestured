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
	-1,
	GRAIL_TYPE_DRAG1,
	GRAIL_TYPE_DRAG2,
	GRAIL_TYPE_DRAG3,
	GRAIL_TYPE_DRAG4,
	GRAIL_TYPE_DRAG5,
};

static void set_props(const struct gesture_inserter *gin,
		      struct combo_model *s, const struct move_model *m,
		      const struct utouch_frame *frame)
{
	if (m->single) {
		s->prop[GRAIL_PROP_DRAG_DX] =
			gin->scale_x * m->fm[FM_X].raw_delta;
		s->prop[GRAIL_PROP_DRAG_DY] =
			gin->scale_y * m->fm[FM_Y].raw_delta;
	} else {
		s->prop[GRAIL_PROP_DRAG_DX] =
			gin->scale_x * m->fm[FM_X].action_delta;
		s->prop[GRAIL_PROP_DRAG_DY] =
			gin->scale_y * m->fm[FM_Y].action_delta;
	}
	s->prop[GRAIL_PROP_DRAG_VX] = gin->scale_x * m->fm[FM_X].velocity;
	s->prop[GRAIL_PROP_DRAG_VY] = gin->scale_y * m->fm[FM_Y].velocity;
	s->prop[GRAIL_PROP_DRAG_X] = gin_prop_x(gin, m->fm[FM_X].value);
	s->prop[GRAIL_PROP_DRAG_Y] = gin_prop_y(gin, m->fm[FM_Y].value);
	s->nprop = 6;
	s->nprop += gin_add_contact_props(gin, s->prop + s->nprop, frame);
}

static const int fm_mask = 0x03;

int gru_drag(struct grail *ge,
	    const struct utouch_frame *frame)
{
	struct gesture_recognizer *gru = ge->gru;
	struct combo_model *state = &gru->drag;
	struct move_model *move = &gru->move;
	int mask = state->active ? (move->active & fm_mask) : fm_mask;
	if (!move->multi && !move->single) {
		if (state->active) {
			gru_end(ge, state->gid, move,
				state->prop, state->nprop);
			state->active = 0;
		}
	}
	if ((move->timeout & fm_mask) == fm_mask) {
		if (state->active) {
			gin_gid_discard(ge, state->gid);
			state->active = 0;
		}
		return 0;
	}
	if (!state->active) {
		int type = getype[move->ntouch];
		if (type < 0)
			return 0;
		state->gid = gin_gid_begin(ge, type, PRIO_GESTURE, frame);
		state->active = 1;
	}
	if (!(move->tickle & mask))
		return 0;
	if (!move->single && !(move->active & fm_mask))
		return 0;
	set_props(ge->gin, state, move, frame);
	gru_event(ge, state->gid, move, state->prop, state->nprop);
	return 1;
}

int gru_windrag(struct grail *ge,
		const struct utouch_frame *frame)
{
	struct gesture_recognizer *gru = ge->gru;
	struct combo_model *state = &gru->windrag;
	struct move_model *move = &gru->move;
	int mask = state->active ? (move->active & fm_mask) : fm_mask;
	if (!move->multi && !move->single) {
		if (state->active && out_of_bounds(state, move)) {
			gru_end(ge, state->gid, move,
				state->prop, state->nprop);
			state->active = 0;
		}
	}
	if ((move->timeout & fm_mask) == fm_mask) {
		if (state->active) {
			gin_gid_discard(ge, state->gid);
			state->active = 0;
		}
		return 0;
	}
	if (!state->active) {
		if (move->ntouch == 4) {
			state->gid = gin_gid_begin(ge, GRAIL_TYPE_MDRAG,
						   PRIO_META, frame);
			state->mintouch = 1;
			state->maxtouch = 4;
			state->active = 1;
		} else if (move->ntouch == 3) {
			state->gid = gin_gid_begin(ge, GRAIL_TYPE_EDRAG,
						   PRIO_ENV, frame);
			state->mintouch = 1;
			state->maxtouch = 3;
			state->active = 1;
		} else {
			return 0;
		}
	}
	if (!(move->tickle & mask))
		return 0;
	if (!move->single && !(move->active & fm_mask))
		return 0;
	set_props(ge->gin, state, move, frame);
	gru_event(ge, state->gid, move, state->prop, state->nprop);
	return 1;
}

