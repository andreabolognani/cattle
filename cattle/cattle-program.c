/* Cattle -- Brainfuck language toolkit
 * Copyright (C) 2008-2011  Andrea Bolognani <eof@kiyuko.org>
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
	gchar             *input;
};

/* Properties */
enum
{
	PROP_0,
	PROP_INSTRUCTIONS,
	PROP_INPUT
};

/* Internal functions */
static CattleInstruction* load (gchar **program);

/* Symbols used by the code loader */
#define SHARP_SYMBOL   0x23
#define BANG_SYMBOL    0x21
#define NEWLINE_SYMBOL 0x0A

static void
cattle_program_init (CattleProgram *self)
{
	self->priv = CATTLE_PROGRAM_GET_PRIVATE (self);

	self->priv->instructions = cattle_instruction_new ();
	self->priv->input = NULL;

	self->priv->disposed = FALSE;
}

static void
cattle_program_dispose (GObject *object)
{
	CattleProgram *self = CATTLE_PROGRAM (object);

	g_return_if_fail (!self->priv->disposed);

	g_object_unref (self->priv->instructions);
	self->priv->instructions = NULL;

	self->priv->disposed = TRUE;

	G_OBJECT_CLASS (cattle_program_parent_class)->dispose (object);
}

static void
cattle_program_finalize (GObject *object)
{
	CattleProgram *self = CATTLE_PROGRAM (object);

	g_free (self->priv->input);
	self->priv->input = NULL;

	G_OBJECT_CLASS (cattle_program_parent_class)->finalize (object);
}

