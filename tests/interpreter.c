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

/* Succesful input handler */
static gboolean
input_success (CattleInterpreter  *interpreter,
               GError            **error,
               gpointer            data)
{
	cattle_interpreter_feed (interpreter,
	                         "whatever");

	return TRUE;
}

/* Succesful input handler that doesn't feed the interpreter */
static gboolean
input_no_feed (CattleInterpreter  *interpreter,
               GError            **error,
               gpointer            data)
{
	return TRUE;
}

/* Successful input handler that returns a valid UTF-8 string */
static gboolean
input_utf8 (CattleInterpreter  *interpreter,
            GError            **error,
            gpointer            data)
{
	cattle_interpreter_feed (interpreter,
	                         "\xe2\x84\xa2 (Trademark symbol)");

	return TRUE;
}

/* Succesful input handler that returns an invalid UTF-8 string */
static gboolean
input_invalid_utf8 (CattleInterpreter  *interpreter,
                    GError            **error,
                    gpointer            data)
{
	/* Return some malformed UTF-8 */
	cattle_interpreter_feed (interpreter,
	                         "\xe2\x28\xa1");

	return TRUE;
}

/* Unsuccesful input handler that sets the error */
static gboolean
input_fail_set_error (CattleInterpreter  *interpreter,
                      GError            **error,
                      gpointer            data)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_BAD_UTF8,
	                     "Spurious error");

	return FALSE;
}

/* Unsuccesful input handler that doesn't set the error */
static gboolean
input_fail_no_set_error (CattleInterpreter  *interpreter,
                         GError            **error,
                         gpointer            data)
{
	return FALSE;
}

/* Successful output handler */
static gboolean
output_success (CattleInterpreter  *interpreter,
                gchar               output,
                GError            **error,
                gpointer            data)
{
	return TRUE;
}

/* Unsuccesful output handler that sets the error */
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

/* Unsuccesful output handler that doesn't set the error */
static gboolean
output_fail_no_set_error (CattleInterpreter  *interpreter,
                          gchar               output,
                          GError            **error,
                          gpointer            data)
{
	return FALSE;
}

/* Succesful debug handler */
static gboolean
debug_success (CattleInterpreter  *interpreter,
               GError            **error,
               gpointer            data)
{
	return TRUE;
}

/* Unsuccesful debug handler that sets the error */
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

/* Unsuccesful debug handler that doesn't set the error */
static gboolean
debug_fail_no_set_error (CattleInterpreter  *interpreter,
                         GError            **error,
                         gpointer            data)
{
	return FALSE;
}

/**
 * test_interpreter_single_input_handler:
 *
 * Check a single handler is called upon emission of the "input-request"
 * signal emission, even if multiple handlers have been connected.
 */
static void
test_interpreter_single_input_handler (void)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	GError *error;
	gboolean success;

	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, ",", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "input-request",
	                  G_CALLBACK (input_success),
	                  NULL);
	g_signal_connect (interpreter,
	                  "input-request",
	                  G_CALLBACK (input_fail_set_error),
	                  NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);

	g_assert (success);
	g_assert (error == NULL);

	g_object_unref (interpreter);
}

/**
 * test_interpreter_single_output_handler:
 *
 * Check a single handler is called upon emission of the "output-request"
 * signal emission, even if multiple handlers have been connected.
 */
static void
test_interpreter_single_output_handler (void)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	GError *error;
	gboolean success;

	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, ".", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "output-request",
	                  G_CALLBACK (output_success),
	                  NULL);
	g_signal_connect (interpreter,
	                  "output-request",
	                  G_CALLBACK (output_fail_set_error),
	                  NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);

	g_assert (success);
	g_assert (error == NULL);

	g_object_unref (interpreter);
}

/**
 * test_interpreter_single_debug_handler:
 *
 * Check a single handler is called upon emission of the "debug-request"
 * signal emission, even if multiple handlers have been connected.
 */
