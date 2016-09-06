/* Cattle - Brainfuck language toolkit
 * Copyright (C) 2008-2016  Andrea Bolognani <eof@kiyuko.org>
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
#include "cattle-error.h"
#include "cattle-constants.h"
#include "cattle-interpreter.h"
#include <unistd.h>
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
 * automatically cleared between executions.
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

	CattleInputHandler   input_handler;
	gpointer             input_handler_data;
	CattleOutputHandler  output_handler;
	gpointer             output_handler_data;
	CattleDebugHandler   debug_handler;
	gpointer             debug_handler_data;

	GSList              *stack; /* Instruction stack */

	gboolean             had_input;
	CattleBuffer        *input;
	gint                 input_offset;
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

/* Internal functions */
static gboolean run                    (CattleInterpreter  *interpreter,
                                        GError            **error);
static gboolean default_input_handler  (CattleInterpreter  *interpreter,
                                        gpointer            data,
                                        GError            **error);
static gboolean default_output_handler (CattleInterpreter  *interpreter,
                                        gint8               output,
                                        gpointer            data,
                                        GError            **error);
static gboolean default_debug_handler  (CattleInterpreter  *interpreter,
                                        gpointer            data,
                                        GError            **error);

static void
cattle_interpreter_init (CattleInterpreter *self)
{
	self->priv = CATTLE_INTERPRETER_GET_PRIVATE (self);

	self->priv->configuration = cattle_configuration_new ();
	self->priv->program = cattle_program_new ();
	self->priv->tape = cattle_tape_new ();

	self->priv->input_handler = NULL;
	self->priv->input_handler_data = NULL;
	self->priv->output_handler = NULL;
	self->priv->output_handler_data = NULL;
	self->priv->debug_handler = NULL;
	self->priv->debug_handler_data = NULL;

	self->priv->stack = NULL;

	self->priv->had_input = FALSE;
	self->priv->input = NULL;
	self->priv->input_offset = 0;
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
	G_OBJECT_CLASS (cattle_interpreter_parent_class)->finalize (object);
}

