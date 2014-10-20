/* Cattle - Brainfuck language toolkit
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
 * CattleEndOfInputAction:
 * @CATTLE_END_OF_INPUT_ACTION_STORE_ZERO: Store a zero in the current cell. This is
 * the default behaviour
 * @CATTLE_END_OF_INPUT_ACTION_STORE_EOF: Store %CATTLE_EOF in the current cell
 * @CATTLE_END_OF_INPUT_ACTION_DO_NOTHING: Do nothing.
 *
 * Possible actions to be performed by a #CattleInterpreter when the end
 * of input is reached.
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
	gboolean               disposed;

	CattleEndOfInputAction end_of_input_action;
	gboolean               debug_is_enabled;
};

/* Properties */
enum
{
	PROP_0,
	PROP_END_OF_INPUT_ACTION,
	PROP_DEBUG_IS_ENABLED
};

static void
cattle_configuration_init (CattleConfiguration *self)
{
	CattleConfigurationPrivate *priv;

	priv = CATTLE_CONFIGURATION_GET_PRIVATE (self);

	priv->end_of_input_action = CATTLE_END_OF_INPUT_ACTION_STORE_ZERO;
	priv->debug_is_enabled = FALSE;

	priv->disposed = FALSE;

	self->priv = priv;
}

static void
cattle_configuration_dispose (GObject *object)
{
	CattleConfiguration        *self;
	CattleConfigurationPrivate *priv;

	self = CATTLE_CONFIGURATION (object);
	priv = self->priv;

	g_return_if_fail (!priv->disposed);

	priv->disposed = TRUE;

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
 * cattle_configuration_set_end_of_input_action:
 * @configuration: a #CattleConfiguration
 * @action: the action to be performed
 *
 * Set the action to be performed when the end of input is reached.
 *
 * Accepted values are from the #CattleEndOfInputAction enumeration.
 */
void
cattle_configuration_set_end_of_input_action (CattleConfiguration    *self,
                                              CattleEndOfInputAction  action)
{
	CattleConfigurationPrivate *priv;
	gpointer                    enum_class;
	GEnumValue                 *enum_value;

	g_return_if_fail (CATTLE_IS_CONFIGURATION (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	/* Get the enum class for actions, and lookup the value.
	 * If it is not present, the action is not valid */
	enum_class = g_type_class_ref (CATTLE_TYPE_END_OF_INPUT_ACTION);
	enum_value = g_enum_get_value (enum_class, action);
	g_type_class_unref (enum_class);
	g_return_if_fail (enum_value != NULL);

	priv->end_of_input_action = action;
}

/**
 * cattle_configuration_get_end_of_input_action:
 * @configuration: a #CattleConfiguration
 *
 * Get the action to be performed when the end of input is reached.
 * See cattle_configuration_set_end_of_input_action().
 *
 * Returns: the current action
 */
CattleEndOfInputAction
cattle_configuration_get_end_of_input_action (CattleConfiguration *self)
{
	CattleConfigurationPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_CONFIGURATION (self), CATTLE_END_OF_INPUT_ACTION_STORE_ZERO);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, CATTLE_END_OF_INPUT_ACTION_STORE_ZERO);

	return priv->end_of_input_action;
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
	CattleConfigurationPrivate *priv;

	g_return_if_fail (CATTLE_IS_CONFIGURATION (self));

	priv = self->priv;
	g_return_if_fail (!priv->disposed);

	priv->debug_is_enabled = enabled;
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
	CattleConfigurationPrivate *priv;

	g_return_val_if_fail (CATTLE_IS_CONFIGURATION (self), FALSE);

	priv = self->priv;
	g_return_val_if_fail (!priv->disposed, FALSE);

	return priv->debug_is_enabled;
}

static void
cattle_configuration_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	CattleConfiguration *self;
	gint                 v_enum;
	gboolean             v_bool;

	self = CATTLE_CONFIGURATION (object);

	switch (property_id)
	{
		case PROP_END_OF_INPUT_ACTION:

			v_enum = g_value_get_enum (value);
			cattle_configuration_set_end_of_input_action (self,
			                                              v_enum);

			break;

		case PROP_DEBUG_IS_ENABLED:

			v_bool = g_value_get_boolean (value);
			cattle_configuration_set_debug_is_enabled (self,
			                                           v_bool);

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
	CattleConfiguration *self;
	gint                 v_enum;
	gboolean             v_bool;

	self = CATTLE_CONFIGURATION (object);

	switch (property_id)
	{
		case PROP_END_OF_INPUT_ACTION:

			v_enum = cattle_configuration_get_end_of_input_action (self);
			g_value_set_enum (value, v_enum);

			break;

		case PROP_DEBUG_IS_ENABLED:

			v_bool = cattle_configuration_get_debug_is_enabled (self);
			g_value_set_boolean (value, v_bool);

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
	GObjectClass *object_class;
	GParamSpec   *pspec;

	object_class = G_OBJECT_CLASS (self);

	object_class->set_property = cattle_configuration_set_property;
	object_class->get_property = cattle_configuration_get_property;
	object_class->dispose = cattle_configuration_dispose;
	object_class->finalize = cattle_configuration_finalize;

	/**
	 * CattleConfiguration:end-of-input-action:
	 *
	 * Action to be performed when the end of input is reached.
	 *
	 * Changes to this property are not notified.
	 */
	pspec = g_param_spec_enum ("end-of-input-action",
	                           "Action to be performed on end of input",
	                           "Get/set end of input action",
	                           CATTLE_TYPE_END_OF_INPUT_ACTION,
	                           CATTLE_END_OF_INPUT_ACTION_STORE_ZERO,
	                           G_PARAM_READWRITE);
	g_object_class_install_property (object_class,
	                                 PROP_END_OF_INPUT_ACTION,
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
