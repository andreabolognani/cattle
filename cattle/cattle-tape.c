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

#include "cattle-tape.h"
#include <stdio.h>

/**
 * SECTION:cattle-tape
 * @short_description: Infinite-length memory tape
 *
 * A #CattleTape represents an infinte-length memory tape, which is used
 * by a #CattleInterpreter to store its data. The tape contains a
 * virtually infinte number of memory cells, each one able to store a
 * single ASCII character.
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

	gint      offset;      /* Offset of the current cell */
	gint      lower_limit; /* Offset of the first valid byte
                            * inside the first chunk */
	gint      upper_limit; /* Offset of the last valid byte inside
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
	gint     offset;
};

/* Size of the tape chunk */
#define CHUNK_SIZE 128

static void
cattle_tape_init (CattleTape *self)
{
	gchar *chunk;

	self->priv = CATTLE_TAPE_GET_PRIVATE (self);

	self->priv->head = NULL;
	self->priv->current = NULL;

	/* Create the first chunk and make it the current one */
	chunk = (gchar *) g_slice_alloc0 (CHUNK_SIZE * sizeof (gchar));
	self->priv->head = g_list_append (self->priv->head, (gpointer) chunk);
	self->priv->current = self->priv->head;

	/* Set initial limits */
	self->priv->offset = 0;
	self->priv->lower_limit = 0;
	self->priv->upper_limit = 0;

	/* Initialize the bookmarks stack */
	self->priv->bookmarks = NULL;

	self->priv->disposed = FALSE;
}

static void
cattle_tape_dispose (GObject *object)
{
	CattleTape *self = CATTLE_TAPE (object);

	g_return_if_fail (!self->priv->disposed);

	self->priv->disposed = TRUE;

	G_OBJECT_CLASS (cattle_tape_parent_class)->dispose (object);
}

static void
chunk_free (gpointer chunk,
            gpointer data)
{
	g_slice_free1 (CHUNK_SIZE * sizeof (gchar), chunk);
}

