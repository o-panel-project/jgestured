/*****************************************************************************
 *
 * grail - Gesture Recognition And Instantiation Library
 *
 * Copyright (C) 2010 Canonical Ltd.
 * Copyright (C) 2010 Henrik Rydberg <rydberg@bitmath.org>
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

#include "grail-recognizer.h"
#include "grail-impl.h"
#include <math.h>

#if defined(GOODIX_TOUCHSCREEN)
static const float FM_SN[DIM_FM] = { 1000, 1000, 1000, 1000 };
static const float FM_BAR[DIM_FM] = { 50, 50, 50, 50 };
static const grail_time_t FM_HOLD_MS[DIM_FM] = { 10, 10, 10, 10 };
static const grail_time_t FM_BAR_MS[DIM_FM] = { 170, 170, 240, 240 };/*tap*/
static const grail_time_t SAMPLE_MS = 0;
static const float EPS = 1e-3;	/* pinchに影響 */
#elif defined(MELFAS_TOUCHSCREEN)
static const float FM_SN[DIM_FM] = { 1000, 1000, 1000, 1000 };
static const float FM_BAR[DIM_FM] = { 50, 50, 50, 50 };
static const grail_time_t FM_HOLD_MS[DIM_FM] = { 10, 10, 10, 10 };
static const grail_time_t FM_BAR_MS[DIM_FM] = { 170, 170, 240, 240 };/*tap*/
static const grail_time_t SAMPLE_MS = 0;
static const float EPS = 1e-3;	/* pinchに影響 */
#else
static const float FM_SN[DIM_FM] = { 1000, 1000, 1000, 1000 };
static const float FM_BAR[DIM_FM] = { 50, 50, 50, 50 };
static const grail_time_t FM_HOLD_MS[DIM_FM] = { 60, 60, 60, 60 };
static const grail_time_t FM_BAR_MS[DIM_FM] = { 300, 300, 500, 500 };
static const grail_time_t SAMPLE_MS = 10;
static const float EPS = 1e-3;
#endif

static void compute_position(float *x, float *y,
			     const struct utouch_frame *frame)
{
	int i, n = frame->num_active;
	*x = 0;
	*y = 0;
	if (n < 1)
		return;
	for (i = 0; i < n; i++) {
		const struct utouch_contact *t = frame->active[i];
		*x += t->x;
		*y += t->y;
	}
	*x /= n;
	*y /= n;
}

static float compute_radius(float x, float y,
			    const struct utouch_frame *frame)
{
	int i, n = frame->num_active;
	float r = 0, r2 = 0;
	if (n < 2)
		return r;
	for (i = 0; i < n; i++) {
		const struct utouch_contact *t = frame->active[i];
		float dx = t->x - x;
		float dy = t->y - y;
		r2 += dx * dx + dy * dy;
	}
	r2 /= n;
	r = sqrt(r2);
	return r;
}

static float compute_rotation(float x, float y, float r,
			      const struct utouch_frame *frame)
{
	int i, n = frame->num_active;
	float da = 0, darc2 = 0;
	if (n < 2)
		return da;
	for (i = 0; i < n; i++) {
		const struct utouch_contact *t = frame->active[i];
		const struct utouch_contact *ot = t->prev;
		float dx = t->x - x;
		float dy = t->y - y;
		float mx = t->x - ot->x;
		float my = t->y - ot->y;
		darc2 += dx * my - dy * mx;
	}
	darc2 /= n;
	da = darc2 / (r * r);
	return da;
}

/* Response-augmented EWMA filter, courtesy of Vojtech Pavlik */
static float move_filter(const struct filter_model *m, float val)
{
	if (val > m->value - m->fuzz / 2 && val < m->value + m->fuzz / 2)
		return m->value;
	if (val > m->value - m->fuzz && val < m->value + m->fuzz)
		return (m->value * 3 + val) / 4;
	if (val > m->value - m->fuzz * 2 && val < m->value + m->fuzz * 2)
		return (m->value + val) / 2;
	return val;
}

static void move_reset(struct move_model *m, int i, float x, grail_time_t t)
{
	struct filter_model *fm = &m->fm[i];
	fm->raw_delta = 0;
	fm->action_delta = 0;
	fm->velocity = 0;
	fm->value = x;
	fm->original = x;
	fm->original_ms = t;
	fm->sample = x;
	fm->sample_ms = t;
	m->tickle &= ~(1 << i);
	m->active &= ~(1 << i);
	m->timeout &= ~(1 << i);
}

