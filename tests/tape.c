/* tape - Check the tape reports the correct size limits
 * Copyright (C) 2008-2020  Andrea Bolognani <eof@kiyuko.org>
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
 * with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Homepage: https://kiyuko.org/software/cattle
 */

#include <glib.h>
#include <glib-object.h>
#include <cattle/cattle.h>

#define STEPS 1024

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
    g_autoptr (CattleTape) tape = NULL;

    tape = cattle_tape_new ();

    g_assert (cattle_tape_is_at_beginning (tape));
    g_assert (cattle_tape_is_at_end (tape));
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
    g_autoptr (CattleTape) tape = NULL;
    gint                   i;

    tape = cattle_tape_new ();

    for (i = 0; i < STEPS; i++) {

        cattle_tape_move_right (tape);

        g_assert (!cattle_tape_is_at_beginning (tape));
        g_assert (cattle_tape_is_at_end (tape));
    }
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
    g_autoptr (CattleTape) tape = NULL;
    gint                   i;

    tape = cattle_tape_new ();

    for (i = 0; i < STEPS; i++) {

        cattle_tape_move_left (tape);

        g_assert (cattle_tape_is_at_beginning (tape));
        g_assert (!cattle_tape_is_at_end (tape));
    }
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
    g_autoptr (CattleTape) tape = NULL;
    gint                   i;

    tape = cattle_tape_new ();

    for (i = 0; i < STEPS; i++) {

        cattle_tape_move_left (tape);
    }

    for (i = 0; i < (STEPS - 1); i++) {

        cattle_tape_move_right (tape);

        g_assert (!cattle_tape_is_at_beginning (tape));
        g_assert (!cattle_tape_is_at_end (tape));
    }
}

/**
 * test_tape_move_right:
 *
 * Ensure moving right a certain number of steps one of the time or with
 * a single call yields the same result.
 */
static void
test_tape_move_right (void)
{
    g_autoptr (CattleTape) tape = NULL;
    gint                   i;

    tape = cattle_tape_new ();

    /* Mark the initial position */
    cattle_tape_set_current_value (tape, G_MININT8);

    /* Move right */
    for (i = 1; i <= STEPS; i++)
    {
        cattle_tape_move_right (tape);
    }

    /* Mark the final position */
    cattle_tape_set_current_value (tape, G_MAXINT8);

    /* Move left by looking for the initial value */
    while (cattle_tape_get_current_value (tape) != G_MININT8)
    {
        cattle_tape_move_left (tape);
    }
    g_assert (cattle_tape_get_current_value (tape) == G_MININT8);

    /* Move right in a single leap */
    cattle_tape_move_right_by (tape, STEPS);
    g_assert (cattle_tape_get_current_value (tape) == G_MAXINT8);
}

/**
 * test_tape_move_left:
 *
 * Ensure moving left a certain number of steps one of the time or with
 * a single call yields the same result.
 */
static void
test_tape_move_left (void)
{
    g_autoptr (CattleTape) tape = NULL;
    gint                   i;

    tape = cattle_tape_new ();

    /* Mark the initial position */
    cattle_tape_set_current_value (tape, G_MININT8);

    /* Move left */
    for (i = 1; i <= STEPS; i++)
    {
        cattle_tape_move_left (tape);
    }

    /* Mark the final position */
    cattle_tape_set_current_value (tape, G_MAXINT8);

    /* Move right by looking for the initial value */
    while (cattle_tape_get_current_value (tape) != G_MININT8)
    {
        cattle_tape_move_right (tape);
    }
    g_assert (cattle_tape_get_current_value (tape) == G_MININT8);

    /* Move left in a single leap */
    cattle_tape_move_left_by (tape, STEPS);
    g_assert (cattle_tape_get_current_value (tape) == G_MAXINT8);
}

/**
 * test_tape_bookmarks:
 *
 * Ensure bookmarks work as intended.
 */
static void
test_tape_bookmarks (void)
{
    g_autoptr (CattleTape) tape = NULL;

    tape = cattle_tape_new ();

    /* Move left a few steps */
    cattle_tape_move_left_by (tape, 20);
    g_assert (cattle_tape_get_current_value (tape) == 0);

    /* Set a value and create a bookmark */
    cattle_tape_set_current_value (tape, 42);
    cattle_tape_push_bookmark (tape);

    /* Move right by a few steps */
    cattle_tape_move_right_by (tape, 70);
    g_assert (cattle_tape_get_current_value (tape) == 0);

    /* Use the bookmark to return to the saved position */
    cattle_tape_pop_bookmark (tape);
    g_assert (cattle_tape_get_current_value (tape) == 42);
}

/**
 * test_tape_current_value:
 *
 * Set and get the current value several times, covering the whole
 * possible range.
 */
