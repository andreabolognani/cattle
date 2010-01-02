/* Cattle -- Flexible Brainfuck interpreter library
 * Copyright (C) 2008-2010  Andrea Bolognani <eof@kiyuko.org>
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
 * #CattleConfiguration represents the configuration for a #CattleInterpreter.
 */

G_DEFINE_TYPE (CattleConfiguration, cattle_configuration, G_TYPE_OBJECT)

/**
 * CattleOnEOFAction:
 * @CATTLE_ON_EOF_STORE_ZERO: store a zero in the memory tape at the current
 * location. This is the default behaviour.
 * @CATTLE_ON_EOF_STORE_EOF: store an EOF character in the memory tape.
 * @CATTLE_ON_EOF_DO_NOTHING: do nothing.
 *
 * Possible actions to be performed by the interpreter when reading an EOF
 * character from the input.
 */

/**
 * CattleConfiguration:
 *
 * Opaque data structure representing a configuration. It should never be
 * accessed directly; use the methods below instead.
 */

#define CATTLE_CONFIGURATION_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), CATTLE_TYPE_CONFIGURATION, CattleConfigurationPrivate))

struct _CattleConfigurationPrivate
{
    gboolean            disposed;

    CattleOnEOFAction   on_eof_action;
    gboolean            debug_is_enabled;
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

    if (G_LIKELY (!self->priv->disposed)) {

        self->priv->disposed = TRUE;

        G_OBJECT_CLASS (cattle_configuration_parent_class)->dispose (object);
    }
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
 * A single configuration object can be shared between multiple interpreters,
 * but modifying it while an interpreter is running can lead to unexpected
 * and unpredictable results.
 *
 * Returns: a new #CattleConfiguration.
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
 * Set the action to be performed when reading an EOF character from the
 * input.
 *
 * Accepted values are in the #CattleOnEOFAction enumeration.
 */
void
cattle_configuration_set_on_eof_action (CattleConfiguration   *self,
                                        CattleOnEOFAction      action)
{
    gpointer enum_class;
    GEnumValue *enum_value;

    g_return_if_fail (CATTLE_IS_CONFIGURATION (self));

    /* Get the enum class for actions, and lookup the value.
     * If it is not present, the action is not valid */
    enum_class = g_type_class_ref (CATTLE_TYPE_ON_EOF_ACTION);
    enum_value = g_enum_get_value (enum_class, action);
    g_type_class_unref (enum_class);
    g_return_if_fail (enum_value != NULL);

    if (G_LIKELY (!self->priv->disposed)) {

        self->priv->on_eof_action = action;
    }
}

/**
 * cattle_configuration_get_on_eof_action:
 * @configuration: a #CattleConfiguration
 *
 * Get the action to be performed when an EOF character is red from the input.
 * See cattle_configuration_set_on_eof_action().
 *
 * Return: the current action.
 */
CattleOnEOFAction
cattle_configuration_get_on_eof_action (CattleConfiguration *self)
{
    CattleOnEOFAction action = CATTLE_ON_EOF_STORE_ZERO;

    g_return_val_if_fail (CATTLE_IS_CONFIGURATION (self), CATTLE_ON_EOF_STORE_ZERO);

    if (G_LIKELY (!self->priv->disposed)) {
        action = self->priv->on_eof_action;
    }

    return action;
}

/**
 * cattle_configuration_set_debug_is_enabled:
 * @configuration: a #CattleConfiguration
 * @enabled: whether or not debug should be enabled
 *
 * Set the status of the debugging support.
 *
 * The debugging instruction #CATTLE_INSTRUCTION_DUMP_TAPE, is not part of the
 * Brainfuck language; anyway, because of its usefulness, it is often used by
 * programmers.
 */
void
cattle_configuration_set_debug_is_enabled (CattleConfiguration   *self,
                                           gboolean             enabled)
{
    g_return_if_fail (CATTLE_IS_CONFIGURATION (self));

    if (G_LIKELY (!self->priv->disposed)) {

        self->priv->debug_is_enabled = enabled;
    }
}

/**
 * cattle_configuration_get_debug_is_enabled:
 * @configuration: a #CattleConfiguration
 *
 * Get the current status of the debugging support.
 * See cattle_configuration_set_debug_is_enabled().
 *
 * Return: the current debugging status.
 */
gboolean
cattle_configuration_get_debug_is_enabled (CattleConfiguration *self)
{
    gboolean enabled = FALSE;

    g_return_val_if_fail (CATTLE_IS_CONFIGURATION (self), FALSE);

    if (G_LIKELY (!self->priv->disposed)) {
        enabled = self->priv->debug_is_enabled;
    }

    return enabled;
}

static void
cattle_configuration_set_property (GObject        *object,
                                   guint           property_id,
                                   const GValue   *value,
                                   GParamSpec     *pspec)
{
    CattleConfiguration *self = CATTLE_CONFIGURATION (object);

    if (G_LIKELY (!self->priv->disposed)) {

        switch (property_id) {

            case PROP_ON_EOF_ACTION:
                cattle_configuration_set_on_eof_action (self, g_value_get_enum (value));
                break;

            case PROP_DEBUG_IS_ENABLED:
                cattle_configuration_set_debug_is_enabled (self, g_value_get_boolean (value));
                break;

            default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
                break;
        }
    }
}

static void
cattle_configuration_get_property (GObject      *object,
                                   guint         property_id,
                                   GValue       *value,
                                   GParamSpec   *pspec)
{
    CattleConfiguration *self = CATTLE_CONFIGURATION (object);

    if (G_LIKELY (!self->priv->disposed)) {

        switch (property_id) {

            case PROP_ON_EOF_ACTION:
                g_value_set_enum (value, cattle_configuration_get_on_eof_action (self));
                break;

            case PROP_DEBUG_IS_ENABLED:
                g_value_set_boolean (value, cattle_configuration_get_debug_is_enabled (self));
                break;

            default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
                break;
        }
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
     * Action to be performed when an EOF character is found in the input.
     *
     * Changes to this property are not notified.
     */
    pspec = g_param_spec_enum ("on-eof-action",
                               "Action to be performed when reading an EOF character",
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
     * Whether or not a #CATTLE_INSTRUCTION_DUMP_TAPE instruction should be
     * executed by the interpreter.
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

    g_type_class_add_private (object_class, sizeof (CattleConfigurationPrivate));
}
