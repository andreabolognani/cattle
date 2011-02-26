/* tape -- Check the tape reports the correct size limits
 * Copyright (C) 2008-2011  Andrea Bolognani <eof@kiyuko.org>
 * This file is part of Cattle
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Homepage: http://kiyuko.org/software/cattle
 */

#include <glib.h>
#include <glib-object.h>
#include <cattle/cattle.h>
#include <stdio.h>

#define STEPS 1024

static gint
mod (gint what,
     gint range)
{
	gint ret;

	ret = what % range;
	if (ret < 0) {
		ret = range + ret;
	}

	return ret;
}

/**
 * test_tape_initial_position:
 *
 * Check the initial position is reported correctly: a newly-created tape
 * is made of a single cell, so the current position is both at the
 * beginning and at the end.
 */
static void
test_tape_initial_position (void)
{
	CattleTape *tape;

	tape = cattle_tape_new ();

	g_assert (cattle_tape_is_at_beginning (tape));
	g_assert (cattle_tape_is_at_end (tape));

	g_object_unref (tape);
}

/**
 * test_tape_right_edge:
 *
 * Starting from the initial position and moving only to the right, the
 * current position must always be at the end and never at the beginning.
 */
static void
test_tape_right_edge (void)
{
	CattleTape *tape;
	gint i;

	tape = cattle_tape_new ();

	for (i = 0; i < STEPS; i++) {

		cattle_tape_move_right (tape);

		g_assert (!cattle_tape_is_at_beginning (tape));
		g_assert (cattle_tape_is_at_end (tape));
	}

	g_object_unref (tape);
}

/**
 * test_tape_left_edge:
 *
 * Starting from the initial position and moving only to the left, the
 * current position must always be at the beginning and never at the end.
 */
static void
test_tape_left_edge (void)
{
	CattleTape *tape;
	gint i;

	tape = cattle_tape_new ();

	for (i = 0; i < STEPS; i++) {

		cattle_tape_move_left (tape);

		g_assert (cattle_tape_is_at_beginning (tape));
		g_assert (!cattle_tape_is_at_end (tape));
	}

	g_object_unref (tape);
}

/**
 * test_tape_in_between:
 *
 * If the curren position is somewhere in between the tape, it should not
 * be reported to be at the beginning or at the end.
 */
static void
test_tape_in_between (void)
{
	CattleTape *tape;
	gint i;

	tape = cattle_tape_new ();

	for (i = 0; i < STEPS; i++) {

		cattle_tape_move_left (tape);
	}

	for (i = 0; i < (STEPS - 1); i++) {

		cattle_tape_move_right (tape);

		g_assert (!cattle_tape_is_at_beginning (tape));
		g_assert (!cattle_tape_is_at_end (tape));
	}

	g_object_unref (tape);
}

/**
 * test_tape_move_right_by:
 *
 * Move the tape a bunch of cells to the right in a single step,
 * then move left the same amount of cells one at a time.
 */
static void
test_tape_move_right_by (void)
{
	CattleTape *tape;
	gint i;
	gint j;

	tape = cattle_tape_new ();

	for (i = 1; i <= 5; i++) {

		cattle_tape_set_current_value (tape, i);
		cattle_tape_move_right_by (tape, STEPS);
	}

	for (i = 5; i >= 1; i--) {
		for (j = 0; j < STEPS; j++) {

			cattle_tape_move_left (tape);
		}
		g_assert (cattle_tape_get_current_value (tape) == i);
	}

	g_object_unref (tape);
}

/**
 * test_tape_move_left_by:
 *
 * Move the tape a bunch of cells to the left in a single step,
 * then move right the same amount of cells one at a time.
 */
static void
test_tape_move_left_by (void)
{
	CattleTape *tape;
	gint i;
	gint j;

	tape = cattle_tape_new ();

	for (i = 1; i <= 5; i++) {

		cattle_tape_set_current_value (tape, i);
		cattle_tape_move_left_by (tape, STEPS);
	}

	for (i = 5; i >= 1; i--) {
		for (j = 0; j < STEPS; j++) {

			cattle_tape_move_right (tape);
		}
		g_assert (cattle_tape_get_current_value (tape) == i);
	}

	g_object_unref (tape);
}

/**
 * test_tape_current_value:
 *
 * Set and get the current value several times.
 */
static void
test_tape_current_value (void)
{
	CattleTape *tape;
	gint i;

	tape = cattle_tape_new ();

	for (i = 1; i < 128; i++) {

		cattle_tape_set_current_value (tape, i);

		g_assert (cattle_tape_get_current_value (tape) == i);
	}

	g_object_unref (tape);
}

/**
 * test_tape_increase_current_value:
 *
 * Increase the current value several times and then decrease it by
 * the number of increase steps taken before.
 */
