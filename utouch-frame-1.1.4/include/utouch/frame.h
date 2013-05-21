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

#ifndef UTOUCH_FRAME_H
#define UTOUCH_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define UTOUCH_FRAME_VERSION	0x00001010

/**
 * struct utouch_surface - device surface details
 * @needs_pointer: device needs a screen pointer to function
 * @is_direct: surface is a direct device (e.g. touchscreen)
 * @is_buttonpad: surface has button(s) under it
 * @is_semi_mt: surface detects bounding rectangle only
 * @use_touch_major: device uses major axis for contact modulation
 * @use_touch_minor: device uses minor axis for contact modulation
 * @use_width_major: device uses approaching major axis for contact modulation
 * @use_width_minor: device uses approaching minor axis for contact modulation
 * @use_orientation: device uses ellipse orientation for contact modulation
 * @use_pressure: device uses pressure for contact modulation
 * @use_distance: device uses hover distance
 * @phys_width: physical width in millimeters (mm)
 * @phys_height: physical height in millimeters (mm)
 * @phys_pressure: maximal physical pressure (N/cm^2)
 * @min_x: minimum horizontal device coordinate
 * @min_y: minimum vertical device coordinate
 * @max_x: maximum horizontal device coordinate
 * @max_y: maximum vertical device coordinate
 * @max_pressure: maximum pressure device coordinate
 * @max_orient: maximum orientation device coordinate
 * @mapped_min_x: minimum horizontal mapped coordinate
 * @mapped_min_y: minimum vertical mapped coordinate
 * @mapped_max_x: maximum horizontal mapped coordinate
 * @mapped_max_y: maximum vertical mapped coordinate
 * @mapped_max_pressure: maximum pressure mapped coordinate
 *
 * The mutable contact given by utouch_frame_get_current_slot() should
 * be set in device coordinates. The contact data is subsequently
 * transformed to mapped (e.g., screen) coordinates in
 * utouch_frame_sync(). To a frame user, all data will appear in
 * mapped coordinates.
 *
 * Device properties and touch surface details. Later versions of this
 * struct may grow in size, but will remain binary compatible with
 * older versions.
 */
struct utouch_surface {
	int needs_pointer;
	int is_direct;
	int is_buttonpad;
	int is_semi_mt;
	int use_touch_major;
	int use_touch_minor;
	int use_width_major;
	int use_width_minor;
	int use_orientation;
	int use_pressure;
	int use_distance;
	float phys_width;
	float phys_height;
	float phys_pressure;
	float min_x;
	float min_y;
	float max_x;
	float max_y;
	float max_pressure;
	float max_orient;
	float mapped_min_x;
	float mapped_min_y;
	float mapped_max_x;
	float mapped_max_y;
	float mapped_max_pressure;
	unsigned int min_id;
	unsigned int max_id;
};

#define UTOUCH_TOOL_FINGER	0
#define UTOUCH_TOOL_PEN		1

/**
 * struct utouch_contact - surface contact details
 * @prev: pointer to same slot of previous frame
 * @active: currently in use
 * @slot: slot occupied by this contact
 * @id: unique id of this contact
 * @tool_type: the tool type of this contact
 * @x: horizontal center position coordinate (surface units)
 * @y: vertical center position coordinate (surface units)
 * @touch_major: major axis of contact (surface units)
 * @touch_minor: minor axis of contact (surface units)
 * @width_major: major axis of approaching contact (surface units)
 * @width_minor: minor axis of approaching contact (surface units)
 * @orientation: direction of ellipse (left: -Pi/2, up: 0, right: Pi/2)
 * @pressure: pressure of contact (pressure units)
 * @distance: distance of contact (surface units)
 * @vx: horizontal velocity coordinate (units / millisecond)
 * @vy: vertical velocity coordinate (units / millisecond)
 *
 * Surface contact details. Later versions of this struct may grow in
 * size, but will remain binary compatible with older versions.
 *
 * Contact structures are connected into one ring per slot. The
 * previous contact pointers are ABI agnostic, owned by the engine,
 * and have engine scope.
 */
struct utouch_contact {
	const struct utouch_contact *prev;
	int active;
	int slot;
	unsigned int id;
	int tool_type;
	float x;
	float y;
	float touch_major;
	float touch_minor;
	float width_major;
	float width_minor;
	float orientation;
	float pressure;
	float distance;
	float vx;
	float vy;
};

/* time in milliseconds */
typedef uint64_t utouch_frame_time_t;

/* the frame engine handle */
typedef struct utouch_frame_engine *utouch_frame_handle;

