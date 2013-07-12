/*****************************************************************************
 *
 * grail - Gesture Recognition And Instantiation Library
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

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <math.h>

#include "grail.h"
#include "utouch/frame.h"

#include "uinput_api.h"

#if defined(GOODIX_XRES) && defined(GOODIX_YRES)
const float m_mapped_xres = GOODIX_XRES;
const float m_mapped_yres = GOODIX_YRES;
#elif defined(MELFAS_XRES) && defined(MELFAS_YRES)
const float m_mapped_xres = MELFAS_XRES;
const float m_mapped_yres = MELFAS_YRES;
#else
const float m_mapped_xres = 600.0;
const float m_mapped_yres = 1024.0;
#endif
const float m_phys_xsize = 222.72;
const float m_phys_ysize = 125.25;

static int opt_flick_dir;
static char *opt_event_file;
static int do_terminate = 0;

/*
 * Gesture type        bit
 * GRAIL_TYPE_DRAG1    0
 * GRAIL_TYPE_DRAG2    3
 * GRAIL_TYPE_PINCH2   4
 * GRAIL_TYPE_TAP1     15
 * GRAIL_TYPE_TAP2     16
 */
const static char *gesture_mask = "0x000000019";
static grail_mask_t flag_mask[DIM_GRAIL_TYPE_BYTES];

/* Debug print */
#define DEBUG_DRAG1_PROPERTY	0
#define DEBUG_DRAG2_PROPERTY	0
#define DEBUG_PINCH2_PROPERTY	0
#define DEBUG_TAP1_PROPERTY		0
#define DEBUG_TAP2_PROPERTY		0
#define DEBUG_TP_EVENT			0
#define FLICK_DEBUG_PRINT		0

/* for Flick event */
#define DIM_FM	4
#define FM_X	0
#define FM_Y	1
#define FM_R	2
#define FM_A	3
static float mDistance[DIM_FM];
static float mVelocity[DIM_FM];
static float mPinchingDistance[DIM_FM];
static int   mSingleDrag = 0;
static int   mMultiPinching = 0;
static grail_time_t mGestureStartTime = 0;

static float scale_x = 0.0;	/* Unit of pixel */
static float scale_y = 0.0;	/* Unit of pixel */
static float scale_ppm_x = 0.0;	/* Unit of mm */
static float scale_ppm_y = 0.0;	/* Unit of mm */
const static float radian_30 = 30 * M_PI / 180;
const static float radian_45 = 45 * M_PI / 180;
const static float radian_60 = 60 * M_PI / 180;
const static float flick_dist_min_threshold = 5.0;		/* min distance */
const static float flick_dist_max_threshold = 100.0;	/* max distance */
const static float flick_velo_min_threshold = 10.0;		/* ave. velocity min */
const static float flick_velo_max_threshold = 35.0;		/* ave. velocity max */
const static grail_time_t drag_timeout_threshold = 500;/* 1sec */
const static float pinch_dist_min_threshold = 4.0;		/* mm */

struct grail_event gev_touch;

static void setup_scale(struct grail *ge)
{
	struct grail_coord min, max, box;
	struct grail_coord smin, smax;
	grail_get_mapped_units(ge, &min, &max);
	grail_get_phys_units(ge, &box);
	scale_ppm_x = (max.x - min.x) / box.x;
	scale_ppm_y = (max.y - min.y) / box.y;
	grail_get_units(ge, &smin, &smax);
	scale_x = (max.x - min.x + 1) / (smax.x - smin.x + 1);
	scale_y = (max.y - min.y + 1) / (smax.y - smin.y + 1);
	fprintf(stdout, "%s() - phys box:%f,%f\n",
			__func__, box.x, box.y);
	fprintf(stdout, "%s() - map  box:%f,%f - %f,%f\n",
			__func__, min.x, min.y, max.x, max.y);
	fprintf(stdout, "%s() - reso box:%f,%f - %f,%f\n",
			__func__, smin.x, smin.y, smax.x, smax.y);
	fprintf(stdout, "%s() - scale_ppm:%f,%f scale:%f,%f\n",
			__func__, scale_ppm_x, scale_ppm_y, scale_x, scale_y);
}

