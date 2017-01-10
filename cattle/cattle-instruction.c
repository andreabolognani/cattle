/* Cattle - Brainfuck language toolkit
 * Copyright (C) 2008-2017  Andrea Bolognani <eof@kiyuko.org>
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
#include "cattle-instruction.h"

/**
 * SECTION:cattle-instruction
 * @short_description: Brainfuck instruction
 *
 * A #CattleInstruction represents a single Brainfuck instruction,
 * repeated one or more times in a row.
 *
 * Multiple instructions of the same type (i.e. multiple increment
 * instruction) are grouped together to reduce memory usage and speed
 * up execution.
 *
 * Consider the following piece of Brainfuck code:
 *
 * |[
 * +++.-----
 * ]|
 *
 * There are nine instructions: three increment instructions, a print
 * instruction, and five decrement instructions.
 *
 * Instead of creating nine separate objects, with all the involved
 * overhead, Cattle creates just three objects (one with value
 * %CATTLE_INSTRUCTION_INCREASE, one with value %CATTLE_INSTRUCTION_PRINT,
 * and one with value %CATTLE_INSTRUCTION_DECREASE) and set their quantity
 * to three, one and five respectively.
 *
 * Each instruction maintains a reference to the next instruction in the
 * execution flow. If the instruction starts a loop (its value is
 * %CATTLE_INSTRUCTION_LOOP_BEGIN) it also holds a reference to the first
 * instruction in the loop.
 */

G_DEFINE_TYPE (CattleInstruction, cattle_instruction, G_TYPE_OBJECT)

/**
 * CattleInstructionValue:
 * @CATTLE_INSTRUCTION_NONE: Do nothing
 * @CATTLE_INSTRUCTION_MOVE_LEFT: Move the tape to the left
 * @CATTLE_INSTRUCTION_MOVE_RIGHT: Move the tape to the right
 * @CATTLE_INSTRUCTION_INCREASE: Increase the current value
 * @CATTLE_INSTRUCTION_DECREASE: Decrease the current value
 * @CATTLE_INSTRUCTION_LOOP_BEGIN: Execute the loop until the current
 * value is zero, then proceed to the next instruction
 * @CATTLE_INSTRUCTION_LOOP_END: Exit from the currently-executing loop
 * @CATTLE_INSTRUCTION_READ: Get one character from the input and save
 * its value at the current position
 * @CATTLE_INSTRUCTION_PRINT: Send the current value to the output.
 * @CATTLE_INSTRUCTION_DEBUG: Show debugging information. This usually
 * means dumping the contents of the tape.
 *
 * Brainfuck instructions supported by Cattle, as #gunichar<!-- -->s.
 *
 * %CATTLE_INSTRUCTION_DEBUG is not part of the Brainfuck language, but
 * it's often used for debugging and implemented in many interpreters,
 * so it's included in Cattle as well.
 */

/**
 * CattleInstruction:
 *
 * Opaque data structure representing an instruction. It should never be
 * accessed directly; use the methods below instead.
 */

#define CATTLE_INSTRUCTION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), CATTLE_TYPE_INSTRUCTION, CattleInstructionPrivate))

struct _CattleInstructionPrivate
{
	gboolean                disposed;

	CattleInstructionValue  value;
	gulong                  quantity;

	CattleInstruction      *next;
	CattleInstruction      *loop;
};

/* Properties */
enum
{
	PROP_0,
	PROP_VALUE,
	PROP_QUANTITY,
	PROP_NEXT,
	PROP_LOOP
};

static void
cattle_instruction_init (CattleInstruction *self)
{
	CattleInstructionPrivate *priv;

	priv = CATTLE_INSTRUCTION_GET_PRIVATE (self);

	priv->value = CATTLE_INSTRUCTION_NONE;
	priv->quantity = 1;
	priv->next = NULL;
	priv->loop = NULL;

	priv->disposed = FALSE;

	self->priv = priv;
}

static void
cattle_instruction_dispose (GObject *object)
{
	CattleInstruction        *self;
	CattleInstructionPrivate *priv;

	self = CATTLE_INSTRUCTION (object);
	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	if (priv->next != NULL)
	{
		g_object_unref (priv->next);
	}

	if (priv->loop != NULL)
	{
		/* Releasing the first instruction in the loop causes
		 * all the instructions in the loop to be released as
		 * well, so we can safely release just the first one */
		g_object_unref (priv->loop);
	}

	priv->disposed = TRUE;

	G_OBJECT_CLASS (cattle_instruction_parent_class)->dispose (object);
}

static void
cattle_instruction_finalize (GObject *object)
{
	G_OBJECT_CLASS (cattle_instruction_parent_class)->finalize (object);
}

/**
 * cattle_instruction_new:
 *
 * Create and initialize a new instruction.
 *
 * The newly-created instruction has a #CattleInstruction:quantity of
 * one, and its #CattleInstruction:value is %CATTLE_INSTRUCTION_NONE.
 *
 * Returns: (transfer full): a new #CattleInstruction
 **/