static gboolean
run (CattleInterpreter  *self,
     GError            **error)
{
	CattleInterpreterPrivate *priv;
	CattleConfiguration      *configuration;
	CattleProgram            *program;
	CattleTape               *tape;
	CattleInstruction        *first;
	CattleInstruction        *current;
	CattleInstruction        *next;
	CattleInstructionValue    value;
	CattleInputHandler        input_handler;
	CattleOutputHandler       output_handler;
	CattleDebugHandler        debug_handler;
	GSList                   *stack;
	GError                   *inner_error;
	gboolean                  success;
	gint8                     temp;
	gulong                    quantity;
	gulong                    size;
	gulong                    i;

	priv = self->priv;

	configuration = priv->configuration;
	program = priv->program;
	tape = priv->tape;

	input_handler = priv->input_handler;
	if (input_handler == NULL)
	{
		input_handler = default_input_handler;
	}
	output_handler = priv->output_handler;
	if (output_handler == NULL)
	{
		output_handler = default_output_handler;
	}
	debug_handler = priv->debug_handler;
	if (debug_handler == NULL)
	{
		debug_handler = default_debug_handler;
	}

	stack = priv->stack;
	success = TRUE;

	first = cattle_program_get_instructions (program);
	g_object_ref (first);

	current = first;

	while (current != NULL)
	{
		value = cattle_instruction_get_value (current);

		switch (value)
		{
			case CATTLE_INSTRUCTION_LOOP_BEGIN:

				next = cattle_instruction_get_loop (current);

				/* Enter the loop only if the value stored in the
				 * current cell is not zero */
				if (cattle_tape_get_current_value (tape) != 0)
				{
					/* Push the current instruction on the stack */
					stack = g_slist_prepend (stack, current);
					priv->stack = stack;
					current = next;

					continue;
				}

				break;

			case CATTLE_INSTRUCTION_LOOP_END:

				/* If the instruction stack is empty, we're not running
				 * a loop, so trying to exit it is an error */
				if (G_UNLIKELY (stack == NULL))
				{
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
				priv->stack = stack;

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

				for (i = 0; i < quantity; i++)
				{
					/* Read and normalize a value */

					if (priv->end_of_input_reached)
					{
						/* End of input reached.
						 * The value will be CATTLE_EOF both for embedded and
						 * runtime input */
						temp = CATTLE_EOF;
					}
					else
					{
						size = cattle_buffer_get_size (priv->input);

						if (priv->input_offset < size)
						{
							/* Current input buffer not consumed.
							 * Get a value from the buffer and move forward */
							temp = cattle_buffer_get_value (priv->input,
											priv->input_offset);
							priv->input_offset++;
						}
						else
						{
							/* Current input buffer consumed */

							if (priv->had_input)
							{
								/* Embedded input consumed.
								 * No more input can be retrieved */
								temp = CATTLE_EOF;
								priv->end_of_input_reached = TRUE;
							}
							else
							{
								/* Runtime input buffer consumed.
								 * Call the input handler to obtain a new
								 * input buffer */
								inner_error = NULL;
								success = (*input_handler) (self,
											    priv->input_handler_data,
											    &inner_error);
								success &= (inner_error == NULL);

								/* Handle input errors */
								if (G_UNLIKELY (success == FALSE))
								{
									/* If the signal handler has set the
									 * error, as it's required to,
									 * propagate that error; if it hasn't,
									 * raise a generic I/O error */
									if (inner_error == NULL)
									{
										g_set_error_literal (error,
												     CATTLE_ERROR,
												     CATTLE_ERROR_IO,
												     "Unknown I/O error");
									}
									else
									{
										g_propagate_error (error,
												   inner_error);
									}

									g_object_unref (current);

									return FALSE;
								}

								/* Update size of the input buffer */
								size = cattle_buffer_get_size (priv->input);

								if (priv->input_offset < size)
								{
									/* Some input was retrieved */
									temp = cattle_buffer_get_value (priv->input,
													priv->input_offset);
									priv->input_offset++;
								}
								else
								{
									/* No more available input */
									temp = CATTLE_EOF;
									priv->end_of_input_reached = TRUE;
								}
							}
						}
					}
				}

				/* Save the value. Executed only once even when multiple subsequent
				 * read instruction are present in the program */

				if (temp == CATTLE_EOF)
				{
					/* End of input.
					 * The new value depends on the configuration */
					switch (cattle_configuration_get_end_of_input_action (configuration))
					{
						case CATTLE_END_OF_INPUT_ACTION_STORE_EOF:

							cattle_tape_set_current_value (tape, temp);
							break;

						case CATTLE_END_OF_INPUT_ACTION_DO_NOTHING:

							/* Do nothing */
							break;

						case CATTLE_END_OF_INPUT_ACTION_STORE_ZERO:
						default:

							cattle_tape_set_current_value (tape, 0);
							break;
					}
				}
				else
				{
					/* Not end of input.
					 * Save the new value */
					cattle_tape_set_current_value (tape, temp);
				}

				break;

			case CATTLE_INSTRUCTION_PRINT:

				quantity = cattle_instruction_get_quantity (current);

				/* Write the value in the current cell to standard
				 * output */
				for (i = 0; i < quantity; i++)
				{
					inner_error = NULL;
					success = (*output_handler) (self,
					                             cattle_tape_get_current_value (tape),
					                             priv->output_handler_data,
					                             &inner_error);
					success &= (inner_error == NULL);

					/* Stop at the first error, even if we should
					 * output the content of the current cell more
					 * than once */
					if (G_UNLIKELY (success == FALSE))
					{
						/* If the signal handler has set the error,
						 * propagate it; otherwise, raise a generic
						 * I/O error */
						if (inner_error == NULL)
						{
							g_set_error_literal (error,
							                     CATTLE_ERROR,
							                     CATTLE_ERROR_IO,
							                     "Unknown I/O error");
						}
						else
						{
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
				if (cattle_configuration_get_debug_is_enabled (configuration))
				{
					quantity = cattle_instruction_get_quantity (current);

					for (i = 0; i < quantity; i++)
					{
						inner_error = NULL;
						success = (*debug_handler) (self,
						                            priv->debug_handler_data,
						                            &inner_error);
						success &= (inner_error == NULL);

						if (G_UNLIKELY (success == FALSE))
						{
							/* If the debug handler hasn't set the
							 * error, raise a generic I/O error */
							if (inner_error == NULL)
							{
								g_set_error_literal (error,
								                     CATTLE_ERROR,
								                     CATTLE_ERROR_IO,
								                     "Unknown I/O error");
							}
							else
							{
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

	g_object_unref (first);

	/* There are some instructions left on the stack: the brackets
	 * are not balanced */
	if (stack != NULL)
	{
		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_UNBALANCED_BRACKETS,
		                     "Unbalanced brackets");

		return FALSE;
	}

	return TRUE;
}

/**
 * cattle_interpreter_new:
 *
 * Create and initialize a new interpreter.
 *
 * Returns: (transfer full): a new #CattleInterpreter
 **/
CattleInterpreter*
cattle_interpreter_new (void)
{
	return g_object_new (CATTLE_TYPE_INTERPRETER, NULL);
}

/**
 * cattle_interpreter_run:
 * @interpreter: a #CattleInterpreter
 * @error: (allow-none): return location for a #GError
 *
 * Make the interpreter run the loaded program.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 */
gboolean
cattle_interpreter_run (CattleInterpreter  *self,
                        GError            **error)
{
	CattleInterpreterPrivate *priv;
	CattleProgram            *program;
	CattleInstruction        *instruction;
	gboolean                  success;

	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, FALSE);

	/* Setup program */
	program = priv->program;

	/* Setup input */
	priv->input = cattle_program_get_input (program);

	if (cattle_buffer_get_size (priv->input) > 0)
	{
		priv->had_input = TRUE;
	}
	else
	{
		priv->had_input = FALSE;
	}
	priv->input_offset = 0;
	priv->end_of_input_reached = FALSE;

	/* Setup stack */
	priv->stack = NULL;

	/* Run program */
	success = run (self, error);

	/* Cleanup stack */
	if (priv->stack != NULL)
	{
		g_slist_free (priv->stack);
	}

	/* Cleanup input */
	g_object_unref (priv->input);

	return success;
}

/**
 * cattle_interpreter_feed:
 * @interpreter: a #CattleInterpreter
 * @input: (transfer full): more input to be used by @interpreter
 *
 * Feed @interpreter with more input.
 *
 * This method is meant to be used inside an input handler assigned to
 * @interpreter; calling it in any other context is pointless, since the
 * input is reset each time cattle_interpreter_run() is called.
 */
void
cattle_interpreter_feed (CattleInterpreter *self,
                         CattleBuffer      *input)
{
	CattleInterpreterPrivate *priv;

	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (CATTLE_IS_BUFFER (input));

	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	/* Release the previous input buffer */
	g_object_unref (priv->input);

	priv->input = input;
	g_object_ref (priv->input);

	priv->input_offset = 0;
	priv->end_of_input_reached = FALSE;
}

/**
 * cattle_interpreter_set_configuration:
 * @interpreter: a #CattleInterpreter
 * @configuration: (transfer full): configuration for @interpreter
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
	CattleInterpreterPrivate *priv;

	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (CATTLE_IS_CONFIGURATION (configuration));

	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	/* Release the reference held on the previous
	 * configuration */
	g_object_unref (priv->configuration);

	priv->configuration = configuration;
	g_object_ref (priv->configuration);
}

/**
 * cattle_interpreter_get_configuration:
 * @interpreter: a #CattleInterpreter
 *
 * Get the configuration for @interpreter.
 * See cattle_interpreter_set_configuration().
 *
 * Returns: (transfer full): configuration for @interpreter
 */
CattleConfiguration*
cattle_interpreter_get_configuration (CattleInterpreter *self)
{
	CattleInterpreterPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), NULL);

	priv = self->priv;

	g_return_val_if_fail (!priv->disposed, NULL);

	g_object_ref (priv->configuration);

	return priv->configuration;
}

/**
 * cattle_interpreter_set_program:
 * @interpreter: a #CattleInterpreter
 * @program: (transfer full): a #CattleProgram
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
	CattleInterpreterPrivate *priv;

	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (CATTLE_IS_PROGRAM (program));

	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	/* Release the reference held to the previous program */
	g_object_unref (priv->program);

	priv->program = program;
	g_object_ref (priv->program);
}

/**
 * cattle_interpreter_get_program:
 * @interpreter: a #CattleInterpreter
 *
 * Get the program for @interpreter.
 * See cattle_interpreter_set_program().
 *
 * Returns: (transfer full): the program for @interpreter
 */
CattleProgram*
cattle_interpreter_get_program (CattleInterpreter *self)
{
	CattleInterpreterPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), NULL);

	priv = self->priv;

	g_return_val_if_fail (!priv->disposed, NULL);

	g_object_ref (priv->program);

	return priv->program;
}

/**
 * cattle_interpreter_set_tape:
 * @interpreter: a #CattleInterpreter
 * @tape: (transfer full): a #CattleTape
 *
 * Set the memory tape used by @interpreter.
 */
void
cattle_interpreter_set_tape (CattleInterpreter *self,
                             CattleTape        *tape)
{
	CattleInterpreterPrivate *priv;

	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (CATTLE_IS_TAPE (tape));

	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	/* Release the reference held to the previous tape */
	g_object_unref (priv->tape);

	priv->tape = tape;
	g_object_ref (priv->tape);
}

/**
 * cattle_interpreter_get_tape:
 * @interpreter: a #CattleInterpreter
 *
 * Get the memory tape used by @interpreter.
 * See cattle_interpreter_set_tape().
 *
 * Returns: (transfer full): the memory tape for @interpreter
 */
CattleTape*
cattle_interpreter_get_tape (CattleInterpreter *self)
{
	CattleInterpreterPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), NULL);

	priv = self->priv;

	g_return_val_if_fail (!priv->disposed, NULL);

	g_object_ref (priv->tape);

	return priv->tape;
}

