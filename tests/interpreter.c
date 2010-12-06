/* interpreter -- Tests for the interpreter implementation
 * Copyright (C) 2009-2010  Andrea Bolognani <eof@kiyuko.org>
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
#include <stdlib.h>

static void
interpreter_create (CattleInterpreter **interpreter,
                    gconstpointer       data)
{
	*interpreter = cattle_interpreter_new ();
}

static void
interpreter_destroy (CattleInterpreter **interpreter,
                     gconstpointer       data)
{
	g_object_unref (*interpreter);
}

#ifdef G_OS_UNIX
static gboolean
single_input_handler_one (CattleInterpreter  *interpreter,
                          gchar             **input,
                          GError            **error,
                          gpointer            data)
{
	g_print ("single_input_handler_one\n");
	g_set_error (error,
	             CATTLE_ERROR,
	             CATTLE_ERROR_BAD_UTF8,
	             "Spurious error");
	*input = NULL;
	return FALSE;
}

static gboolean
single_input_handler_two (CattleInterpreter  *interpreter,
                          gchar             **input,
                          GError            **error,
                          gpointer            data)
{
	g_print ("single_input_handler_two\n");
	*input = NULL;
	return TRUE;
}

/**
 * test_interpreter_single_input_handler:
 *
 * Check a single handler is called upon emission of the "input-request"
 * signal emission, even if multiple handlers have been connected.
 */
static void
test_interpreter_single_input_handler (CattleInterpreter **interpreter,
                                       gconstpointer       data)
{
	CattleProgram *program;

	program = cattle_interpreter_get_program (*interpreter);
	cattle_program_load (program, ",", NULL);
	g_object_unref (program);

	g_signal_connect (*interpreter,
	                  "input-request",
	                  G_CALLBACK (single_input_handler_one),
	                  NULL);
	g_signal_connect (*interpreter,
	                  "input-request",
	                  G_CALLBACK (single_input_handler_two),
	                  NULL);

	if (g_test_trap_fork (5, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
		cattle_interpreter_run (*interpreter, NULL);
		exit (1);
	}

	g_test_trap_assert_failed ();
	g_test_trap_assert_stdout_unmatched ("*two*");
}

static gboolean
single_output_handler_one (CattleInterpreter  *interpreter,
                           gchar               output,
                           GError            **error,
                           gpointer            data)
{
	g_print ("single_output_handler_one\n");
	g_set_error (error,
	             CATTLE_ERROR,
	             CATTLE_ERROR_BAD_UTF8,
	             "Spurious error");
	return FALSE;
}

static gboolean
single_output_handler_two (CattleInterpreter  *interpreter,
                           gchar               output,
                           GError            **error,
                           gpointer            data)
{
	g_print ("single_output_handler_two\n");
	return TRUE;
}

/**
 * test_interpreter_single_output_handler:
 *
 * Check a single handler is called upon emission of the "output-request"
 * signal emission, even if multiple handlers have been connected.
 */
static void
test_interpreter_single_output_handler (CattleInterpreter **interpreter,
                                        gconstpointer       data)
{
	CattleProgram *program;

	program = cattle_interpreter_get_program (*interpreter);
	cattle_program_load (program, ".", NULL);
	g_object_unref (program);

	g_signal_connect (*interpreter,
	                  "output-request",
	                  G_CALLBACK (single_output_handler_one),
	                  NULL);
	g_signal_connect (*interpreter,
	                  "output-request",
	                  G_CALLBACK (single_output_handler_two),
	                  NULL);

	if (g_test_trap_fork (5, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
		cattle_interpreter_run (*interpreter, NULL);
		exit (1);
	}

	g_test_trap_assert_failed ();
	g_test_trap_assert_stdout_unmatched ("*two*");
}

static gboolean
single_debug_handler_one (CattleInterpreter  *interpreter,
                          GError            **error,
                          gpointer            data)
{
	g_print ("single_debug_handler_one\n");
	g_set_error (error,
	             CATTLE_ERROR,
	             CATTLE_ERROR_BAD_UTF8,
	             "Spurious error");
	return FALSE;
}

static gboolean
single_debug_handler_two (CattleInterpreter  *interpreter,
                          GError            **error,
                          gpointer            data)
{
	g_print ("single_debug_handler_two\n");
	return TRUE;
}

/**
 * test_interpreter_single_debug_handler:
 *
 * Check a single handler is called upon emission of the "debug-request"
 * signal emission, even if multiple handlers have been connected.
 */
static void
test_interpreter_single_debug_handler (CattleInterpreter **interpreter,
                                       gconstpointer       data)
{
	CattleConfiguration *configuration;
	CattleProgram *program;

	configuration = cattle_interpreter_get_configuration (*interpreter);
	cattle_configuration_set_debug_is_enabled (configuration, TRUE);
	g_object_unref (configuration);

	program = cattle_interpreter_get_program (*interpreter);
	cattle_program_load (program, "#", NULL);
	g_object_unref (program);

	g_signal_connect (*interpreter,
	                  "debug-request",
	                  G_CALLBACK (single_debug_handler_one),
	                  NULL);
	g_signal_connect (*interpreter,
	                  "debug-request",
	                  G_CALLBACK (single_debug_handler_two),
	                  NULL);

	if (g_test_trap_fork (5, G_TEST_TRAP_SILENCE_STDOUT | G_TEST_TRAP_SILENCE_STDERR)) {
		cattle_interpreter_run (*interpreter, NULL);
		exit (1);
	}

	g_test_trap_assert_failed ();
	g_test_trap_assert_stdout_unmatched ("*two*");
}
#endif /* G_OS_UNIX */

/* Fail an input request and set the error */
static gboolean
input_fail_set_error (CattleInterpreter  *interpreter,
                      gchar             **input,
                      GError            **error,
                      gpointer            data)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_BAD_UTF8,
	                     "Spurious error");

	return FALSE;
}

