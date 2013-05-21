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

#ifndef _GRAIL_TYPES_H
#define _GRAIL_TYPES_H

#define GRAIL_TYPE_DRAG1	0	/* one-finger drag/pan/scroll */
#define GRAIL_TYPE_PINCH1	1	/* one-finger pinch-to-zoom */
#define GRAIL_TYPE_ROTATE1	2	/* one-finger rotation */

#define GRAIL_TYPE_DRAG2	3	/* two-finger drag/pan/scroll */
#define GRAIL_TYPE_PINCH2	4	/* two-finger pinch-to-zoom */
#define GRAIL_TYPE_ROTATE2	5	/* two-finger rotation */

#define GRAIL_TYPE_DRAG3	6	/* three-finger drag/pan */
#define GRAIL_TYPE_PINCH3	7	/* three-finger pinch */
#define GRAIL_TYPE_ROTATE3	8	/* three-finger rotate */

#define GRAIL_TYPE_DRAG4	9	/* four-finger drag/pan */
#define GRAIL_TYPE_PINCH4	10	/* four-finger pinch */
#define GRAIL_TYPE_ROTATE4	11	/* four-finger rotate */

#define GRAIL_TYPE_DRAG5	12	/* five-finger drag/pan */
#define GRAIL_TYPE_PINCH5	13	/* five-finger pinch */
#define GRAIL_TYPE_ROTATE5	14	/* five-finger rotate */

#define GRAIL_TYPE_TAP1		15	/* one-finger single tap */
#define GRAIL_TYPE_TAP2		16	/* two-finger single tap */
#define GRAIL_TYPE_TAP3		17	/* three-finger single tap */
#define GRAIL_TYPE_TAP4		18	/* four-finger single tap */
#define GRAIL_TYPE_TAP5		19	/* five-finger single tap */

#define GRAIL_TYPE_EDRAG	20	/* three-finger environment drag */
#define GRAIL_TYPE_EPINCH	21	/* three-finger environment drag */
#define GRAIL_TYPE_EROTATE	22	/* three-finger environment drag */

#define GRAIL_TYPE_MDRAG	23	/* four-finger meta drag */
#define GRAIL_TYPE_MPINCH	24	/* four-finger meta drag */
#define GRAIL_TYPE_MROTATE	25	/* four-finger meta drag */

#define GRAIL_TYPE_SYSFLAG1	26	/* reserved system flag */

#define GRAIL_MAIN_DRAG		0
#define GRAIL_MAIN_PINCH	1
#define GRAIL_MAIN_ROTATE	2
#define GRAIL_MAIN_TAP		3
#define GRAIL_MAIN_SYSFLAG	4

#define GRAIL_PROP_DRAG_DX	0	/* horizontal position delta */
#define GRAIL_PROP_DRAG_DY	1	/* vertical position delta */
#define GRAIL_PROP_DRAG_VX	2	/* horizontal velocity */
#define GRAIL_PROP_DRAG_VY	3	/* vertical velocity */
#define GRAIL_PROP_DRAG_X	4	/* horizontal position */
#define GRAIL_PROP_DRAG_Y	5	/* vertical position */
#define GRAIL_PROP_DRAG_X1	6	/* bounding box x1 */
#define GRAIL_PROP_DRAG_Y1	7	/* bounding box y1 */
#define GRAIL_PROP_DRAG_X2	8	/* bounding box x2 */
#define GRAIL_PROP_DRAG_Y2	9	/* bounding box y2 */
#define GRAIL_PROP_DRAG_ID_T0	10	/* first touch id */
#define GRAIL_PROP_DRAG_X_T0	11	/* first touch horizontal position */
#define GRAIL_PROP_DRAG_Y_T0	12	/* first touch vertical position */
#define GRAIL_PROP_DRAG_ID_T1	13
#define GRAIL_PROP_DRAG_X_T1	14
#define GRAIL_PROP_DRAG_Y_T1	15
#define GRAIL_PROP_DRAG_ID_T2	16
#define GRAIL_PROP_DRAG_X_T2	17
#define GRAIL_PROP_DRAG_Y_T2	18
#define GRAIL_PROP_DRAG_ID_T3	19
#define GRAIL_PROP_DRAG_X_T3	20
#define GRAIL_PROP_DRAG_Y_T3	21
#define GRAIL_PROP_DRAG_ID_T4	22
#define GRAIL_PROP_DRAG_X_T4	23
#define GRAIL_PROP_DRAG_Y_T4	24

