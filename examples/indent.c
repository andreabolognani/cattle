/* indent -- Indent a Brainfuck program
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
#include "common.h"

static void
indent (CattleProgram *program)
{
	CattleInstruction *first;
	CattleInstruction *current;
	CattleInstruction *next;
	CattleInstructionValue value;
	GSList *stack;
	gint level;
	gint quantity;
	gint i;

	/* Initialize instruction stack, start at indentation level 0 */
	stack = NULL;
	level = 0;

	first = cattle_program_get_instructions (program);
	g_object_ref (first);
	current = first;

	while (current != NULL) {

		value = cattle_instruction_get_value (current);
		quantity = cattle_instruction_get_quantity (current);

		/* Decrease the indentation level at the end of a loop */
		if (value == CATTLE_INSTRUCTION_LOOP_END) {
			level--;
		}
	
		/* Print tabs for indentation */
		for (i = 0; i < level; i++) {
			g_print ("\t");
		}

		/* Print the correct number of instructions */
		for (i = 0; i < quantity; i++) {
			g_print ("%c", value);
		}

		/* End the line */
		g_print ("\n");

		/* Increase the indentation level at the beginning of a loop */
		if (value == CATTLE_INSTRUCTION_LOOP_BEGIN) {
			level++;
		}

		switch (value) {

			case CATTLE_INSTRUCTION_LOOP_BEGIN:

				/* Push the next instruction on top of the stack */
				next = cattle_instruction_get_next (current);
				stack = g_slist_prepend (stack, next);

				/* Start indenting the loop */
				next = cattle_instruction_get_loop (current);
				g_object_unref (current);
				current = next;

				break;

			case CATTLE_INSTRUCTION_LOOP_END:

				g_assert (stack != NULL);

				/* Pop the next instruction off the stack */
				next = CATTLE_INSTRUCTION (stack->data);
				stack = g_slist_delete_link (stack, stack);
				g_object_unref (current);
				current = next;

				break;

			default:

				/* Go on with the next instruction */
				next = cattle_instruction_get_next (current);
				g_object_unref (current);
				current = next;

				break;
		}
	}

	g_object_unref (first);
}

gint
main (gint argc, gchar **argv)
{
	CattleProgram *program;
	GError *error;
	gchar *contents;

	g_type_init ();
	g_set_prgname ("indent");

	if (argc != 2) {
		g_warning ("Usage: %s FILENAME", argv[0]);
		return 1;
	}

	error = NULL;
	contents = read_file_contents (argv[1], &error);

	if (!contents) {

		g_warning ("%s: %s", argv[1], error->message);

		g_error_free (error);

		return 1;
	}

	/* Create a new program */
	program = cattle_program_new ();

	/* Load the program from file, aborting on error */
	error = NULL;
	if (!cattle_program_load (program, contents, &error)) {

		g_warning ("Load error: %s", error->message);

		g_error_free (error);
		g_object_unref (program);
		g_free (contents);

		return 1;
	}

	/* Indent the program */
	indent (program);

	g_free (contents);
	g_object_unref (program); 

	return 0;
}
