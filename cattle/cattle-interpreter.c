/* Cattle -- Flexible Brainfuck interpreter library
 * Copyright (C) 2008-2010  Andrea Bolognani <eof@kiyuko.org>
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

#include "cattle-enums.h"
#include "cattle-marshal.h"
#include "cattle-error.h"
#include "cattle-interpreter.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/**
 * SECTION:cattle-interpreter
 * @short_description: Brainfuck interpreter
 *
 * An instance of #CattleInterpreter represents a Brainfuck interpreter,
 * that is, an object which is capable of executing a #CattleProgram.
 *
 * #CattleInterpreter handles all the aspects of execution, including
 * input and output. It also hides all the details to the user, who
 * only needs to initialize the interpreter and call
 * cattle_interpreter_run() to execute a Brainfuck program.
 *
 * The behaviour of an interpreter can be modified by providing a
 * suitable #CattleConfiguration object.
 *
 * Once initialized, a #CattleInterpreter can run the assigned program
 * as many times as needed; the memory tape, however, is not
 * re-initialized automatically between executions.
 */

G_DEFINE_TYPE (CattleInterpreter, cattle_interpreter, G_TYPE_OBJECT)
/**
 * CattleInterpreter:
 *
 * Opaque data structure representing an interpreter. It should never be
 * accessed directly; use the methods below instead.
 */

#define CATTLE_INTERPRETER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), CATTLE_TYPE_INTERPRETER, CattleInterpreterPrivate))

struct _CattleInterpreterPrivate
{
	gboolean             disposed;

	CattleConfiguration *configuration;
	CattleProgram       *program;
	CattleTape          *tape;

	GSList              *stack; /* Instruction stack */

	gboolean             had_input;
	gchar               *input;
	gchar               *input_cursor;
	gboolean             end_of_input_reached;
};

/* Properties */
enum
{
	PROP_0,
	PROP_CONFIGURATION,
	PROP_PROGRAM,
	PROP_TAPE
};

/* Signals */
enum
{
	INPUT_REQUEST,
	OUTPUT_REQUEST,
	DEBUG_REQUEST,
	LAST_SIGNAL
};

static gint signals[LAST_SIGNAL] = {0};

/* Internal functions */
static gboolean run                        (CattleInterpreter      *interpreter,
                                            GError                **error);
static gboolean single_handler_accumulator (GSignalInvocationHint  *hint,
                                            GValue                 *signal_retval,
                                            const GValue           *handler_retval,
                                            gpointer                data);

static void
cattle_interpreter_init (CattleInterpreter *self)
{
	self->priv = CATTLE_INTERPRETER_GET_PRIVATE (self);

	self->priv->configuration = cattle_configuration_new ();
	self->priv->program = cattle_program_new ();
	self->priv->tape = cattle_tape_new ();

	self->priv->stack = NULL;

	self->priv->had_input = FALSE;
	self->priv->input = NULL;
	self->priv->input_cursor = NULL;
	self->priv->end_of_input_reached = FALSE;

	self->priv->disposed = FALSE;
}

static void
cattle_interpreter_dispose (GObject *object)
{
	CattleInterpreter *self = CATTLE_INTERPRETER (object);

	g_return_if_fail (!self->priv->disposed);

	g_object_unref (self->priv->configuration);
	self->priv->configuration = NULL;

	g_object_unref (self->priv->program);
	self->priv->program = NULL;

	g_object_unref (self->priv->tape);
	self->priv->tape = NULL;

	self->priv->disposed = TRUE;

	G_OBJECT_CLASS (cattle_interpreter_parent_class)->dispose (object);
}

static void
cattle_interpreter_finalize (GObject *object)
{
	/* FIXME
	 * We should release the input here, but we don't do it because
	 * it would be really tricky: since the allocator is chosen by
	 * the user, we have no way to know how to release it */

	G_OBJECT_CLASS (cattle_interpreter_parent_class)->finalize (object);
}

