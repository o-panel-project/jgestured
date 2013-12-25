/*
 * eGestured emulator for Multitouch protocol type B device.
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>

#include "mtdev.h"
#include "uinput_api.h"
#include "frame.h"
#include "gesture.h"

#define DEBUG_EVENT_PRINT 0

static int mRunning = 0;
static struct mtdev *mpDev = NULL;
static struct uinput_api *mpUa = NULL;
static int mFd = -1;
static int mMultiTouch = 0;

#define MAX_TOUCH 2
static struct utouch_frame *mpFrame = NULL;

static void print_event(const struct input_event *ev)
{
#if DEBIG_EVENT_PRINT
	static const utouch_frame_time_t ms = 1000;
	static int slot = 0;
	utouch_frame_time_t evtime = ev->time.tv_usec / ms + ev->time.tv_sec * ms;
	if (ev->type == EV_ABS && ev->code == ABS_MT_SLOT)
		slot = ev->value;
	fprintf(stderr, "%012llx %02d(%02d) %01d %04x %d\n",
		evtime, slot, mpFrame->current_slot, ev->type, ev->code, ev->value);
#endif
}

/* Gesture recognizer */
static int frame_sync()
{
	struct utouch_contact *t;
	int i;
	int num_active;

	num_active = frame_active_nslot(mpFrame);

	for (i = 0; i < MAX_TOUCH; i++) {
		if (!frame_set_active_slot(mpFrame, i)) continue;
		t = frame_get_slot(mpFrame);
		switch (frame_get_slot_status(mpFrame)) {
		case FRAME_STATUS_BEGIN:
			if (num_active > 1)
				mMultiTouch = 1;
			else
				mMultiTouch = 0;
			if (!mMultiTouch)
				flick_reset(mpFrame);
			touch_down_event(mpUa, mpFrame);
			frame_set_slot_status(mpFrame, FRAME_STATUS_UPDATE);
			if (mMultiTouch)
				pinch_reset(mpFrame);
			break;
		case FRAME_STATUS_UPDATE:
			touch_move_event(mpUa, mpFrame);
			if (mMultiTouch && pinch_check(mpFrame)) {
				pinch_event(mpUa, mpFrame);
			}
			if (!mMultiTouch)
				flick_update(mpFrame);
			break;
		case FRAME_STATUS_END:
			if (!mMultiTouch && flick_check(mpFrame)) {
				flick_event(mpUa, mpFrame);
			}
			touch_up_event(mpUa, mpFrame);
			frame_set_slot_inactive(mpFrame);
			break;
		}
	}
	return 1;
}

/* input event handler */
static void tp_event(const struct input_event *ev)
{
	print_event(ev);

	if (ev->type == EV_SYN && ev->code == SYN_REPORT) {
		frame_set_evtime(mpFrame, ev);
		frame_sync();
	} else if (ev->type == EV_ABS) {
		frame_abs_event(mpFrame, ev);
	}
}

/* input event input */
#define NUM_EVENTS 32
static int event_pull(struct mtdev *dev, int fd)
{
	struct input_event ev[NUM_EVENTS];
	int count = 0, i, len = sizeof(ev);

	while (len == sizeof(ev)) {
		len = mtdev_get(dev, fd, ev, NUM_EVENTS) * sizeof(struct input_event);
		if (len <= 0) break;
		if (len % sizeof(ev[0])) break;
		for (i = 0; i < len / sizeof(ev[0]); i++) {
			tp_event(&ev[i]);
			count++;
		}
	}

	return count > 0 ? count : 0;
}

static void loop_mt_device(struct mtdev *dev, int fd)
{
	int count;
	while (!mtdev_idle(dev, fd, 5000)) {
		count = event_pull(dev, fd);
	}
}

#define MTCHECK(dev, name)				\
	if (mtdev_has_mt_event(dev, name))	\
		fprintf(stderr, "   %s\n", #name)

static void show_mt_props(const struct mtdev *dev)
{
	fprintf(stderr, "supported mt events:\n");
	MTCHECK(dev, ABS_MT_SLOT);
	MTCHECK(dev, ABS_MT_TOUCH_MAJOR);
	MTCHECK(dev, ABS_MT_TOUCH_MINOR);
	MTCHECK(dev, ABS_MT_WIDTH_MAJOR);
	MTCHECK(dev, ABS_MT_WIDTH_MINOR);
	MTCHECK(dev, ABS_MT_ORIENTATION);
	MTCHECK(dev, ABS_MT_POSITION_X);
	MTCHECK(dev, ABS_MT_POSITION_Y);
	MTCHECK(dev, ABS_MT_TOOL_TYPE);
	MTCHECK(dev, ABS_MT_BLOB_ID);
	MTCHECK(dev, ABS_MT_TRACKING_ID);
	MTCHECK(dev, ABS_MT_PRESSURE);
	MTCHECK(dev, ABS_MT_DISTANCE);
}

static void on_terminate(int signal)
{
	fprintf(stderr, "jgestured caught signal %d, terminate.\n", signal);
	mRunning = 0;
}

static void set_signal_handler()
{
	sigset_t mask;
	sigemptyset(&mask);
	signal(SIGTERM, on_terminate);
	signal(SIGINT, on_terminate);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int main(int argc, char *argv[])
{
	struct stat fs;
	int opt;
	int opt_dir;
	char *input_event_file = strdup("/dev/input/melfas0");

	while ((opt = getopt(argc, argv, "d:i:")) != -1) {
		switch (opt) {
		case 'd':
			opt_dir = atoi(optarg);
			flick_set_dir_div(opt_dir);
			break;
		case 'i':
			free(input_event_file);
			input_event_file = strdup(optarg);
			break;
		default:
			fprintf(stderr, "Usage: %s [-d 4|8] [-i device]\n", argv[0]);
			free(input_event_file);
			return -1;
		}
	}

	mFd = open(input_event_file, O_RDONLY | O_NONBLOCK);
	if (mFd < 0) {
		fprintf(stderr, "error: could not open device\n");
		free(input_event_file);
		return -1;
	}
	if (fstat(mFd, &fs)) {
		fprintf(stderr, "error: could not stat the device\n");
		goto exit_lbl;
	}
	if (fs.st_rdev && ioctl(mFd, EVIOCGRAB, 1)) {
		fprintf(stderr, "error: could not grab the device\n");
		goto exit_lbl;
	}

	mpDev = mtdev_new_open(mFd);
	if (!mpDev) {
		ioctl(mFd, EVIOCGRAB, 0);
		fprintf(stderr, "error: could not open touch device\n");
		goto exit_lbl;
	}
	show_mt_props(mpDev);

	mpUa = uinput_new();
	mpFrame = create_frame(MAX_TOUCH);
	gesture_init();
	touch_init();
	flick_init();
	pinch_init();

	mRunning = 1;
	set_signal_handler();
	while (mRunning)
		loop_mt_device(mpDev, mFd);

	uinput_destroy(mpUa);
	mtdev_close_delete(mpDev);

	if (fs.st_rdev)
		ioctl(mFd, EVIOCGRAB, 0);

exit_lbl:
	close(mFd);
	free(input_event_file);
	if (mpFrame)
		destroy_frame(mpFrame, MAX_TOUCH);

	return 0;
}
/* EOF */
