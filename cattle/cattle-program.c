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
#include "cattle-program.h"

/**
 * SECTION:cattle-program
 * @short_description: Brainfuck program (and possibly its input)
 *
 * A #CattleProgram represents a complete Brainfuck program, that is,
 * the instructions to be executed and possibly its input.
 *
 * The input for a program can optionally be specified in the source
 * file, and it's separated from the program code by a bang (!)
 * symbol. For example, given the following input:
 *
 * |[
 * ,+.!sometext
 * ]|
 *
 * the program's code is ",+." while the program's input is
 * "sometext".
 *
 * Any Brainfuck instruction after the bang symbol is considered part
 * of the input, and as such is not executed. Subsequent bang symbols
 * are also considered part of the input.
 */

G_DEFINE_TYPE (CattleProgram, cattle_program, G_TYPE_OBJECT)

/**
 * CattleProgram:
 *
 * Opaque data structure representing a program. It should never be
 * accessed directly; use the methods below instead.
 */

#define CATTLE_PROGRAM_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), CATTLE_TYPE_PROGRAM, CattleProgramPrivate))

struct _CattleProgramPrivate
{
	gboolean           disposed;

	CattleInstruction *instructions;
	CattleBuffer      *input;
};

/* Properties */
enum
{
	PROP_0,
	PROP_INSTRUCTIONS,
	PROP_INPUT
};

/* Internal functions */
static gulong load (CattleBuffer       *buffer,
                    gulong              offset,
                    CattleInstruction **instructions,
                    CattleBuffer      **input);

/* Symbols used by the code loader */
#define BANG_SYMBOL    0x21 /*  !  */
#define NEWLINE_SYMBOL 0x0A /* \n  */

static void
cattle_program_init (CattleProgram *self)
{
	CattleProgramPrivate *priv;

	priv = CATTLE_PROGRAM_GET_PRIVATE (self);

	priv->instructions = cattle_instruction_new ();
	priv->input = cattle_buffer_new (0);

	priv->disposed = FALSE;

	self->priv = priv;
}

static void
cattle_program_dispose (GObject *object)
{
	CattleProgram        *self;
	CattleProgramPrivate *priv;

	self = CATTLE_PROGRAM (object);

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	g_object_unref (priv->instructions);
	g_object_unref (priv->input);

	priv->disposed = TRUE;

	G_OBJECT_CLASS (cattle_program_parent_class)->dispose (object);
}

static void
cattle_program_finalize (GObject *object)
{
	G_OBJECT_CLASS (cattle_program_parent_class)->finalize (object);
}

static gulong
load (CattleBuffer       *buffer,
      gulong              offset,
      CattleInstruction **instructions,
      CattleBuffer      **input)
{
	CattleInstruction *first;
	CattleInstruction *current;
	CattleInstruction *previous;
	CattleInstruction *loop;
	gint8              value;
	gint8              temp;
	gulong             quantity;
	gulong             size;
	gulong             i;
	gulong             c;

	first = NULL;
	previous = NULL;

	i = offset;
	size = cattle_buffer_get_size (buffer);

	while (i < size)
	{
		current = NULL;

		/* Read a value from the input buffer */
		value = cattle_buffer_get_value (buffer, i);
		quantity = 1;

		/* Start of program's input, stop parsing */
		if (value == BANG_SYMBOL)
		{
			i++;
			break;
		}

		/* Normalize symbol: if value is not a recognized symbol,
		 * set it to CATTLE_INSTRUCTION_NONE */
		switch (value)
		{
			case CATTLE_INSTRUCTION_INCREASE:
			case CATTLE_INSTRUCTION_DECREASE:
			case CATTLE_INSTRUCTION_MOVE_LEFT:
			case CATTLE_INSTRUCTION_MOVE_RIGHT:
			case CATTLE_INSTRUCTION_LOOP_BEGIN:
			case CATTLE_INSTRUCTION_LOOP_END:
			case CATTLE_INSTRUCTION_READ:
			case CATTLE_INSTRUCTION_PRINT:
			case CATTLE_INSTRUCTION_DEBUG:

				break;

			default:

				value = CATTLE_INSTRUCTION_NONE;
				break;
		}

		/* Not an instruction, move on */
		if (value == CATTLE_INSTRUCTION_NONE)
		{
			i++;
			continue;
		} 

		/* Read a sequence of identical symbols, counting them.
		 * Loops can't be optimized this way */
		if (value != CATTLE_INSTRUCTION_LOOP_BEGIN &&
		    value != CATTLE_INSTRUCTION_LOOP_END)
		{
			while (i + 1 < size)
			{
				temp = cattle_buffer_get_value (buffer, i + 1);

				if (temp == value)
				{
					/* Same value: increase the quantity */
					quantity++;
					i++;
				}
				else
				{
					break;
				}
			}
		}

		/* Create a new instruction */
		current = cattle_instruction_new ();

		cattle_instruction_set_value (current, value);
		cattle_instruction_set_quantity (current, quantity);

		if (value == CATTLE_INSTRUCTION_LOOP_BEGIN)
		{
			/* Parse the loop */
			i = load (buffer,
			          i + 1,
			          &loop,
			          NULL);
			i--;

			cattle_instruction_set_loop (current, loop);
			g_object_unref (loop);
		}

		if (first == NULL)
		{
			first = current;

			/* Acquire an extra reference to the first
			 * instruction to make sure the whole loop
			 * is kept alive */
			g_object_ref (first);
		}

		if (previous != NULL)
		{
			/* Link the current instruction to the previous one */
			cattle_instruction_set_next (previous, current);
		}

		previous = current;
		g_object_unref (current);

		/* Move to the next byte */
		i++;

		/* Exit on loop end */
		if (value == CATTLE_INSTRUCTION_LOOP_END)
		{
			break;
		}
	}

	if (first == NULL)
	{
		/* Empty branch. Create a no-op */
		first = cattle_instruction_new ();
	}

	*instructions = first;

	/* Collect any input */
	if (input != NULL)
	{
		if (i < size)
		{
			*input = cattle_buffer_new (size - i + 1);

			c = 0;
			while (i < size)
			{
				value = cattle_buffer_get_value (buffer, i);
				cattle_buffer_set_value (*input, c, value);

				c++;
				i++;
			}
		}
		else
		{
			*input = cattle_buffer_new (0);
		}
	}

	return i;
}

