/*
 * Uinput event for X11.
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/time.h>
#include <errno.h>

#include "uinput_api.h"

#define UINPUT_DEBUG_PRINT	0
//#define INTERVAL(x) hard_sleep(x)
#define INTERVAL(x)

static int hard_sleep(unsigned long msec)
{
	struct timespec ts, tsrem;
	int ret;

	ts.tv_sec = (time_t)msec/1000;
	ts.tv_nsec = (long)(msec%1000)*1000*1000;
	while ((ret = nanosleep(&ts, &tsrem)) == -1) {
		if (errno != EINTR)
			break;  /* error */
		ts.tv_sec = tsrem.tv_sec;
		ts.tv_nsec = tsrem.tv_nsec;
	}
	return ret;
}

static void ioctl_set(int fd, int set, int value)
{
	if (ioctl(fd, set, value) < 0)
		perror("error: ioctl set");
}

static int create_uinput_device(int fd)
{
	struct uinput_user_dev uidev;
	memset(&uidev, 0, sizeof(uidev));

	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "j3 gesture proxy");
	uidev.id.bustype = BUS_VIRTUAL;
	uidev.id.vendor  = 0xdead;
	uidev.id.product = 0xbeef;
	uidev.id.version = 1;
	uidev.absmax [ABS_X] = 2047;
	uidev.absmax [ABS_Y] = 2047;
	uidev.absmin [ABS_X] = 0;
	uidev.absmin [ABS_Y] = 0;
	uidev.absfuzz[ABS_X] = 0;
	uidev.absfuzz[ABS_Y] = 0;
	uidev.absflat[ABS_X] = 0;
	uidev.absflat[ABS_Y] = 0;

	if (write(fd, &uidev, sizeof(uidev)) < 0) {
		perror("create_uinput_device: write");
		return -1;
	}

	ioctl_set(fd, UI_SET_EVBIT, EV_SYN);

	ioctl_set(fd, UI_SET_EVBIT, EV_KEY);
	ioctl_set(fd, UI_SET_KEYBIT, BTN_LEFT);
	ioctl_set(fd, UI_SET_KEYBIT, BTN_RIGHT);
	ioctl_set(fd, UI_SET_KEYBIT, BTN_EXTRA);
	ioctl_set(fd, UI_SET_EVBIT, EV_ABS);
	ioctl_set(fd, UI_SET_ABSBIT, ABS_X);
	ioctl_set(fd, UI_SET_ABSBIT, ABS_Y);
	ioctl_set(fd, UI_SET_ABSBIT, ABS_RX);
	ioctl_set(fd, UI_SET_ABSBIT, ABS_RY);
	ioctl_set(fd, UI_SET_ABSBIT, ABS_DISTANCE);

	ioctl_set(fd, UI_SET_EVBIT, EV_MSC);
	ioctl_set(fd, UI_SET_MSCBIT, MSC_GESTURE);

	if (ioctl(fd, UI_DEV_CREATE) < 0) {
		perror("create_uinput_device: ioctl");
		return -1;
	}

	return 0;
}

static void destroy_uinput_device(int fd)
{
	if (ioctl(fd, UI_DEV_DESTROY) < 0)
		perror("destroy_uinput_device: ioctl");
}

static void send_event(int fd, int type, int code, int value)
{
	struct input_event ev;

	ev.type = type;
	ev.code = code;
	ev.value = value;

	if (write(fd, &ev, sizeof(ev)) < 0)
		perror("sent_event write");
}

void uinput_write(struct uinput_api *ua, struct input_event *ie)
{
	gettimeofday(&ie->time, NULL);
	write(ua->fd, ie, sizeof(*ie));
}

struct uinput_api *uinput_new()
{
	struct uinput_api *x;
	int fd;

	fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
		if (fd < 0) {
			perror("error: open uinput (out)");
			return NULL;
		}
	}

	if (create_uinput_device(fd) < 0) {
		close(fd);
		return NULL;
	}

	x = calloc(1, sizeof(*x));
	if (!x) {
		close(fd);
		return NULL;
	}

	x->fd = fd;

	return x;
}

void uinput_destroy(struct uinput_api *ua)
{
	if (ua) {
		destroy_uinput_device(ua->fd);
		close(ua->fd);
		free(ua);
	}
}