static void move_update(struct move_model *m, int i, float x, grail_time_t t)
{
	struct filter_model *fm = &m->fm[i];
	float dt = t - fm->sample_ms;
	fm->raw_delta = x - fm->value;
	fm->action_delta = fm->raw_delta;
	fm->value = x;
	if (dt > SAMPLE_MS) {
		fm->velocity = (x - fm->sample) / dt;
		fm->sample = x;
		fm->sample_ms = t;
	}
	if (fabs(fm->raw_delta) > EPS) {
		m->tickle |= (1 << i);
	} else
		m->tickle &= ~(1 << i);
	if (m->active & (1 << i))
		return;
	fm->action_delta = x - fm->original;
	if (t - fm->original_ms > fm->hold_ms &&
	    fabs(fm->action_delta) > fm->bar) {
		m->active |= (1 << i);
	} else if (t - fm->original_ms > fm->bar_ms) {
		m->active |= (1 << i);
		m->timeout |= (1 << i);
	} else {
		fm->action_delta = 0;
	}
}

void gru_init_motion(struct grail *ge)
{
	struct utouch_surface *s = utouch_frame_get_surface(ge->impl->fh);
	struct gesture_recognizer *gru = ge->gru;
	struct move_model *m = &gru->move;
	float D[DIM_FM];
	int i;
#if defined(JPANEL_TOUCHSCREEN)
	D[FM_X] = 0;
	D[FM_Y] = 0;
	D[FM_R] = 0;
	D[FM_A] = 0;
#else
	D[FM_X] = s->max_x - s->min_x;
	D[FM_Y] = s->max_y - s->min_y;
	D[FM_R] = sqrt(D[FM_X] * D[FM_X] + D[FM_Y] * D[FM_Y]);
	D[FM_A] = 2 * M_PI;
#endif
	for (i = 0; i < DIM_FM; i++) {
		m->fm[i].fuzz = D[i] / FM_SN[i];
		m->fm[i].bar = D[i] / FM_BAR[i];
		m->fm[i].hold_ms = FM_HOLD_MS[i];
		m->fm[i].bar_ms = FM_BAR_MS[i];
	}
}

void gru_motion(struct grail *ge,
		const struct utouch_frame *frame)
{
	struct gesture_recognizer *gru = ge->gru;
	struct move_model *m = &gru->move;
	grail_time_t t = frame->time;
	float x, y, r, a;

	compute_position(&x, &y, frame);
	if (frame->prev->revision != frame->revision) {
		r = compute_radius(x, y, frame);
		a = 0;
		move_reset(m, FM_X, x, t);
		move_reset(m, FM_Y, y, t);
		move_reset(m, FM_R, r, t);
		move_reset(m, FM_A, a, t);
		m->single = 0;
		m->multi = 0;
	} else if (frame->num_active < 2) {
		r = 0;
		a = 0;
		move_update(m, FM_X, x, t);
		move_update(m, FM_Y, y, t);
		move_reset(m, FM_R, r, t);
		move_reset(m, FM_A, a, t);
		m->single = 1;
		m->multi = 0;
	} else {
		x = move_filter(&m->fm[FM_X], x);
		y = move_filter(&m->fm[FM_Y], y);
		r = compute_radius(x, y, frame);
		r = move_filter(&m->fm[FM_R], r);
		a = m->fm[FM_A].value;
		a += compute_rotation(x, y, r, frame);
		a = move_filter(&m->fm[FM_A], a);
		move_update(m, FM_X, x, t);
		move_update(m, FM_Y, y, t);
		move_update(m, FM_R, r, t);
		move_update(m, FM_A, a, t);
		m->single = 0;
		m->multi = 1;
	}
	m->ntouch = frame->num_active;
	m->time = t;
}

void gru_event(struct grail *ge, int gid,
	       const struct move_model *m,
	       const grail_prop_t *prop, int nprop)
{
	gin_gid_event(ge, gid, m->fm[FM_X].value, m->fm[FM_Y].value, m->ntouch,
		      prop, nprop, 0);
}

void gru_end(struct grail *ge, int gid, const struct move_model *m,
	     const grail_prop_t *prop, int nprop)
{
	gin_gid_end(ge, gid, m->fm[FM_X].value, m->fm[FM_Y].value, m->ntouch,
		    prop, nprop);
}
