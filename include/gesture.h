/*
 *
 */

#ifndef __GESTURE_H__
#define __GESTURE_H__

#include "frame.h"
#include "uinput_api.h"

#define DIM_FM	4
#define FM_X	0
#define FM_Y	1
#define FM_R	2
#define FM_A	3

void gesture_init();

/* touch */
void touch_init();
void touch_down_event(struct uinput_api *ua, struct utouch_frame *f);
void touch_up_event(struct uinput_api *ua, struct utouch_frame *f);
void touch_move_event(struct uinput_api *ua, struct utouch_frame *f);

/* flick */
void flick_init();
void flick_set_dir_div(int d);
void flick_reset(const struct utouch_frame *f);
void flick_update(const struct utouch_frame *f);
int  flick_check(const struct utouch_frame *f);
void flick_event(struct uinput_api *ua, const struct utouch_frame *f);

/* pinching */
void pinch_init();
void pinch_reset(const struct utouch_frame *f);
int  pinch_check(const struct utouch_frame *f);
void pinch_event(struct uinput_api *ua, const struct utouch_frame *f);

#endif
