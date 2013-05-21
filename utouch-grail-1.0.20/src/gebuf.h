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

#ifndef _GRAIL_GESTURE_BUFFER_H
#define _GRAIL_GESTURE_BUFFER_H

#include "grail-impl.h"

#define DIM_GESTURE_EVENTS 512

struct gesture_event {
	int status;
	int ntouch;
	int nprop;
	utouch_frame_time_t time;
	struct grail_coord pos;
	grail_prop_t prop[DIM_GRAIL_PROP];
};

struct gebuf {
	int head;
	int tail;
	struct gesture_event buffer[DIM_GESTURE_EVENTS];
};

static inline void gebuf_clear(struct gebuf *gebuf)
{
	gebuf->head = gebuf->tail = 0;
}

static inline int gebuf_empty(const struct gebuf *gebuf)
{
	return gebuf->head == gebuf->tail;
}

static inline void gebuf_put(struct gebuf *gebuf,
			     const struct gesture_event *ev)
{
	gebuf->buffer[gebuf->head++] = *ev;
	gebuf->head &= DIM_GESTURE_EVENTS - 1;
}

static inline void gebuf_get(struct gebuf *gebuf,
			     struct gesture_event *ev)
{
	*ev = gebuf->buffer[gebuf->tail++];
	gebuf->tail &= DIM_GESTURE_EVENTS - 1;
}

#endif
