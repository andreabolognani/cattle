/* references-tests -- Look for reference leaks
 * Copyright (C) 2009  Andrea Bolognani <eof@kiyuko.org>
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

/**
 * test_references_program_owns_instructions:
 *
 * The instructions belonging to a program must be unreferenced when the
 * program is finalized.
 */
static void
test_references_program_owns_instructions (void)
{
    CattleProgram *program;
    CattleInstruction *first;

    program = cattle_program_new ();
    cattle_program_load_from_string (program, "++[-]", NULL);

    first = cattle_program_get_instructions (program);

    g_assert (G_IS_OBJECT (first));

    g_object_unref (program);
    g_object_unref (first);

    g_assert (!G_IS_OBJECT (first));
}

gint
main (gint argc, gchar **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/references/program-owns-instructions",
                     test_references_program_owns_instructions);

    g_test_run ();

    return 0;
}

