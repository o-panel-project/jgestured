/*
 * Pinching event recognizer
 */

#include <stdio.h>
#include <math.h>

#include "frame.h"
#include "gesture.h"

#define PINCH_DEBUG_PRINT 0

extern float m_scale_ppm_x;
extern float m_scale_ppm_y;

extern float pinch_dist_min_threshold;
static float mPinchingDistance[DIM_FM];

static float compute_distance(const struct utouch_frame *f)
{
	float x1, y1, x2, y2, r;

	x1 = f->slots[0]->x * m_scale_ppm_x;
	y1 = f->slots[0]->y * m_scale_ppm_y;
	x2 = f->slots[1]->x * m_scale_ppm_x;
	y2 = f->slots[1]->y * m_scale_ppm_y;
	r = hypotf((x2 - x1), (y2 - y1));
	return r;
}

void pinch_init()
{
}

void pinch_reset(const struct utouch_frame *f)
{
	int i;
	for (i = 0; i < DIM_FM; i++) {
		mPinchingDistance[i] = 0.0;
	}
	if (f->num_active < 2)
		mPinchingDistance[FM_R] = 0.0;
	else
		mPinchingDistance[FM_R] = compute_distance(f);
}

int pinch_check(const struct utouch_frame *f)
{
	float dw, dh;

	if (f->num_active < 2)
		return 0;
	dw = fabsf(f->slots[1]->x - f->slots[0]->x) / m_scale_ppm_x;
	dh = fabsf(f->slots[1]->y - f->slots[0]->y) / m_scale_ppm_y;
	if ((mPinchingDistance[FM_X] == 0) && (mPinchingDistance[FM_Y] == 0)) {
		mPinchingDistance[FM_X] = dw;
		mPinchingDistance[FM_Y] = dh;
		return 0;   /* First pinching update */
	}
	if ((fabsf(dw - mPinchingDistance[FM_X]) >= pinch_dist_min_threshold) ||
		(fabsf(dh - mPinchingDistance[FM_Y]) >= pinch_dist_min_threshold)) {
		mPinchingDistance[FM_X] = dw;
		mPinchingDistance[FM_Y] = dh;
		return 1;   /* Need pinching report */
	}
	return 0;
}

static int pinch_direction(const struct utouch_frame *f)
{
	float fCurr = mPinchingDistance[FM_R];
	float r;
	if (f->num_active < 2)
		r = 0.0;
	else
		r = compute_distance(f);
	mPinchingDistance[FM_R] = r;
	if (r >= fCurr)
		return 28;  /* pinch in *//* zoom in */
	return 29;      /* pinch out *//* zoom out */
}

void pinch_event(struct uinput_api *ua, const struct utouch_frame *f)
{
	ua->gestureId = pinch_direction(f);
	ua->valuators[0] = (u_int16_t)mPinchingDistance[FM_X];
	ua->valuators[1] = (u_int16_t)mPinchingDistance[FM_Y];
	ua->valuators[2] = 0;
	uinput_Gesture(ua);
}
/* EOF */