static void
test_tape_increase_current_value (void)
{
	CattleTape *tape;
	gint i;

	tape = cattle_tape_new ();

	for (i = 0; i < 128; i++) {

		g_assert (cattle_tape_get_current_value (tape) == i);
		cattle_tape_increase_current_value (tape);
	}

	cattle_tape_decrease_current_value_by (tape, 128);
	g_assert (cattle_tape_get_current_value (tape) == 0);

	cattle_tape_increase_current_value_by (tape, 128 * 5);
	g_assert (cattle_tape_get_current_value (tape) == 0);

	g_object_unref (tape);
}

/**
 * test_tape_decrease_current_value:
 *
 * Decrease the current value several times and then increase it by
 * the number of decrease steps taken before.
 */
static void
test_tape_decrease_current_value (void)
{
	CattleTape *tape;
	gint i;

	tape = cattle_tape_new ();

	cattle_tape_increase_current_value_by (tape, 127);
	g_assert (cattle_tape_get_current_value (tape) == 127);

	for (i = 127; i >= 0; i--) {

		g_assert (cattle_tape_get_current_value (tape) == i);
		cattle_tape_decrease_current_value (tape);
	}

	g_assert (cattle_tape_get_current_value (tape) == 127);

	cattle_tape_decrease_current_value_by (tape, 128 * 5);
	g_assert (cattle_tape_get_current_value (tape) == 127);

	g_object_unref (tape);
}

/**
 * test_tape_positive_wrap:
 *
 * Wrap the current value by increasing it several times.
 */
static void
test_tape_positive_wrap (void)
{
	CattleTape *tape;
	gint i;

	tape = cattle_tape_new ();

	cattle_tape_set_current_value (tape, 42);
	g_assert (cattle_tape_get_current_value (tape) == 42);

	for (i = 0; i < 128; i++) {

		g_assert (cattle_tape_get_current_value (tape) >= 0);
		g_assert (cattle_tape_get_current_value (tape) <= 127);
		g_assert (cattle_tape_get_current_value (tape) == mod ((42 + i), 128));

		cattle_tape_increase_current_value (tape);
	}

	g_assert (cattle_tape_get_current_value (tape) == 42);

	g_object_unref (tape);
}

/**
 * test_tape_negative_wrap:
 *
 * Wrap the current value by decreasing it several times.
 */
static void
test_tape_negative_wrap (void)
{
	CattleTape *tape;
	gint i;

	tape = cattle_tape_new ();

	cattle_tape_set_current_value (tape, 42);
	g_assert (cattle_tape_get_current_value (tape) == 42);

	for (i = 0; i < 128; i++) {

		g_assert (cattle_tape_get_current_value (tape) >= 0);
		g_assert (cattle_tape_get_current_value (tape) <= 127);
		g_assert (cattle_tape_get_current_value (tape) == mod ((42 - i), 128));

		cattle_tape_decrease_current_value (tape);
	}

	g_assert (cattle_tape_get_current_value (tape) == 42);

	g_object_unref (tape);
}

/**
 * test_tape_eof_wrap:
 *
 * Check wrapping works as expected when an EOF is involved.
 */
static void
test_tape_eof_wrap (void)
{
	CattleTape *tape;

	tape = cattle_tape_new ();

	cattle_tape_set_current_value (tape, EOF);
	g_assert (cattle_tape_get_current_value (tape) == EOF);

	cattle_tape_set_current_value (tape, EOF);
	cattle_tape_increase_current_value (tape);
	g_assert (cattle_tape_get_current_value (tape) == 0);

	cattle_tape_set_current_value (tape, EOF);
	cattle_tape_increase_current_value_by (tape, 5 + (5 * 128));
	g_assert (cattle_tape_get_current_value (tape) == 4);

	cattle_tape_set_current_value (tape, EOF);
	cattle_tape_decrease_current_value (tape);
	g_assert (cattle_tape_get_current_value (tape) == 127);

	cattle_tape_set_current_value (tape, EOF);
	cattle_tape_decrease_current_value_by (tape, 5 + (5 * 128));
	g_assert (cattle_tape_get_current_value (tape) == 123);

	g_object_unref (tape);
}

gint
main (gint argc, gchar **argv)
{
	g_type_init ();
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/tape/initial-position",
	                 test_tape_initial_position);
	g_test_add_func ("/tape/right-edge",
	                 test_tape_right_edge);
	g_test_add_func ("/tape/left-edge",
	                 test_tape_left_edge);
	g_test_add_func ("/tape/in-between",
	                 test_tape_in_between);
	g_test_add_func ("/tape/move-right-by",
	                 test_tape_move_right_by);
	g_test_add_func ("/tape/move-left-by",
	                 test_tape_move_left_by);
	g_test_add_func ("/tape/current-value",
	                 test_tape_current_value);
	g_test_add_func ("/tape/increase-current-value",
	                 test_tape_increase_current_value);
	g_test_add_func ("/tape/decrease-current-value",
	                 test_tape_decrease_current_value);
	g_test_add_func ("/tape/positive-wrap",
	                 test_tape_positive_wrap);
	g_test_add_func ("/tape/negative-wrap",
	                 test_tape_negative_wrap);
	g_test_add_func ("/tape/eof-wrap",
	                 test_tape_eof_wrap);

	return g_test_run ();
}