static void
test_interpreter_single_debug_handler (void)
{
	CattleInterpreter *interpreter;
	CattleConfiguration *configuration;
	CattleProgram *program;
	GError *error;
	gboolean success;

	interpreter = cattle_interpreter_new ();

	configuration = cattle_interpreter_get_configuration (interpreter);
	cattle_configuration_set_debug_is_enabled (configuration, TRUE);
	g_object_unref (configuration);

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, "#", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "debug-request",
	                  G_CALLBACK (debug_success),
	                  NULL);
	g_signal_connect (interpreter,
	                  "debug-request",
	                  G_CALLBACK (debug_fail_set_error),
	                  NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);

	g_assert (success);
	g_assert (error == NULL);

	g_object_unref (interpreter);
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

/**
 * test_interpreter_input_no_feed:
 *
 * Make sure the interpreter keeps working when a misbehaving input
 * handler is used.
 */
static void
test_interpreter_input_no_feed (void)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	GError *error;
	gboolean success;

	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, ",", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "input-request",
	                  G_CALLBACK (input_no_feed),
	                  NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);

	g_assert (success);
	g_assert (error == NULL);

	g_object_unref (interpreter);
}

/**
 * test_interpreter_unicode_input:
 *
 * Feed the interpreter with non-ASCII input.
 */
static void
test_interpreter_unicode_input (void)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	GError *error;
	gboolean success;

	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, ",", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "input-request",
	                  G_CALLBACK (input_utf8),
	                  NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_INPUT_OUT_OF_RANGE));

	g_error_free (error);

	g_object_unref (interpreter);
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
	gboolean success;

	interpreter = cattle_interpreter_new ();

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, ",", NULL);
	g_object_unref (program);

	g_signal_connect (interpreter,
	                  "input-request",
	                  G_CALLBACK (input_invalid_utf8),
	                  NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
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
test_interpreter_unbalanced_brackets (void)
{
	CattleInterpreter *interpreter;
	CattleProgram *program;
	CattleInstruction *instructions;
	CattleInstruction *next;
	CattleTape *tape;
	GError *error;

	interpreter = cattle_interpreter_new ();

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
	program = cattle_interpreter_get_program (interpreter);
	cattle_program_set_instructions (program, instructions);
	g_object_unref (program);

	/* Set the current value to something so that the loop gets
	 * executed */
	tape = cattle_interpreter_get_tape (interpreter);
	cattle_tape_set_current_value (tape, 42);
	g_object_unref (tape);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_UNBALANCED_BRACKETS));

	g_error_free (error);

	/* Now make the start of loop instruction an end of loop instruction:
	 * the program is now ]+++ */
	cattle_instruction_set_value (instructions,
	                              CATTLE_INSTRUCTION_LOOP_END);

	error = NULL;
	g_assert (!cattle_interpreter_run (interpreter, &error));
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_UNBALANCED_BRACKETS));

	g_object_unref (instructions);
	g_object_unref (interpreter);
}

gint
main (gint argc, gchar **argv)
{
	g_type_init ();
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/interpreter/single-input-handler",
	                 test_interpreter_single_input_handler);
	g_test_add_func ("/interpreter/single-output-handler",
	                 test_interpreter_single_output_handler);
	g_test_add_func ("/interpreter/single-debug-handler",
	                 test_interpreter_single_debug_handler);
	g_test_add_func ("/interpreter/failed-input",
	                 test_interpreter_failed_input);
	g_test_add_func ("/interpreter/failed-output",
	                 test_interpreter_failed_output);
	g_test_add_func ("/interpreter/failed-debug",
	                 test_interpreter_failed_debug);
	g_test_add_func ("/interpreter/input-no-feed",
	                 test_interpreter_input_no_feed);
	g_test_add_func ("/interpreter/unicode-input",
	                 test_interpreter_unicode_input);
	g_test_add_func ("/interpreter/invalid-input",
	                 test_interpreter_invalid_input);
	g_test_add_func ("/interpreter/unbalanced-brackets",
	                 test_interpreter_unbalanced_brackets);

	return g_test_run ();
}