static void flick_reset(struct grail *ge, const struct grail_event *ev)
{
	int i;
	for (i = 0; i < DIM_FM; i++) {
		mDistance[i] = 0;
		mVelocity[i] = 0;
	}
	if (ev)
		mGestureStartTime = ev->time;
	else
		mGestureStartTime = 0;
}

static void flick_update(struct grail *ge, const struct grail_event *ev)
{
	mDistance[FM_X] += ev->prop[GRAIL_PROP_DRAG_DX];
	mDistance[FM_Y] += ev->prop[GRAIL_PROP_DRAG_DY];
	mVelocity[FM_X] += fabs(ev->prop[GRAIL_PROP_DRAG_VX]);
	mVelocity[FM_Y] += fabs(ev->prop[GRAIL_PROP_DRAG_VY]);
}

static void flick_transform(struct grail *ge, const struct grail_event *ev)
{
	float phys_dist_x = mDistance[FM_X] / scale_ppm_x;
	float phys_dist_y = mDistance[FM_Y] / scale_ppm_y;
	float phys_velo_x = mVelocity[FM_X] / scale_ppm_x;
	float phys_velo_y = mVelocity[FM_Y] / scale_ppm_y;

	mDistance[FM_X] = phys_dist_x;
	mDistance[FM_Y] = phys_dist_y;
	mDistance[FM_R] = hypotf(phys_dist_x, phys_dist_y);
	mDistance[FM_A] = fabs(atan(phys_dist_y / phys_dist_x));
	mVelocity[FM_R] = hypotf(phys_velo_x, phys_velo_y);
#if FLICK_DEBUG_PRINT
	fprintf(stdout, "\t%s() - dist:%f(mm), velo:%f(mm/ms), dir:%f(rad)\n",
			__func__, mDistance[FM_R], mVelocity[FM_R], mDistance[FM_A]);
#endif
}

static int flick_check(struct grail *ge, const struct grail_event *ev)
{
	float dt;

	if (mDistance[0] == 0 && mDistance[1] == 0)
		return 0;

	flick_transform(ge, ev);

	if (mVelocity[FM_R] == 0)
		return 0;
	if (mDistance[FM_R] == 0)
		return 0;
	if (mDistance[FM_R] > flick_dist_max_threshold)
		return 0;
	if (mDistance[FM_R] < flick_dist_min_threshold)
		return 0;
	dt = mDistance[FM_R] / mVelocity[FM_R];
	if (dt > flick_velo_max_threshold)
		return 0;
	if (dt < flick_velo_min_threshold)
		return 0;
	if ((ev->time - mGestureStartTime) > drag_timeout_threshold)
		return 0;
#if FLICK_DEBUG_PRINT
	fprintf(stdout, "\t%s() - dist:%f(mm), velo:%f(mm/ms), time:%f(ms)\n",
		__func__, mDistance[FM_R], mVelocity[FM_R], dt);
#endif
	return 1;	/* flick */
}

static int flick_direction_4(struct grail *ge, const struct grail_event *ev)
{
	int dir;
	if (mDistance[FM_A] < radian_45)
		dir = 1;
	else
		dir = 2;
	if (dir == 1) {
		if (mDistance[FM_X] >= 0)
			return 4;					/* for East */
		return 8;						/* for West */
	}
	if (mDistance[FM_Y] >= 0)
		return 2;						/* for South */
	return 6;							/* for North */
}

