/* interpreter -- Simple Brainfuck interpreter
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

gint
main (gint argc, gchar **argv)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	GError *error;
	gchar *contents;

	g_type_init ();
	g_set_prgname ("run");

	if (argc != 2) {
		g_warning ("Usage: %s FILENAME", argv[0]);
		return 1;
	}

	error = NULL;
	contents = read_file_contents (argv[1], &error);

	if (error != NULL) {

		g_warning ("%s: %s", argv[1], error->message);

		g_error_free (error);

		return 1;
	}

	/* Create a new interpreter */
	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);

	/* Load the program, aborting on failure */
	error = NULL;
	if (!cattle_program_load (program, contents, &error)) {

		g_warning ("Load error: %s", error->message);

		g_error_free (error);
		g_object_unref (program);
		g_object_unref (interpreter);
		g_free (contents);

		return 1;
	}
	g_object_unref (program);

	/* Start the execution */
	error = NULL;
	if (!cattle_interpreter_run (interpreter, &error)) {

		g_warning ("Runtime error: %s", error->message);

		g_error_free (error);
		g_object_unref (interpreter);
		g_free (contents);

		return 1;
	}
	g_object_unref (interpreter);
	g_free (contents);

	return 0;
}
