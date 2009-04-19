/* interpreter-tests -- Tests for the interpreter implementation
 * Copyright (C) 2009  Andrea Bolognani <eof@kiyuko.org>
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
#include <stdlib.h>

static void
interpreter_create (CattleInterpreter   **interpreter,
                    gconstpointer         data)
{
    *interpreter = cattle_interpreter_new ();
}

static void
interpreter_destroy (CattleInterpreter   **interpreter,
                     gconstpointer         data)
{
    g_object_unref (*interpreter);
}

static gboolean
single_handler_input_one (CattleInterpreter    *interpreter,
                          gchar               **input,
                          GError              **error,
                          gpointer              data)
{
    g_print ("single_handler_input_one\n");
    g_set_error (error,
                 CATTLE_PROGRAM_ERROR,
                 CATTLE_PROGRAM_ERROR_BAD_UTF8,
                 "Spurious error");
    *input = NULL;
    return FALSE;
}

static gboolean
single_handler_input_two (CattleInterpreter    *interpreter,
                          gchar               **input,
                          GError              **error,
                          gpointer              data)
{
    g_print ("single_handler_input_two\n");
    *input = NULL;
    return TRUE;
}

static gboolean
single_handler_output_one (CattleInterpreter    *interpreter,
                           gchar                 output,
                           GError              **error,
                           gpointer              data)
{
    g_print ("single_handler_output_one\n");
    g_set_error (error,
                 CATTLE_PROGRAM_ERROR,
                 CATTLE_PROGRAM_ERROR_BAD_UTF8,
                 "Spurious error");
    return FALSE;
}

static gboolean
single_handler_output_two (CattleInterpreter    *interpreter,
                           gchar                 output,
                           GError              **error,
                           gpointer              data)
{
    g_print ("single_handler_output_two\n");
    return TRUE;
}

static gboolean
single_handler_debug_one (CattleInterpreter    *interpreter,
                          GError              **error,
                          gpointer              data)
{
    g_print ("single_handler_debug_one\n");
    g_set_error (error,
                 CATTLE_PROGRAM_ERROR,
                 CATTLE_PROGRAM_ERROR_BAD_UTF8,
                 "Spurious error");
    return FALSE;
}

static gboolean
single_handler_debug_two (CattleInterpreter    *interpreter,
                          GError              **error,
                          gpointer              data)
{
    g_print ("single_handler_debug_two\n");
    return TRUE;
}

/**
 * test_interpreter_single_handler:
 *
 * Check a single handler is called on I/O signal emission, even if more
 * handlers have been connected.
 */
#ifdef G_OS_UNIX
static void
test_interpreter_single_handler (CattleInterpreter   **interpreter,
                                 gconstpointer         data)
{
    CattleConfiguration *configuration;
    CattleProgram *program;

    configuration = cattle_interpreter_get_configuration (*interpreter);
    cattle_configuration_set_debug_is_enabled (configuration, TRUE);
    g_object_unref (configuration);

    program = cattle_interpreter_get_program (*interpreter);
    cattle_program_load_from_string (program, ",", NULL);
    g_object_unref (program);

    g_signal_connect (*interpreter,
                      "input-request",
                      G_CALLBACK (single_handler_input_one),
                      NULL);
    g_signal_connect (*interpreter,
                      "input-request",
                      G_CALLBACK (single_handler_input_two),
                      NULL);

    if (g_test_trap_fork (5, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
        cattle_interpreter_run (*interpreter, NULL);
        exit (1);
    }

    g_test_trap_assert_failed ();
    g_test_trap_assert_stdout_unmatched ("*two*");

    program = cattle_interpreter_get_program (*interpreter);
    cattle_program_load_from_string (program, ".", NULL);
    g_object_unref (program);

    g_signal_connect (*interpreter,
                      "output-request",
                      G_CALLBACK (single_handler_output_one),
                      NULL);
    g_signal_connect (*interpreter,
                      "output-request",
                      G_CALLBACK (single_handler_output_two),
                      NULL);

    if (g_test_trap_fork (5, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
        cattle_interpreter_run (*interpreter, NULL);
        exit (1);
    }

    g_test_trap_assert_failed ();
    g_test_trap_assert_stdout_unmatched ("*two*");

    program = cattle_interpreter_get_program (*interpreter);
    cattle_program_load_from_string (program, "#", NULL);
    g_object_unref (program);

    g_signal_connect (*interpreter,
                      "debug-request",
                      G_CALLBACK (single_handler_debug_one),
                      NULL);
    g_signal_connect (*interpreter,
                      "debug-request",
                      G_CALLBACK (single_handler_debug_two),
                      NULL);

    if (g_test_trap_fork (5, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
        cattle_interpreter_run (*interpreter, NULL);
        exit (1);
    }

    g_test_trap_assert_failed ();
    g_test_trap_assert_stdout_unmatched ("*two*");
}
#endif /* G_OS_UNIX */

gint
main (gint argc, gchar **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

#ifdef G_OS_UNIX
    g_test_add ("/interpreter/single-handler",
                CattleInterpreter*,
                NULL,
                interpreter_create,
                test_interpreter_single_handler,
                interpreter_destroy);
#endif /* G_OS_UNIX */

    g_test_run ();

    return 0;
}
