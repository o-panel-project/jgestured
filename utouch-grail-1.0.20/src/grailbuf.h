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

#ifndef _GRAIL_BUFFER_H
#define _GRAIL_BUFFER_H

#include <grail.h>

#define DIM_GRAIL_EVENTS 512

struct grailbuf {
	int head;
	int tail;
	struct grail_event buffer[DIM_GRAIL_EVENTS];
};

static inline void grailbuf_clear(struct grailbuf *buf)
{
	buf->head = buf->tail = 0;
}

static inline int grailbuf_empty(const struct grailbuf *buf)
{
	return buf->head == buf->tail;
}

static inline void grailbuf_put(struct grailbuf *buf,
				const struct grail_event *ev)
{
	buf->buffer[buf->head++] = *ev;
	buf->head &= DIM_GRAIL_EVENTS - 1;
}

static inline void grailbuf_get(struct grailbuf *buf,
				struct grail_event *ev)
{
	*ev = buf->buffer[buf->tail++];
	buf->tail &= DIM_GRAIL_EVENTS - 1;
}

#endif
