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

#include "cattle-tape.h"
#include "cattle-buffer.h"

/**
 * SECTION:cattle-tape
 * @short_description: Infinite-length memory tape
 *
 * A #CattleTape represents an infinte-length memory tape, which is used
 * by a #CattleInterpreter to store its data. The tape contains a
 * virtually infinte number of memory cells, each one able to store a
 * single byte.
 *
 * A tape supports three kinds of operations: reading the value of the
 * current cell, updating the value of the current cell (either by
 * increasing/decreasing it or by setting it to a certain value), and
 * moving the current cell pointer (either to the left or to the right).
 *
 * The tape grows automatically as more cells are needed, the only limit
 * being the amount of available memory. It is possible to check if the
 * current cell is at the beginning or at the end of the tape using
 * cattle_tape_is_at_beginning() and cattle_tape_is_at_end().
 */

G_DEFINE_TYPE (CattleTape, cattle_tape, G_TYPE_OBJECT)

/**
 * CattleTape:
 *
 * Opaque data structure representing a memory tape. It should never
 * be accessed directly; use the methods below instead.
 */

#define CATTLE_TAPE_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), CATTLE_TYPE_TAPE, CattleTapePrivate))

struct _CattleTapePrivate
{
	gboolean  disposed;

	GList    *current;     /* Current chunk */
	GList    *head;        /* First chunk */

	gulong    offset;      /* Offset of the current cell */
	gulong    lower_limit; /* Offset of the first valid byte
	                        * inside the first chunk */
	gulong    upper_limit; /* Offset of the last valid byte inside
	                        * the last chunk */

	GSList   *bookmarks;   /* Bookmarks stack */
};

/* Properties */
enum {
	PROP_0,
	PROP_CURRENT_VALUE
};

typedef struct _CattleTapeBookmark CattleTapeBookmark;

struct _CattleTapeBookmark
{
	GList   *chunk;
	gulong   offset;
};

/* Size of the tape chunk */
#define CHUNK_SIZE 256

static void
cattle_tape_init (CattleTape *self)
{
	CattleTapePrivate *priv;

	priv = CATTLE_TAPE_GET_PRIVATE (self);

	priv->head = NULL;
	priv->current = NULL;

	/* Create the first chunk */
	priv->head = g_list_append (priv->head, (gpointer) cattle_buffer_new (CHUNK_SIZE));
	priv->current = priv->head;

	/* Set the initial limits */
	priv->offset = 0;
	priv->lower_limit = 0;
	priv->upper_limit = 0;

	/* Initialize the bookmarks stack */
	priv->bookmarks = NULL;

	priv->disposed = FALSE;

	self->priv = priv;
}

static void
chunk_unref (gpointer chunk,
             gpointer data)
{
	g_object_unref (G_OBJECT (chunk));
}

static void
cattle_tape_dispose (GObject *object)
{
	CattleTape        *self;
	CattleTapePrivate *priv;

	self = CATTLE_TAPE (object);
	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	g_list_foreach (priv->head, (GFunc) chunk_unref, NULL);

	priv->disposed = TRUE;

	G_OBJECT_CLASS (cattle_tape_parent_class)->dispose (object);
}

static void
bookmark_free (gpointer bookmark,
               gpointer data)
{
	g_free (bookmark);
}

static void
cattle_tape_finalize (GObject *object)
{
	CattleTape        *self;
	CattleTapePrivate *priv;

	self = CATTLE_TAPE (object);
	priv = self->priv;

	g_slist_foreach (priv->bookmarks, (GFunc) bookmark_free, NULL);

	g_list_free (priv->head);
	g_slist_free (priv->bookmarks);

	G_OBJECT_CLASS (cattle_tape_parent_class)->finalize (object);
}

/**
 * cattle_tape_new:
 *
 * Create and initialize a new memory tape.
 *
 * Returns: (transfer full): a new #CattleTape
 */
