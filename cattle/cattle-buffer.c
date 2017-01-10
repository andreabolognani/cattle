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

#include "cattle-buffer.h"

/**
 * SECTION:cattle-buffer
 * @short_description: Memory buffer
 *
 * A #CattleBuffer represents a memory buffer.
 */

G_DEFINE_TYPE (CattleBuffer, cattle_buffer, G_TYPE_OBJECT)

/**
 * CattleBuffer:
 *
 * Opaque data structure representing a memory buffer. It should never
 * be accessed directly; use the methods below instead.
 */

#define CATTLE_BUFFER_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), CATTLE_TYPE_BUFFER, CattleBufferPrivate))

struct _CattleBufferPrivate
{
	gboolean  disposed;

	gint8    *data;
	gulong    size;
};

/* Properties */
enum {
	PROP_0,
	PROP_SIZE
};

static void
cattle_buffer_init (CattleBuffer *self)
{
	CattleBufferPrivate *priv;

	priv = CATTLE_BUFFER_GET_PRIVATE (self);

	priv->data = NULL;
	priv->size = 1;

	priv->disposed = FALSE;

	self->priv = priv;
}

static void
cattle_buffer_constructed (GObject *object)
{
	CattleBuffer        *self;
	CattleBufferPrivate *priv;

	self = CATTLE_BUFFER (object);
	priv = self->priv;

	G_OBJECT_CLASS (cattle_buffer_parent_class)->constructed (object);

	if (priv->size > 0)
	{
		priv->data = (gint8 *) g_slice_alloc0 (priv->size);
	}
}

static void
cattle_buffer_dispose (GObject *object)
{
	CattleBuffer        *self;
	CattleBufferPrivate *priv;

	self = CATTLE_BUFFER (object);
	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	priv->disposed = TRUE;

	G_OBJECT_CLASS (cattle_buffer_parent_class)->dispose (object);
}

static void
cattle_buffer_finalize (GObject *object)
{
	CattleBuffer        *self;
	CattleBufferPrivate *priv;

	self = CATTLE_BUFFER (object);
	priv = self->priv;

	/* Free allocated data */
	if (priv->data != NULL)
	{
		g_slice_free1 (priv->size, priv->data);
	}

	G_OBJECT_CLASS (cattle_buffer_parent_class)->finalize (object);
}

/**
 * cattle_buffer_new:
 * @size: size of the buffer
 *
 * Create and initialize a new memory buffer.
 *
 * Returns: (transfer full): a new #CattleBuffer
 */
CattleBuffer*
cattle_buffer_new (gulong size)
{
	return g_object_new (CATTLE_TYPE_BUFFER,
	                     "size",
	                     size,
	                     NULL);
}

/**
 * cattle_buffer_set_contents: (skip)
 * @buffer: a #CattleBuffer
 * @contents: (transfer none): data to copy inside the memory buffer
 *
 * Set the contents of a memory buffer.
 *
 * The size of @contents is assumed to be the same as the size of @buffer.
 */
void
cattle_buffer_set_contents (CattleBuffer *self,
                            gint8        *contents)
{
	CattleBufferPrivate *priv;

	g_return_if_fail (CATTLE_IS_BUFFER (self));
	g_return_if_fail (contents != NULL);

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	cattle_buffer_set_contents_full (self, contents, priv->size);
}

/**
 * cattle_buffer_set_contents_full: (rename-to cattle_buffer_set_contents)
 * @buffer: a #CattleBuffer
 * @contents: (transfer none) (array length=size): data to copy inside the memory buffer
 * @size: size of @contents
 *
 * Set the contents of the memory buffer.
 *
 * This method exists mainly for bindings; cattle_buffer_set_contents() is
 * more convenient when writing C code.
 */
void
cattle_buffer_set_contents_full (CattleBuffer *self,
                                 gint8        *contents,
                                 gulong        size)
{
	CattleBufferPrivate *priv;
	gulong               i;

	g_return_if_fail (CATTLE_IS_BUFFER (self));
	g_return_if_fail (contents != NULL);

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	g_return_if_fail (size <= priv->size);

	/* Copy the data one byte at a time */
	for (i = 0; i < size; i++)
	{
		priv->data[i] = contents[i];
	}
}

/**
 * cattle_buffer_set_value:
 * @buffer: a #CattleBuffer
 * @position: offset inside the memory buffer
 * @value: new value
 *
 * Set the value of a specific byte inside the memory buffer.
 *
 * The value of @position must be smaller than the size of the
 * memory buffer, as returned by cattle_buffer_get_size().
 */
void
cattle_buffer_set_value (CattleBuffer *self,
                         gulong        position,
                         gint8         value)
{
	CattleBufferPrivate *priv;

	g_return_if_fail (CATTLE_IS_BUFFER (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);
	g_return_if_fail (position < priv->size);

	priv->data[position] = value;
}

/**
 * cattle_buffer_get_value:
 * @buffer: a #CattleBuffer
 * @position: offset inside the memory buffer
 *
 * Get the value of a specific byte inside the memory buffer.
 *
 * The value of @position must be smaller than the size of the
 * memory buffer, as returned by cattle_buffer_get_size().
 *
 * Returns: the value of the selected byte.
 */
gint8
cattle_buffer_get_value (CattleBuffer *self,
                         gulong        position)
{
	CattleBufferPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_BUFFER (self), 0);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, 0);
	g_return_val_if_fail (position < priv->size, 0);

	return priv->data[position];
}

/**
 * cattle_buffer_get_size:
 * @buffer: a #CattleBuffer
 *
 * Get the size of the memory buffer.
 *
 * Returns: the size of the memory buffer
 */
gulong
cattle_buffer_get_size (CattleBuffer *self)
{
	CattleBufferPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_BUFFER (self), 0);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, 0);

	return priv->size;
}

static void
cattle_buffer_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
	CattleBuffer        *self;
	CattleBufferPrivate *priv;
	gulong               v_ulong;

	self = CATTLE_BUFFER (object);
	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	switch (property_id)
	{
		case PROP_SIZE:

			v_ulong = g_value_get_ulong (value);
			priv->size = v_ulong;

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);

			break;
	}
}

static void
cattle_buffer_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
	CattleBuffer *self;
	gulong        v_ulong;

	self = CATTLE_BUFFER (object);

	switch (property_id)
	{
		case PROP_SIZE:

			v_ulong = cattle_buffer_get_size (self);
			g_value_set_ulong (value, v_ulong);

			break;

		default:

			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);

			break;
	}
}

static void
cattle_buffer_class_init (CattleBufferClass *self)
{
	GObjectClass *object_class;
	GParamSpec   *pspec;

	object_class = G_OBJECT_CLASS (self);

	object_class->set_property = cattle_buffer_set_property;
	object_class->get_property = cattle_buffer_get_property;
	object_class->constructed = cattle_buffer_constructed;
	object_class->dispose = cattle_buffer_dispose;
	object_class->finalize = cattle_buffer_finalize;

	/**
	 * CattleBuffer:size:
	 *
	 * Size of the memory buffer.
	 */
	pspec = g_param_spec_ulong ("size",
	                            "Size of the memory buffer",
	                            "Get size of the memory buffer",
	                            0,
	                            G_MAXULONG,
	                            0,
	                            G_PARAM_READWRITE|G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
	                                 PROP_SIZE,
	                                 pspec);

	g_type_class_add_private (object_class,
	                          sizeof (CattleBufferPrivate));
}