#define GRAIL_PROP_PINCH_DR	0	/* radius delta */
#define GRAIL_PROP_PINCH_VR	1	/* radial velocity */
#define GRAIL_PROP_PINCH_R	2	/* radius */
#define GRAIL_PROP_PINCH_X1	3	/* bounding box x1 */
#define GRAIL_PROP_PINCH_Y1	4	/* bounding box y1 */
#define GRAIL_PROP_PINCH_X2	5	/* bounding box x2 */
#define GRAIL_PROP_PINCH_Y2	6	/* bounding box y2 */
#define GRAIL_PROP_PINCH_ID_T0	7	/* first touch id */
#define GRAIL_PROP_PINCH_X_T0	8	/* first touch horizontal position */
#define GRAIL_PROP_PINCH_Y_T0	9	/* first touch vertical position */
#define GRAIL_PROP_PINCH_ID_T1	10
#define GRAIL_PROP_PINCH_X_T1	11
#define GRAIL_PROP_PINCH_Y_T1	12
#define GRAIL_PROP_PINCH_ID_T2	13
#define GRAIL_PROP_PINCH_X_T2	14
#define GRAIL_PROP_PINCH_Y_T2	15
#define GRAIL_PROP_PINCH_ID_T3	16
#define GRAIL_PROP_PINCH_X_T3	17
#define GRAIL_PROP_PINCH_Y_T3	18
#define GRAIL_PROP_PINCH_ID_T4	19
#define GRAIL_PROP_PINCH_X_T4	20
#define GRAIL_PROP_PINCH_Y_T4	21

#define GRAIL_PROP_ROTATE_DA	0	/* angle delta */
#define GRAIL_PROP_ROTATE_VA	1	/* angular velocity */
#define GRAIL_PROP_ROTATE_A	2	/* angle */
#define GRAIL_PROP_ROTATE_X1	3	/* bounding box x1 */
#define GRAIL_PROP_ROTATE_Y1	4	/* bounding box y1 */
#define GRAIL_PROP_ROTATE_X2	5	/* bounding box x2 */
#define GRAIL_PROP_ROTATE_Y2	6	/* bounding box y2 */
#define GRAIL_PROP_ROTATE_ID_T0	7	/* first touch id */
#define GRAIL_PROP_ROTATE_X_T0	8	/* first touch horizontal position */
#define GRAIL_PROP_ROTATE_Y_T0	9	/* first touch vertical position */
#define GRAIL_PROP_ROTATE_ID_T1	10
#define GRAIL_PROP_ROTATE_X_T1	11
#define GRAIL_PROP_ROTATE_Y_T1	12
#define GRAIL_PROP_ROTATE_ID_T2	13
#define GRAIL_PROP_ROTATE_X_T2	14
#define GRAIL_PROP_ROTATE_Y_T2	15
#define GRAIL_PROP_ROTATE_ID_T3	16
#define GRAIL_PROP_ROTATE_X_T3	17
#define GRAIL_PROP_ROTATE_Y_T3	18
#define GRAIL_PROP_ROTATE_ID_T4	19
#define GRAIL_PROP_ROTATE_X_T4	20
#define GRAIL_PROP_ROTATE_Y_T4	21

#define GRAIL_PROP_TAP_DT	0	/* tap time (ms) */
#define GRAIL_PROP_TAP_X	1	/* horizontal position */
#define GRAIL_PROP_TAP_Y	2	/* vertical position */
#define GRAIL_PROP_TAP_X1	3	/* bounding box x1 */
#define GRAIL_PROP_TAP_Y1	4	/* bounding box y1 */
#define GRAIL_PROP_TAP_X2	5	/* bounding box x2 */
#define GRAIL_PROP_TAP_Y2	6	/* bounding box y2 */
#define GRAIL_PROP_TAP_ID_T0	7	/* first touch id */
#define GRAIL_PROP_TAP_X_T0	8	/* first touch horizontal position */
#define GRAIL_PROP_TAP_Y_T0	9	/* first touch vertical position */
#define GRAIL_PROP_TAP_ID_T1	10
#define GRAIL_PROP_TAP_X_T1	11
#define GRAIL_PROP_TAP_Y_T1	12
#define GRAIL_PROP_TAP_ID_T2	13
#define GRAIL_PROP_TAP_X_T2	14
#define GRAIL_PROP_TAP_Y_T2	15
#define GRAIL_PROP_TAP_ID_T3	16
#define GRAIL_PROP_TAP_X_T3	17
#define GRAIL_PROP_TAP_Y_T3	18
#define GRAIL_PROP_TAP_ID_T4	19
#define GRAIL_PROP_TAP_X_T4	20
#define GRAIL_PROP_TAP_Y_T4	21


#endif
