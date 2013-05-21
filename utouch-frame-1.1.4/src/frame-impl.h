/*****************************************************************************
 *
 * utouch-frame - Touch Frame Library
 *
 * Copyright (C) 2011 Canonical Ltd.
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

#ifndef FRAME_IMPL_H
#define FRAME_IMPL_H

#include <utouch/frame.h>

struct utouch_frame_engine {
	int num_frames;
	int num_slots;
	int hold_ms;
	int frame;
	int slot;
	struct utouch_surface *surface;
	struct utouch_frame **frames;
	struct utouch_frame *next;
	int *evmap;
	float map[9];
	unsigned int semi_mt_num_active;
};

#endif
