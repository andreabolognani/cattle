/* tape-tests -- Check the tape reports the correct size limits
 * Copyright (C) 2008-2009  Andrea Bolognani <eof@kiyuko.org>
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

#define STEPS 5

static void
tape_create (CattleTape      **tape,
             gconstpointer     data)
{
    *tape = cattle_tape_new ();
}

static void
tape_destroy (CattleTape      **tape,
              gconstpointer     data)
{
    g_object_unref (*tape);
}

/**
 * test_tape_initial_position:
 *
 * Check the initial position is reported correctly: a newly-created tape
 * is made of a single cell, so the current position is both at the
 * beginning and at the end.
 */
static void
test_tape_initial_position (CattleTape      **tape,
                            gconstpointer     data)
{
    g_assert (cattle_tape_is_at_beginning (*tape));
    g_assert (cattle_tape_is_at_end (*tape));
}

/**
  * test_tape_right_edge:
  *
  * Starting from the initial position and moving only to the right, the
  * current position must always be at the end and never at the beginning.
  */
static void
test_tape_right_edge (CattleTape      **tape,
                      gconstpointer     data)
{
    gint i;

    for (i = 0; i < STEPS; i++) {

        cattle_tape_move_right (*tape);

        g_assert (!cattle_tape_is_at_beginning (*tape));
        g_assert (cattle_tape_is_at_end (*tape));
    }
}

/**
  * test_tape_left_edge:
  *
  * Starting from the initial position and moving only to the left, the
  * current position must always be at the beginning and never at the end.
  */
static void
test_tape_left_edge (CattleTape      **tape,
                     gconstpointer     data)
{
    gint i;

    for (i = 0; i < STEPS; i++) {

        cattle_tape_move_left (*tape);

        g_assert (cattle_tape_is_at_beginning (*tape));
        g_assert (!cattle_tape_is_at_end (*tape));
    }
}

/**
  * test_tape_in_between:
  *
  * If the curren position is somewhere in between the tape, it should not
  * be reported to be at the beginning or at the end.
  */
static void
test_tape_in_between (CattleTape      **tape,
                      gconstpointer     data)
{
    gint i;

    for (i = 0; i < STEPS; i++) {

        cattle_tape_move_left (*tape);
    }

    for (i = 0; i < (STEPS - 1); i++) {

        cattle_tape_move_right (*tape);

        g_assert (!cattle_tape_is_at_beginning (*tape));
        g_assert (!cattle_tape_is_at_end (*tape));
    }
}

gint
main (gint argc, gchar **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/tape/initial-position",
                CattleTape*,
                NULL,
                tape_create,
                test_tape_initial_position,
                tape_destroy);

    g_test_add ("/tape/right-edge",
                CattleTape*,
                NULL,
                tape_create,
                test_tape_right_edge,
                tape_destroy);

    g_test_add ("/tape/left-edge",
                CattleTape*,
                NULL,
                tape_create,
                test_tape_left_edge,
                tape_destroy);

    g_test_add ("/tape/in-between",
                CattleTape*,
                NULL,
                tape_create,
                test_tape_in_between,
                tape_destroy);

    return g_test_run ();
}
