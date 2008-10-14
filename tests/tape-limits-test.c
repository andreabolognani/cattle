/* tape-limits-test -- Check the tape reports the correct size limits
 * Copyright (C) 2008  Andrea Bolognani <eof@kiyuko.org>
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
 * Homepage: http://www.kiyuko.org/software/cattle
 */

#include <glib.h>
#include <glib-object.h>
#include <cattle/cattle.h>

#define SIZE 5

gint
main (gint argc, gchar **argv)
{
    CattleTape *tape;
    gint i;

    g_type_init ();
    g_set_prgname ("tape-limits-test");

    tape = cattle_tape_new ();

	g_assert (cattle_tape_is_at_beginning (tape));
	g_assert (cattle_tape_is_at_end (tape));

    for (i = 0; i < SIZE; i++) {

        cattle_tape_move_right (tape);

		g_assert (!cattle_tape_is_at_beginning (tape));
		g_assert (cattle_tape_is_at_end (tape));
    } 

	for (i = 0; i < (SIZE - 1); i++) {

		cattle_tape_move_left (tape);

		g_assert (!cattle_tape_is_at_beginning (tape));
		g_assert (!cattle_tape_is_at_end (tape));
	}

	cattle_tape_move_left (tape);

	g_assert (cattle_tape_is_at_beginning (tape));
	g_assert (!cattle_tape_is_at_end (tape));

	for (i = 0; i < SIZE; i++) {

		cattle_tape_move_left (tape);

		g_assert (cattle_tape_is_at_beginning (tape));
		g_assert (!cattle_tape_is_at_end (tape));
	}

    g_object_unref (tape);

    return 0;
}
