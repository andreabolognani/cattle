/* references - Look for reference leaks
 * Copyright (C) 2008-2017  Andrea Bolognani <eof@kiyuko.org>
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
check_refcount (CattleInstruction *instruction)
{
	CattleInstruction *next;

	while (CATTLE_IS_INSTRUCTION (instruction))
	{
		g_assert (!(G_OBJECT (instruction)->ref_count < 2));
		g_assert (!(G_OBJECT (instruction)->ref_count > 2));

		if (cattle_instruction_get_value (instruction) == CATTLE_INSTRUCTION_LOOP_BEGIN)
		{
			next = cattle_instruction_get_loop (instruction);
			check_refcount (next);
			g_object_unref (next);
		}

		next = cattle_instruction_get_next (instruction);
		g_object_unref (instruction);
		instruction = next;
	}
}

/**
 * test_references_single_reference:
 *
 * Make sure there is a single reference to each instruction owned by a
 * program. This is achieved by looking at GObject's private field ref_count,
 * which is discouraged, but makes the check easy to perform.
 */
static void
test_references_single_reference (void)
{
	CattleProgram     *program;
	CattleInstruction *instruction;
	CattleBuffer      *buffer;

	program = cattle_program_new ();

	buffer = cattle_buffer_new (1);
	cattle_buffer_set_contents (buffer, (gint8 *) "+");

	cattle_program_load (program, buffer, NULL);

	instruction = cattle_program_get_instructions (program);
	check_refcount (instruction);

	g_object_unref (buffer);
	g_object_unref (program);
}


gint
main (gint    argc,
      gchar **argv)
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init ();
#endif

	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/references/single-reference",
	                 test_references_single_reference);

	return g_test_run ();
}
