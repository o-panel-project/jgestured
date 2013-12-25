/**
 * @file utouch/frame.h
 * Definitions of the main and platform-generic API
 */

#ifndef UTOUCH_FRAME_UTOUCH_FRAME_H_
#define UTOUCH_FRAME_UTOUCH_FRAME_H_

#include <stdint.h>
#include <linux/input.h>

#define FRAME_STATUS_BEGIN	0
#define FRAME_STATUS_UPDATE	1
#define FRAME_STATUS_END	2

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
	unsigned int num_slots;
	unsigned int num_active;
	unsigned int current_slot;
	utouch_frame_time_t time;
	utouch_frame_time_t mod_time;
	utouch_frame_time_t slot_mod_time;
	struct utouch_contact **active;
	struct utouch_contact **slots;
};

struct utouch_frame *create_frame(int nslot);
void destroy_frame(struct utouch_frame *frame, int nslot);
int frame_active_nslot(struct utouch_frame *f);
int frame_set_active_slot(struct utouch_frame *frame, int slot);
int frame_get_slot_status(struct utouch_frame *frame);
void frame_set_slot_status(struct utouch_frame *frame, int status);
void frame_set_slot_inactive(struct utouch_frame *frame);
struct utouch_contact *frame_get_slot(struct utouch_frame *frame);
utouch_frame_time_t frame_set_evtime(
	struct utouch_frame *frame, const struct input_event *syn);
int frame_abs_event(struct utouch_frame *frame, const struct input_event *ev);

#endif
