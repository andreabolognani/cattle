/* interpreter -- Tests for the interpreter implementation
 * Copyright (C) 2008-2014  Andrea Bolognani <eof@kiyuko.org>
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
               gpointer            data,
               GError            **error)
{
	CattleBuffer *input;

	input = cattle_buffer_new (9);
	cattle_buffer_set_contents (input, "whatever");

	cattle_interpreter_feed (interpreter, input);

	return TRUE;
}

/* Succesful input handler that doesn't feed the interpreter */
static gboolean
input_no_feed (CattleInterpreter  *interpreter,
               gpointer            data,
               GError            **error)
{
	return TRUE;
}

/* Successful input handler that returns a valid UTF-8 string */
static gboolean
input_utf8 (CattleInterpreter  *interpreter,
            gpointer            data,
            GError            **error)
{
	CattleBuffer *input;

	input = cattle_buffer_new (22);
	cattle_buffer_set_contents (input, "\xe2\x84\xa2 (Trademark symbol)");

	cattle_interpreter_feed (interpreter, input);

	return TRUE;
}

/* Succesful input handler that returns an invalid UTF-8 string */
static gboolean
input_invalid_utf8 (CattleInterpreter  *interpreter,
                    gpointer            data,
                    GError            **error)
{
	CattleBuffer *input;

	input = cattle_buffer_new (3);
	cattle_buffer_set_contents (input, "\xe2\x28\xa1");

	/* Return some malformed UTF-8 */
	cattle_interpreter_feed (interpreter, input);

	return TRUE;
}

/* Unsuccesful input handler that sets the error */
static gboolean
input_fail_set_error (CattleInterpreter  *interpreter,
                      gpointer            data,
                      GError            **error)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_IO,
	                     "Spurious error");

	return FALSE;
}

/* Unsuccesful input handler that doesn't set the error */
static gboolean
input_fail_no_set_error (CattleInterpreter  *interpreter,
                         gpointer            data,
                         GError            **error)
{
	return FALSE;
}

/* Unsuccesful input handler that sets the error but returns TRUE */
static gboolean
input_fail_only_error (CattleInterpreter  *interpreter,
                       gpointer            data,
                       GError            **error)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_IO,
	                     "Spurious error");

	return TRUE;
}

/* Successful output handler */
static gboolean
output_success (CattleInterpreter  *interpreter,
                gint8               output,
                gpointer            data,
                GError            **error)
{
	return TRUE;
}

/* Succesfull output handler working on a buffer */
static gboolean
output_success_buffer (CattleInterpreter  *interpreter,
                       gint8               output,
                       gpointer            data,
                       GError            **error)
{
	GString *buffer;

	buffer = (GString*) data;

	g_string_append_c (buffer,
	                   (gchar) output);

	return TRUE;
}

/* Unsuccesful output handler that sets the error */
static gboolean
output_fail_set_error (CattleInterpreter  *interpreter,
                       gint8               output,
                       gpointer            data,
                       GError            **error)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_IO,
	                     "Spurious error");

	return FALSE;
}

/* Unsuccesful output handler that doesn't set the error */
static gboolean
output_fail_no_set_error (CattleInterpreter  *interpreter,
                          gint8               output,
                          gpointer            data,
                          GError            **error)
{
	return FALSE;
}

/* Unsuccesful output handler that sets the error but returns TRUE */
static gboolean
output_fail_only_error (CattleInterpreter  *interpreter,
                        gint8               output,
                        gpointer            data,
                        GError            **error)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_IO,
	                     "Spurious error");

	return TRUE;
}

/* Succesful debug handler */
static gboolean
debug_success (CattleInterpreter  *interpreter,
               gpointer            data,
               GError            **error)
{
	return TRUE;
}

/* Succesfut debug handler working on a buffer */
static gboolean
debug_success_buffer (CattleInterpreter  *interpreter,
                      gpointer            data,
                      GError            **error)
{
	GString *buffer;

	buffer = (GString*) data;

	g_string_append_c (buffer,
	                   '0');

	return TRUE;
}

/* Unsuccesful debug handler that sets the error */
static gboolean
debug_fail_set_error (CattleInterpreter  *interpreter,
                      gpointer            data,
                      GError            **error)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_IO,
	                     "Spurious error");

	return FALSE;
}

/* Unsuccesful debug handler that doesn't set the error */
static gboolean
debug_fail_no_set_error (CattleInterpreter  *interpreter,
                         gpointer            data,
                         GError            **error)
{
	return FALSE;
}

/* Unsuccesful debug handler which sets the error but returns TRUE */
static gboolean
debug_fail_only_error (CattleInterpreter  *interpreter,
                       gpointer            data,
                       GError            **error)
{
	g_set_error_literal (error,
	                     CATTLE_ERROR,
	                     CATTLE_ERROR_IO,
	                     "Spurious error");

	return TRUE;
}

/**
 * test_interpreter_handlers:
 */
