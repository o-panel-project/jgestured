/*
 * Flick event recognizer
 */

#include <stdio.h>
#include <math.h>

#include "frame.h"
#include "gesture.h"

/* debug switch */
extern int flick_debug_print;

extern float m_scale_ppm_x; /* Unit of mm */
extern float m_scale_ppm_y; /* Unit of mm */

extern float flick_dist_min_threshold;
extern float flick_dist_max_threshold;
extern float flick_velo_min_threshold;
extern float flick_time_min_threshold;
extern float flick_time_max_threshold;

static const float radian_30 = 30 * M_PI / 180;
static const float radian_45 = 45 * M_PI / 180;
static const float radian_60 = 60 * M_PI / 180;
static const float radian_90 = 90 * M_PI / 180;
static int flick_dir = 4;

static utouch_frame_time_t mFlickStartTime = 0;
static float mFlickPos_x = 0.0;
static float mFlickPos_y = 0.0;
static float mFlickDistance[DIM_FM];
static float mFlickVelocity[DIM_FM];

void flick_init()
{
	flick_reset(NULL);
}

void flick_set_dir_div(int d)
{
	if (d == 4 || d == 8)
		flick_dir = d;
}

void flick_reset(const struct utouch_frame *f)
{
	int i;
	for (i = 0; i < DIM_FM; i++) {
		mFlickDistance[i] = 0.0;
		mFlickVelocity[i] = 0.0;
	}
	if (f) {
		mFlickStartTime = f->time;
		mFlickPos_x = f->slots[f->current_slot]->x;
		mFlickPos_y = f->slots[f->current_slot]->y;
	} else {
		mFlickStartTime = 0;
		mFlickPos_x = 0.0;
		mFlickPos_y = 0.0;
	}
}

void flick_update(const struct utouch_frame *f)
{
	utouch_frame_time_t dt;

	if (flick_debug_print == 1) {
	fprintf(stdout, "\t%s() - new xpos %.2f, before xpos %.2f\n", __func__,
		f->slots[f->current_slot]->x, mFlickPos_x);
	fprintf(stdout, "\t%s() - new ypos %.2f, before ypos %.2f\n", __func__,
		f->slots[f->current_slot]->y, mFlickPos_y);
	}

	mFlickDistance[FM_X] +=
		(f->slots[f->current_slot]->x - mFlickPos_x) / m_scale_ppm_x;
	mFlickDistance[FM_Y] +=
		(f->slots[f->current_slot]->y - mFlickPos_y) / m_scale_ppm_y;
	mFlickPos_x = f->slots[f->current_slot]->x;
	mFlickPos_y = f->slots[f->current_slot]->y;
	dt = f->time - mFlickStartTime;
	if (dt > 0) {
		mFlickVelocity[FM_X] = mFlickDistance[FM_X] / dt;
		mFlickVelocity[FM_Y] = mFlickDistance[FM_Y] / dt;
	}

	if (flick_debug_print == 1)
	fprintf(stdout, "\t%s() - xDist:%.2f(mm), yDist:%.2f(mm), "
		"xVelo:%.2f(mm/ms), yVelo:%.2f(mm/ms), time:%llu(ms)\n", __func__,
		mFlickDistance[FM_X], mFlickDistance[FM_Y],
		mFlickVelocity[FM_X], mFlickVelocity[FM_Y], dt);
}

static void flick_transform(const struct utouch_frame *f)
{
	mFlickDistance[FM_R] =
		hypotf(mFlickDistance[FM_X], mFlickDistance[FM_Y]);
	if (mFlickDistance[FM_X] != 0.0)
		mFlickDistance[FM_A] =
			fabs(atan(mFlickDistance[FM_Y] / mFlickDistance[FM_X]));
	else mFlickDistance[FM_A] = radian_90;
	mFlickVelocity[FM_R] =
		hypotf(mFlickVelocity[FM_X], mFlickVelocity[FM_Y]);

	if (flick_debug_print == 1)
	fprintf(stdout, "\t%s() - dist:%.2f(mm), velo:%.2f(mm/ms), "
			"dir:%.2f(rad), time:%llu(ms)\n", __func__,
			mFlickDistance[FM_R], mFlickVelocity[FM_R], mFlickDistance[FM_A],
			(f->time - mFlickStartTime));
}

int flick_check(const struct utouch_frame *f)
{
	utouch_frame_time_t dt;

	flick_update(f);

	if (mFlickDistance[0] == 0 && mFlickDistance[1] == 0)
		return 0;

	flick_transform(f);

	if (mFlickVelocity[FM_R] == 0)
		return 0;
	if (mFlickDistance[FM_R] == 0)
		return 0;
	if (mFlickDistance[FM_R] > flick_dist_max_threshold)
		return 0;
	if (mFlickDistance[FM_R] < flick_dist_min_threshold)
		return 0;
	if (mFlickVelocity[FM_R] < flick_velo_min_threshold)
		return 0;
	dt = f->time - mFlickStartTime;
	if (dt > flick_time_max_threshold)
		return 0;
	if (dt < flick_time_min_threshold)
		return 0;
	if (flick_debug_print == 1)
	fprintf(stdout, "\t%s() - dist:%.2f(mm), velo:%.2f(mm/ms), time:%llu(ms)\n",
			__func__, mFlickDistance[FM_R], mFlickVelocity[FM_R], dt);
	return 1;   /* flick */
}

static int flick_direction_4()
{
	int dir;
	if (mFlickDistance[FM_A] < radian_45)
		dir = 1;
	else
		dir = 2;
	if (dir == 1) {
		if (mFlickDistance[FM_X] >= 0)
			return 4;                   /* for East */
		return 8;                       /* for West */
	}
	if (mFlickDistance[FM_Y] >= 0)
		return 2;                       /* for South */
	return 6;                           /* for North */
}

static int flick_direction_8()
{
	int dir;
	if (mFlickDistance[FM_A] < radian_30)
		dir = 1;
	else if (mFlickDistance[FM_A] > radian_60)
		dir = 3;
	else
		dir = 2;
	if (dir == 1) {
		if (mFlickDistance[FM_X] >= 0)
			return 4;       /* for East */
		return 8;           /* for West */
	}
	if (dir == 3) {
		if (mFlickDistance[FM_Y] >= 0)
			return 2;       /* for South */
		return 6;           /* for North */
	}
	if (mFlickDistance[FM_X] > 0 && mFlickDistance[FM_Y] > 0)
		return 3;           /* for SouthEast */
	if (mFlickDistance[FM_X] < 0 && mFlickDistance[FM_Y] < 0)
		return 7;           /* for NorthWest */
	if (mFlickDistance[FM_X] > 0 && mFlickDistance[FM_Y] < 0)
		return 5;           /* for NorthEast */
	return 9;               /* for SouthWest */
}

static int flick_direction()
{
	if (flick_dir == 4)
		return flick_direction_4();
	return flick_direction_8();
}

void flick_event(struct uinput_api *ua, const struct utouch_frame *f)
{
	ua->gestureId = flick_direction();
	ua->valuators[0] = (u_int16_t)fabs(mFlickDistance[FM_X]);
	ua->valuators[1] = (u_int16_t)fabs(mFlickDistance[FM_Y]);
	ua->valuators[2] = (u_int16_t)(f->time - mFlickStartTime);
	uinput_Gesture(ua);
}
/* EOF */
