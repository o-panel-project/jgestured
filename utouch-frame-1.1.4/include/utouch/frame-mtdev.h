/*****************************************************************************
 *
 * utouch-frame - Touch Frame Library
 *
 * Copyright (C) 2010-2011 Canonical Ltd.
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

#ifndef UTOUCH_FRAME_MTDEV_H
#define UTOUCH_FRAME_MTDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#define MTDEV_NO_LEGACY_API

#include <utouch/frame.h>
#include <evemu.h>
#include <mtdev.h>

int utouch_frame_is_supported_mtdev(const struct evemu_device *dev);

int utouch_frame_init_mtdev(utouch_frame_handle fh,
			    const struct evemu_device *dev);

const struct utouch_frame *
utouch_frame_pump_mtdev(utouch_frame_handle fh, const struct input_event *ev);

#ifdef __cplusplus
}
#endif

#endif
