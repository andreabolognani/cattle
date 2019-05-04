/* minimize - Strip all comments from a Brainfuck program
 * Copyright (C) 2008-2020  Andrea Bolognani <eof@kiyuko.org>
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
 * with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Homepage: https://kiyuko.org/software/cattle
 */

#include <glib.h>
#include <glib-object.h>
#include <cattle/cattle.h>
#include "common.h"

#define WIDTH 75 /* Line lenght */

static void
minimize (CattleProgram *program)
{
    CattleInstruction        *first;
    CattleInstruction        *current;
    CattleInstruction        *next;
    g_autoptr (CattleBuffer)  input = NULL;
    g_autoptr (GSList)        stack = NULL;
    gchar                     value;
    gulong                    quantity;
    gulong                    position;
    gulong                    size;
    gulong                    i;

    stack = NULL;
    position = 0;

    first = cattle_program_get_instructions (program);
    g_object_ref (first);

    current = first;

    while (current != NULL)
    {
        value = cattle_instruction_get_value (current);
        quantity = cattle_instruction_get_quantity (current);

        for (i = 0; i < quantity; i++) {

            /* When position is equal to WIDTH, print a newline
             * and reset it */
            if (position >= WIDTH)
            {
                g_print ("\n");
                position = 0;
            }

            g_print ("%c", value);

            position++;
        }

        if (value == CATTLE_INSTRUCTION_LOOP_BEGIN)
        {
            /* Get the first instruction after the loop and push it
             * on top of the stack */
            next = cattle_instruction_get_next (current);
            stack = g_slist_prepend (stack, next);

            /* Go on printing the loop */
            next = cattle_instruction_get_loop (current);
            g_object_unref (current);
            current = next;
        }
        else if (value == CATTLE_INSTRUCTION_LOOP_END)
        {
            g_assert (stack != NULL);

            /* Pop the next instruction off the stack */
            next = CATTLE_INSTRUCTION (stack->data);
            stack = g_slist_delete_link (stack, stack);
            g_object_unref (current);
            current = next;
        }
        else
        {
            /* Go straight to the next instruction */
            next = cattle_instruction_get_next (current);
            g_object_unref (current);
            current = next;
        }
    }

    /* Print an ending newline only if one hasn't just been printed */
    if (position > 0)
    {
        g_print ("\n");
    }

    g_object_unref (first);

    input = cattle_program_get_input (program);
    size = cattle_buffer_get_size (input);

    /* Print program's input if available */
    if (size > 0)
    {
        g_print ("!");

        for (i = 0; i < size; i++)
        {
            value = cattle_buffer_get_value (input, i);

            g_print ("%c", value);
        }
    }
}

gint
main (gint    argc,
      gchar **argv)
{
    g_autoptr (CattleProgram) program = NULL;
    g_autoptr (CattleBuffer)  buffer = NULL;
    g_autoptr (GError)        error = NULL;

    g_set_prgname ("minimize");

    if (argc != 2)
    {
        g_warning ("Usage: %s FILENAME", argv[0]);

        return 1;
    }

    /* Read file contents */
    error = NULL;
    buffer = read_file_contents (argv[1], &error);

    if (error != NULL)
    {
        g_warning ("%s: %s", argv[1], error->message);

        return 1;
    }

    /* Load program */
    program = cattle_program_new ();

    error = NULL;
    if (!cattle_program_load (program, buffer, &error))
    {
        g_warning ("Load error: %s", error->message);

        return 1;
    }

    /* Run minimization */
    minimize (program);

    return 0;
}
