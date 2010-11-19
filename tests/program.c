/* program -- Tests related to program loading
 * Copyright (C) 2009-2010  Andrea Bolognani <eof@kiyuko.org>
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

static void
program_create (CattleProgram **program,
                gconstpointer   data)
{
	*program = cattle_program_new ();
}

static void
program_destroy (CattleProgram **program,
                 gconstpointer   data)
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
test_program_load_empty (CattleProgram **program,
                         gconstpointer   data)
{
	CattleInstruction *instruction;
	CattleInstructionValue value;
	GError *error = NULL;
	gboolean success;

	success = cattle_program_load_from_string (*program, "", &error);

	g_assert (success);
	g_assert (error == NULL);

	instruction = cattle_program_get_instructions (*program);

	g_assert (CATTLE_IS_INSTRUCTION (instruction));
	g_assert (cattle_instruction_get_next (instruction) == NULL);
	g_assert (cattle_instruction_get_loop (instruction) == NULL);

	value = cattle_instruction_get_value (instruction);

	g_assert (value == CATTLE_INSTRUCTION_NONE);

	g_object_unref (instruction);
}

/**
 * test_program_load_unbalanced_brackets:
 *
 * Make sure a program containing unbalanced brackets is not loaded, and that
 * the correct error is reported.
 */
static void
test_program_load_unbalanced_brackets (CattleProgram **program,
                                       gconstpointer   data)
{
	CattleInstruction *instruction;
	CattleInstructionValue value;
	GError *error = NULL;
	gboolean success;

	success = cattle_program_load_from_string (*program, "[", &error);

	g_assert (!success);
	g_assert (error != NULL);
	g_assert (error->domain == CATTLE_PROGRAM_ERROR);
	g_assert (error->code == CATTLE_PROGRAM_ERROR_UNBALANCED_BRACKETS);

	instruction = cattle_program_get_instructions (*program);

	g_assert (CATTLE_IS_INSTRUCTION (instruction));
	g_assert (cattle_instruction_get_next (instruction) == NULL);
	g_assert (cattle_instruction_get_loop (instruction) == NULL);

	value = cattle_instruction_get_value (instruction);

	g_assert (value == CATTLE_INSTRUCTION_NONE);

	g_object_unref (instruction);
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
	g_test_add ("/program/load-unbalanced-brackets",
	            CattleProgram*,
	            NULL,
	            program_create,
	            test_program_load_unbalanced_brackets,
	            program_destroy);

	return g_test_run ();
}
