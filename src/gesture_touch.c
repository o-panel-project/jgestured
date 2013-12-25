/*
 * Touch event recognizer
 */

#include <stdio.h>

#include "frame.h"
#include "uinput_api.h"

#define TOUCH_DEBUG_PRINT 0

extern float m_scale_x;
extern float m_scale_y;

void touch_init()
{
}

void touch_down_event(struct uinput_api *ua, struct utouch_frame *f)
{
	struct utouch_contact *t;
	float val;

	t = frame_get_slot(f);

	val = (u_int16_t)t->x * m_scale_x;
	ua->valuators[0] = (u_int16_t)val;
	val = (u_int16_t)t->y * m_scale_y;
	ua->valuators[1] = (u_int16_t)val;
#if TOUCH_DEBUG_PRINT > 0
	fprintf(stdout, "%s() %d:(%f,%f)\n",
			__func__, f->slot_revision, t->x, t->y);
#endif
	if (f->slot_revision == 0) {
		uinput_PenDown_1st(ua);
	} else if (f->slot_revision == 1) {
		uinput_PenDown_2nd(ua);
	}
}

void touch_up_event(struct uinput_api *ua, struct utouch_frame *f)
{
	struct utouch_contact *t;
	float val;

	t = frame_get_slot(f);

	val = (u_int16_t)t->x * m_scale_x;
	ua->valuators[0] = (u_int16_t)val;
	val = (u_int16_t)t->y * m_scale_y;
	ua->valuators[1] = (u_int16_t)val;
#if TOUCH_DEBUG_PRINT > 0
	fprintf(stdout, "%s() %d:(%f,%f)\n",
			__func__, f->slot_revision, t->x, t->y);
#endif
	if (f->slot_revision == 0) {
		uinput_PenUp_1st(ua);
	} else if (f->slot_revision == 1) {
		uinput_PenUp_2nd(ua);
	}
}

void touch_move_event(struct uinput_api *ua, struct utouch_frame *f)
{
	struct utouch_contact *t;
	float val;

	t = frame_get_slot(f);

	val = (u_int16_t)t->x * m_scale_x;
	ua->valuators[0] = (u_int16_t)val;
	val = (u_int16_t)t->y * m_scale_y;
	ua->valuators[1] = (u_int16_t)val;
#if TOUCH_DEBUG_PRINT > 0
	fprintf(stdout, "%s() %d:(%f,%f)\n",
			__func__, f->slot_revision, t->x, t->y);
#endif
	if (f->slot_revision == 0) {
		uinput_PenMove_1st(ua);
	} else if (f->slot_revision == 1) {
		uinput_PenMove_2nd(ua);
	}
}
/* EOF */
