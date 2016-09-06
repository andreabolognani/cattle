/* program - Tests related to program loading
 * Copyright (C) 2008-2016  Andrea Bolognani <eof@kiyuko.org>
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

/**
 * test_program_load_unbalanced_brackets:
 *
 * Make sure a program containing unbalanced brackets is not loaded,
 * and that the correct error is reported.
 */
static void
test_program_load_unbalanced_brackets (void)
{
	CattleProgram          *program;
	CattleBuffer           *buffer;
	CattleInstruction      *instruction;
	CattleInstructionValue  value;
	GError                 *error;
	gboolean                success;

	program = cattle_program_new ();

	buffer = cattle_buffer_new (2);
	cattle_buffer_set_contents (buffer, (gint8 *) "[");

	error = NULL;
	success = cattle_program_load (program, buffer, &error);

	g_assert (!success);
	g_assert (error != NULL);
	g_assert (error->domain == CATTLE_ERROR);
	g_assert (error->code == CATTLE_ERROR_UNBALANCED_BRACKETS);

	instruction = cattle_program_get_instructions (program);

	g_assert (CATTLE_IS_INSTRUCTION (instruction));
	g_assert (cattle_instruction_get_next (instruction) == NULL);
	g_assert (cattle_instruction_get_loop (instruction) == NULL);

	value = cattle_instruction_get_value (instruction);

	g_assert (value == CATTLE_INSTRUCTION_NONE);

	g_object_unref (instruction);
	g_object_unref (buffer);
	g_object_unref (program);
}

/**
 * test_program_load_empty:
 *
 * Make sure an empty program can be loaded, and that the resulting
 * program consists of a single instruction with value
 * CATTLE_INSTRUCTION_NONE.
 */
static void
test_program_load_empty (void)
{
	CattleProgram          *program;
	CattleBuffer           *buffer;
	CattleInstruction      *instruction;
	CattleInstructionValue  value;
	GError                 *error;
	gboolean                success;

	program = cattle_program_new ();

	buffer = cattle_buffer_new (0);

	error = NULL;
	success = cattle_program_load (program, buffer, &error);

	g_assert (success);
	g_assert (error == NULL);

	instruction = cattle_program_get_instructions (program);

	g_assert (CATTLE_IS_INSTRUCTION (instruction));
	g_assert (cattle_instruction_get_next (instruction) == NULL);
	g_assert (cattle_instruction_get_loop (instruction) == NULL);

	value = cattle_instruction_get_value (instruction);

	g_assert (value == CATTLE_INSTRUCTION_NONE);

	g_object_unref (instruction);
	g_object_unref (buffer);
	g_object_unref (program);
}

/**
 * test_program_load_without_input:
 *
 * Load a program with no input.
 */
static void
test_program_load_without_input (void)
{
	CattleProgram          *program;
	CattleBuffer           *buffer;
	CattleInstruction      *instructions;
	CattleBuffer           *input;
	CattleInstructionValue  value;
	GError                 *error;
	gulong                  quantity;
	gboolean                success;

	program = cattle_program_new ();

	buffer = cattle_buffer_new (9);
	cattle_buffer_set_contents (buffer, (gint8 *) "+++>-<[-]");

	error = NULL;
	success = cattle_program_load (program, buffer, &error);

	g_assert (success);
	g_assert (error == NULL);

	instructions = cattle_program_get_instructions (program);
	input = cattle_program_get_input (program);

	g_assert (instructions != NULL);
	g_assert (input != NULL);

	value = cattle_instruction_get_value (instructions);
	quantity = cattle_instruction_get_quantity (instructions);

	g_assert (value == CATTLE_INSTRUCTION_INCREASE);
	g_assert (quantity == 3);

	g_assert (cattle_buffer_get_size (input) == 0);

	g_object_unref (input);
	g_object_unref (instructions);
	g_object_unref (buffer);
	g_object_unref (program);
}

/**
 * test_program_load_with_input:
 *
 * Load a program which containst some input along with the code.
 */