CattleInstruction*
cattle_instruction_new (void)
{
	return g_object_new (CATTLE_TYPE_INSTRUCTION, NULL);
}

/**
 * cattle_instruction_set_value:
 * @instruction: a #CattleInstruction
 * @value: value of @instruction
 *
 * Set the value of @instruction. Accepted values are from the
 * #CattleInstructionValue enumeration.
 */
void
cattle_instruction_set_value (CattleInstruction      *self,
                              CattleInstructionValue  value)
{
	CattleInstructionPrivate *priv;
	gpointer                  enum_class;
	GEnumValue               *enum_value;

	g_return_if_fail (CATTLE_IS_INSTRUCTION (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Get the enum class for instruction values, and lookup the value.
	 * If it is not present, the value is not valid */
	enum_class = g_type_class_ref (CATTLE_TYPE_INSTRUCTION_VALUE);
	enum_value = g_enum_get_value (enum_class, value);
	g_type_class_unref (enum_class);
	g_return_if_fail (enum_value != NULL);

	priv->value = value;
}

/**
 * cattle_instruction_get_value:
 * @instruction: a #CattleInstruction
 *
 * Get the value of @instruction. See cattle_instruction_set_value().
 *
 * Returns: the value of @instruction
 */
CattleInstructionValue
cattle_instruction_get_value (CattleInstruction *self)
{
	CattleInstructionPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_INSTRUCTION (self), CATTLE_INSTRUCTION_NONE);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, CATTLE_INSTRUCTION_NONE);

	return priv->value;
}

/**
 * cattle_instruction_set_quantity:
 * @instruction: a #CattleInstruction
 * @quantity: quantity of @instruction
 *
 * Set the number of times @instruction has to be executed.
 *
 * This value is used at runtime for faster execution, and allows to
 * use less memory for storing the code.
 */
void
cattle_instruction_set_quantity (CattleInstruction *self,
                                 gulong             quantity)
{
	CattleInstructionPrivate *priv;

	g_return_if_fail (CATTLE_IS_INSTRUCTION (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	priv->quantity = quantity;
}

/**
 * cattle_instruction_get_quantity:
 * @instruction: a #CattleInstruction
 *
 * Get the quantity of @instruction.
 * See cattle_instruction_set_quantity().
 *
 * Returns: the quantity of @instruction
 */
gulong
cattle_instruction_get_quantity (CattleInstruction *self)
{
	CattleInstructionPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_INSTRUCTION (self), 0);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, 0);

	return priv->quantity;
}

/**
 * cattle_instruction_set_next:
 * @instruction: a #CattleInstruction
 * @next: (allow-none) (transfer full): next #CattleInstruction to execute, or %NULL
 *
 * Set the next instruction to be executed.
 *
 * If @instruction has value %CATTLE_INSTRUCTION_LOOP_BEGIN, @next
 * will be executed only after the loop has returned.
 */
void
cattle_instruction_set_next (CattleInstruction *self,
                             CattleInstruction *next)
{
	CattleInstructionPrivate *priv;

	g_return_if_fail (CATTLE_IS_INSTRUCTION (self));
	g_return_if_fail (CATTLE_IS_INSTRUCTION (next) || next == NULL);

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Release the reference held on the previous value */
	if (priv->next != NULL)
	{
		g_object_unref (priv->next);
	}

	/* Set the new instruction and acquire a reference to it */
	priv->next = next;
	if (priv->next != NULL)
	{
		g_object_ref (priv->next);
	}
}

/**
 * cattle_instruction_get_next:
 * @instruction: a #CattleInstruction
 *
 * Get the next instruction.
 *
 * Please note that the returned instruction might not be the next
 * instruction in the execution flow: if @instruction marks the
 * beginning of a loop (its value is %CATTLE_INSTRUCTION_LOOP_BEGIN),
 * the returned instruction will be executed only after the loop has
 * ended.
 *
 * Returns: (allow-none) (transfer full): the next instruction, or %NULL
 */
CattleInstruction*
cattle_instruction_get_next (CattleInstruction *self)
{
	CattleInstructionPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_INSTRUCTION (self), NULL);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, NULL);

	if (priv->next != NULL)
	{
		g_object_ref (priv->next);
	}

	return priv->next;
}

/**
 * cattle_instruction_set_loop:
 * @instruction: a #CattleInstruction
 * @loop: (allow-none) (transfer full): first #CattleInstruction in the loop, or %NULL
 *
 * Set the instructions to be executed in the loop.
 *
 * This method should only be called on instructions whose value is
 * %CATTLE_INSTRUCTION_LOOP_BEGIN.
 */
