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
input_handler (GObject     *object,
               gchar      **input,
               GError     **error,
               gpointer     data)
{
    CattleInterpreter *self;
    gchar *string;

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
    g_return_val_if_fail (CATTLE_IS_INTERPRETER (object), FALSE);
    self = CATTLE_INTERPRETER (object);

    /* If the pointer is not null, it points to a
     * previously-allocated buffer we need to release */
    if (*input != NULL) {

        g_free (*input);
        *input = NULL;
    }

    /* Create a buffer (hopefully) big enough to hold the next line */
    string = g_new0 (gchar, 256);

    /* Try to read a whole line from standard input */
    if (fgets (string, 256, stdin) == NULL) {

        /* We reached the end of input: we have to let the interpreter
         * know this by returning a NULL pointer */
        if (feof (stdin)) {

            g_free (string);
            *input = NULL;

            return TRUE;
        }

        /* If fgets() returned NULL but we aren't at the end of input, it
         * means an I/O error occurred. In this case, we need to report
         * it to the caller.
         *
         * FIXME G_FILE_ERROR is used as error domain, but it's not really
         *       correct in this case. A generic error domain is probably
         *       needed for situations like this.
         */
        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_IO,
                     "Read error");

        return FALSE;
    }

    *input = string;

    return TRUE;
}

static gboolean
output_handler (GObject     *object,
                gchar        output,
                GError     **error,
                gpointer     data)
{
    CattleInterpreter *self;

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
    g_return_val_if_fail (CATTLE_IS_INTERPRETER (object), FALSE);
    self = CATTLE_INTERPRETER (object);

    /* Just dump the character to stdout */
    g_print ("%c", output);

    return TRUE;
}

static gboolean
debug_handler (GObject     *object,
               GError     **error,
               gpointer     data)
{
    CattleInterpreter *self;
    CattleTape *tape;
    gchar value;
    gint count;

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
    g_return_val_if_fail (CATTLE_IS_INTERPRETER (object), FALSE);
    self = CATTLE_INTERPRETER (object);

    tape = cattle_interpreter_get_tape (self);

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
            g_print ("\\%x", (gint) value);
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
    g_object_unref (G_OBJECT (configuration));

    program = cattle_interpreter_get_program (interpreter);
    g_object_ref (G_OBJECT (program));

    /* Load the program, aborting on failure */
    if (!cattle_program_load_from_file (program, argv[1], &error)) {

        g_warning ("Cannot load program: %s", error->message);

        g_error_free (error);
        g_object_unref (G_OBJECT (program));
        g_object_unref (G_OBJECT (interpreter));

        return 1;
    }
    g_object_unref (G_OBJECT (program));

    /* Connect the input/output and debug signal handlers */
    g_signal_connect (G_OBJECT (interpreter),
                      "input-request",
                      G_CALLBACK (input_handler),
                      NULL);
    g_signal_connect (G_OBJECT (interpreter),
                      "output-request",
                      G_CALLBACK (output_handler),
                      NULL);
    g_signal_connect (G_OBJECT (interpreter),
                      "debug-request",
                      G_CALLBACK (debug_handler),
                      NULL);

    /* Start the execution */
    if (!cattle_interpreter_run (interpreter, &error)) {

        g_warning ("Cannot run program: %s", error->message);

        g_error_free (error);
        g_object_unref (G_OBJECT (interpreter));

        return 1;
    }
    g_object_unref (G_OBJECT (interpreter));

    return 0;
}