static int flick_direction_8(struct grail *ge, const struct grail_event *ev)
{
	int dir;
	if (mDistance[FM_A] < radian_30)
		dir = 1;
	else if (mDistance[FM_A] > radian_60)
		dir = 3;
	else
		dir = 2;
	if (dir == 1) {
		if (mDistance[FM_X] >= 0)
			return 4;		/* for East */
		return 8;			/* for West */
	}
	if (dir == 3) {
		if (mDistance[FM_Y] >= 0)
			return 2;		/* for South */
		return 6;			/* for North */
	}
	if (mDistance[FM_X] > 0 && mDistance[FM_Y] > 0)
		return 3;			/* for SouthEast */
	if (mDistance[FM_X] < 0 && mDistance[FM_Y] < 0)
		return 7;			/* for NorthWest */
	if (mDistance[FM_X] > 0 && mDistance[FM_Y] < 0)
		return 5;			/* for NorthEast */
	return 9;				/* for SouthWest */
}

static int flick_direction(struct grail *ge, const struct grail_event *ev)
{
	if (opt_flick_dir == 4)
		return flick_direction_4(ge, ev);
	return flick_direction_8(ge, ev);
}

static void pinch_reset(struct grail *ge, const struct grail_event *ev)
{
	int i;
	for (i = 0; i < DIM_FM; i++) {
		mPinchingDistance[i] = 0;
	}
	mPinchingDistance[FM_R] = ev->prop[GRAIL_PROP_PINCH_R];
}

static int pinch_check(struct grail *ge, const struct grail_event *ev)
{
	float dw, dh;

	dw = fabs(ev->prop[GRAIL_PROP_PINCH_X2]
				- ev->prop[GRAIL_PROP_PINCH_X1]) / scale_ppm_x;
	dh = fabs(ev->prop[GRAIL_PROP_PINCH_Y2]
				- ev->prop[GRAIL_PROP_PINCH_Y1]) / scale_ppm_y;
	if ((mPinchingDistance[FM_X] == 0) && (mPinchingDistance[FM_Y] == 0)) {
		mPinchingDistance[FM_X] = dw;
		mPinchingDistance[FM_Y] = dh;
		return 0;	/* First pinching update */
	}
	if ((fabs(dw - mPinchingDistance[FM_X]) >= pinch_dist_min_threshold) ||
		(fabs(dh - mPinchingDistance[FM_Y]) >= pinch_dist_min_threshold)) {
		mPinchingDistance[FM_X] = dw;
		mPinchingDistance[FM_Y] = dh;
		return 1;	/* Need pinching report */
	}
	return 0;
}

static int pinch_direction(struct grail *ge, const struct grail_event *ev)
{
	float fCurr = mPinchingDistance[FM_R];
	mPinchingDistance[FM_R] = ev->prop[GRAIL_PROP_PINCH_R];
	if (ev->prop[GRAIL_PROP_PINCH_R] > fCurr)
		return 28;	/* pinch in *//* zoom in */
	return 29;		/* pinch out *//* zoom out */
}

static void gesture_property(struct grail *ge, const struct grail_event *ev)
{
	struct grail_contact touch[32];
	int i, ntouch;

	ntouch = grail_get_contacts(ge, touch, 32);

	fprintf(stderr, "gesture %d %d %d %d - %f %f %d - %d\n",
		ev->type, ev->id, ev->client_id.client, ev->client_id.event,
		ev->pos.x, ev->pos.y, ev->ntouch, ev->status);
	for (i = 0; i < ntouch; i++) {
		const struct grail_contact *t = &touch[i];
		fprintf(stderr, "\t%d: %d %d\n", i, t->id, t->tool_type);
		fprintf(stderr, "\t   %f %f\n", t->pos.x, t->pos.y);
		fprintf(stderr, "\t   %f %f %f %f\n", t->touch_major,
			t->touch_minor, t->width_major, t->width_minor);
		fprintf(stderr, "\t   %f %f\n", t->angle, t->pressure);
	}
	for (i = 0; i < ev->nprop; i++)
		fprintf(stderr, "\t%d: %f\n", i, ev->prop[i]);
}