static void
test_tape_current_value (void)
{
    g_autoptr (CattleTape) tape = NULL;
    gint                   i;

    tape = cattle_tape_new ();

    for (i = G_MININT8; i <= G_MAXINT8; i++)
    {
        cattle_tape_set_current_value (tape, i);
        g_assert (cattle_tape_get_current_value (tape) == i);
    }
}

/**
 * test_tape_increase_current_value:
 *
 * Ensure increasing the current value by one each time or in a
 * single step yields the same result.
 */
static void
test_tape_increase_current_value (void)
{
    g_autoptr (CattleTape) tape = NULL;
    gint                   i;

    tape = cattle_tape_new ();

    /* Set the initial value */
    cattle_tape_set_current_value (tape, 12);
    g_assert (cattle_tape_get_current_value (tape) == 12);

    /* Increase the value by one each iteration */
    for (i = 0; i < 30; i++)
    {
        cattle_tape_increase_current_value (tape);
    }
    g_assert (cattle_tape_get_current_value (tape) == 42);

    /* Set the initial value again */
    cattle_tape_set_current_value (tape, 12);
    g_assert (cattle_tape_get_current_value (tape) == 12);

    /* Increase the value by the number of iterations */
    cattle_tape_increase_current_value_by (tape, 30);
    g_assert (cattle_tape_get_current_value (tape) == 42);
}

/**
 * test_tape_decrease_current_value:
 *
 * Ensure decreasing the current value by one each time or in a
 * single step yields the same result.
 */
static void
test_tape_decrease_current_value (void)
{
    g_autoptr (CattleTape) tape = NULL;
    gint                   i;

    tape = cattle_tape_new ();

    /* Set the initial value */
    cattle_tape_set_current_value (tape, 42);
    g_assert (cattle_tape_get_current_value (tape) == 42);

    /* Decrease the value by one each iteration */
    for (i = 0; i < 30; i++)
    {
        cattle_tape_decrease_current_value (tape);
    }
    g_assert (cattle_tape_get_current_value (tape) == 12);

    /* Set the initial value again */
    cattle_tape_set_current_value (tape, 42);
    g_assert (cattle_tape_get_current_value (tape) == 42);

    /* Decrease the value by the number of iterations */
    cattle_tape_decrease_current_value_by (tape, 30);
    g_assert (cattle_tape_get_current_value (tape) == 12);
}

/**
 * test_tape_positive_wrap:
 *
 * Wrap the current value by increasing it several times.
 */
static void
test_tape_positive_wrap (void)
{
    g_autoptr (CattleTape) tape = NULL;
    gint                   range = G_MAXINT8 - G_MININT8 + 1;

    tape = cattle_tape_new ();

    /* Set the initial value */
    cattle_tape_set_current_value (tape, 42);
    g_assert (cattle_tape_get_current_value (tape) == 42);

    /* Increase the value by three times the whole allowed range */
    cattle_tape_increase_current_value_by (tape, 3 * range);
    g_assert (cattle_tape_get_current_value (tape) == 42);

    /* Increase the value by the whole range in two steps */
    cattle_tape_increase_current_value_by (tape, 100);
    g_assert (cattle_tape_get_current_value (tape) < 0);
    cattle_tape_increase_current_value_by (tape, range - 100);
    g_assert (cattle_tape_get_current_value (tape) == 42);
}

/**
 * test_tape_negative_wrap:
 *
 * Wrap the current value by decreasing it several times.
 */
static void
test_tape_negative_wrap (void)
{
    g_autoptr (CattleTape) tape = NULL;
    gint                   range = G_MAXINT8 - G_MININT8 + 1;

    tape = cattle_tape_new ();

    /* Set the initial value */
    cattle_tape_set_current_value (tape, 42);
    g_assert (cattle_tape_get_current_value (tape) == 42);

    /* Decrease the value by three times the whole allowed range */
    cattle_tape_decrease_current_value_by (tape, 3 * range);
    g_assert (cattle_tape_get_current_value (tape) == 42);

    /* Decrease the value by the whole range in two steps */
    cattle_tape_decrease_current_value_by (tape, 100);
    g_assert (cattle_tape_get_current_value (tape) < 0);
    cattle_tape_decrease_current_value_by (tape, range - 100);
    g_assert (cattle_tape_get_current_value (tape) == 42);
}

gint
main (gint argc, gchar **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/tape/initial-position",
                     test_tape_initial_position);
    g_test_add_func ("/tape/right-edge",
                     test_tape_right_edge);
    g_test_add_func ("/tape/left-edge",
                     test_tape_left_edge);
    g_test_add_func ("/tape/in-between",
                     test_tape_in_between);
    g_test_add_func ("/tape/move-right",
                     test_tape_move_right);
    g_test_add_func ("/tape/move-left",
                     test_tape_move_left);
    g_test_add_func ("/tape/bookmarks",
                     test_tape_bookmarks);
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

    return g_test_run ();
}