static gboolean
run (CattleInterpreter  *self,
     GError            **error)
{
	CattleConfiguration *configuration;
	CattleProgram *program;
	CattleTape *tape;
	CattleInstruction *current;
	CattleInstruction *next;
	CattleInstructionValue value;
	GSList *stack;
	GError *inner_error;
	gboolean success;
	gchar temp;
	gint quantity;
	gint i;

	configuration = self->priv->configuration;
	program = self->priv->program;
	tape = self->priv->tape;
	stack = self->priv->stack;
	success = TRUE;

	current = cattle_program_get_instructions (program);

	while (current != NULL) {

		value = cattle_instruction_get_value (current);

		switch (value) {

			case CATTLE_INSTRUCTION_LOOP_BEGIN:

				next = cattle_instruction_get_loop (current);

				/* Enter the loop only if the value stored in the
				 * current cell is not zero */
				if (cattle_tape_get_current_value (tape) != 0) {

					/* Push the current instruction on the stack */
					stack = g_slist_prepend (stack, current);
					current = next;

					continue;
				}
				break;

			case CATTLE_INSTRUCTION_LOOP_END:

				/* If the instruction stack is empty, we're not running
				 * a loop, so trying to exit it is an error */
				if (G_UNLIKELY (stack == NULL)) {

					g_set_error_literal (error,
					                     CATTLE_ERROR,
					                     CATTLE_ERROR_UNBALANCED_BRACKETS,
					                     "Unbalanced brackets");

					g_object_unref (current);

					return FALSE;
				}

				/* Pop an instruction off the stack */
				g_object_unref (current);
				current = CATTLE_INSTRUCTION (stack->data);
				stack = g_slist_delete_link (stack, stack);

				continue;

			case CATTLE_INSTRUCTION_MOVE_LEFT:

				quantity = cattle_instruction_get_quantity (current);
				cattle_tape_move_left_by (tape, quantity);
				break;

			case CATTLE_INSTRUCTION_MOVE_RIGHT:

				quantity = cattle_instruction_get_quantity (current);
				cattle_tape_move_right_by (tape, quantity);
				break;

			case CATTLE_INSTRUCTION_INCREASE:

				quantity = cattle_instruction_get_quantity (current);
				cattle_tape_increase_current_value_by (tape, quantity);
				break;

			case CATTLE_INSTRUCTION_DECREASE:

				quantity = cattle_instruction_get_quantity (current);
				cattle_tape_decrease_current_value_by (tape, quantity);
				break;

			case CATTLE_INSTRUCTION_READ:

				quantity = cattle_instruction_get_quantity (current);

				for (i = 0; i < quantity; i++) {

					/* The following code is quite complicated because
					 * it needs to handle different situations.
					 * The execution of a read instruction is divided
					 * into three steps:
					 *
					 *   1) Fetch more input if needed: the
					 *      "input-request" signal is emitted, and the
					 *      signal handler is responsible for fetching
					 *      more input from the source. This step is
					 *      obviously skipped if some input was
					 *      included in the program.
					 *
					 *   2) Get the char and normalize it: this step
					 *      allows us to handle all the input
					 *      consistently in the last step, regardless
					 *      of its source.
					 *
					 *   3) Perform the actual operation: the easy
					 *      part ;)
					 *
					 * TODO
					 * This step can probably be semplified.
					 */

					/* STEP 1: Fetch more input if needed */

					/* We don't even try to fetch more input if the
					 * input was included in the program or if the end
					 * of input has alredy been reached */
					if (self->priv->had_input == FALSE && !self->priv->end_of_input_reached) {

						/* We are at the end of the current line of
						 * input, or we haven't fetched any input yet.
						 * We need to emit the "input-request" signal
						 * to get more input */
						if (self->priv->input_cursor == NULL || g_utf8_get_char (self->priv->input_cursor) == 0) {

							inner_error = NULL;
							g_signal_emit (self,
							               signals[INPUT_REQUEST],
							               0,
							               &(self->priv->input),
							               &inner_error,
							               &success);

							/* The operation failed: we abort
							 * immediately */
							if (G_UNLIKELY (success == FALSE)) {

								/* If the signal handler has set the
								 * error, as it's required to,
								 * propagate that error; if it hasn't,
								 * raise a generic I/O error */
								if (inner_error == NULL) {
									g_set_error_literal (error,
									                     CATTLE_ERROR,
									                     CATTLE_ERROR_IO,
									                     "Unknown I/O error");
								}
								else {
									g_propagate_error (error,
									                   inner_error);
								}

								g_object_unref (current);

								return FALSE;
							}

							/* A return value of NULL from the signal
							 * handler means the end of input was
							 * reached. We set the appropriate flag */
							if (self->priv->input == NULL) {
								self->priv->end_of_input_reached = TRUE;
							}

							/* Move the cursor to the beginning of the
							 * new input line */
							self->priv->input_cursor = self->priv->input;
						}
					}

					/* STEP 2: Get the char and normalize it */

					/* If we have already reached the end of input,
					 * the current char is obviously an EOF */
					if (self->priv->end_of_input_reached) {
						temp = (gchar) EOF;
					}

					else {
						temp = g_utf8_get_char (self->priv->input_cursor);

						/* The end of the saved input is converted into
						 * an EOF character for consistency. We don't
						 * need to move the cursor forward if we are
						 * already at the end of the input */
						if (temp == 0) {

							if (self->priv->had_input == FALSE) {
								temp = (gchar) EOF;
								self->priv->end_of_input_reached = TRUE;
							}
						}

						/* There is some more input: we have to move
						 * the cursor one position forward */
						else {
							self->priv->input_cursor = g_utf8_next_char (self->priv->input_cursor);
						}
					}
				}

				/* STEP 3: Perform the actual operation */

				/* We are at the end of the input: we have to check
				 * the configuration to know which action we should
				 * perform */
				if (temp == (gchar) EOF) {

					switch (cattle_configuration_get_on_eof_action (configuration)) {

						case CATTLE_ON_EOF_STORE_ZERO:
							cattle_tape_set_current_value (tape, (gchar) 0);
							break;

						case CATTLE_ON_EOF_STORE_EOF:
							cattle_tape_set_current_value (tape, temp);
							break;

						case CATTLE_ON_EOF_DO_NOTHING:
						default:
							/* Do nothing */
							break;
					}
				}

				/* We are not at the end of the input: just save the
				 * input into the current cell of the tape */
				else {
					cattle_tape_set_current_value (tape, temp);
				}
				break;

			case CATTLE_INSTRUCTION_PRINT:

				quantity = cattle_instruction_get_quantity (current);

				/* Write the value in the current cell to standard
				 * output */
				for (i = 0; i < quantity; i++) {

					inner_error = NULL;
					g_signal_emit (self,
					               signals[OUTPUT_REQUEST],
					               0,
					               cattle_tape_get_current_value (tape),
					               &inner_error,
					               &success);

					/* Stop at the first error, even if we should
					 * output the content of the current cell more
					 * than once */
					if (G_UNLIKELY (success == FALSE)) {

						/* If the signal handler has set the error,
						 * propagate it; otherwise, raise a generic
						 * I/O error */
						if (inner_error == NULL) {
							g_set_error_literal (error,
							                     CATTLE_ERROR,
							                     CATTLE_ERROR_IO,
							                     "Unknown I/O error");
						}
						else {
							g_propagate_error (error,
							                   inner_error);
						}

						g_object_unref (current);

						return FALSE;
					}
				}
				break;

			case CATTLE_INSTRUCTION_DEBUG:

				/* Dump the tape only if debugging is enabled in the
				 * configuration */
				if (cattle_configuration_get_debug_is_enabled (configuration)) {

					quantity = cattle_instruction_get_quantity (current);

					for (i = 0; i < quantity; i++) {

						inner_error = NULL;
						g_signal_emit (self,
						               signals[DEBUG_REQUEST],
						               0,
						               &inner_error,
						               &success);

						if (G_UNLIKELY (success == FALSE)) {

							/* If the debug handler hasn't set the
							 * error, raise a generic I/O error */
							if (inner_error == NULL) {
								g_set_error_literal (error,
								                     CATTLE_ERROR,
								                     CATTLE_ERROR_IO,
								                     "Unknown I/O error");
							}
							else {
								g_propagate_error (error,
								                   inner_error);
							}

							g_object_unref (current);

							return FALSE;
						}
					}
				}
				break;

			case CATTLE_INSTRUCTION_NONE:

				/* Do nothing */
				break;
		}

		next = cattle_instruction_get_next (current);
		g_object_unref (current);
		current = next;
	}

	/* There are some instructions left on the stack: the brackets
	 * are not balanced */
	if (stack != NULL) {

		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_UNBALANCED_BRACKETS,
		                     "Unbalanced brackets");

		return FALSE;
	}

	return TRUE;
}

