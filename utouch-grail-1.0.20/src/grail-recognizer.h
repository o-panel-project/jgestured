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

#ifndef _GRAIL_RECOGNIZER_H
#define _GRAIL_RECOGNIZER_H

#include "grail-gestures.h"

struct gesture_recognizer {
	struct move_model move;
	struct combo_model drag;
	struct combo_model pinch;
	struct combo_model rotate;
	struct combo_model windrag;
	struct combo_model winpinch;
	struct combo_model winrotate;
	struct tapping_model tapping;
};

int gru_init(struct grail *ge);
void gru_recognize(struct grail *ge, const struct utouch_frame *frame);
void gru_destroy(struct grail *ge);

#endif
