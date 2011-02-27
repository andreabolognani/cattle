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
#include "cattle-configuration.h"

/**
 * SECTION:cattle-configuration
 * @short_description: Configuration for an interpreter
 *
 * A #CattleConfiguration contains the configuration for a
 * #CattleInterpreter.
 */

G_DEFINE_TYPE (CattleConfiguration, cattle_configuration, G_TYPE_OBJECT)

/**
 * CattleOnEOFAction:
 * @CATTLE_ON_EOF_STORE_ZERO: Store a zero in the current cell. This is
 * the default behaviour
 * @CATTLE_ON_EOF_STORE_EOF: Store an EOF character in the current cell
 * @CATTLE_ON_EOF_DO_NOTHING: Do nothing.
 *
 * Possible actions to be performed by a #CattleInterpreter when an EOF
 * character is encountered in the input.
 */

/**
 * CattleConfiguration:
 *
 * Opaque data structure representing a configuration. It should never
 * be accessed directly; use the methods below instead.
 */

#define CATTLE_CONFIGURATION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), CATTLE_TYPE_CONFIGURATION, CattleConfigurationPrivate))

struct _CattleConfigurationPrivate
{
	gboolean          disposed;

	CattleOnEOFAction on_eof_action;
	gboolean          debug_is_enabled;
};

/* Properties */
enum
{
	PROP_0,
	PROP_ON_EOF_ACTION,
	PROP_DEBUG_IS_ENABLED
};

static void
cattle_configuration_init (CattleConfiguration *self)
{
	self->priv = CATTLE_CONFIGURATION_GET_PRIVATE (self);

	self->priv->on_eof_action = CATTLE_ON_EOF_STORE_ZERO;
	self->priv->debug_is_enabled = FALSE;

	self->priv->disposed = FALSE;
}

static void
cattle_configuration_dispose (GObject *object)
{
	CattleConfiguration *self = CATTLE_CONFIGURATION (object);

	g_return_if_fail (!self->priv->disposed);

	self->priv->disposed = TRUE;

	G_OBJECT_CLASS (cattle_configuration_parent_class)->dispose (object);
}

static void
cattle_configuration_finalize (GObject *object)
{
	G_OBJECT_CLASS (cattle_configuration_parent_class)->finalize (object);
}

/**
 * cattle_configuration_new:
 *
 * Create and initialize a new configuration.
 *
 * A single configuration object can be shared between multiple
 * interpreters, but modifying it while an interpreter is running can
 * lead to unexpected and unpredictable results.
 *
 * Returns: (transfer full): a new #CattleConfiguration
 **/
CattleConfiguration*
cattle_configuration_new (void)
{
	return g_object_new (CATTLE_TYPE_CONFIGURATION, NULL);
}

/**
 * cattle_configuration_set_on_eof_action:
 * @configuration: a #CattleConfiguration
 * @action: the action to be performed
 *
 * Set the action to be performed when an EOF character is encountered
 * in the input.
 *
 * Accepted values are from the #CattleOnEOFAction enumeration.
 */
void
cattle_configuration_set_on_eof_action (CattleConfiguration *self,
                                        CattleOnEOFAction    action)
{
	gpointer enum_class;
	GEnumValue *enum_value;

	g_return_if_fail (CATTLE_IS_CONFIGURATION (self));
	g_return_if_fail (!self->priv->disposed);

	/* Get the enum class for actions, and lookup the value.
	 * If it is not present, the action is not valid */
	enum_class = g_type_class_ref (CATTLE_TYPE_ON_EOF_ACTION);
	enum_value = g_enum_get_value (enum_class, action);
	g_type_class_unref (enum_class);
	g_return_if_fail (enum_value != NULL);

	self->priv->on_eof_action = action;
}

/**
 * cattle_configuration_get_on_eof_action:
 * @configuration: a #CattleConfiguration
 *
 * Get the action to be performed when an EOF character is read from
 * the input source. See cattle_configuration_set_on_eof_action().
 *
 * Returns: the current action
 */