static CattleInstruction*
load (gchar  **program)
{
	CattleInstruction *first;
	CattleInstruction *current;
	CattleInstruction *previous;
	CattleInstruction *loop;
	gint quantity;
	gunichar value;
	gunichar temp;

	first = NULL;
	previous = NULL;

	do {

		current = NULL;

		/* Pick the first symbol from the input string */
		value = g_utf8_get_char (*program);
		quantity = 1;

		/* End of input */
		if (value == 0) {
			break;
		}

		/* Move one position to the right */
		*program = g_utf8_next_char (*program);

		/* Start of program's input */
		if (value == BANG_SYMBOL) {
			break;
		}

		/* Read a sequence of identical symbols, counting them. Don't
		 * do that for loop instructions, those can't be optimized */
		if (value != CATTLE_INSTRUCTION_LOOP_BEGIN &&
		    value != CATTLE_INSTRUCTION_LOOP_END)
		{
			do {
				temp = g_utf8_get_char (*program);

				/* Same value: increase the quantity, move the
				 * pointer to the right */
				if (temp == value) {
					quantity++;
					*program = g_utf8_next_char (*program);
				}
				else {
					break;
				}
			} while (temp == value);
		}

		g_assert (value == CATTLE_INSTRUCTION_LOOP_BEGIN ||
		          value == CATTLE_INSTRUCTION_LOOP_END ||
		          value != g_utf8_get_char (*program));

		/* Normalize symbol: if value is not a recognized symbol,
		 * set it to CATTLE_INSTRUCTION_NONE */
		switch (value) {

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

		/* Not an instruction, restart the loop */
		if (value == CATTLE_INSTRUCTION_NONE) {
			continue;
		} 

		/* Create a new instruction and set its value and quantity */
		current = cattle_instruction_new ();

		cattle_instruction_set_value (current, value);
		cattle_instruction_set_quantity (current, quantity);

		/* This is the first instruction */
		if (first == NULL) {
			first = current;
			g_object_ref (first);
		}

		if (value == CATTLE_INSTRUCTION_LOOP_BEGIN) {

			/* Read the loop's contents */
			loop = load (program);

			g_assert (loop != NULL);

			if (loop != NULL) {
				cattle_instruction_set_loop (current, loop);
				g_object_unref (loop);
			}
		}

		/* Link the current instruction to the previous one (if any) */
		if (previous != NULL) {
			cattle_instruction_set_next (previous, current);
			g_object_unref (previous);
		}
		previous = current;

		/* Exit on loop end */
		if (value == CATTLE_INSTRUCTION_LOOP_END) {
			break;
		}

	} while (value != 0);

	/* Drop the remaining reference to the current instruction, or
	 * if no current instruction is present, to the previous one */
	if (current != NULL) {
		g_object_unref (current);
	}
	else {
		if (previous != NULL) {
			g_object_unref (previous);
		}
	}

	return first;
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
 * @string: the source code of the program
 * @error: (allow-none): return location for a #GError
 *
 * Load @program from @string.
 *
 * The string can optionally contain also the input for the program:
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
                     const gchar    *program,
                     GError        **error)
{
	CattleInstruction *instructions;
	GError *inner_error = NULL;
	gchar *position;
	gunichar temp;
	glong brackets_count = 0;

	g_return_val_if_fail (CATTLE_IS_PROGRAM (self), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (!self->priv->disposed, FALSE);

	/* Check the provided string is valid UTF-8 before proceeding */
	if (!g_utf8_validate (program, -1, NULL)) {
		g_set_error (error,
		             CATTLE_ERROR,
		             CATTLE_ERROR_BAD_UTF8,
		             "Invalid UTF-8");
		return FALSE;
	}

	/* Check the number of brackets to ensure the loops are balanced */
	position = (gchar *) program;
	do {

		temp = g_utf8_get_char (position);

		if (temp == CATTLE_INSTRUCTION_LOOP_BEGIN) {
			brackets_count++;
		}
		else if (temp == CATTLE_INSTRUCTION_LOOP_END) {
			brackets_count--;
		}

		/* Ignore brackets in the program's input, if present */
		if (temp != 0 && temp != BANG_SYMBOL) {
			position = g_utf8_next_char (position);
		}
	} while (temp != 0 && temp != BANG_SYMBOL);

	/* Report an error to the caller if the number of open brackets
	 * is not equal to the number of closed brackets */
	if (brackets_count != 0) {
		g_set_error (error,
		             CATTLE_ERROR,
		             CATTLE_ERROR_UNBALANCED_BRACKETS,
		             "Unbalanced brackets");
		return FALSE;
	}

	/* Load the instructions from the string */
	position = (gchar *) program;
	instructions = load ((gchar **) &position);

	/* The load routine returns NULL for the empty program.
	 * Create a single instruction to use in the program */
	if (instructions == NULL) {
		instructions = cattle_instruction_new ();
	}

	/* Set the instructions for the program */
	cattle_program_set_instructions (self, instructions);
	g_object_unref (instructions);

	/* Set the input for the program, if present; otherwise,
	 * reset it */
	if (g_utf8_strlen (position, -1) > 0) {
		cattle_program_set_input (self, position);
	}
	else {
		cattle_program_set_input (self, NULL);
	}

	return TRUE;
}

/**
 * cattle_program_set_instructions:
 * @program: a #CattleProgram
 * @instructions: instructions for @program
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
	g_return_if_fail (CATTLE_IS_PROGRAM (self));
	g_return_if_fail (CATTLE_IS_INSTRUCTION (instructions));
	g_return_if_fail (!self->priv->disposed);

	/* Release the reference held on the current instructions */
	g_object_unref (self->priv->instructions);

	self->priv->instructions = instructions;
	g_object_ref (self->priv->instructions);
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
	g_return_val_if_fail (CATTLE_IS_PROGRAM (self), NULL);
	g_return_val_if_fail (!self->priv->disposed, NULL);

	g_object_ref (self->priv->instructions);

	return self->priv->instructions;
}

/**
 * cattle_program_set_input:
 * @program: a #CattleProgram
 * @input: (allow-none): input for @program, or %NULL
 *
 * Set the input for @program.
 *
 * If @input is %NULL, the input will be retrieved at runtime.
 */
void
cattle_program_set_input (CattleProgram *self,
                          const gchar   *input)
{
	g_return_if_fail (CATTLE_IS_PROGRAM (self));
	g_return_if_fail (!self->priv->disposed);

	/* Free the existing input */
	g_free (self->priv->input);

	self->priv->input = g_strdup (input);
}

/**
 * cattle_program_get_input:
 * @program: a #CattleProgram
 *
 * Get the input for @program.
 * See cattle_program_set_input().
 *
 * Returns: (transfer full): input for @program, or %NULL
 */
gchar*
cattle_program_get_input (CattleProgram *self)
{
	g_return_val_if_fail (CATTLE_IS_PROGRAM (self), NULL);
	g_return_val_if_fail (!self->priv->disposed, NULL);

	return g_strdup (self->priv->input);
}

static void
cattle_program_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	CattleProgram *self = CATTLE_PROGRAM (object);
	CattleInstruction *t_inst;
	gchar *t_str;

	g_return_if_fail (!self->priv->disposed);

	switch (property_id) {

		case PROP_INSTRUCTIONS:
			t_inst = g_value_get_object (value);
			cattle_program_set_instructions (self,
			                                 t_inst);
			break;

		case PROP_INPUT:
			t_str = (gchar *) g_value_get_string (value);
			cattle_program_set_input (self,
			                          t_str);
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
	CattleProgram *self = CATTLE_PROGRAM (object);
	CattleInstruction *t_inst;
	gchar *t_str;

	g_return_if_fail (!self->priv->disposed);

	switch (property_id) {

		case PROP_INSTRUCTIONS:
			t_inst = cattle_program_get_instructions (self);
			g_value_set_object (value, t_inst);
			break;

		case PROP_INPUT:
			t_str = cattle_program_get_input (self);
			g_value_set_string (value, t_str);
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
	GObjectClass *object_class = G_OBJECT_CLASS (self);
	GParamSpec *pspec;

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
	                             "Get/set instruction",
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
	pspec = g_param_spec_string ("input",
	                             "Input for the program",
	                             "Get/set program's input",
	                             NULL,
	                             G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_INPUT,
	                                 pspec);

	g_type_class_add_private (object_class,
                              sizeof (CattleProgramPrivate));
}