static gboolean
single_handler_accumulator (GSignalInvocationHint *hint,
                            GValue                *signal_retval,
                            const GValue          *handler_retval,
                            gpointer               data)
{
	g_value_copy (handler_retval, signal_retval);

	/* Stop the signal emission so other signal handlers
	 * are not called */
	return FALSE;
}

static gboolean
input_default_handler (CattleInterpreter  *self,
                       gchar             **input,
                       GError            **error,
                       gpointer            data)
{
	gchar *buffer;

	/* The previous input buffer is not needed anymore */
	if (*input != NULL) {

		g_free (*input);
		*input = NULL;
	}

	/* The buffer size is not really important: if the input cannot
	 * fit a single buffer, the signal will be emitted again */
	buffer = g_new0 (gchar, 256);

	if (fgets (buffer, 256, stdin) == NULL) {

		/* A NULL return value from fgets could either mean a read
		 * error has occurred or the end of input has been reached.
		 * In the latter case, we have to notify the interpreter */
		if (G_LIKELY (feof (stdin))) {

			*input = NULL;
			return TRUE;
		}
		else {

			g_set_error_literal (error,
			                     CATTLE_ERROR,
			                     CATTLE_ERROR_IO,
			                     strerror (errno));
			return FALSE;
		}
	}

	*input = buffer;
	return TRUE;
}

