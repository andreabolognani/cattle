/* interpreter -- Simple Brainfuck interpreter
 * Copyright (C) 2008-2009  Andrea Bolognani <eof@kiyuko.org>
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
#include <stdio.h>

static gboolean
output_handler (GObject     *object,
                gchar        output,
                GError     **error,
                gpointer     data)
{
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
    g_return_val_if_fail (CATTLE_IS_INTERPRETER (object), FALSE);

    /* Just dump the character to stdout */
    g_print ("%c", output);

    return TRUE;
}

static gboolean
debug_handler (GObject     *object,
               GError     **error,
               gpointer     data)
{
    CattleInterpreter *interpreter;
    CattleTape *tape;
    gchar value;
    gint count;

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
    g_return_val_if_fail (CATTLE_IS_INTERPRETER (object), FALSE);

    interpreter = CATTLE_INTERPRETER (object);
    g_object_ref (interpreter);

    tape = cattle_interpreter_get_tape (interpreter);

    /* Save the current position */
    cattle_tape_push_bookmark (tape);

    /* Move to the beginning of the tape, counting how many steps it takes
     * to get there. We will use this value later to mark the current
     * position */
    count = 0;
    while (TRUE) {

        if (cattle_tape_is_at_beginning (tape)) {
            break;
        }

        cattle_tape_move_left (tape);
        count++;
    }

    g_print ("[");

    while (TRUE) {

        /* Mark the current position */
        if (count == 0) {
            g_print ("<");
        }

        /* Get the value of the current cell, and print it if it's a
         * graphical character, or its hexadecimal value otherwise */
        value = cattle_tape_get_current_value (tape);

        if (g_ascii_isgraph (value)) {
            g_print ("%c", value);
        }
        else {
            g_print ("\\0x%X", (gint) value);
        }

        /* Mark the current position */
        if (count == 0) {
            g_print (">");
        }

        /* Exit after printing the last value */
        if (cattle_tape_is_at_end (tape)) {
            break;
        }

        /* Print a space and move to the right */
        g_print (" ");
        cattle_tape_move_right (tape);
        count--;
    }

    g_print ("]\n");

    /* Restore the previously-saved position */
    cattle_tape_pop_bookmark (tape);

    g_object_unref (tape);
    g_object_unref (interpreter);

    return TRUE;
}

gint
main (gint argc, gchar **argv)
{
    CattleInterpreter *interpreter;
    CattleProgram *program;
    CattleConfiguration *configuration;
    GError *error = NULL;

    g_type_init ();
    g_set_prgname ("interpreter");

    if (argc != 2) {
        g_warning ("Usage: %s FILENAME", argv[0]);
        return 1;
    }

    /* Create a new interpreter */
    interpreter = cattle_interpreter_new ();

    configuration = cattle_interpreter_get_configuration (interpreter);
    cattle_configuration_set_debug_is_enabled (configuration, TRUE);
    g_object_unref (configuration);

    program = cattle_interpreter_get_program (interpreter);

    /* Load the program, aborting on failure */
    if (!cattle_program_load_from_file (program, argv[1], &error)) {

        g_warning ("Cannot load program: %s", error->message);

        g_error_free (error);
        g_object_unref (program);
        g_object_unref (interpreter);

        return 1;
    }
    g_object_unref (program);

    /* Connect the input/output and debug signal handlers */
    g_signal_connect (interpreter,
                      "output-request",
                      G_CALLBACK (output_handler),
                      NULL);
    g_signal_connect (interpreter,
                      "debug-request",
                      G_CALLBACK (debug_handler),
                      NULL);

    /* Start the execution */
    if (!cattle_interpreter_run (interpreter, &error)) {

        g_warning ("Cannot run program: %s", error->message);

        g_error_free (error);
        g_object_unref (interpreter);

        return 1;
    }

    g_object_unref (interpreter);

    return 0;
}