static void
test_interpreter_handlers (void)
{
	CattleInterpreter   *interpreter;
	CattleConfiguration *configuration;
	CattleProgram       *program;
	CattleBuffer        *buffer;
	GError              *error;
	GString             *output;
	gboolean             success;

	interpreter = cattle_interpreter_new ();

	buffer = cattle_buffer_new (5);
	cattle_buffer_set_contents (buffer, ",.,#.");

	configuration = cattle_interpreter_get_configuration (interpreter);
	cattle_configuration_set_debug_is_enabled (configuration, TRUE);
	g_object_unref (configuration);

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, buffer, NULL);
	g_object_unref (program);
	g_object_unref (buffer);

	output = g_string_new ("");

	cattle_interpreter_set_input_handler (interpreter,
	                                      input_success,
	                                      output);
	cattle_interpreter_set_output_handler (interpreter,
	                                       output_success_buffer,
	                                       output);
	cattle_interpreter_set_debug_handler (interpreter,
	                                      debug_success_buffer,
	                                      output);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (success);
	g_assert (g_utf8_collate (output->str, "w0h") == 0);

	g_string_free (output, TRUE);
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
	CattleProgram     *program;
	CattleBuffer      *buffer;
	GError            *error;
	gboolean           success;

	interpreter = cattle_interpreter_new ();

	buffer = cattle_buffer_new (1);
	cattle_buffer_set_contents (buffer, ",");

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, buffer, NULL);
	g_object_unref (program);
	g_object_unref (buffer);

	cattle_interpreter_set_input_handler (interpreter,
	                                      input_fail_set_error,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	/* Replace the signal handler */
	cattle_interpreter_set_input_handler (interpreter,
	                                      input_fail_no_set_error,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	/* Replace the signal handler */
	cattle_interpreter_set_input_handler (interpreter,
	                                      input_fail_only_error,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
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
	CattleProgram     *program;
	CattleBuffer      *buffer;
	GError            *error;
	gboolean           success;

	interpreter = cattle_interpreter_new ();

	buffer = cattle_buffer_new (1);
	cattle_buffer_set_contents (buffer, ".");

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, buffer, NULL);

	cattle_interpreter_set_output_handler (interpreter,
	                                       output_fail_set_error,
	                                       NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	/* Replace the signal handler */
	cattle_interpreter_set_output_handler (interpreter,
	                                       output_fail_no_set_error,
	                                       NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	/* Replace the signal handler */
	cattle_interpreter_set_output_handler (interpreter,
	                                       output_fail_only_error,
	                                       NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	g_object_unref (buffer);
	g_object_unref (program);
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
	CattleInterpreter   *interpreter;
	CattleConfiguration *configuration;
	CattleProgram       *program;
	CattleBuffer        *buffer;
	GError              *error;
	gboolean             success;

	interpreter = cattle_interpreter_new ();

	buffer = cattle_buffer_new (1);
	cattle_buffer_set_contents (buffer, "#");

	configuration = cattle_interpreter_get_configuration (interpreter);
	cattle_configuration_set_debug_is_enabled (configuration, TRUE);
	g_object_unref (configuration);

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, buffer, NULL);

	cattle_interpreter_set_debug_handler (interpreter,
	                                      debug_fail_set_error,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	/* Replace the signal handler */
	cattle_interpreter_set_debug_handler (interpreter,
	                                      debug_fail_no_set_error,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	/* Replace the signal handler */
	cattle_interpreter_set_debug_handler (interpreter,
	                                      debug_fail_only_error,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_IO));

	g_error_free (error);

	g_object_unref (buffer);
	g_object_unref (program);
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
	CattleProgram     *program;
	CattleBuffer      *buffer;
	GError            *error;
	gboolean           success;

	interpreter = cattle_interpreter_new ();

	buffer = cattle_buffer_new (1);
	cattle_buffer_set_contents (buffer, ",");

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, buffer, NULL);

	cattle_interpreter_set_input_handler (interpreter,
	                                      input_no_feed,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);

	g_assert (success);
	g_assert (error == NULL);

	g_object_unref (buffer);
	g_object_unref (program);
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
	CattleProgram     *program;
	CattleBuffer      *buffer;
	GError            *error;
	gboolean           success;

	interpreter = cattle_interpreter_new ();

	buffer = cattle_buffer_new (1);
	cattle_buffer_set_contents (buffer, ",");

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, buffer, NULL);

	cattle_interpreter_set_input_handler (interpreter,
	                                      input_utf8,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);

	g_assert (success);
	g_assert (error == NULL);

	g_object_unref (buffer);
	g_object_unref (program);
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
	CattleProgram     *program;
	CattleBuffer      *buffer;
	GError            *error;
	gboolean           success;

	interpreter = cattle_interpreter_new ();

	buffer = cattle_buffer_new (1);
	cattle_buffer_set_contents (buffer, ",");

	program = cattle_interpreter_get_program (interpreter);
	cattle_program_load (program, buffer, NULL);

	cattle_interpreter_set_input_handler (interpreter,
	                                      input_invalid_utf8,
	                                      NULL);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (success);
	g_assert (error == NULL);

	g_object_unref (buffer);
	g_object_unref (program);
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
	CattleProgram     *program;
	CattleInstruction *instructions;
	CattleInstruction *next;
	CattleTape        *tape;
	GError            *error;
	gboolean           success;

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
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_UNBALANCED_BRACKETS));

	g_error_free (error);

	/* Now make the start of loop instruction an end of loop instruction:
	 * the program is now ]+++ */
	cattle_instruction_set_value (instructions,
	                              CATTLE_INSTRUCTION_LOOP_END);

	error = NULL;
	success = cattle_interpreter_run (interpreter, &error);
	g_assert (!success);
	g_assert (g_error_matches (error, CATTLE_ERROR, CATTLE_ERROR_UNBALANCED_BRACKETS));

	g_object_unref (instructions);
	g_object_unref (interpreter);
}

gint
main (gint    argc,
      gchar **argv)
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init ();
#endif

	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/interpreter/handlers",
	                 test_interpreter_handlers);
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