CattleOnEOFAction
cattle_configuration_get_on_eof_action (CattleConfiguration *self)
{
	g_return_val_if_fail (CATTLE_IS_CONFIGURATION (self),
	                      CATTLE_ON_EOF_STORE_ZERO);
	g_return_val_if_fail (!self->priv->disposed,
	                      CATTLE_ON_EOF_STORE_ZERO);

	return self->priv->on_eof_action;
}

/**
 * cattle_configuration_set_debug_is_enabled:
 * @configuration: a #CattleConfiguration
 * @enabled: %TRUE to enable debug, %FALSE otherwise
 *
 * Set the status of the debugging support. It is disabled by default.
 *
 * If debugging is disabled, instructions whose value is
 * %CATTLE_INSTRUCTION_DEBUG will be ignored by the interpreter.
 */
void
cattle_configuration_set_debug_is_enabled (CattleConfiguration *self,
                                           gboolean             enabled)
{
	g_return_if_fail (CATTLE_IS_CONFIGURATION (self));
	g_return_if_fail (!self->priv->disposed);

	self->priv->debug_is_enabled = enabled;
}

/**
 * cattle_configuration_get_debug_is_enabled:
 * @configuration: a #CattleConfiguration
 *
 * Get the current status of the debugging support.
 * See cattle_configuration_set_debug_is_enabled().
 *
 * Returns: %TRUE if debugging is enabled, %FALSE otherwise
 */
gboolean
cattle_configuration_get_debug_is_enabled (CattleConfiguration *self)
{
	g_return_val_if_fail (CATTLE_IS_CONFIGURATION (self), FALSE);
	g_return_val_if_fail (!self->priv->disposed, FALSE);

	return self->priv->debug_is_enabled;
}

static void
cattle_configuration_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	CattleConfiguration *self = CATTLE_CONFIGURATION (object);
	gint t_enum;
	gboolean t_bool;

	g_return_if_fail (!self->priv->disposed);

	switch (property_id) {

		case PROP_ON_EOF_ACTION:
			t_enum = g_value_get_enum (value);
			cattle_configuration_set_on_eof_action (self,
			                                        t_enum);
			break;

		case PROP_DEBUG_IS_ENABLED:
			t_bool = g_value_get_boolean (value);
			cattle_configuration_set_debug_is_enabled (self,
			                                           t_bool);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);
			break;
	}
}

static void
cattle_configuration_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
	CattleConfiguration *self = CATTLE_CONFIGURATION (object);
	gint t_enum;
	gboolean t_bool;

	g_return_if_fail (!self->priv->disposed);

	switch (property_id) {

		case PROP_ON_EOF_ACTION:
			t_enum = cattle_configuration_get_on_eof_action (self);
			g_value_set_enum (value, t_enum);
			break;

		case PROP_DEBUG_IS_ENABLED:
			t_bool = cattle_configuration_get_debug_is_enabled (self);
			g_value_set_boolean (value, t_bool);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
			                                   property_id,
			                                   pspec);
			break;
	}
}

static void
cattle_configuration_class_init (CattleConfigurationClass *self)
{
	GObjectClass *object_class = G_OBJECT_CLASS (self);
	GParamSpec *pspec;

	object_class->set_property = cattle_configuration_set_property;
	object_class->get_property = cattle_configuration_get_property;
	object_class->dispose = cattle_configuration_dispose;
	object_class->finalize = cattle_configuration_finalize;

	/**
	 * CattleConfiguration:on-eof-action:
	 *
	 * Action to be performed when an EOF character is encountered in
	 * the input.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_enum ("on-eof-action",
	                           "Action to be performed when an EOF is read",
	                           "Get/set on EOF action",
	                           CATTLE_TYPE_ON_EOF_ACTION,
	                           CATTLE_ON_EOF_STORE_ZERO,
	                           G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_ON_EOF_ACTION,
	                                 pspec);

	/**
	 * CattleConfiguration:debug-is-enabled:
	 *
	 * If %FALSE, instructions whose value is
	 * %CATTLE_INSTRUCTION_DEBUG are not executed by the interpreter.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_boolean ("debug-is-enabled",
	                              "Whether or not debugging is enabled",
	                              "Get/set debug support",
	                              FALSE,
	                              G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_DEBUG_IS_ENABLED,
	                                 pspec);

	g_type_class_add_private (object_class,
	                          sizeof (CattleConfigurationPrivate));
}
