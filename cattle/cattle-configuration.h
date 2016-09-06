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

#if !defined (__CATTLE_H_INSIDE__) && !defined (CATTLE_COMPILATION)
#error "Only <cattle/cattle.h> can be included directly."
#endif

#ifndef __CATTLE_CONFIGURATION_H__
#define __CATTLE_CONFIGURATION_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define CATTLE_TYPE_CONFIGURATION                (cattle_configuration_get_type ())
#define CATTLE_CONFIGURATION(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), CATTLE_TYPE_CONFIGURATION, CattleConfiguration))
#define CATTLE_CONFIGURATION_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), CATTLE_TYPE_CONFIGURATION, CattleConfigurationClass))
#define CATTLE_IS_CONFIGURATION(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), CATTLE_TYPE_CONFIGURATION))
#define CATTLE_IS_CONFIGURATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), CATTLE_TYPE_CONFIGURATION))
#define CATTLE_CONFIGURATION_GET_CLASS(object)   (G_TYPE_INSTANCE_GET_CLASS ((object), CATTLE_TYPE_CONFIGURATION, CattleConfigurationClass))

typedef enum
{
	CATTLE_END_OF_INPUT_ACTION_STORE_ZERO,
	CATTLE_END_OF_INPUT_ACTION_STORE_EOF,
	CATTLE_END_OF_INPUT_ACTION_DO_NOTHING
} CattleEndOfInputAction;

typedef struct _CattleConfiguration        CattleConfiguration;
typedef struct _CattleConfigurationClass   CattleConfigurationClass;
typedef struct _CattleConfigurationPrivate CattleConfigurationPrivate;

struct _CattleConfiguration
{
	GObject parent;
	CattleConfigurationPrivate *priv;
};

struct _CattleConfigurationClass
{
	GObjectClass parent;
};

CattleConfiguration*    cattle_configuration_new                     (void);
void                    cattle_configuration_set_end_of_input_action (CattleConfiguration    *configuration,
                                                                      CattleEndOfInputAction  action);
CattleEndOfInputAction  cattle_configuration_get_end_of_input_action (CattleConfiguration    *configuration);
void                    cattle_configuration_set_debug_is_enabled    (CattleConfiguration    *configuration,
                                                                      gboolean                enabled);
gboolean                cattle_configuration_get_debug_is_enabled    (CattleConfiguration    *configuration);

GType                   cattle_configuration_get_type                (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __CATTLE_CONFIGURATION_H__ */
