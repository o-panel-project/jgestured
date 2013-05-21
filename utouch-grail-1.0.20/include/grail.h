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

#ifndef _GRAIL_H
#define _GRAIL_H

#include <grail-bits.h>
#include <grail-types.h>
#include <linux/input.h>

#define DIM_GRAIL_TYPE		64
#define DIM_GRAIL_TYPE_BYTES	((DIM_GRAIL_TYPE + 7) >> 3)

#define DIM_GRAIL_PROP		32
#define DIM_GRAIL_PROP_BYTES	((DIM_GRAIL_PROP + 7) >> 3)

#define GRAIL_STATUS_BEGIN	0
#define GRAIL_STATUS_UPDATE	1
#define GRAIL_STATUS_END	2

typedef float grail_prop_t;		/* gesture properties */
typedef __u64 grail_time_t;		/* time in milliseconds */

/**
 * struct grail_coord - coordinate in bounding box units
 * @x: the horizontal position (bbox units)
 * @y: the vertical position (bbox units)
 */
struct grail_coord {
	float x, y;
};

/**
 * struct grail_contact - MT event information in bounding box units
 * @id: Contact tracking id
 * @tool_type: Tool type (ABS_MT_TOOL_TYPE)
 * @pos: Position of contact (bbox units)
 * @touch_major: Major axis of contact shape (bbox units)
 * @touch_minor: Minor axis of contact shape (bbox units)
 * @width_major: Major axis of perimeter (bbox units)
 * @width_minor: Minor axis of perimeter (bbox units)
 * @angle: Angle of orientation (vertical: 0 horizontal: +-M_PI_2)
 * @pressure: Pressure of contact (min: 0 max: 1)
 *
 * Depending on the native support of the underlying device, some or all of
 * the listed properties may be computed.
 */
struct grail_contact {
	int id;
	int tool_type;
	struct grail_coord pos;
	float touch_major;
	float touch_minor;
	float width_major;
	float width_minor;
	float angle;
	float pressure;
};

/**
 * struct grail_client_id - Gesture client information
 * @client: Client id
 * @root: Root window
 * @event: Window to route events to
 * @child: Window the event occured in
 *
 * This struct is treated opaquely, and only has meaning to the gesture
 * client. Details are subject to change.
 */
struct grail_client_id {
	int client;
	int root, event, child;
};

/**
 * struct grail_client_info - Gesture request information
 * @id: Gesture client id
 * @mask: Gestures the client is listening to
 */
struct grail_client_info {
	struct grail_client_id id;
	grail_mask_t mask[DIM_GRAIL_TYPE_BYTES];
};

/**
 * struct grail_event - Gesture event
 * @type: The gesture type
 * @id: Unique identifier foof the gesture instance
 * @status: Gesture status (begin, update, end)
 * @ntouch: Number of current touches
 * @nprop: Number of properties in the gesture
 * @pos: Focus point of the gesture (bbox coordinates)
 * @touch: Array of individual touch information
 * @client_id: The gesture client to route the gesture to
 * @time: Time of event (milliseconds)
 * @prop: Array of properties of the event
 *
 * Gesture events are passed to the client via the gesture() callback.
 */
struct grail_event {
	int type;
	int id;
	int status;
	int ntouch;
	int nprop;
	struct grail_coord pos;
	struct grail_client_id client_id;
	grail_time_t time;
	grail_prop_t prop[DIM_GRAIL_PROP];
};

/**
 * struct grail - Main grail device
 * @get_clients: Called at the onset of new gestures to retrieve the list
 * of listening clients.
 * @event: Callback for kernel events passing through grail.
 * @gesture: Main gesture callback.
 * @impl: Grail implementation details.
 * @gin: Gesture instatiation details.
 * @gru: Gesture recognition details.
 * @priv: Generic pointer to user-defined content.
 *
 * The grail device pulls events from the underlying device, detects
 * gestures, and passes them on to the client via the gesture()
 * callback. Events that are not gesture or for other reasons held back are
 * passed on via the event() callback. The user provides information about
 * windows and listening clients via the get_clients callback, which is
 * called during gesture instantiation.
 *
 */
