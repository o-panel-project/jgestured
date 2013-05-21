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

#ifndef _GRAIL_BITS_H
#define _GRAIL_BITS_H

typedef unsigned char grail_mask_t;

static inline void grail_mask_set(grail_mask_t *mask, int i)
{
	mask[i >> 3] |= (1 << (i & 7));
}

static inline void grail_mask_clear(grail_mask_t *mask, int i)
{
	mask[i >> 3] &= ~(1 << (i & 7));
}

static inline void grail_mask_modify(grail_mask_t *mask, int i, int v)
{
	if (v)
		grail_mask_set(mask, i);
	else
		grail_mask_clear(mask, i);
}

static inline int grail_mask_get(const grail_mask_t *mask, int i)
{
	return (mask[i >> 3] >> (i & 7)) & 1;
}

void grail_mask_set_mask(grail_mask_t *a, const grail_mask_t *b, int bytes);
void grail_mask_clear_mask(grail_mask_t *a, const grail_mask_t *b, int bytes);

int grail_mask_count(const grail_mask_t *mask, int bytes);
int grail_mask_get_first(const grail_mask_t *mask, int bytes);
int grail_mask_get_next(int i, const grail_mask_t *mask, int bytes);

#define grail_mask_foreach(i, mask, bytes)				\
	for (i = grail_mask_get_first(mask, bytes);			\
	     i >= 0;							\
	     i = grail_mask_get_next(i, mask, bytes))

#endif