static gboolean
output_default_handler (CattleInterpreter  *self,
                        gchar               output,
                        GError            **error,
                        gpointer            data)
{
	if (G_UNLIKELY (fputc (output, stdout) == EOF)) {

		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_IO,
		                     strerror (errno));
		return FALSE;
	}

	return TRUE;
}

static gboolean
debug_default_handler (CattleInterpreter  *self,
                       GError            **error,
                       gpointer            data)
{
	CattleTape *tape;
	gchar value;
	gint steps;

	tape = cattle_interpreter_get_tape (self);

	/* Save the current position so it can be restored later */
	cattle_tape_push_bookmark (tape);

	/* Move to the beginning of the tape, counting how many steps
	 * it takes to get there. This value will be used later to mark
	 * the current position */
	steps = 0;
	while (TRUE) {

		if (cattle_tape_is_at_beginning (tape)) {
			break;
		}

		cattle_tape_move_left (tape);
		steps++;
	}

	if (G_UNLIKELY (fputc ('[', stderr) == EOF)) {
		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_IO,
		                     strerror (errno));
		cattle_tape_pop_bookmark (tape);
		g_object_unref (tape);
		return FALSE;
	}

	while (TRUE) {

		/* Mark the current position */
		if (steps == 0) {
			if (G_UNLIKELY (fputc ('<', stderr) == EOF)) {
				g_set_error_literal (error,
				                     CATTLE_ERROR,
				                     CATTLE_ERROR_IO,
				                     strerror (errno));
				cattle_tape_pop_bookmark (tape);
				g_object_unref (tape);
				return FALSE;
			}
		}

		value = cattle_tape_get_current_value (tape);

		/* Print the value of the current cell if it is a graphical char;
		 * otherwise, print its hexadecimal value */
		if (g_ascii_isgraph (value)) {
			if (G_UNLIKELY (fputc (value, stderr) == EOF)) {
				g_set_error_literal (error,
				                     CATTLE_ERROR,
				                     CATTLE_ERROR_IO,
				                     strerror (errno));
				cattle_tape_pop_bookmark (tape);
				g_object_unref (tape);
				return FALSE;
			}
		}
		else {
			if (G_UNLIKELY (fprintf (stderr, "0x%X", (gint) value) < 0)) {
				g_set_error_literal (error,
				                     CATTLE_ERROR,
				                     CATTLE_ERROR_IO,
				                     strerror (errno));
				cattle_tape_pop_bookmark (tape);
				g_object_unref (tape);
				return FALSE;
			}
		}

		/* Mark the current position */
		if (steps == 0) {
			if (G_UNLIKELY (fputc ('>', stderr) == EOF)) {
				g_set_error_literal (error,
				                     CATTLE_ERROR,
				                     CATTLE_ERROR_IO,
				                     strerror (errno));
				cattle_tape_pop_bookmark (tape);
				g_object_unref (tape);
				return FALSE;
			}
		}

		/* Exit after printing the last value */
		if (cattle_tape_is_at_end (tape)) {
			break;
		}

		/* Print a space and move forward */
		if (G_UNLIKELY (fputc (' ', stderr) == EOF)) {
			g_set_error_literal (error,
			                     CATTLE_ERROR,
			                     CATTLE_ERROR_IO,
			                     strerror (errno));
			cattle_tape_pop_bookmark (tape);
			g_object_unref (tape);
			return FALSE;
		}
		cattle_tape_move_right (tape);
		steps--;
	}

	if (G_UNLIKELY (fputc (']', stderr) == EOF)) {
		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_IO,
		                     strerror (errno));
		cattle_tape_pop_bookmark (tape);
		g_object_unref (tape);
		return FALSE;
	}
	if (G_UNLIKELY (fputc ('\n', stderr) == EOF)) {
		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_IO,
		                     strerror (errno));
		cattle_tape_pop_bookmark (tape);
		g_object_unref (tape);
		return FALSE;
	}

	/* Restore the previously-saved position */
	cattle_tape_pop_bookmark (tape);

	g_object_unref (tape);
	return TRUE;
}