static void gesture_reset(struct grail *ge, const struct grail_event *ev)
{
	mSingleDrag = 0;
	mMultiPinching = 0;
	flick_reset(ge, ev);
	pinch_reset(ge, ev);
}

static void gesture_tap1(struct grail *ge, const struct grail_event *ev)
{
	struct uinput_api *ua = ge->priv;

#if DEBUG_TAP1_PROPERTY
	gesture_property(ge, ev);
#endif
	ua->valuators[2] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_DT];
	ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_X_T0];
	ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_Y_T0];
	uinput_PenDown(ua);
	uinput_PenUp(ua);
}

static void gesture_tap2(struct grail *ge, const struct grail_event *ev)
{
	struct uinput_api *ua = ge->priv;

#if DEBUG_TAP2_PROPERTY
	gesture_property(ge, ev);
#endif

	ua->valuators[2] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_DT];
	ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_X_T0];
	ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_Y_T0];
	uinput_PenDown(ua);
	ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_X_T1];
	ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_Y_T1];
	uinput_PenDown_2nd(ua);
	ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_X_T0];
	ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_Y_T0];
	uinput_PenUp(ua);
	ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_X_T1];
	ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_TAP_Y_T1];
	uinput_PenUp_2nd(ua);
}

static void gesture_drag1(struct grail *ge, const struct grail_event *ev)
{
	struct uinput_api *ua = ge->priv;

#if DEBUG_DRAG1_PROPERTY
	gesture_property(ge, ev);
#endif

	switch (ev->status) {
	case GRAIL_STATUS_BEGIN:
		mSingleDrag = 1;
		if (mMultiPinching) {
			mMultiPinching = 0;
		} else {
			ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_DRAG_X_T0];
			ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_DRAG_Y_T0];
			uinput_PenDown(ua);
		}
		flick_reset(ge, ev);
		break;

	case GRAIL_STATUS_UPDATE:
		ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_DRAG_X_T0];
		ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_DRAG_Y_T0];
		uinput_PenMove(ua);
		flick_update(ge, ev);
		break;

	case GRAIL_STATUS_END:
		/* Check flick */
		if (flick_check(ge, ev)) {
			ua->gestureId = flick_direction(ge, ev);
			ua->valuators[0] = (int)fabs(mDistance[FM_X]);
			ua->valuators[1] = (int)fabs(mDistance[FM_Y]);
		//	ua->valuators[2] = (int)(mDistance[FM_R] / mVelocity[FM_R]);
			ua->valuators[2] = ev->time - mGestureStartTime;
			uinput_Gesture(ua);
		}
		ua->valuators[2] = (u_int16_t)(ev->time - mGestureStartTime);
		ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_DRAG_X_T0];
		ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_DRAG_Y_T0];
		uinput_PenUp(ua);
		mSingleDrag = 0;
		break;

	default:
		fprintf(stdout, "%s() - Unknown gesture status.\n", __func__);
		break;
	}
}

static void gesture_drag2(struct grail *ge, const struct grail_event *ev)
{
#if DEBUG_DRAG2_PROPERTY
	gesture_property(ge, ev);
#endif
}