static void
cattle_tape_finalize (GObject *object)
{
	CattleTape *self = CATTLE_TAPE (object);

	self->priv->current = NULL;

	g_list_foreach (self->priv->head, (GFunc) chunk_free, NULL);
	g_list_free (self->priv->head);
	self->priv->head = NULL;

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
 * Accepted values are ASCII values or EOF.
 */
void
cattle_tape_set_current_value (CattleTape *self,
                               gchar       value)
{
	gchar *chunk = NULL;

	g_return_if_fail (CATTLE_IS_TAPE (self));
	g_return_if_fail (!self->priv->disposed);
	g_return_if_fail ((value >= 0 && value <= 127) || value	== EOF);

	chunk = (char *) self->priv->current->data;
	chunk[self->priv->offset] = value;
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
gchar
cattle_tape_get_current_value (CattleTape *self)
{
	gchar *chunk;

	g_return_val_if_fail (CATTLE_IS_TAPE (self), (gchar) 0);
	g_return_val_if_fail (!self->priv->disposed, (gchar) 0);

	chunk = (gchar *) self->priv->current->data;

	return chunk[self->priv->offset];
}

/**
 * cattle_tape_increase_current_value:
 * @tape: a #CattleTape
 *
 * Increase the value in the current cell by one.
 *
 * If the value in the current cell has ASCII code 127, the new value
 * will have ASCII code 0.
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
 * The value is wrapped if needed to keep it in the ASCII code.
 *
 * Increasing the value this way is much faster than calling
 * cattle_tape_increase_current_value() multiple times.
 */
void
cattle_tape_increase_current_value_by (CattleTape *self,
                                       gint        value)
{
	gchar *chunk;
	gchar current;

	g_return_if_fail (CATTLE_IS_TAPE (self));
	g_return_if_fail (!self->priv->disposed);

	/* Return right away in case no increment is needed */
	if (value == 0) {
		return;
	}

	chunk = (gchar *) self->priv->current->data;
	current = chunk[self->priv->offset];

	/* Special handling for EOF character */
	if (current == EOF) {
		if (value > 0) {
			chunk[self->priv->offset] = 0;
			cattle_tape_increase_current_value_by (self, value - 1);
			return;
		}
		else {
			chunk[self->priv->offset] = 127;
			cattle_tape_increase_current_value_by (self, value + 1);
			return;
		}
	}

	current = (current + value) % 128;

	/* If the remainder is negative the value needs to be adjusted */
	if (current < 0) {
		current = 128 + current;
	}

	chunk[self->priv->offset] = current;
}

/**
 * cattle_tape_decrease_current_value:
 * @tape: a #CattleTape
 *
 * Decrease the value in the current cell by one.
 *
 * If the value in the current cell has ASCII value 0, the new value
 * will have ASCII value 127.
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
 * The value is wrapped if needed to keep it in the ASCII code.
 *
 * Decreasing the value this way is much faster than calling
 * cattle_tape_decrease_current_value() multiple times.
 */
void
cattle_tape_decrease_current_value_by (CattleTape *self,
                                       gint        value)
{
	cattle_tape_increase_current_value_by (self, -value);
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
                          gint        steps)
{
	gchar *chunk;

	g_return_if_fail (CATTLE_IS_TAPE (self));
	g_return_if_fail (!self->priv->disposed);

	/* Move backwards until the correct chunk is found */
	while (steps > self->priv->offset) {

		/* If there is no previous chunk, create it */
		if (g_list_previous (self->priv->current) == NULL) {

			chunk = (gchar *) g_slice_alloc0 (CHUNK_SIZE * sizeof (gchar));
			self->priv->head = g_list_prepend (self->priv->head, chunk);
			self->priv->lower_limit = CHUNK_SIZE - 1;
		}

		self->priv->current = g_list_previous (self->priv->current);

		steps -= (self->priv->offset + 1);
		self->priv->offset = CHUNK_SIZE - 1;
	}

	self->priv->offset -= steps;

	/* If the current chunk is the first one, the lower limit
	 * might need to be updated */
	if (g_list_previous (self->priv->current) == NULL) {

		if (self->priv->offset < self->priv->lower_limit) {
			self->priv->lower_limit = self->priv->offset;
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
                           gint        steps)
{
	gchar *chunk;

	g_return_if_fail (CATTLE_IS_TAPE (self));
	g_return_if_fail (!self->priv->disposed);

	/* Move forward until the correct chunk is found */
	while (self->priv->offset + steps >= CHUNK_SIZE ) {

		/* If there is no next chunk, create it */
		if (g_list_next (self->priv->current) == NULL) {

			chunk = (gchar *) g_slice_alloc0 (CHUNK_SIZE * sizeof (gchar));
			self->priv->head = g_list_append (self->priv->head, chunk);
			self->priv->upper_limit = 0;
		}

		self->priv->current = g_list_next (self->priv->current);

		steps -= (CHUNK_SIZE - self->priv->offset);
		self->priv->offset = 0;
	}

	self->priv->offset += steps;

	/* If the current chunk is the last one, the upper limit
	 * might need to be updated */
	if (g_list_next (self->priv->current) == NULL) {

		if (self->priv->offset > self->priv->upper_limit) {
			self->priv->upper_limit = self->priv->offset;
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
	gboolean check = FALSE;

	g_return_val_if_fail (CATTLE_IS_TAPE (self), FALSE);
	g_return_val_if_fail (!self->priv->disposed, FALSE);

	/* If the current chunk is the first one and the current offset
	 * is equal to the lower limit, we are at the beginning of the
	 * tape */
	if (g_list_previous (self->priv->current) == NULL && (self->priv->offset == self->priv->lower_limit)) {
		check = TRUE;
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
	gboolean check = FALSE;

	g_return_val_if_fail (CATTLE_IS_TAPE (self), FALSE);
	g_return_val_if_fail (!self->priv->disposed, FALSE);

	/* If we are in the last chunk and the current offset is equal
	 * to the upper limit, we are in the last valid position */
	if (g_list_next (self->priv->current) == NULL && (self->priv->offset == self->priv->upper_limit)) {
		check = TRUE;
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
	CattleTapeBookmark *bookmark;

	g_return_if_fail (CATTLE_IS_TAPE (self));
	g_return_if_fail (!self->priv->disposed);

	/* Create a new bookmark and store the current position */
	bookmark = g_new0 (CattleTapeBookmark, 1);
	bookmark->chunk = self->priv->current;
	bookmark->offset = self->priv->offset;

	self->priv->bookmarks = g_slist_prepend (self->priv->bookmarks, bookmark);
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
	CattleTapeBookmark *bookmark;
	gboolean check = FALSE;

	g_return_val_if_fail (CATTLE_IS_TAPE (self), FALSE);
	g_return_val_if_fail (!self->priv->disposed, FALSE);

	if (self->priv->bookmarks != NULL) {

		/* Get the bookmark and remove it from the stack */
		bookmark = self->priv->bookmarks->data;
		self->priv->bookmarks = g_slist_remove (self->priv->bookmarks, bookmark);

		/* Restore the position */
		self->priv->current = bookmark->chunk;
		self->priv->offset = bookmark->offset;

		/* Delete the bookmark */
		g_free (bookmark);

		check = TRUE;
	}

	return check;
}

static void
cattle_tape_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
	CattleTape *self = CATTLE_TAPE (object);
	gchar t_char;

	g_return_if_fail (!self->priv->disposed);

	switch (property_id) {

		case PROP_CURRENT_VALUE:
			t_char = g_value_get_char (value);
			cattle_tape_set_current_value (self, t_char);
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
	CattleTape *self = CATTLE_TAPE (object);
	gchar t_char;

	g_return_if_fail (!self->priv->disposed);

	switch (property_id) {

		case PROP_CURRENT_VALUE:
			t_char = cattle_tape_get_current_value (self);
			g_value_set_char (value, t_char);
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
	GObjectClass *object_class = G_OBJECT_CLASS (self);
	GParamSpec *pspec;

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
	                           MIN (0, EOF),
	                           MAX (127, EOF),
	                           0,
	                           G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_CURRENT_VALUE,
	                                 pspec);

	g_type_class_add_private (object_class,
	                          sizeof (CattleTapePrivate));
}