/**
 * cattle_program_new:
 *
 * Create a new #CattleProgram.
 *
 * A single instance of a program can be shared between multiple
 * interpreters, as long as the object is not modified after it
 * has been initialized.
 *
 * Returns: (transfer full): a new #CattleProgram
 **/
CattleProgram*
cattle_program_new (void)
{
	return g_object_new (CATTLE_TYPE_PROGRAM, NULL);
}

/**
 * cattle_program_load:
 * @program: a #CattleProgram
 * @buffer: (transfer full): a #CattleBuffer containing the code
 * @error: (allow-none): return location for a #GError
 *
 * Load @program from @buffer.
 *
 * The buffer can optionally contain also the input for the program:
 * in that case, the input must be separated from the code by a bang
 * (!) character.
 *
 * In case of failure, @error is filled with detailed information.
 * The error domain is %CATTLE_ERROR, and the error code is from the
 * #CattleError enumeration.
 *
 * Returns: %TRUE on success, %FALSE otherwise
 */
gboolean
cattle_program_load (CattleProgram  *self,
                     CattleBuffer   *buffer,
                     GError        **error)
{
	CattleProgramPrivate *priv;
	CattleInstruction    *instructions;
	CattleBuffer         *input;
	gint8                 value;
	glong                 brackets_count;
	gulong                size;
	gulong                i;

	g_return_val_if_fail (CATTLE_IS_PROGRAM (self), FALSE);
	g_return_val_if_fail (CATTLE_IS_BUFFER (buffer), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, FALSE);

	size = cattle_buffer_get_size (buffer);

	/* Check the number of brackets to ensure the loops are balanced */
	brackets_count = 0;
	for (i = 0; i < size; i++)
	{
		value = cattle_buffer_get_value (buffer, i);

		if (value == CATTLE_INSTRUCTION_LOOP_BEGIN)
		{
			brackets_count++;
		}
		else if (value == CATTLE_INSTRUCTION_LOOP_END)
		{
			brackets_count--;
		}
		else if (value == BANG_SYMBOL)
		{
			/* Brackets in the program's input should not
			 * be taken into account here */
			break;
		}
	}

	/* Report an error if the number of open brackets
	 * is not equal to the number of closed brackets */
	if (brackets_count != 0)
	{
		g_set_error (error,
		             CATTLE_ERROR,
		             CATTLE_ERROR_UNBALANCED_BRACKETS,
		             "Unbalanced brackets");
		return FALSE;
	}

	/* Parse the program */
	load (buffer,
	      0,
	      &instructions,
	      &input);

	/* Set instructions and input */
	cattle_program_set_instructions (self, instructions);
	cattle_program_set_input (self, input);

	g_object_unref (instructions);
	g_object_unref (input);

	return TRUE;
}

