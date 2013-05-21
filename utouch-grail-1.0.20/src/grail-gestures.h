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

#ifndef _GRAIL_GESTURES_H
#define _GRAIL_GESTURES_H

#include "grail-inserter.h"

#define PRIO_POINTER	1
#define PRIO_GESTURE	2
#define PRIO_ENV	3
#define PRIO_META	4
#define PRIO_TAP	5

#define DIM_FM		4

#define FM_X		0
#define FM_Y		1
#define FM_R		2
#define FM_A		3

struct filter_model {
	float raw_delta;
	float action_delta;
	float velocity;
	float value;
	float original;
	float sample;
	float fuzz;
	float bar;
	grail_time_t original_ms;
	grail_time_t sample_ms;
	grail_time_t hold_ms;
	grail_time_t bar_ms;
};

struct move_model {
	struct filter_model fm[DIM_FM];
	int tickle, active, timeout;
	int single, multi, ntouch;
	grail_time_t time;
};

void gru_init_motion(struct grail *ge);
void gru_motion(struct grail *ge,
		const struct utouch_frame *frame);
void gru_event(struct grail *ge, int gid,
	       const struct move_model *move,
	       const grail_prop_t *prop, int nprop);
void gru_end(struct grail *ge, int gid,
	     const struct move_model *move,
	     const grail_prop_t *prop, int nprop);

struct combo_model {
	int active, gid;
	int mintouch, maxtouch;
	int nprop;
	grail_prop_t prop[DIM_GRAIL_PROP];
};

int gru_drag(struct grail *ge,
	     const struct utouch_frame *frame);
int gru_pinch(struct grail *ge,
	      const struct utouch_frame *frame);
int gru_rotate(struct grail *ge,
	       const struct utouch_frame *frame);

static inline int out_of_bounds(const struct combo_model *s,
				const struct move_model *m)
{
	return m->ntouch < s->mintouch || m->ntouch > s->maxtouch;
}

int gru_windrag(struct grail *ge,
		const struct utouch_frame *frame);
int gru_winpinch(struct grail *ge,
		 const struct utouch_frame *frame);
int gru_winrotate(struct grail *ge,
		  const struct utouch_frame *frame);

struct tapping_model {
	grail_time_t start;
	int mintouch, maxtouch;
	int active, gid, tap;
	int nprop;
	grail_prop_t prop[DIM_GRAIL_PROP];
};

int gru_tapping(struct grail *ge,
		const struct utouch_frame *frame);

#endif

