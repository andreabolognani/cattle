/* program-tests -- Tests related to program loading
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

#define STEPS 5

static void
program_create (CattleProgram   **program,
                gconstpointer     data)
{
    *program = cattle_program_new ();
}

static void
program_destroy (CattleProgram   **program,
                 gconstpointer     data)
{
    g_object_unref (*program);
}

/**
 * test_program_load_empty:
 *
 * Make sure an empty program can be loaded, and that the resulting program
 * consists of a single instruction with value CATTLE_INSTRUCTION_NONE.
 */
static void
test_program_load_empty (CattleProgram   **program,
                         gconstpointer     data)
{
    CattleInstruction *instruction;
    CattleInstructionValue value;
    GError *error = NULL;
    gboolean result;

    result = cattle_program_load_from_string (*program, "", &error);

    g_assert (result);
    g_assert (error == NULL);

    instruction = cattle_program_get_instructions (*program);

    g_assert (instruction != NULL);
    g_assert (cattle_instruction_get_next (instruction) == NULL);
    g_assert (cattle_instruction_get_loop (instruction) == NULL);

    value = cattle_instruction_get_value (instruction);

    g_assert (value == CATTLE_INSTRUCTION_NONE);
}

gint
main (gint argc, gchar **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

    g_test_add ("/program/load-empty",
                CattleProgram*,
                NULL,
                program_create,
                test_program_load_empty,
                program_destroy);

    g_test_run ();

    return 0;
}