static void gesture_pinch2(struct grail *ge, const struct grail_event *ev)
{
	struct uinput_api *ua = ge->priv;
	int ntouch;
	struct grail_contact touch[32];
	const struct grail_contact *gc;

#if DEBUG_PINCH2_PROPERTY
	gesture_property(ge, ev);
#endif

	switch (ev->status) {
	case GRAIL_STATUS_BEGIN:
		mMultiPinching = 1;
		if (mSingleDrag) {
			mSingleDrag = 0;
		} else {
			ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_X_T0];
			ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_Y_T0];
			uinput_PenDown(ua);
			flick_reset(ge, ev);
		}
		ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_X_T1];
		ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_Y_T1];
		uinput_PenDown_2nd(ua);
		pinch_reset(ge, ev);
		break;

	case GRAIL_STATUS_UPDATE:
		/* Move report */
		ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_X_T0];
		ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_Y_T0];
		uinput_PenMove(ua);
		ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_X_T1];
		ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_Y_T1];
		uinput_PenMove_2nd(ua);
		if (pinch_check(ge, ev)) {
			/* Pinch report */
			ua->valuators[0] = (int)mPinchingDistance[FM_X];
			ua->valuators[1] = (int)mPinchingDistance[FM_Y];
			ua->valuators[2] = 0;
			ua->gestureId = pinch_direction(ge, ev);
			uinput_Gesture(ua);
		}
		break;

	case GRAIL_STATUS_END:
		ua->valuators[2] = (u_int16_t)(ev->time - mGestureStartTime);
		if (ev->ntouch == 0) {
			ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_X_T0];
			ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_Y_T0];
			uinput_PenUp(ua);
			ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_X_T1];
			ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_Y_T1];
			mMultiPinching = 0;
		} else {
			ntouch = grail_get_contacts(ge, touch, 32);
			gc = &touch[0];
			if (gc->id == GRAIL_PROP_DRAG_ID_T0) {
				ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_X_T0];
				ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_Y_T0];
			} else {
				ua->valuators[0] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_X_T1];
				ua->valuators[1] = (u_int16_t)ev->prop[GRAIL_PROP_PINCH_Y_T1];
			}
		}
		uinput_PenUp_2nd(ua);
		break;

	default:
		fprintf(stdout, "%s() - Unknown gesture status.\n", __func__);
		break;
	}
}

static void tp_gesture(struct grail *ge, const struct grail_event *ev)
{
	switch (ev->type) {
	case GRAIL_TYPE_TAP1:
		gesture_tap1(ge, ev);
		gesture_reset(ge, ev);
		break;
	case GRAIL_TYPE_TAP2:
//		gesture_tap2(ge, ev);
		gesture_reset(ge, ev);
		break;
	case GRAIL_TYPE_DRAG1:
		gesture_drag1(ge, ev);
		break;
	case GRAIL_TYPE_DRAG2:
		gesture_drag2(ge, ev);
		break;
	case GRAIL_TYPE_PINCH2:
		gesture_pinch2(ge, ev);
		break;
	default:
		gesture_property(ge, ev);
		break;
	}
}

static int tp_get_clients(struct grail *ge,
			  struct grail_client_info *clients, int max_clients,
			  const struct grail_coord *coords, int num_coords,
			  const grail_mask_t *types, int type_bytes)
{
	memset(&clients[0], 0, sizeof(clients[0]));
	clients[0].id.client = 999;
	clients[0].id.root = 1;
	clients[0].id.event = 2;
	clients[0].id.child = 3;
	memcpy(clients[0].mask, flag_mask, sizeof(flag_mask));
	return 1;
}

/*
 * Single touch event.
 * Grail callback when not gesture.
 */
static void tp_event(struct grail *ge, const struct input_event *ev)
{
	struct uinput_api *ua = ge->priv;

#if DEBUG_TP_EVENT
	fprintf(stderr, "%lu.%06u %04x %04x %08x\n",
		ev->time.tv_sec, (unsigned)ev->time.tv_usec,
		ev->type, ev->code, ev->value);
#endif

	switch (ev->type) {
	case EV_ABS:
		if (ev->code == ABS_MT_TRACKING_ID) {
			if (ev->value != -1) {
				if (gev_touch.id != ev->value) {
					gev_touch.id = ev->value;
					gev_touch.status = GRAIL_STATUS_BEGIN;
				}
			} else {
				gev_touch.id = -1;
				gev_touch.status = GRAIL_STATUS_END;
			}
			break;
		}
		if (ev->code == ABS_MT_POSITION_X) {
			gev_touch.pos.x = ev->value * scale_x;
			break;
		}
		if (ev->code == ABS_MT_POSITION_Y) {
			gev_touch.pos.y = ev->value * scale_y;
			break;
		}
		break;
	case EV_SYN:
		if (ev->code == SYN_REPORT) {
			if (gev_touch.status == GRAIL_STATUS_BEGIN) {
				ua->valuators[0] = gev_touch.pos.x;
				ua->valuators[1] = gev_touch.pos.y;
				uinput_PenDown(ua);
				gev_touch.status = GRAIL_STATUS_UPDATE;
				break;
			}
			if (gev_touch.status == GRAIL_STATUS_UPDATE) {
				ua->valuators[0] = gev_touch.pos.x;
				ua->valuators[1] = gev_touch.pos.y;
				uinput_PenMove(ua);
				break;
			}
			if (gev_touch.status == GRAIL_STATUS_END) {
				ua->valuators[0] = gev_touch.pos.x;
				ua->valuators[1] = gev_touch.pos.y;
				uinput_PenUp(ua);
				break;
			}
		}
		break;
	default:
		fprintf(stderr, "Unknown event %lu.%06u %04x %04x %08x\n",
			ev->time.tv_sec, (unsigned)ev->time.tv_usec,
			ev->type, ev->code, ev->value);
		break;
	}
}

