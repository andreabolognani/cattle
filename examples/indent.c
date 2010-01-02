/* indent -- Indent a Brainfuck program
 * Copyright (C) 2008-2010  Andrea Bolognani <eof@kiyuko.org>
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

/* Each time a loop is started, its content is indented by
 * INDENT_STEP blank spaces */
#define INDENT_STEP 4

static void
indent_real (CattleInstruction *instruction, gint level)
{
    CattleInstruction *temp;
    CattleInstructionValue value;
    gint quantity;
    gint i;

    g_return_if_fail (CATTLE_IS_INSTRUCTION (instruction));
    g_object_ref (instruction);

    do {

        value = cattle_instruction_get_value (instruction);
        quantity = cattle_instruction_get_quantity (instruction);

        /* The closing bracket must be exactly below the opening one, so we
         * have to decrease the indent level here */
        if (value == CATTLE_INSTRUCTION_LOOP_END) {
            level -= 1;
        }

        /* Output the whitespace */
        for (i = 0; i < level * INDENT_STEP; i++) {
            g_print (" ");
        }

        /* Output the instruction, as many times as needed */
        for (i = 0; i < quantity; i++) {
            g_print ("%c", value);
        }
        g_print ("\n");

        /* Recurse to process the inner loop */
        if (value == CATTLE_INSTRUCTION_LOOP_BEGIN) {
            temp = cattle_instruction_get_loop (instruction);
            indent_real (temp, level + 1);
            g_object_unref (temp);
        }

        /* Drop the reference to the current instruction */
        temp = cattle_instruction_get_next (instruction);
        g_object_unref (instruction);

        instruction = temp;
    }
    while (CATTLE_IS_INSTRUCTION (instruction));
}

void
indent (CattleProgram *program)
{
    CattleInstruction *instruction;

    g_return_if_fail (CATTLE_IS_PROGRAM (program));

    /* Get the first instruction and start indenting */
    instruction = cattle_program_get_instructions (program);
    indent_real (instruction, 0);
    g_object_unref (instruction);
}

gint
main (gint argc, gchar **argv)
{
    CattleProgram *program;
    GError *error = NULL;

    g_type_init ();
    g_set_prgname ("indent");

    if (argc != 2) {
        g_warning ("Usage: %s FILENAME", argv[0]);
        return 1;
    }

    /* Create a new program */
    program = cattle_program_new ();

    /* Load the program from file, aborting on error */
    if (!cattle_program_load_from_file (program, argv[1], &error)) {

        g_warning ("Load error: %s", error->message);

        g_error_free (error);
        g_object_unref (program);

        return 1;
    }

    /* Indent the program */
    indent (program);

    g_object_unref (program); 

    return 0;
}
