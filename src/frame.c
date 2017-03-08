/*
 * Framing input event.
 */

#include <stdlib.h>

#include "mtdev.h"
#include "frame.h"

static void destroy_slots(struct utouch_contact **slots, int nslot)
{
	int i;

	if (slots) {
		for (i = nslot - 1; i >= 0; i--)
			free(slots[i]);
		free(slots);
	}
}

static struct utouch_contact **create_slots(int nslot, int size)
{
	struct utouch_contact **slots;
	struct utouch_contact *s;
	int i;

	slots = calloc(nslot, sizeof(slots[0]));
	if (!slots)
		return 0;

	for (i = 0; i < nslot; i++) {
		s = calloc(1, size);
		if (!s)
			goto out;
		slots[i] = s;
		s->slot = i;
		s->active = -1;
		s->id = -1;
	}

	return slots;
out:
	destroy_slots(slots, nslot);
	return 0;
}

int frame_active_nslot(struct utouch_frame *frame)
{
	int i;
	struct utouch_contact *t;

	frame->num_active = 0;
	for (i = 0; i < frame->num_slots; i++) {
		t = frame->slots[i];
		if (t->active == -1) continue;
			frame->num_active++;
	}
	return frame->num_active;
}

int frame_set_active_slot(struct utouch_frame *frame, int slot)
{
	struct utouch_contact *t;
	frame->slot_revision = slot;
	t = frame->slots[frame->slot_revision];
	if (t->active == -1)
		return 0;	/* inactive */
	return 1;		/* active */
}

int frame_get_slot_status(struct utouch_frame *frame)
{
	return frame->slots[frame->slot_revision]->active;
}

void frame_set_slot_status(struct utouch_frame *frame, int status)
{
	frame->slots[frame->slot_revision]->active = status;
}

void frame_set_slot_inactive(struct utouch_frame *frame)
{
	frame->slots[frame->slot_revision]->active = -1;
	frame->slots[frame->slot_revision]->id = -1;
}

struct utouch_contact *frame_get_slot(struct utouch_frame *frame)
{
	return frame->slots[frame->slot_revision];
}

void destroy_frame(struct utouch_frame *frame, int nslot)
{
	if (frame) {
		destroy_slots(frame->slots, nslot);
		free(frame);
	}
}

struct utouch_frame *create_frame(int nslot)
{
	struct utouch_frame *frame;

	frame = calloc(1, sizeof(struct utouch_frame));
	if (!frame)
		return 0;

	frame->slots = create_slots(nslot, sizeof(struct utouch_contact));
	if (!frame->slots)
		goto out;
	frame->num_slots = nslot;
	frame->slot_revision = 0;
	frame->current_slot = 0;

	return frame;
out:
	destroy_frame(frame, nslot);
	return 0;
}

static utouch_frame_time_t get_evtime_ms(const struct input_event *syn)
{
	static const utouch_frame_time_t ms = 1000;
	return syn->time.tv_usec / ms + syn->time.tv_sec * ms;
}

utouch_frame_time_t frame_set_evtime(
		struct utouch_frame *frame, const struct input_event *syn)
{
	frame->time = get_evtime_ms(syn);
	return frame->time;
}

int frame_abs_event(struct utouch_frame *frame, const struct input_event *ev)
{
	struct utouch_contact *t = frame->slots[frame->current_slot];

	switch (ev->code) {
	case ABS_MT_SLOT:
		if (ev->value < frame->num_slots)
			frame->current_slot = ev->value;
		return 1;
	case ABS_MT_TRACKING_ID:
		if (ev->value == -1) {
			t->active = FRAME_STATUS_END;
		} else {
			if (t->id != ev->value)
				t->active = FRAME_STATUS_BEGIN;
			else
				t->active = FRAME_STATUS_UPDATE;
			t->id = ev->value;
		}
		return 1;
	case ABS_MT_POSITION_X:
		t->x = ev->value;
		return 1;
	case ABS_MT_POSITION_Y:
		t->y = ev->value;
		return 1;
	case ABS_MT_TOUCH_MAJOR:
		t->touch_major = ev->value;
		return 1;
	case ABS_MT_WIDTH_MAJOR:
		t->width_major = ev->value;
		return 1;
	case ABS_MT_PRESSURE:
		t->pressure = ev->value;
		return 1;
	default:
		break;
	}
	return 0;
}
/* EOF */