/**
 * CattleInputHandler:
 * @interpreter: a #CattleInterpreter
 * @data: user data passed to the handler
 * @error: return location for a #GError
 *
 * Handler for an input operation.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 */

/**
 * cattle_interpreter_set_input_handler:
 * @interpreter: a #CattleInterpreter
 * @handler: (scope notified) (allow-none): input handler, or %NULL
 * @user_data: (allow-none): user data for @handler
 *
 * Set the input handler for @interpreter.
 *
 * The handler will be invoked every time @interpreter needs to perform
 * an input action; if @handler is %NULL, the default input handler will
 * be used.
 */
void
cattle_interpreter_set_input_handler (CattleInterpreter  *self,
                                      CattleInputHandler  handler,
                                      gpointer            user_data)
{
	CattleInterpreterPrivate *priv;

	g_return_if_fail (CATTLE_IS_INTERPRETER (self));

	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	priv->input_handler = handler;
	priv->input_handler_data = user_data;
}

/**
 * CattleOutputHandler:
 * @interpreter: a #CattleInterpreter
 * @output: a #gchar to output
 * @data: user data passed to the handler
 * @error: return location for a #GError
 *
 * Handler for an output operation.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 */

/**
 * cattle_interpreter_set_output_handler:
 * @interpreter: a #CattleInterpreter
 * @handler: (scope notified) (allow-none): output handler, or %NULL
 * @user_data: (allow-none): user data for @handler
 *
 * Set the output handler for @interpreter.
 *
 * The handler will be invoked every time @interpreter needs to perform
 * an output action; if @handler is %NULL, the default output handler will
 * be used.
 */
