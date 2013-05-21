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

#ifndef _GRAIL_IMPL_H
#define _GRAIL_IMPL_H

#define MTDEV_NO_LEGACY_API
#define GRAIL_NO_LEGACY_API

#include <grail.h>
#include <utouch/frame-mtdev.h>
#include "evbuf.h"
#include "grailbuf.h"

#define DIM_TOUCH		32
#define DIM_TOUCH_BYTES		((DIM_TOUCH + 7) >> 3)

struct grail_impl {
	struct evemu_device *evemu;
	struct mtdev *mtdev;
	utouch_frame_handle fh;
	const struct utouch_frame *frame;
	struct evbuf evbuf;
	struct grailbuf gbuf;
	int filter_abs;
	int ongoing;
	int gesture;
};

#endif
