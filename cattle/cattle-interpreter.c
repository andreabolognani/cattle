/* Cattle -- Brainfuck language toolkit
 * Copyright (C) 2008-2014  Andrea Bolognani <eof@kiyuko.org>
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
static gboolean run                        (CattleInterpreter      *interpreter,
                                            GError                **error);
static gboolean default_input_handler      (CattleInterpreter      *interpreter,
                                            gpointer                data,
                                            GError                **error);
static gboolean default_output_handler     (CattleInterpreter      *interpreter,
                                            gint8                   output,
                                            gpointer                data,
                                            GError                **error);
static gboolean default_debug_handler      (CattleInterpreter      *interpreter,
                                            gpointer                data,
                                            GError                **error);

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
	CattleInterpreter *self;

	self = CATTLE_INTERPRETER (object);

	if (self->priv->input != NULL)
	{
		g_object_unref (self->priv->input);
	}

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

					if (priv->end_of_input_reached)
					{
						temp = CATTLE_EOF;
					}
					else
					{
						size = cattle_buffer_get_size (priv->input);

						if (priv->input_offset < size)
						{
							temp = cattle_buffer_get_value (priv->input,
											priv->input_offset);
							priv->input_offset++;
						}
						else
						{
							if (priv->had_input)
							{
								temp = CATTLE_EOF;
								priv->end_of_input_reached = TRUE;
							}
							else
							{
								inner_error = NULL;
								success = (*input_handler) (self,
											    priv->input_handler_data,
											    &inner_error);
								success &= (inner_error == NULL);

								/* The operation failed: we abort
								 * immediately */
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

								size = cattle_buffer_get_size (priv->input);

								if (priv->input_offset < size)
								{
									temp = cattle_buffer_get_value (priv->input,
													priv->input_offset);
									priv->input_offset++;
								}
								else
								{
									temp = CATTLE_EOF;
									priv->end_of_input_reached = TRUE;
								}
							}
						}
					}
				}

				/* STEP 3: Perform the actual operation */

				/* XXX If there are multiple read instructions in a row,
				 *     only the last one is actually performed, because
				 *     it would overwrite the results of all the previous
				 *     ones anyway.
				 */

				/* We are at the end of the input: we have to check
				 * the configuration to know which action we should
				 * perform */
				if (temp == CATTLE_EOF)
				{
					switch (cattle_configuration_get_on_eof_action (configuration))
					{

						case CATTLE_ON_EOF_STORE_ZERO:

							cattle_tape_set_current_value (tape, 0);
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
				else
				{
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
	if (priv->had_input == TRUE)
	{
		g_object_unref (priv->input);
	}

	return success;
}

/**
 * cattle_interpreter_feed:
 * @interpreter: a #CattleInterpreter
 * @input: (allow-none): more input to be used by @interpreter, or %NULL
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
	g_return_if_fail (CATTLE_IS_BUFFER (input) || input == NULL);

	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	/* A return value of NULL from the signal
	 * handler means the end of input was
	 * reached. We set the appropriate flag */
	if (input == NULL)
	{
		priv->input = NULL;
		priv->input_offset = 0;
		priv->end_of_input_reached = TRUE;

		return;
	}

	/* Release the previous input buffer */
	if (priv->input != NULL)
	{
		g_object_unref (priv->input);
	}

	priv->input = input;
	g_object_ref (priv->input);

	priv->input_offset = 0;
	priv->end_of_input_reached = FALSE;
}

/**
 * cattle_interpreter_set_configuration:
 * @interpreter: a #CattleInterpreter
 * @configuration: configuration for @interpreter
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
 * Returns: (transfer full): configuration for @interpreter
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
 * Get the program for @interpreter.
 * See cattle_interpreter_set_program().
 *
 * Returns: (transfer full): the program for @interpreter
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
 * @tape: a #CattleTape
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
 * Returns: (transfer full): the memory tape for @interpreter
 */
CattleTape*
cattle_interpreter_get_tape (CattleInterpreter *self)
{
	g_return_val_if_fail (CATTLE_IS_INTERPRETER (self), NULL);
	g_return_val_if_fail (!self->priv->disposed, NULL);

	g_object_ref (self->priv->tape);

	return self->priv->tape;
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
	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (!self->priv->disposed);

	self->priv->input_handler = handler;
	self->priv->input_handler_data = user_data;
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
	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (!self->priv->disposed);

	self->priv->output_handler = handler;
	self->priv->output_handler_data = user_data;
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
	g_return_if_fail (CATTLE_IS_INTERPRETER (self));
	g_return_if_fail (!self->priv->disposed);

	self->priv->debug_handler = handler;
	self->priv->debug_handler_data = user_data;
}

static gboolean
default_input_handler (CattleInterpreter  *self,
                       gpointer            data,
                       GError            **error)
{
	CattleBuffer *input;
	gint8         buffer[256];
	gulong        size;

	size = read (0, buffer, 256);

	input = cattle_buffer_new (size);
	cattle_buffer_set_contents (input, (gint8 *) buffer);

	cattle_interpreter_feed (self, input);

	g_object_unref (input);

#if 0
	/* The buffer size is not really important: if the input cannot
	 * fit a single buffer, the signal will be emitted again */
	size = 256;

	buffer = (gchar *) g_slice_alloc0 (size);

	if (fgets (buffer, size, stdin) == NULL)
	{
		/* A NULL return value from fgets could either mean a read
		 * error has occurred or the end of input has been reached.
		 * In the latter case, we have to notify the interpreter */
		if (G_LIKELY (feof (stdin)))
		{
			cattle_interpreter_feed (self, NULL);

			g_slice_free1 (size, buffer);

			return TRUE;
		}
		else
		{
			g_set_error_literal (error,
			                     CATTLE_ERROR,
			                     CATTLE_ERROR_IO,
			                     strerror (errno));

			g_slice_free1 (size, buffer);

			return FALSE;
		}
	}
#endif

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

	write (1, buffer, 1);

	/*
	if (G_UNLIKELY (fputc ((char) output, stdout) == EOF)) {

		g_set_error_literal (error,
		                     CATTLE_ERROR,
		                     CATTLE_ERROR_IO,
		                     strerror (errno));
		return FALSE;
	}
	*/

	return TRUE;
}

static gboolean
default_debug_handler (CattleInterpreter  *self,
                       gpointer            data,
                       GError            **error)
{
	CattleTape *tape;
	gint8 value;
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
		if (g_ascii_isgraph ((gchar) value)) {
			if (G_UNLIKELY (fputc ((char) value, stderr) == EOF)) {
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
			if (G_UNLIKELY (fprintf (stderr, "0x%X", value) < 0)) {
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
