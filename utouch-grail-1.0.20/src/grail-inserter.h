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

#ifndef _GRAIL_INSERTER_H
#define _GRAIL_INSERTER_H

#include "grail-impl.h"
#include "gebuf.h"

#define DIM_EV_TYPE		EV_CNT
#define DIM_EV_TYPE_BYTES	((DIM_EV_TYPE + 7) >> 3)

#define DIM_INSTANCE		32
#define DIM_INSTANCE_BYTES	((DIM_INSTANCE + 7) >> 3)

#define DIM_CLIENT		32

struct slot_state {
	int type;
	int priority;
	int id;
	int status;
	int nclient;
	struct grail_client_id client_id[DIM_CLIENT];
	grail_mask_t span[DIM_TOUCH_BYTES];
	struct gebuf buf;
};

struct gesture_inserter {
	struct slot_state state[DIM_INSTANCE];
	grail_mask_t types[DIM_GRAIL_TYPE_BYTES];
	grail_mask_t unused[DIM_INSTANCE_BYTES];
	grail_mask_t fresh[DIM_INSTANCE_BYTES];
	grail_mask_t used[DIM_INSTANCE_BYTES];
	grail_time_t time;
	int gestureid;
	int grab_active;
	int grab_client;
	float scale_x, scale_y, scale_r;
	float trans_x, trans_y;
};

static inline float gin_prop_x(const struct gesture_inserter *gin, float x)
{
	return gin->scale_x * x + gin->trans_x;
}

static inline float gin_prop_y(const struct gesture_inserter *gin, float y)
{
	return gin->scale_y * y + gin->trans_y;
}

int gin_add_contact_props(const struct gesture_inserter *gin,
			  grail_prop_t *prop, const struct utouch_frame *frame);

int gin_get_clients(struct grail *ge,
		    struct grail_client_info *info, int maxinfo,
		    const grail_mask_t* types, int btypes,
		    const grail_mask_t* span, int bspan,
		    const struct utouch_frame *frame);
void gin_send_event(struct grail *ge, struct slot_state *s,
		    const struct gesture_event *ev,
		    const struct utouch_frame *frame);

int gin_init(struct grail *ge);
void gin_destroy(struct grail *ge);

void gin_frame_begin(struct grail *ge, const struct utouch_frame *frame);
void gin_frame_end(struct grail *ge, const struct utouch_frame *frame);

int gin_gid_begin(struct grail *ge, int type, int priority,
		  const struct utouch_frame *frame);
void gin_gid_discard(struct grail *ge, int gid);

void gin_gid_event(struct grail *ge, int gid,
		   float x, float y, int ntouch,
		   const grail_prop_t *prop, int nprop,
		   int transient);
void gin_gid_end(struct grail *ge, int gid,
		 float x, float y, int ntouch,
		 const grail_prop_t *prop, int nprop);

#endif