static void
test_program_load_with_input (void)
{
	CattleProgram          *program;
	CattleBuffer           *buffer;
	CattleInstruction      *instructions;
	CattleBuffer           *input;
	CattleInstructionValue  value;
	gulong                  i;
	GError                 *error;
	gboolean                success;

	program = cattle_program_new ();

	buffer = cattle_buffer_new (17);
	cattle_buffer_set_contents (buffer, (gint8 *) ",[+.,]!some input");

	error = NULL;
	success = cattle_program_load (program, buffer, &error);

	g_assert (success);
	g_assert (error == NULL);

	instructions = cattle_program_get_instructions (program);
	input = cattle_program_get_input (program);

	g_assert (instructions != NULL);
	g_assert (input != NULL);

	/* Create a new buffer containing just the input,
	 * for comparison's purposes */
	g_object_unref (buffer);
	buffer = cattle_buffer_new (10);
	cattle_buffer_set_contents (buffer, (gint8 *) "some input");

	/* Match the parsed input with the expected one */
	for (i = 0; i < 10; i++)
	{
		value = cattle_buffer_get_value (input, i);
		g_assert (value == cattle_buffer_get_value (buffer, i));
	}

	value = cattle_instruction_get_value (instructions);
	g_assert (value == CATTLE_INSTRUCTION_READ);

	g_object_unref (input);
	g_object_unref (instructions);
	g_object_unref (buffer);
	g_object_unref (program);
}

/**
 * test_program_load_double_loop:
 *
 * Load a program that is nothing but two loops nested.
 */
static void
test_program_load_double_loop (void)
{
	CattleProgram          *program;
	CattleBuffer           *buffer;
	CattleInstruction      *current;
	CattleInstruction      *outer_loop;
	CattleInstruction      *inner_loop;
	CattleInstruction      *next;
	CattleInstructionValue  value;
	GError                 *error;
	gint                    quantity;
	gboolean                success;

	program = cattle_program_new ();

	buffer = cattle_buffer_new (4);
	cattle_buffer_set_contents (buffer, (gint8 *) "[[]]");

	error = NULL;
	success = cattle_program_load (program, buffer, &error);

	g_assert (success);
	g_assert (error == NULL);

	/* First instruction: [ */
	outer_loop = cattle_program_get_instructions (program);
	current = outer_loop;

	g_assert (current != NULL);

	value = cattle_instruction_get_value (current);
	quantity = cattle_instruction_get_quantity (current);

	g_assert (value == CATTLE_INSTRUCTION_LOOP_BEGIN);
	g_assert (quantity == 1);

	/* Enter the outer loop: [ */
	inner_loop = cattle_instruction_get_loop (current);
	current = inner_loop;

	g_assert (current != NULL);

	value = cattle_instruction_get_value (current);
	quantity = cattle_instruction_get_quantity (current);

	g_assert (value == CATTLE_INSTRUCTION_LOOP_BEGIN);
	g_assert (quantity == 1);

	/* Enter the inner loop: ] */
	next = cattle_instruction_get_loop (current);
	current = next;

	g_assert (current != NULL);

	value = cattle_instruction_get_value (current);
	quantity = cattle_instruction_get_quantity (current);

	g_assert (value == CATTLE_INSTRUCTION_LOOP_END);
	g_assert (quantity == 1);

	/* Inner loop is over */
	next = cattle_instruction_get_next (current);
	current = next;

	g_assert (current == NULL);

	/* After the inner loop: ] */
	next = cattle_instruction_get_next (inner_loop);
	g_object_unref (inner_loop);
	current = next;

	g_assert (current != NULL);

	value = cattle_instruction_get_value (current);
	quantity = cattle_instruction_get_quantity (current);

	g_assert (value == CATTLE_INSTRUCTION_LOOP_END);
	g_assert (quantity == 1);

	/* Outer loop is over */
	next = cattle_instruction_get_next (current);
	g_object_unref (current);
	current = next;

	g_assert (current == NULL);

	/* After the outer loop */
	next = cattle_instruction_get_next (outer_loop);
	g_object_unref (outer_loop);
	current = next;

	g_assert (current == NULL);

	g_object_unref (buffer);
	g_object_unref (program);
}

gint
main (gint argc, gchar **argv)
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init ();
#endif

	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/program/load-unbalanced-brackets",
	                 test_program_load_unbalanced_brackets);
	g_test_add_func ("/program/load-empty",
	                 test_program_load_empty);
	g_test_add_func ("/program/load-without-input",
	                 test_program_load_without_input);
	g_test_add_func ("/program/load-with-input",
	                 test_program_load_with_input);
	g_test_add_func ("/program/load-double-loop",
	                 test_program_load_double_loop);

	return g_test_run ();
}