static void loop_device(struct grail *ge, int fd)
{
	while (!grail_idle(ge, fd, 5000))
		grail_pull(ge, fd);
}

static void on_terminate(int signal)
{
	fprintf(stderr, "jgestured caught signal %d, terminate.\n", signal);
	do_terminate = 1;
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
	struct grail ge;
	struct stat fs;
	struct uinput_api *ua;
	int i, fd, opt, opt_dir;

	opt_flick_dir = 4;
	opt_event_file = strdup("/dev/input/event1");

	while ((opt = getopt(argc, argv, "d:i:")) != -1) {
		switch (opt) {
		case 'd':
			opt_dir = atoi(optarg);
			if ((opt_dir == 4) || (opt_dir == 8))
				opt_flick_dir = opt_dir;
			break;
		case 'i':
			free(opt_event_file);
			opt_event_file = strdup(optarg);
			break;
		default:
			fprintf(stderr, "Usage: %s [-d 4|8] [-i device]\n", argv[0]);
			free(opt_event_file);
			return -1;
		}
	}

	memset(&ge, 0, sizeof(ge));
	ge.get_clients = tp_get_clients;
	ge.event = tp_event;
	ge.gesture = tp_gesture;

	unsigned long long mask = strtoull(gesture_mask, 0, 0);
	for (i = 0; i < DIM_GRAIL_TYPE; i++)
		if ((mask >> i) & 1) {
			grail_mask_set(flag_mask, i);
		}

	set_signal_handler();

	fd = open(opt_event_file, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "error: could not open device\n");
		free(opt_event_file);
		return -1;
	}
	if (fstat(fd, &fs)) {
		fprintf(stderr, "error: could not stat the device\n");
		goto exit_lbl;
	}
	if (fs.st_rdev && ioctl(fd, EVIOCGRAB, 1)) {
		fprintf(stderr, "error: could not grab the device\n");
		goto exit_lbl;
	}

	if (grail_open(&ge, fd)) {
		fprintf(stderr, "error: could not open touch device\n");
		goto exit_lbl;
	}

	struct grail_coord min = { 0, 0 };
	struct grail_coord max = { m_mapped_xres-1, m_mapped_yres-1 };
	struct grail_coord pbox = { m_phys_xsize, m_phys_ysize };
	grail_set_bbox(&ge, &min, &max, &pbox);

	setup_scale(&ge);
	flick_reset(&ge, NULL);
	gev_touch.id = 0;

	ua = uinput_new();
	ge.priv = (void*)ua;

	while (!do_terminate)
		loop_device(&ge, fd);

	uinput_destroy(ua);
	grail_close(&ge, fd);

	if (fs.st_rdev)
		ioctl(fd, EVIOCGRAB, 0);

exit_lbl:
	close(fd);
	free(opt_event_file);

	return 0;
}