/* Fail an input request without setting the error */
static gboolean
input_fail_no_set_error (CattleInterpreter  *interpreter,
                         gchar             **input,
                         GError            **error,
                         gpointer            data)
{
	return FALSE;
}

/**
 * test_interpreter_failed_input:
 *
 * Check the correct error is reported when an input request fails, and
 * that a generic error is reported when the input handler doesn't set
 * the error itself.
 */
static void
test_interpreter_failed_input (void)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	GError *error;

	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, ",", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "input-request",
	                  G_CALLBACK (input_fail_set_error),
	                  NULL);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_BAD_UTF8));

	g_error_free (error);

	/* Replace the signal handler */
	g_signal_handlers_disconnect_by_func (interpreter,
	                                      G_CALLBACK (input_fail_set_error),
	                                      NULL);
	g_signal_connect (interpreter,
	                  "input-request",
	                  G_CALLBACK (input_fail_no_set_error),
	                  NULL);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	g_object_unref (interpreter);
}

/* Fail an output request and set the error */
static gboolean
output_fail_set_error (CattleInterpreter  *interpreter,
                       gchar               output,
                       GError            **error,
                       gpointer            data)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_BAD_UTF8,
	                     "Spurious error");

	return FALSE;
}

/* Fail an output request without setting the error */
static gboolean
output_fail_no_set_error (CattleInterpreter  *interpreter,
                          gchar               output,
                          GError            **error,
                          gpointer            data)
{
	return FALSE;
}

/**
 * test_interpreter_failed_output:
 *
 * Check the correct error is reported when an output request fails, and
 * that a generic error is reported when the output handler doesn't set
 * the error itself.
 */
static void
test_interpreter_failed_output (void)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	GError *error;

	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, ".", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "output-request",
	                  G_CALLBACK (output_fail_set_error),
	                  NULL);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_BAD_UTF8));

	g_error_free (error);

	/* Replace the signal handler */
	g_signal_handlers_disconnect_by_func (interpreter,
	                                      G_CALLBACK (output_fail_set_error),
	                                      NULL);
	g_signal_connect (interpreter,
	                  "output-request",
	                  G_CALLBACK (output_fail_no_set_error),
	                  NULL);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	g_object_unref (interpreter);
}

/* Fail a debug request and set the error */
static gboolean
debug_fail_set_error (CattleInterpreter  *interpreter,
                      GError            **error,
                      gpointer            data)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_BAD_UTF8,
	                     "Spurious error");

	return FALSE;
}

/* Fail a debug request without setting the error */
static gboolean
debug_fail_no_set_error (CattleInterpreter  *interpreter,
                         GError            **error,
                         gpointer            data)
{
	return FALSE;
}

/**
 * test_interpreter_failed_debug:
 *
 * Check the correct error is reported when a debug request fails, and
 * that a generic error is reported when the debug handler doesn't set
 * the error itself.
 */