/**
 * cattle_program_set_instructions:
 * @program: a #CattleProgram
 * @instructions: (transfer full): instructions for @program
 *
 * Set the instructions for @program.
 *
 * You shouldn't usually need to use this: see cattle_program_load()
 * for the standard way to load a program.
 */
void
cattle_program_set_instructions (CattleProgram     *self,
                                 CattleInstruction *instructions)
{
	CattleProgramPrivate *priv;

	g_return_if_fail (CATTLE_IS_PROGRAM (self));
	g_return_if_fail (CATTLE_IS_INSTRUCTION (instructions));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Release the reference held on the current instructions */
	g_object_unref (priv->instructions);

	priv->instructions = instructions;
	g_object_ref (priv->instructions);
}

/**
 * cattle_program_get_instructions:
 * @program: a #CattleProgram
 *
 * Get the instructions for @program.
 * See cattle_program_load() and cattle_program_set_instructions().
 *
 * Returns: (transfer full): the first instruction in @program
 */
CattleInstruction*
cattle_program_get_instructions (CattleProgram *self)
{
	CattleProgramPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_PROGRAM (self), NULL);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, NULL);

	/* Increase the reference count */
	g_object_ref (priv->instructions);

	return priv->instructions;
}

/**
 * cattle_program_set_input:
 * @program: a #CattleProgram
 * @input: (transfer full): input for @program
 *
 * Set the input for @program.
 *
 * If the size of @input is zero, the program's input will be retrieved
 * at runtime.
 */
void
cattle_program_set_input (CattleProgram *self,
                          CattleBuffer  *input)
{
	CattleProgramPrivate *priv;

	g_return_if_fail (CATTLE_IS_PROGRAM (self));
	g_return_if_fail (CATTLE_IS_BUFFER (input));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Release any existing input */
	g_object_unref (priv->input);

	priv->input = input;
	g_object_ref (priv->input);
}

/**
 * cattle_program_get_input:
 * @program: a #CattleProgram
 *
 * Get the input for @program.
 * See cattle_program_set_input().
 *
 * Returns: (transfer full): input for @program
 */
CattleBuffer*
cattle_program_get_input (CattleProgram *self)
{
	CattleProgramPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_PROGRAM (self), NULL);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, NULL);

	/* Increase the reference count */
	g_object_ref (priv->input);

	return priv->input;
}

static void
cattle_program_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	CattleProgram     *self;
	CattleInstruction *v_instruction;
	CattleBuffer      *v_buffer;

	self = CATTLE_PROGRAM (object);

	switch (property_id)
	{
		case PROP_INSTRUCTIONS:

			v_instruction = g_value_get_object (value);
			cattle_program_set_instructions (self,
			                                 v_instruction);

			break;

		case PROP_INPUT:

			v_buffer = g_value_get_object (value);
			cattle_program_set_input (self,
			                          v_buffer);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);

			break;
	}
}

static void
cattle_program_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
	CattleProgram     *self;
	CattleInstruction *v_instruction;
	CattleBuffer      *v_buffer;

	self = CATTLE_PROGRAM (object);

	switch (property_id)
	{
		case PROP_INSTRUCTIONS:

			v_instruction = cattle_program_get_instructions (self);
			g_value_set_object (value,
			                    v_instruction);

			break;

		case PROP_INPUT:

			v_buffer = cattle_program_get_input (self);
			g_value_set_object (value,
			                    v_buffer);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);

			break;
	}
}

static void
cattle_program_class_init (CattleProgramClass *self)
{
	GObjectClass *object_class;
	GParamSpec   *pspec;

	object_class = G_OBJECT_CLASS (self);

	object_class->set_property = cattle_program_set_property;
	object_class->get_property = cattle_program_get_property;
	object_class->dispose = cattle_program_dispose;
	object_class->finalize = cattle_program_finalize;

	/**
	 * CattleProgram:instructions:
	 *
	 * Instructions for the program.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_object ("instructions",
	                             "Instructions to be executed",
	                             "Get/set instructions",
	                             CATTLE_TYPE_INSTRUCTION,
	                             G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_INSTRUCTIONS,
	                                 pspec);

	/**
	 * CattleProgram:input:
	 *
	 * Input for the program, or %NULL if no input was available
	 * at the time of loading.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_object ("input",
	                             "Input for the program",
	                             "Get/set program's input",
	                             CATTLE_TYPE_BUFFER,
	                             G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_INPUT,
	                                 pspec);

	g_type_class_add_private (object_class,
	                          sizeof (CattleProgramPrivate));
}