void uinput_PenDown_1st(struct uinput_api *ua)
{
	send_event(ua->fd, EV_ABS, ABS_X, ua->valuators[0]);
	send_event(ua->fd, EV_ABS, ABS_Y, ua->valuators[1]);
	send_event(ua->fd, EV_KEY, BTN_LEFT, 1);
	send_event(ua->fd, EV_SYN, SYN_REPORT, 0);
#if UINPUT_DEBUG_PRINT
	fprintf(stdout, "%s() - PenDown xpos:%4d  ypos:%4d\n",
			__func__, ua->valuators[0], ua->valuators[1]);
#endif
	INTERVAL(10);
}

void uinput_PenDown_2nd(struct uinput_api *ua)
{
	send_event(ua->fd, EV_ABS, ABS_RX, ua->valuators[0]);
	send_event(ua->fd, EV_ABS, ABS_RY, ua->valuators[1]);
	send_event(ua->fd, EV_KEY, BTN_EXTRA, 1);
	send_event(ua->fd, EV_SYN, SYN_REPORT, 0);
#if UINPUT_DEBUG_PRINT
	fprintf(stdout, "%s() - Second PenDown xpos:%4d  ypos:%4d\n",
			__func__, ua->valuators[0], ua->valuators[1]);
#endif
}

void uinput_PenUp_1st(struct uinput_api *ua)
{
	send_event(ua->fd, EV_ABS, ABS_X, ua->valuators[0]);
	send_event(ua->fd, EV_ABS, ABS_Y, ua->valuators[1]);
	send_event(ua->fd, EV_KEY, BTN_LEFT, 0);
	send_event(ua->fd, EV_SYN, SYN_REPORT, 0);
#if UINPUT_DEBUG_PRINT
	fprintf(stdout, "%s() - Sending PenUp (%dms)\n",
			__func__, ua->valuators[2]);
#endif
	INTERVAL(10);
}

void uinput_PenUp_2nd(struct uinput_api *ua)
{
	send_event(ua->fd, EV_ABS, ABS_RX, ua->valuators[0]);
	send_event(ua->fd, EV_ABS, ABS_RY, ua->valuators[1]);
	send_event(ua->fd, EV_KEY, BTN_EXTRA, 0);
	send_event(ua->fd, EV_SYN, SYN_REPORT, 0);
#if UINPUT_DEBUG_PRINT
	fprintf(stdout, "%s() - Second Sending PenUp (%dms)\n",
			__func__, ua->valuators[2]);
#endif
}

void uinput_PenMove_1st(struct uinput_api *ua)
{
	send_event(ua->fd, EV_ABS, ABS_X, ua->valuators[0]);
	send_event(ua->fd, EV_ABS, ABS_Y, ua->valuators[1]);
	send_event(ua->fd, EV_SYN, SYN_REPORT, 0);
#if UINPUT_DEBUG_PRINT
	fprintf(stdout, "%s() - Drag- x:%4d   y:%4d\n",
			__func__, ua->valuators[0], ua->valuators[1]);
#endif
	INTERVAL(10);
}

void uinput_PenMove_2nd(struct uinput_api *ua)
{
	send_event(ua->fd, EV_ABS, ABS_RX, ua->valuators[0]);
	send_event(ua->fd, EV_ABS, ABS_RY, ua->valuators[1]);
	send_event(ua->fd, EV_SYN, SYN_REPORT, 0);
#if UINPUT_DEBUG_PRINT
	fprintf(stdout, "%s() - Second Drag- x:%4d  y:%4d\n",
			__func__, ua->valuators[0], ua->valuators[1]);
#endif
}

void uinput_Gesture(struct uinput_api *ua)
{
	int value;
	value = ua->valuators[0];
	value = value << 16 | ua->valuators[1];
	send_event(ua->fd, EV_ABS, ABS_DISTANCE, value);
	value = ua->gestureId;
	value = value << 24 | ua->valuators[2];
	send_event(ua->fd, EV_MSC, MSC_GESTURE, value);
	send_event(ua->fd, EV_SYN, SYN_REPORT, 0);
#if UINPUT_DEBUG_PRINT
	fprintf(stdout, "%s() - GestureId: %d, Period:%d, "
					"xDist:%d, yDist:%d\n", __func__,
					ua->gestureId, ua->valuators[2],
					ua->valuators[0], ua->valuators[1]);
#endif
}
/* EOF */