/**
 * struct utouch_frame - emitted frame details
 * @prev: pointer to previous frame
 * @sequence_id: frame sequence number
 * @revision: changes whenever the contact count changes
 * @slot_revision: changes whenever the slot id array change
 * @num_active: the number of contacts in the active array
 * @time: time of frame completion (ms)
 * @mod_time: time of last contact count change (ms)
 * @slot_mod_time: time of last slot id array change (ms)
 * @active: the array of active contacts
 *
 * Contact frame details. Later versions of this struct may grow in
 * size, but will remain binary compatible with older versions.
 *
 * Frames are connected into a ring. The previous frame pointer is ABI
 * agnostic, owned by the engine, and has engine scope.
 */
struct utouch_frame {
	const struct utouch_frame *prev;
	unsigned int sequence_id;
	unsigned int revision;
	unsigned int slot_revision;
	unsigned int num_active;
	utouch_frame_time_t time;
	utouch_frame_time_t mod_time;
	utouch_frame_time_t slot_mod_time;
	struct utouch_contact **active;
	struct utouch_contact **slots;
};

/**
 * utouch_frame_get_version - get library abi version
 *
 * Returns the version of the library, which may be different
 * from the api version of the compiled user program.
 */
unsigned int utouch_frame_get_version(void);

/**
 * utouch_frame_get_num_frames - get number of supported frames
 *
 * Returns the number of frames supported by this engine.
 */
unsigned int utouch_frame_get_num_frames(utouch_frame_handle fh);

/**
 * utouch_frame_get_num_slots - get number of supported slots
 *
 * Returns the number of simultaneous contacts supported by this engine.
 */
unsigned int utouch_frame_get_num_slots(utouch_frame_handle fh);

utouch_frame_handle utouch_frame_new_engine_raw(unsigned int num_frames,
						unsigned int num_slots,
						unsigned int frame_rate,
						unsigned int version,
						unsigned int surface_size,
						unsigned int frame_size,
						unsigned int slot_size);

/**
 * utouch_frame_new_engine - allocate a new frame engine
 * @num_frames: number of frames in cyclic buffer
 * @num_slots: maximum number of slots per frame
 * @frame_rate: maximum frame rate (frames/s)
 *
 * Allocates memory, initializes the internal engine and returns a
 * handle to it. A rate of 100 frames per second is normal.
 */
#define utouch_frame_new_engine(num_frames, num_slots, frame_rate)	\
	utouch_frame_new_engine_raw(num_frames,				\
				    num_slots,				\
				    frame_rate,				\
				    UTOUCH_FRAME_VERSION,		\
				    sizeof(struct utouch_surface),	\
				    sizeof(struct utouch_frame),	\
				    sizeof(struct utouch_contact))

/**
 * utouch_frame_delete_engine - deallocate a frame engine
 * @fh: frame engine in use
 *
 * Deallocates all memory associated with the engine.
 */
void utouch_frame_delete_engine(utouch_frame_handle fh);

/**
 * utouch_frame_get_surface - get the mutable device surface information
 * @fh: the frame engine in use
 *
 * Returns a pointer to the mutable device surface information. It is
 * preferrably set up by one of the input handlers. The pointer is ABI
 * agnostic, has frame engine scope, and is owned by the engine.
 */
struct utouch_surface *utouch_frame_get_surface(utouch_frame_handle fh);

/**
 * utouch_frame_get_current_slot - get the current mutable slot contact
 * @fh: the frame engine in use
 *
 * Returns a pointer to the contact current being modified. The
 * pointer is ABI agnostic, has frame engine scope, and is owned by
 * the engine.
 */
struct utouch_contact *utouch_frame_get_current_slot(utouch_frame_handle fh);

/**
 * utouch_frame_set_current_slot - set the current slot number
 * @fh: the frame engine in use
 * @slot: the slot number
 *
 * Sets the slot currently being modified. Returns zero if successful,
 * negative error otherwise.
 */
int utouch_frame_set_current_slot(utouch_frame_handle fh, int slot);

/**
 * utouch_frame_set_current_id - set the current slot by touch id
 * @fh: the frame engine in use
 * @id: the touch id
 *
 * Sets the slot currently being modified, by touch id. If the id is
 * not currently in use (does not have an active slot), a new slot is
 * assigned. Returns zero if successful, negative error otherwise.
 */
int utouch_frame_set_current_id(utouch_frame_handle fh, int id);

/**
 * utouch_frame_sync - synchronize and return new frame
 * @fh: the frame engine in use
 * @time: the frame synchronization time (ms)
 *
 * Scans through the updates, and in case the changes make up a new
 * frame, returns the updated frame.
 *
 * If time is zero, a time-of-receipt will be used instead.
 *
 * The frame returned is always the next in the cyclic list, and
 * always points back at the previous frame returned by this function.
 *
 * The returned pointer is ABI agnostic and owned by the frame
 * engine. It may very well be zero if there is nothing to report or
 * if the frame rate is limited.
 */
const struct utouch_frame *utouch_frame_sync(utouch_frame_handle fh,
					     utouch_frame_time_t time);

#ifdef __cplusplus
}
#endif

#endif