struct grail {
	int (*get_clients)(struct grail *ge,
			   struct grail_client_info *client, int max_clients,
			   const struct grail_coord *coords, int num_coords,
			   const grail_mask_t *types, int type_bytes);
	void (*event)(struct grail *ge,
		      const struct input_event *ev);
	void (*gesture)(struct grail *ge,
			const struct grail_event *ev);
	struct grail_impl *impl;
	struct gesture_inserter *gin;
	struct gesture_recognizer *gru;
	void *priv;
};

/**
 * grail_open - open a grail device
 * @ge: the grail device to open
 * @fd: file descriptor of the kernel device
 *
 * Initialize the internal grail structures and configure it by reading the
 * protocol capabilities through the file descriptor.
 *
 * The callbacks, parameters and priv pointer should be set prior to this
 * call.
 *
 * Returns zero on success, negative error number otherwise.
 */
int grail_open(struct grail *ge, int fd);

/**
 * grail_idle - check state of kernel device
 * @ge: the grail device in use
 * @fd: file descriptor of the kernel device
 * @ms: number of milliseconds to wait for activity
 *
 * Returns true if the device is idle, i.e., there are no fetched
 * events in the pipe and there is nothing to fetch from the device.
 */
int grail_idle(struct grail *ge, int fd, int ms);

/**
 * grail_pull - pull and process available events from the kernel device
 * @ge: the grail device in use
 * @fd: file descriptor of the kernel device
 *
 * Pull all available events and process them. The grail callbacks are
 * invoked during this call.
 *
 * The underlying file descriptor must have O_NONBLOCK set, or this method
 * will not return until the file is closed.
 *
 * On success, returns the number of events read. Otherwise,
 * a standard negative error number is returned.
 */
int grail_pull(struct grail *ge, int fd);

/**
 * grail_close - close the grail device
 * @ge: the grail device to close
 * @fd: file descriptor of the kernel device
 *
 * Deallocates all memory associated with grail, and clears the grail
 * structure.
 */
void grail_close(struct grail *ge, int fd);

/**
 * grail_set_bbox - set the grail unit bounding box
 * @ge: the grail device in use
 * @min: the minimum (lower-left) corner of the bounding box
 * @max: the maximum (upper-right) corner of the bounding box
 *
 * Sets the box within which the device coordinates should be presented.
 */
#if defined(JPANEL_TOUCHSCREEN)
void grail_set_bbox(struct grail *ge,
		    const struct grail_coord *min,
		    const struct grail_coord *max,
		    const struct grail_coord *pbox);
#else
void grail_set_bbox(struct grail *ge,
		    const struct grail_coord *min,
		    const struct grail_coord *max);
#endif

/**
 * grail_filter_abs_events - filter kernel motion events
 * @ge: the grail device in use
 * @usage: When true, filter kernel motion events.
 *
 * Single-finger pointer events are treated as pointer gestures in
 * grail. When filter_motion_events is non-zero, the kernel events
 * corresponding to pointer movement are removed from the event
 * stream.
 *
 */
void grail_filter_abs_events(struct grail *ge, int usage);

/**
 * grail_get_units - get device coordinate ranges
 * @ge: the grail device in use
 * @min: minimum x and y coordinates
 * @max: maximum x and y coordinates
 *
 * The grail event attributes pos, touch_major, touch_minor,
 * width_major, and width_minor are all given in device coordinate
 * units, unless specified otherwise using the grail_set_bbox()
 * function. This function reports the device coordinate ranges.
 *
 */
void grail_get_units(const struct grail *ge,
		     struct grail_coord *min, struct grail_coord *max);
#if defined(JPANEL_TOUCHSCREEN)
void grail_get_mapped_units(const struct grail *ge,
		     struct grail_coord *min, struct grail_coord *max);
void grail_get_phys_units(const struct grail *ge,
		     struct grail_coord *box);
#endif

/**
 * grail_get_contacts - get current contact state
 * @ge: the grail device in use
 * @touch: array of contacts to be filled in
 * @max_touch: maximum number of contacts supported by the array
 *
 * Extract the contact state as currently seen by grail.
 *
 */
int grail_get_contacts(const struct grail *ge,
		       struct grail_contact *touch, int max_touch);

#endif
