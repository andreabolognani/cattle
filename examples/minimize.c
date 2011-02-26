/* minimize -- Strip all comments from a Brainfuck program
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

#define WIDTH 75 /* Line lenght */

static void
minimize (CattleProgram *program)
{
	CattleInstruction *first;
	CattleInstruction *current;
	CattleInstruction *next;
	GSList *stack;
	gchar value;
	gint quantity;
	gint position;
	gint i;

	stack = NULL;
	position = 0;

	first = cattle_program_get_instructions (program);
	g_object_ref (first);

	current = first;

	while (current != NULL) {

		value = cattle_instruction_get_value (current);
		quantity = cattle_instruction_get_quantity (current);

		for (i = 0; i < quantity; i++) {

			/* When position is equal to WIDTH, print a newline
			 * and reset it */
			if (position >= WIDTH) {
				g_print ("\n");
				position = 0;
			}

			g_print ("%c", value);

			position++;
		}

		if (value == CATTLE_INSTRUCTION_LOOP_BEGIN) {

			/* Get the first instruction after the loop and push it
			 * on top of the stack */
			next = cattle_instruction_get_next (current);
			stack = g_slist_prepend (stack, next);

			/* Go on printing the loop */
			next = cattle_instruction_get_loop (current);
			g_object_unref (current);
			current = next;
		}
		else if (value == CATTLE_INSTRUCTION_LOOP_END) {

			g_assert (stack != NULL);

			/* Pop the next instruction off the stack */
			next = CATTLE_INSTRUCTION (stack->data);
			stack = g_slist_delete_link (stack, stack);
			g_object_unref (current);
			current = next;
		}
		else {

			/* Go straight to the next instruction */
			next = cattle_instruction_get_next (current);
			g_object_unref (current);
			current = next;
		}
	}

	/* Print an ending newline only if one hasn't just been printed */
	if (position > 0) {
		g_print ("\n");
	}

	g_object_unref (first);
}

gint
main (gint argc, char **argv)
{
	CattleProgram *program;
	GError *error;
	gchar *contents;

	g_type_init ();
	g_set_prgname ("minimize");

	if (argc != 2) {
		g_warning ("Usage: %s FILENAME", argv[0]);
		return 1;
	}

	/* Read file contents */
	error = NULL;
	contents = read_file_contents (argv[1], &error);

	if (contents == NULL) {

		g_warning ("%s: %s", argv[1], error->message);

		g_error_free (error);

		return 1;
	}

	/* Load program */
	program = cattle_program_new ();

	error = NULL;
	if (!cattle_program_load (program, contents, &error)) {

		g_warning ("Load error: %s", error->message);

		g_error_free (error);
		g_object_unref (program);
		g_free (contents);

		return 1;
	}

	/* Run minimization */
	minimize (program);

	g_free (contents);
	g_object_unref (program);

	return 0;
}