CattleTape*
cattle_tape_new (void)
{
	return g_object_new (CATTLE_TYPE_TAPE, NULL);
}

/**
 * cattle_tape_set_current_value:
 * @tape: a #CattleTape
 * @value: the current cell's new value
 *
 * Set the value of the current cell.
 *
 * Accepted values range from %G_MININT8 to %G_MAXINT8.
 */
void
cattle_tape_set_current_value (CattleTape *self,
                               gint8       value)
{
	CattleTapePrivate *priv;
	CattleBuffer      *chunk;

	g_return_if_fail (CATTLE_IS_TAPE (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	chunk = CATTLE_BUFFER (priv->current->data);

	cattle_buffer_set_value (chunk, priv->offset, value);
}

/**
 * cattle_tape_get_current_value:
 * @tape: a #CattleTape
 *
 * Get the value of the current cell. See
 * cattle_tape_set_current_value().
 *
 * Returns: the value of the current cell
 */
gint8
cattle_tape_get_current_value (CattleTape *self)
{
	CattleTapePrivate *priv;
	CattleBuffer      *chunk;

	g_return_val_if_fail (CATTLE_IS_TAPE (self), 0);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, 0);

	chunk = CATTLE_BUFFER (priv->current->data);

	return cattle_buffer_get_value (chunk, priv->offset);
}

/**
 * cattle_tape_increase_current_value:
 * @tape: a #CattleTape
 *
 * Increase the value in the current cell by one.
 */
void
cattle_tape_increase_current_value (CattleTape *self)
{
	cattle_tape_increase_current_value_by (self, 1);
}

/**
 * cattle_tape_increase_current_value_by:
 * @tape: a #CattleTape
 * @value: increase amount
 *
 * Increase the value in the current cell by @value.
 *
 * Increasing the value this way is much faster than calling
 * cattle_tape_increase_current_value() multiple times.
 */
void
cattle_tape_increase_current_value_by (CattleTape *self,
                                       gulong      value)
{
	CattleTapePrivate *priv;
	CattleBuffer      *chunk;
	gint8              current;

	g_return_if_fail (CATTLE_IS_TAPE (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Return right away if no increment is needed */
	if (value == 0)
	{
		return;
	}

	chunk = CATTLE_BUFFER (priv->current->data);

	current = cattle_buffer_get_value (chunk, priv->offset);
	cattle_buffer_set_value (chunk, priv->offset, current + value);
}

/**
 * cattle_tape_decrease_current_value:
 * @tape: a #CattleTape
 *
 * Decrease the value in the current cell by one.
 */
void
cattle_tape_decrease_current_value (CattleTape *self)
{
	cattle_tape_decrease_current_value_by (self, 1);
}

/**
 * cattle_tape_decrease_current_value_by:
 * @tape: a #CattleTape
 * @value: decrease amount
 *
 * Decrease the value in the current cell by @value.
 *
 * Decreasing the value this way is much faster than calling
 * cattle_tape_decrease_current_value() multiple times.
 */
void
cattle_tape_decrease_current_value_by (CattleTape *self,
                                       gulong      value)
{
	CattleTapePrivate *priv;
	CattleBuffer      *chunk;
	gint8              current;

	g_return_if_fail (CATTLE_IS_TAPE (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Return right away if no decrement is needed */
	if (value == 0)
	{
		return;
	}

	chunk = CATTLE_BUFFER (priv->current->data);

	current = cattle_buffer_get_value (chunk, priv->offset);
	cattle_buffer_set_value (chunk, priv->offset, current - value);
}

/**
 * cattle_tape_move_left:
 * @tape: a #CattleTape
 *
 * Move @tape one cell to the left.
 *
 * If there are no memory cells on the left of the current one,
 * one will be created on the fly.
 */
void
cattle_tape_move_left (CattleTape *self)
{
	cattle_tape_move_left_by (self, 1);
}

/**
 * cattle_tape_move_left_by:
 * @tape: a #CattleTape
 * @steps: number of steps
 *
 * Move @tape @steps cells to the left.
 *
 * Moving this way is much faster than calling
 * cattle_tape_move_left() multiple times.
 */
void
cattle_tape_move_left_by (CattleTape *self,
                          gulong      steps)
{
	CattleTapePrivate *priv;
	CattleBuffer      *chunk;

	g_return_if_fail (CATTLE_IS_TAPE (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Move backwards until the correct chunk is found */
	while (steps > priv->offset)
	{
		/* If there is no previous chunk, create it */
		if (g_list_previous (priv->current) == NULL)
		{
			chunk = cattle_buffer_new (CHUNK_SIZE);
			priv->head = g_list_prepend (priv->head, chunk);
			priv->lower_limit = CHUNK_SIZE - 1;
		}

		priv->current = g_list_previous (priv->current);

		steps -= (priv->offset + 1);
		priv->offset = CHUNK_SIZE - 1;
	}

	priv->offset -= steps;

	/* If the current chunk is the first one, the lower limit
	 * might need to be updated */
	if (g_list_previous (priv->current) == NULL)
	{
		if (priv->offset < priv->lower_limit)
		{
			priv->lower_limit = priv->offset;
		}
	}
}

/**
 * cattle_tape_move_right:
 * @tape: a #CattleTape
 *
 * Move @tape one cell to the right.
 *
 * If there are no memory cells on the right of the current one,
 * one will be created on the fly.
 */
void
cattle_tape_move_right (CattleTape *self)
{
	cattle_tape_move_right_by (self, 1);
}

/**
 * cattle_tape_move_right_by:
 * @tape: a #CattleTape
 * @steps: number of steps
 *
 * Move @tape @steps cells to the right.
 *
 * Moving this way is much faster than calling
 * cattle_tape_move_right() multiple times.
 */
void
cattle_tape_move_right_by (CattleTape *self,
                           gulong      steps)
{
	CattleTapePrivate *priv;
	CattleBuffer      *chunk;

	g_return_if_fail (CATTLE_IS_TAPE (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Move forward until the correct chunk is found */
	while (priv->offset + steps >= CHUNK_SIZE )
	{
		/* If there is no next chunk, create it */
		if (g_list_next (priv->current) == NULL)
		{
			chunk = cattle_buffer_new (CHUNK_SIZE);
			priv->head = g_list_append (priv->head, chunk);
			priv->upper_limit = 0;
		}

		priv->current = g_list_next (priv->current);

		steps -= (CHUNK_SIZE - priv->offset);
		priv->offset = 0;
	}

	priv->offset += steps;

	/* If the current chunk is the last one, the upper limit
	 * might need to be updated */
	if (g_list_next (priv->current) == NULL)
	{
		if (priv->offset > priv->upper_limit)
		{
			priv->upper_limit = priv->offset;
		}
	}
}

/**
 * cattle_tape_is_at_beginning:
 * @tape: a #CattleTape
 *
 * Check if the current cell is the first one of @tape.
 *
 * Since the tape grows automatically as more cells are needed, it is
 * possible to move left from the first cell.
 *
 * Returns: %TRUE if the current cell is the first one, %FALSE otherwise
 */
gboolean
cattle_tape_is_at_beginning (CattleTape *self)
{
	CattleTapePrivate *priv;
	gboolean           check;

	g_return_val_if_fail (CATTLE_IS_TAPE (self), FALSE);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, FALSE);

	/* If the current chunk is the first one and the current offset
	 * is equal to the lower limit, we are at the beginning of the
	 * tape */
	if (g_list_previous (priv->current) == NULL && (priv->offset == priv->lower_limit))
	{
		check = TRUE;
	}
	else
	{
		check = FALSE;
	}

	return check;
}

/**
 * cattle_tape_is_at_end:
 * @tape: a #CattleTape
 *
 * Check if the current cell is the last one of @tape.
 * 
 * Since the tape grows automatically as more cells are needed, it is
 * possible to move right from the last cell.
 *
 * Returns: %TRUE if the current cell is the last one, %FALSE otherwise
 */
gboolean
cattle_tape_is_at_end (CattleTape *self)
{
	CattleTapePrivate *priv;
	gboolean           check;

	g_return_val_if_fail (CATTLE_IS_TAPE (self), FALSE);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, FALSE);

	/* If we are in the last chunk and the current offset is equal
	 * to the upper limit, we are in the last valid position */
	if (g_list_next (priv->current) == NULL && (priv->offset == priv->upper_limit))
	{
		check = TRUE;
	}
	else
	{
		check = FALSE;
	}

	return check;
}

/**
 * cattle_tape_push_bookmark:
 * @tape: a #CattleTape
 *
 * Create a bookmark to the current tape position and save it on the
 * bookmark stack.
 */
void
cattle_tape_push_bookmark (CattleTape *self)
{
	CattleTapePrivate  *priv;
	CattleTapeBookmark *bookmark;

	g_return_if_fail (CATTLE_IS_TAPE (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Create a new bookmark and store the current position */
	bookmark = g_new0 (CattleTapeBookmark, 1);
	bookmark->chunk = priv->current;
	bookmark->offset = priv->offset;

	priv->bookmarks = g_slist_prepend (priv->bookmarks, bookmark);
}

/**
 * cattle_tape_pop_bookmark:
 * @tape: a #CattleTape
 *
 * Restore the previously-saved tape position.
 * See cattle_tape_push_bookmark().
 *
 * Returns: %FALSE if the bookmarks stack is empty, %TRUE otherwise
 */
gboolean
cattle_tape_pop_bookmark (CattleTape *self)
{
	CattleTapePrivate  *priv;
	CattleTapeBookmark *bookmark;
	gboolean            check;

	g_return_val_if_fail (CATTLE_IS_TAPE (self), FALSE);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, FALSE);

	if (priv->bookmarks != NULL) {

		/* Get the bookmark and remove it from the stack */
		bookmark = priv->bookmarks->data;
		priv->bookmarks = g_slist_remove (priv->bookmarks, bookmark);

		/* Restore the position */
		priv->current = bookmark->chunk;
		priv->offset = bookmark->offset;

		/* Delete the bookmark */
		g_free (bookmark);

		check = TRUE;
	}
	else
	{
		check = FALSE;
	}

	return check;
}

static void
cattle_tape_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
	CattleTape *self;
	gint8       v_int8;

	self = CATTLE_TAPE (object);

	switch (property_id)
	{
		case PROP_CURRENT_VALUE:

			v_int8 = g_value_get_schar (value);
			cattle_tape_set_current_value (self, v_int8);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);

			break;
	}
}

static void
cattle_tape_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
	CattleTape *self;
	gint8       v_int8;

	self = CATTLE_TAPE (object);

	switch (property_id)
	{
		case PROP_CURRENT_VALUE:

			v_int8 = cattle_tape_get_current_value (self);
			g_value_set_schar (value, v_int8);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);

			break;
	}
}

static void
cattle_tape_class_init (CattleTapeClass *self)
{
	GObjectClass *object_class;
	GParamSpec   *pspec;

	object_class = G_OBJECT_CLASS (self);

	object_class->set_property = cattle_tape_set_property;
	object_class->get_property = cattle_tape_get_property;
	object_class->dispose = cattle_tape_dispose;
	object_class->finalize = cattle_tape_finalize;

	/**
	 * CattleTape:current-value:
	 *
	 * Value of the current cell.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_char ("current-value",
	                           "Value of the current cell",
	                           "Get/set current value",
	                           G_MININT8,
	                           G_MAXINT8,
	                           0,
	                           G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_CURRENT_VALUE,
	                                 pspec);

	g_type_class_add_private (object_class,
	                          sizeof (CattleTapePrivate));
}