static void
test_interpreter_failed_debug (void)
{
	CattleInterpreter *interpreter;
	CattleConfiguration *configuration;
	CattleProgram *program;
	GError *error;

	interpreter = cattle_interpreter_new ();

	configuration = cattle_interpreter_get_configuration (interpreter);
	cattle_configuration_set_debug_is_enabled (configuration, TRUE);
	g_object_unref (configuration);

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, "#", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "debug-request",
	                  G_CALLBACK (debug_fail_set_error),
	                  NULL);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_BAD_UTF8));

	g_error_free (error);

	/* Replace the signal handler */
	g_signal_handlers_disconnect_by_func (interpreter,
	                                      G_CALLBACK (debug_fail_set_error),
	                                      NULL);
	g_signal_connect (interpreter,
	                  "debug-request",
	                  G_CALLBACK (debug_fail_no_set_error),
	                  NULL);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	g_object_unref (interpreter);
}

static gboolean
input_invalid_utf8 (CattleInterpreter  *interpreter,
                    gchar             **input,
                    GError            **error,
                    gpointer            data)
{
	/* Return some malformed UTF-8 */
	*input = "\xe2\x28\xa1";

	return TRUE;
}

/**
 * test_interpreter_invalid_input:
 *
 * Feed the interpreter with some input not encoded in UTF-8.
 */
static void
test_interpreter_invalid_input (void)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	GError *error;

	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, ",", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "input-request",
	                  G_CALLBACK (input_invalid_utf8),
	                  NULL);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_BAD_UTF8));

	g_error_free (error);

	g_object_unref (interpreter);
}

/**
 * test_interpreter_unbalanced_brackets:
 *
 * Try to run a program containing unbalanced brackets.
 *
 * The program is created by hand to bypass the checks in the program
 * loading routine.
 */
static void
test_interpreter_unbalanced_brackets (CattleInterpreter **interpreter,
                                      gconstpointer       data)
{
	CattleProgram *program;
	CattleInstruction *instructions;
	CattleInstruction *next;
	CattleTape *tape;
	GError *error;

	/* Build a program containing an unbalanced start of loop: [+++ */
	instructions = cattle_instruction_new ();
	cattle_instruction_set_value (instructions,
	                              CATTLE_INSTRUCTION_LOOP_BEGIN);

	next = cattle_instruction_new ();
	cattle_instruction_set_value (next,
	                              CATTLE_INSTRUCTION_INCREASE);
	cattle_instruction_set_quantity (next, 3);

	cattle_instruction_set_loop (instructions, next);
	g_object_unref (next);

	/* Load the buggy program */
	program = cattle_interpreter_get_program (*interpreter);
	cattle_program_set_instructions (program, instructions);
	g_object_unref (program);

	/* Set the current value to something so that the loop gets
	 * executed */
	tape = cattle_interpreter_get_tape (*interpreter);
	cattle_tape_set_current_value (tape, 42);
	g_object_unref (tape);

	error = NULL;
	g_assert (!cattle_interpreter_run (*interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_UNBALANCED_BRACKETS));

	g_error_free (error);

	/* Now make the start of loop instruction an end of loop instruction:
	 * the program is now ]+++ */
	cattle_instruction_set_value (instructions,
	                              CATTLE_INSTRUCTION_LOOP_END);

	error = NULL;
	g_assert (!cattle_interpreter_run (*interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_UNBALANCED_BRACKETS));

	g_object_unref (instructions);
}

gint
main (gint argc, gchar **argv)
{
	g_type_init ();
	g_test_init (&argc, &argv, NULL);

#ifdef G_OS_UNIX
	g_test_add ("/interpreter/single-input-handler",
	            CattleInterpreter*,
	            NULL,
	            interpreter_create,
	            test_interpreter_single_input_handler,
	            interpreter_destroy);
	g_test_add ("/interpreter/single-output-handler",
	            CattleInterpreter*,
	            NULL,
	            interpreter_create,
	            test_interpreter_single_output_handler,
	            interpreter_destroy);
	g_test_add ("/interpreter/single-debug-handler",
	            CattleInterpreter*,
	            NULL,
	            interpreter_create,
	            test_interpreter_single_debug_handler,
	            interpreter_destroy);
#endif /* G_OS_UNIX */
	g_test_add_func ("/interpreter/failed-input",
	                 test_interpreter_failed_input);
	g_test_add_func ("/interpreter/failed-output",
	                 test_interpreter_failed_output);
	g_test_add_func ("/interpreter/failed-debug",
	                 test_interpreter_failed_debug);
	g_test_add_func ("/interpreter/invalid-input",
	                 test_interpreter_invalid_input);
	g_test_add ("/interpreter/unbalanced-brackets",
	            CattleInterpreter*,
	            NULL,
	            interpreter_create,
	            test_interpreter_unbalanced_brackets,
	            interpreter_destroy);

	return g_test_run ();
}