void
cattle_interpreter_set_output_handler (CattleInterpreter   *self,
                                       CattleOutputHandler  handler,
                                       gpointer             user_data)
{
	CattleInterpreterPrivate *priv;

	g_return_if_fail (CATTLE_IS_INTERPRETER (self));

	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	priv->output_handler = handler;
	priv->output_handler_data = user_data;
}

/**
 * CattleDebugHandler:
 * @interpreter: a #CattleInterpreter
 * @data: user data passed to the handler
 * @error: return location for a #GError
 *
 * Handler for a debug operation.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 */

/**
 * cattle_interpreter_set_debug_handler:
 * @interpreter: a #CattleInterpreter
 * @handler: (scope notified) (allow-none): debug handler, or %NULL
 * @user_data: (allow-none): user data for @handler
 *
 * Set the debug handler for @interpreter.
 *
 * The handler will be invoked every time @interpreter needs to perform
 * a debug action; if @handler is %NULL, the default debug handler will
 * be used.
 */
void
cattle_interpreter_set_debug_handler (CattleInterpreter  *self,
                                      CattleDebugHandler  handler,
                                      gpointer            user_data)
{
	CattleInterpreterPrivate *priv;

	g_return_if_fail (CATTLE_IS_INTERPRETER (self));

	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	priv->debug_handler = handler;
	priv->debug_handler_data = user_data;
}

static gboolean
default_input_handler (CattleInterpreter  *self,
                       gpointer            data,
                       GError            **error)
{
	CattleBuffer *input;
	gint8         buffer[256];
	gssize        size;

	size = read (0, buffer, 256);

	if (size < 0)
	{
		g_set_error_literal (error,
				     CATTLE_ERROR,
				     CATTLE_ERROR_IO,
				     strerror (errno));

		return FALSE;
	}

	/* Copy the contents to a buffer */
	input = cattle_buffer_new ((gulong) size);
	cattle_buffer_set_contents (input, (gint8 *) buffer);

	/* Feed the buffer to the interpreter */
	cattle_interpreter_feed (self, input);

	g_object_unref (input);

	return TRUE;
}

