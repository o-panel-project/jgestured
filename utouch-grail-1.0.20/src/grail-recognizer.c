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
#include <string.h>
#include <malloc.h>
#include <errno.h>

int gru_init(struct grail *ge)
{
	struct gesture_recognizer *gru;
	gru = calloc(1, sizeof(struct gesture_recognizer));
	if (!gru)
		return -ENOMEM;
	ge->gru = gru;
	gru_init_motion(ge);
	return 0;
}

void gru_destroy(struct grail *ge)
{
	free(ge->gru);
	ge->gru = NULL;
}

void gru_recognize(struct grail *ge, const struct utouch_frame *frame)
{
	if (!ge->gin || !ge->gru)
		return;
	gru_motion(ge, frame);
#if defined(JPANEL_TOUCHSCREEN)
	gru_drag(ge, frame);
	gru_pinch(ge, frame);
	gru_rotate(ge, frame);
	gru_tapping(ge, frame);
#else
	gru_drag(ge, frame);
	gru_pinch(ge, frame);
	gru_rotate(ge, frame);
	gru_windrag(ge, frame);
	gru_winpinch(ge, frame);
	gru_winrotate(ge, frame);
	gru_tapping(ge, frame);
#endif
}