/**
 * cattle_interpreter_new:
 *
 * Create and initialize a new interpreter.
 *
 * Returns: a new #CattleInterpreter.
 **/
CattleInterpreter*
cattle_interpreter_new (void)
{
	return g_object_new (CATTLE_TYPE_INTERPRETER, NULL);
}

/**
 * cattle_interpreter_run:
 * @interpreter: a #CattleInterpreter
 * @error: #GError used for error reporting
 *
 * Make the interpreter run the loaded program.
 *
 * Return: #TRUE if @interpreter completed the execution of its
 * program successfully, #FALSE otherwise.
 */
gboolean
cattle_interpreter_run (CattleInterpreter  *self,
                        GError            **error)
{
	CattleProgram *program;
	CattleInstruction *instruction;
	gboolean success = FALSE;

	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (!self->priv->disposed, FALSE);

	program = self->priv->program;
	instruction = cattle_program_get_instructions (program);
	self->priv->stack = NULL;

	self->priv->input = cattle_program_get_input (program);
	if (self->priv->input != NULL) {
		self->priv->had_input = TRUE;
	}
	self->priv->input_cursor = self->priv->input;

	success = run (self, error);

	if (self->priv->had_input == TRUE) {
		g_free (self->priv->input);
	}

	g_object_unref (instruction);

	return success;
}

/**
 * cattle_interpreter_set_configuration:
 * @interpreter: a #CattleInterpreter
 * @configuration: configuration for the interpreter
 *
 * Set the configuration for @interpreter.
 *
 * The same configuration can be used for several interpreters, but
 * modifying it after it has been assigned to an interpreter may result
 * in undefined behaviour, and as such is discouraged.
 */
void
cattle_interpreter_set_configuration (CattleInterpreter   *self,
                                      CattleConfiguration *configuration)
{
	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (CATTLE_IS_CONFIGURATION (configuration));
	g_return_if_fail (!self->priv->disposed);

	/* Release the reference held on the previous
	 * configuration */
	g_object_unref (self->priv->configuration);

	self->priv->configuration = configuration;
	g_object_ref (self->priv->configuration);
}

/**
 * cattle_interpreter_get_configuration:
 * @interpreter: a #CattleInterpreter
 *
 * Get the configuration for @interpreter.
 * See cattle_interpreter_set_configuration().
 *
 * The returned object must be unreferenced when no longer
 * needed.
 *
 * Return: configuration for @interpreter.
 */
CattleConfiguration*
cattle_interpreter_get_configuration (CattleInterpreter *self)
{
	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), NULL);
	g_return_val_if_fail (!self->priv->disposed, NULL);

	g_object_ref (self->priv->configuration);

	return self->priv->configuration;
}

/**
 * cattle_interpreter_set_program:
 * @interpreter: a #CattleInterpreter
 * @program: a #CattleProgram
 *
 * Set the program to be executed by @interpreter.
 *
 * A single program can be shared between multiple interpreters,
 * as long as care is taken not to modify it after it has been
 * assigned to any of them.
 */
