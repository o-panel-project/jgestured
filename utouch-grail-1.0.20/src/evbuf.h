/*****************************************************************************
 *
 * mtdev - Multitouch Protocol Translation Library (MIT license)
 *
 * Copyright (C) 2010 Canonical Ltd.
 * Copyright (C) 2010 Henrik Rydberg <rydberg@bitmath.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#ifndef GRAIL_EVBUF_H
#define GRAIL_EVBUF_H

#define DIM_EVENTS 4096

struct evbuf {
	int head;
	int tail;
	struct input_event buffer[DIM_EVENTS];
};

static inline void evbuf_clear(struct evbuf *evbuf)
{
	evbuf->head = evbuf->tail = 0;
}

static inline int evbuf_empty(const struct evbuf *evbuf)
{
	return evbuf->head == evbuf->tail;
}

static inline void evbuf_put(struct evbuf *evbuf,
			     const struct input_event *ev)
{
	evbuf->buffer[evbuf->head++] = *ev;
	evbuf->head &= DIM_EVENTS - 1;
}

static inline void evbuf_get(struct evbuf *evbuf,
			     struct input_event *ev)
{
	*ev = evbuf->buffer[evbuf->tail++];
	evbuf->tail &= DIM_EVENTS - 1;
}

#endif