static gboolean
default_output_handler (CattleInterpreter  *self,
                        gint8               output,
                        gpointer            data,
                        GError            **error)
{
	gint8 buffer[1];

	buffer[0] = output;

	if (G_UNLIKELY (write (1, buffer, 1) < 0))
	{
		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_IO,
		                     strerror (errno));
		return FALSE;
	}

	return TRUE;
}

static gboolean
default_debug_handler (CattleInterpreter  *self,
                       gpointer            data,
                       GError            **error)
{
	CattleTape *tape;
	gint8       buffer[4];
	gint8       value;
	gulong      size;
	gulong      steps;

	tape = cattle_interpreter_get_tape (self);

	/* Save the current position so it can be restored later */
	cattle_tape_push_bookmark (tape);

	/* Move to the beginning of the tape, counting how many steps
	 * it takes to get there. This value will be used later to mark
	 * the current position */
	steps = 0;
	while (TRUE)
	{
		if (cattle_tape_is_at_beginning (tape))
		{
			break;
		}

		cattle_tape_move_left (tape);
		steps++;
	}

	buffer[0] = '[';
	if (G_UNLIKELY (write (2, buffer, 1) < 0))
	{
		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_IO,
		                     strerror (errno));

		cattle_tape_pop_bookmark (tape);
		g_object_unref (tape);

		return FALSE;
	}

	while (TRUE)
	{
		/* Mark the current position */
		if (steps == 0)
		{
			buffer[0] = '<';
			if (G_UNLIKELY (write (2, buffer, 1) < 0))
			{
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
		if (g_ascii_isgraph ((gchar) value))
		{
			buffer[0] = value;
			if (G_UNLIKELY (write (2, buffer, 1) < 0))
			{
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
			size = snprintf ((gchar *) buffer, 4, "0x%X", value);

			if (G_UNLIKELY (write (2, buffer, size) < 0)) {
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
		if (steps == 0)
		{
			buffer[0] = '>';
			if (G_UNLIKELY (write (2, buffer, 1) < 0))
			{
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
		if (cattle_tape_is_at_end (tape))
		{
			break;
		}

		/* Print a space and move forward */
		buffer[0] = ' ';
		if (G_UNLIKELY (write (2, buffer, 1) < 0))
		{
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

	buffer[0] = ']';
	if (G_UNLIKELY (write (2, buffer, 1) < 0))
	{
		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_IO,
		                     strerror (errno));

		cattle_tape_pop_bookmark (tape);
		g_object_unref (tape);

		return FALSE;
	}

	buffer[0] = '\n';
	if (G_UNLIKELY (write (2, buffer, 1) < 0))
	{
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

static void
cattle_interpreter_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
	CattleInterpreter        *self;
	CattleInterpreterPrivate *priv;
	CattleConfiguration      *v_conf;
	CattleProgram            *v_prog;
	CattleTape               *v_tape;

	self = CATTLE_INTERPRETER (object);
	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	switch (property_id)
	{
		case PROP_CONFIGURATION:

			v_conf = g_value_get_object (value);
			cattle_interpreter_set_configuration (self,
							      v_conf);

			break;

		case PROP_PROGRAM:

			v_prog = g_value_get_object (value);
			cattle_interpreter_set_program (self,
							v_prog);

			break;

		case PROP_TAPE:

			v_tape = g_value_get_object (value);
			cattle_interpreter_set_tape (self,
						     v_tape);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
							   property_id,
							   pspec);

			break;
	}
}

static void
cattle_interpreter_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
	CattleInterpreter        *self;
	CattleInterpreterPrivate *priv;
	CattleConfiguration      *v_conf;
	CattleProgram            *v_prog;
	CattleTape               *v_tape;

	self = CATTLE_INTERPRETER (object);
	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	switch (property_id)
	{
		case PROP_CONFIGURATION:

			v_conf = cattle_interpreter_get_configuration (self);
			g_value_set_object (value, v_conf);

			break;

		case PROP_PROGRAM:

			v_prog = cattle_interpreter_get_program (self);
			g_value_set_object (value, v_prog);

			break;

		case PROP_TAPE:

			v_tape = cattle_interpreter_get_tape (self);
			g_value_set_object (value, v_tape);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);

			break;
	}
}

static void
cattle_interpreter_class_init (CattleInterpreterClass *self)
{
	GObjectClass *object_class;
	GParamSpec   *pspec;

	object_class = G_OBJECT_CLASS (self);

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

	g_type_class_add_private (object_class,
	                          sizeof (CattleInterpreterPrivate));
}