void
cattle_interpreter_set_program (CattleInterpreter *self,
                                CattleProgram     *program)
{
	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (CATTLE_IS_PROGRAM (program));
	g_return_if_fail (!self->priv->disposed);

	/* Release the reference held to the previous
	 * program, if any */
	g_object_unref (self->priv->program);

	self->priv->program = program;
	g_object_ref (self->priv->program);
}

/**
 * cattle_interpreter_get_program:
 * @interpreter: a #CattleInterpreter
 *
 * Get the current program for @interpreter.
 * See cattle_interpreter_set_program().
 *
 * The returned object must be unreferenced when no longer
 * needed.
 *
 * Return: the program @interpreter will run.
 */
CattleProgram*
cattle_interpreter_get_program (CattleInterpreter *self)
{
	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), NULL);
	g_return_val_if_fail (!self->priv->disposed, NULL);

	g_object_ref (self->priv->program);

	return self->priv->program;
}

/**
 * cattle_interpreter_set_tape:
 * @interpreter: a #CattleInterpreter
 * @tape: memory tape for the interpreter
 *
 * Set the memory tape used by @interpreter.
 */
void
cattle_interpreter_set_tape (CattleInterpreter *self,
                             CattleTape        *tape)
{
	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (CATTLE_IS_TAPE (tape));
	g_return_if_fail (!self->priv->disposed);

	/* Release the reference held to the previous tape */
	g_object_unref (self->priv->tape);

	self->priv->tape = tape;
	g_object_ref (self->priv->tape);
}

/**
 * cattle_interpreter_get_tape:
 * @interpreter: a #CattleInterpreter
 *
 * Get the memory tape used by @interpreter.
 * See cattle_interpreter_set_tape().
 *
 * The returned object must be unreferenced when no longer
 * needed.
 *
 * Return: the memory tape for @interpreter.
 */
CattleTape*
cattle_interpreter_get_tape (CattleInterpreter *self)
{
	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), NULL);
	g_return_val_if_fail (!self->priv->disposed, NULL);

	g_object_ref (self->priv->tape);

	return self->priv->tape;
}

static void
cattle_interpreter_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
	CattleInterpreter *self = CATTLE_INTERPRETER (object);
	CattleConfiguration *t_conf;
	CattleProgram *t_prog;
	CattleTape *t_tape;

	if (G_LIKELY (!self->priv->disposed)) {

		switch (property_id) {

			case PROP_CONFIGURATION:
				t_conf = g_value_get_object (value);
				cattle_interpreter_set_configuration (self,
				                                      t_conf);
				break;

			case PROP_PROGRAM:
				t_prog = g_value_get_object (value);
				cattle_interpreter_set_program (self,
				                                t_prog);
				break;

			case PROP_TAPE:
				t_tape = g_value_get_object (value);
				cattle_interpreter_set_tape (self,
				                             t_tape);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
				                                   property_id,
				                                   pspec);
				break;
		}
	}
}

static void
cattle_interpreter_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
	CattleInterpreter *self = CATTLE_INTERPRETER (object);
	CattleConfiguration *t_conf;
	CattleProgram *t_prog;
	CattleTape *t_tape;

	if (G_LIKELY (!self->priv->disposed)) {

		switch (property_id) {

			case PROP_CONFIGURATION:
				t_conf = cattle_interpreter_get_configuration (self);
				g_value_set_object (value, t_conf);
				break;

			case PROP_PROGRAM:
				t_prog = cattle_interpreter_get_program (self);
				g_value_set_object (value, t_prog);
				break;

			case PROP_TAPE:
				t_tape = cattle_interpreter_get_tape (self);
				g_value_set_object (value, t_tape);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
	}
}