void
cattle_instruction_set_loop (CattleInstruction *self,
                             CattleInstruction *loop)
{
	CattleInstructionPrivate *priv;

	g_return_if_fail (CATTLE_IS_INSTRUCTION (self));
	g_return_if_fail (CATTLE_IS_INSTRUCTION (loop) || loop == NULL);

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Release the reference held on the previous loop */
	if (priv->loop != NULL)
	{
		/* Releasing the first instruction in the loop causes
		 * all the instructions to be disposed */
		g_object_unref (priv->loop);
	}

	/* Get a reference to the new instructions */
	priv->loop = loop;
	if (priv->loop != NULL)
	{
		g_object_ref (priv->loop);
	}
}

/**
 * cattle_instruction_get_loop:
 * @instruction: a #CattleInstruction
 *
 * Get the first instruction of the loop.
 *
 * This method should only be called on instructions whose value is
 * %CATTLE_INSTRUCTION_LOOP_BEGIN.
 *
 * Returns: (allow-none) (transfer full): a #CattleInstruction, or %NULL
 */
CattleInstruction*
cattle_instruction_get_loop (CattleInstruction *self)
{
	CattleInstructionPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_INSTRUCTION (self), NULL);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, NULL);

	if (priv->loop != NULL)
	{
		g_object_ref (priv->loop);
	}

	return priv->loop;
}

static void
cattle_instruction_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
	CattleInstruction *self;
	CattleInstruction *v_instruction;
	gint               v_int;
	gulong             v_ulong;

	self = CATTLE_INSTRUCTION (object);

	switch (property_id)
	{
		case PROP_VALUE:

			v_int = g_value_get_enum (value);
			cattle_instruction_set_value (self,
			                              v_int);

			break;

		case PROP_QUANTITY:

			v_ulong = g_value_get_ulong (value);
			cattle_instruction_set_quantity (self,
			                                 v_ulong);

			break;

		case PROP_NEXT:

			v_instruction = g_value_get_object (value);
			cattle_instruction_set_next (self,
			                             v_instruction);

			break;

		case PROP_LOOP:

			v_instruction = g_value_get_object (value);
			cattle_instruction_set_loop (self,
			                             v_instruction);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);

			break;
	}
}

static void
cattle_instruction_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
	CattleInstruction *self;
	CattleInstruction *v_instruction;
	gulong             v_ulong;
	gint               v_int;

	self = CATTLE_INSTRUCTION (object);

	switch (property_id)
	{
		case PROP_VALUE:

			v_int = cattle_instruction_get_value (self);
			g_value_set_enum (value, v_int);

			break;

		case PROP_QUANTITY:

			v_ulong = cattle_instruction_get_quantity (self);
			g_value_set_ulong (value, v_ulong);

			break;

		case PROP_NEXT:

			v_instruction = cattle_instruction_get_next (self);
			g_value_set_object (value, v_instruction);

			break;

		case PROP_LOOP:

			v_instruction = cattle_instruction_get_loop (self);
			g_value_set_object (value, v_instruction);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
							   property_id,
							   pspec);

			break;
	}
}

static void
cattle_instruction_class_init (CattleInstructionClass *self)
{
	GObjectClass *object_class;
	GParamSpec   *pspec;

	object_class = G_OBJECT_CLASS (self);

	object_class->set_property = cattle_instruction_set_property;
	object_class->get_property = cattle_instruction_get_property;
	object_class->dispose = cattle_instruction_dispose;
	object_class->finalize = cattle_instruction_finalize;

	/**
	 * CattleInstruction:value:
	 *
	 * Value of the instruction. Accepted values are in the
	 * #CattleInstructionValue enumeration.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_enum ("value",
	                           "Value of the instruction",
	                           "Get/set instruction's value",
	                           CATTLE_TYPE_INSTRUCTION_VALUE,
	                           CATTLE_INSTRUCTION_NONE,
	                           G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_VALUE,
	                                 pspec);

	/**
	 * CattleInstruction:quantity:
	 *
	 * Number of times the instruction has to be executed.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_ulong ("quantity",
	                            "Number of times the instruction is repeated",
	                            "Get/set instruction's quantity",
	                            0,
	                            G_MAXULONG,
	                            1,
	                            G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_QUANTITY,
	                                 pspec);

	/**
	 * CattleInstruction:next:
	 *
	 * Next instruction in the execution flow. Can be %NULL if there
	 * are no more instructions to be executed.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_object ("next",
	                             "Next instruction to be executed",
	                             "Get/set next instruction",
	                             CATTLE_TYPE_INSTRUCTION,
	                             G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_NEXT,
	                                 pspec);

	/**
	 * CattleInstruction:loop:
	 *
	 * Instructions to be executed in the loop. Should be %NULL unless
	 * the value of the instruction is %CATTLE_INSTRUCTION_LOOP_BEGIN.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_object ("loop",
	                             "Instructions executed in the loop",
	                             "Get/set loop code",
	                             CATTLE_TYPE_INSTRUCTION,
	                             G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_LOOP,
	                                 pspec);

	g_type_class_add_private (object_class,
	                          sizeof (CattleInstructionPrivate));
}
