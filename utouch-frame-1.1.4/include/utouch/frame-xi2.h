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

#ifndef UTOUCH_FRAME_XI2_H
#define UTOUCH_FRAME_XI2_H

#ifdef __cplusplus
extern "C" {
#endif

#define MTDEV_NO_LEGACY_API

#include <utouch/frame.h>
#include <X11/extensions/XInput2.h>

int utouch_frame_is_supported_xi2(Display *dpy, const XIDeviceInfo *dev);

int utouch_frame_init_xi2(utouch_frame_handle fh,
			  Display *dpy, const XIDeviceInfo *dev);

int utouch_frame_configure_xi2(utouch_frame_handle fh,
			       const XConfigureEvent *ev);

const struct utouch_frame *
utouch_frame_pump_xi2(utouch_frame_handle fh, const XIDeviceEvent *ev);

#ifdef __cplusplus
}
#endif

#endif
