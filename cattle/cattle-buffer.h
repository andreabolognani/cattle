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

#if !defined (__CATTLE_H_INSIDE__) && !defined (CATTLE_COMPILATION)
#error "Only <cattle/cattle.h> can be included directly."
#endif

#ifndef __CATTLE_BUFFER_H__
#define __CATTLE_BUFFER_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define CATTLE_TYPE_BUFFER              (cattle_buffer_get_type ())
#define CATTLE_BUFFER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CATTLE_TYPE_BUFFER, CattleBuffer))
#define CATTLE_BUFFER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CATTLE_TYPE_BUFFER, CattleBufferClass))
#define CATTLE_IS_BUFFER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CATTLE_TYPE_BUFFER))
#define CATTLE_IS_BUFFER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CATTLE_TYPE_BUFFER))
#define CATTLE_BUFFER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), CATTLE_TYPE_BUFFER, CattleBufferClass))

typedef struct _CattleBuffer        CattleBuffer;
typedef struct _CattleBufferClass   CattleBufferClass;
typedef struct _CattleBufferPrivate CattleBufferPrivate;

struct _CattleBuffer
{
	GObject parent;
	CattleBufferPrivate *priv;
};

struct _CattleBufferClass
{
	GObjectClass parent;
};

CattleBuffer* cattle_buffer_new                  (gulong        size);
/*
void          cattle_buffer_set_data             (CattleBuffer *buffer,
                                                  gint8        *data);
void          cattle_buffer_set_data_at_position (CattleBuffer *buffer,
                                                  gulong        position,
                                                  gint8         data);
gint8         cattle_buffer_get_data_at_position (CattleBuffer *buffes,
                                                  gulong        position);
*/
gulong        cattle_buffer_get_size             (CattleBuffer *buffer);

GType         cattle_buffer_get_type             (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __CATTLE_BUFFER_H__ */