static void
cattle_interpreter_class_init (CattleInterpreterClass *self)
{
	GObjectClass *object_class = G_OBJECT_CLASS (self);
	GParamSpec *pspec;
	GClosure *closure;
	GType ptypes[2];

	object_class->set_property = cattle_interpreter_set_property;
	object_class->get_property = cattle_interpreter_get_property;
	object_class->dispose = cattle_interpreter_dispose;
	object_class->finalize = cattle_interpreter_finalize;

	/**
	 * CattleInterpreter:configuration:
	 *
	 * Configuration used by the interpreter.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_object ("configuration",
	                             "Configuration for the interpreter",
	                             "Get/set interpreter's configuration",
	                             CATTLE_TYPE_CONFIGURATION,
	                             G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_CONFIGURATION,
	                                 pspec);

	/**
	 * CattleInterpreter:program:
	 *
	 * Program executed by the interpreter.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_object ("program",
	                             "Program to be executed",
	                             "Get/set interpreter's program",
	                             CATTLE_TYPE_PROGRAM,
	                             G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_PROGRAM,
	                                 pspec);

	/**
	 * CattleInterpreter:tape:
	 *
	 * Tape used to store the data needed by the program.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_object ("tape",
	                             "Tape used by the interpreter",
	                             "Get/set interpreter's tape",
	                             CATTLE_TYPE_TAPE,
	                             G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_TAPE,
	                                 pspec);

	/**
	 * CattleInterpreter::input-request:
	 * @interpreter: a #CattleInterpreter
	 * @input: return location for the input
	 * @error: a #GError to be used for reporting, or %NULL
	 *
	 * Emitted whenever the interpreter needs input.
	 *
	 * If the operation fails, @error must be filled with
	 * detailed information about the error.
	 *
	 * Return: #TRUE if the operation is successful, #FALSE
	 * otherwise.
	 *
	 * Since: 0.9.1
	 */
	ptypes[0] = G_TYPE_POINTER;
	ptypes[1] = G_TYPE_POINTER;
	closure = g_cclosure_new (G_CALLBACK (input_default_handler),
	                          NULL,
	                          NULL);
	signals[INPUT_REQUEST] = g_signal_newv ("input-request",
	                                        CATTLE_TYPE_INTERPRETER,
	                                        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
	                                        closure,
	                                        single_handler_accumulator,
	                                        NULL,
	                                        cattle_marshal_BOOLEAN__POINTER_POINTER,
	                                        G_TYPE_BOOLEAN,
	                                        2,
	                                        ptypes);

	/**
	 * CattleInterpreter::output-request:
	 * @interpreter: a #CattleInterpreter
	 * @output: the character that needs to be printed
	 * @error: a #GError to be used for reporting, or %NULL
	 *
	 * Emitted whenever the interpreter needs to perform output.
	 *
	 * If the operation fails, @error has to be filled with
	 * detailed information about the error.
	 *
	 * Return: #TRUE if the operation is successful, #FALSE
	 * otherwise.
	 *
	 * Since: 0.9.1
	 */
	ptypes[0] = G_TYPE_CHAR;
	ptypes[1] = G_TYPE_POINTER;
	closure = g_cclosure_new (G_CALLBACK (output_default_handler),
	                          NULL,
	                          NULL);
	signals[OUTPUT_REQUEST] = g_signal_newv ("output-request",
                                             CATTLE_TYPE_INTERPRETER,
                                             G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
                                             closure,
                                             single_handler_accumulator,
                                             NULL,
                                             cattle_marshal_BOOLEAN__CHAR_POINTER,
                                             G_TYPE_BOOLEAN,
                                             2,
                                             ptypes);

	/**
	 * CattleInterpreter::debug-request:
	 * @interpreter: a #CattleInterpreter
	 * @error: a #GError used for error reporting, or %NULL
	 *
	 * Emitted whenever debugging information are needed.
	 *
	 * If the operation fails, @error has to be filled with
	 * detailed information about the error.
	 *
	 * Return: #TRUE if the operation is successful, #FALSE
	 * otherwise.
	 *
	 * Since: 0.9.2
	 */
	ptypes[0] = G_TYPE_POINTER;
	closure = g_cclosure_new (G_CALLBACK (debug_default_handler),
	                          NULL,
	                          NULL);
	signals[DEBUG_REQUEST] = g_signal_newv ("debug-request",
	                                        CATTLE_TYPE_INTERPRETER,
	                                        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE,
	                                        closure,
	                                        single_handler_accumulator,
	                                        NULL,
	                                        cattle_marshal_BOOLEAN__POINTER,
	                                        G_TYPE_BOOLEAN,
	                                        1,
	                                        ptypes);

	g_type_class_add_private (object_class,
	                          sizeof (CattleInterpreterPrivate));
}
