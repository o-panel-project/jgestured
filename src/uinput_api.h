/*
 * $Id$
 */

#ifndef _UINPUT_API_H_
#define _UINPUT_API_H_

#include <sys/types.h>

struct uinput_api {
	int			fd;
	u_int8_t	gestureId;
	u_int16_t	valuators[3];	
};

void uinput_write(struct uinput_api *ua, struct input_event *ie);
struct uinput_api *uinput_new();
void uinput_destroy(struct uinput_api *ua);

void uinput_PenDown(struct uinput_api *ua);
void uinput_PenUp(struct uinput_api *ua);
void uinput_PenMove(struct uinput_api *ua);
void uinput_PenDown_2nd(struct uinput_api *ua);
void uinput_PenUp_2nd(struct uinput_api *ua);
void uinput_PenMove_2nd(struct uinput_api *ua);

void uinput_Gesture(struct uinput_api *ua);

#endif /* _UINPUT_API_H_ */
