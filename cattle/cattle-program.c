/* Cattle -- Flexible Brainfuck interpreter library
 * Copyright (C) 2008-2009  Andrea Bolognani <eof@kiyuko.org>
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
 * Homepage: http://www.kiyuko.org/software/cattle
 */

#include "cattle-enums.h"
#include "cattle-program.h"

/**
 * SECTION:cattle-program
 * @short_description: A program and its input
 * @see_also: #CattleInstruction
 *
 * #CattleProgram represents a complete Brainfuck program, that is, the
 * instructions to be executed and possibly its input.
 *
 * The input for a program can optionally be specified in the source file, and
 * it's separated from the program code by a bang (!) symbol. For example,
 * given the following input:
 *
 * <informalexample><programlisting>
 * ,+.!sometext
 * </programlisting></informalexample>
 *
 * the program's code is ",+." while the program's input is "sometext".
 *
 * It's important to remember that any Brainfuck instruction after the bang
 * symbol is considered part of the input, and as such is not executed. Also,
 * subsequent bang symbols are considered part of the input.
 */

G_DEFINE_TYPE (CattleProgram, cattle_program, G_TYPE_OBJECT)

/**
 * CattleProgramError:
 * @CATTLE_PROGRAM_ERROR_BAD_UTF8: the provided input is not valid UTF-8.
 * @CATTLE_PROGRAM_ERROR_UNMATCHED_BRACKET: an unmatched bracket (i.e. a loop
 * which is closed before being opened) was found in the input.
 * @CATTLE_PROGRAM_ERROR_UNBALANCED_BRACKETS: the number of open and closed
 * brackets in the input doesn't match.
 *
 * Errors detected on code loading.
 *
 * Cattle only supports UTF-8, so any input not using this encoding is
 * rejected by the code loader.
 */

/**
 * CattleProgram:
 *
 * Opaque data structure representing a program. It should never be accessed
 * directly; use the methods below instead.
 */

#define CATTLE_PROGRAM_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), CATTLE_TYPE_PROGRAM, CattleProgramPrivate))

struct _CattleProgramPrivate {
    gboolean disposed;

    CattleInstruction   *instructions;
    gchar               *input;
};

/* Properties */
enum {
    PROP_0,
    PROP_INSTRUCTIONS,
    PROP_INPUT
};

/* Internal functions */
static CattleInstruction*   load_from_string_real   (gchar    **program,
                                                     GError   **error);

/* Symbols used by the code loader */
#define SHARP_SYMBOL    0x23
#define BANG_SYMBOL     0x21
#define NEWLINE_SYMBOL  0x0A

/**
 * CATTLE_PROGRAM_ERROR:
 *
 * Error domain for program operations. Errors in this domain will be from
 * the #CattleProgramError enumeration.
 */
GQuark
cattle_program_error_quark (void)
{
    return g_quark_from_static_string ("cattle-program-error-quark");
}

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

    if (G_LIKELY (!self->priv->disposed)) {

        g_object_unref (self->priv->instructions);
        self->priv->instructions = NULL;

        self->priv->disposed = TRUE;

        G_OBJECT_CLASS (cattle_program_parent_class)->dispose (object);
    }
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
load_from_string_real (gchar    **program,
                       GError   **error)
{
    CattleInstruction *first = NULL;
    CattleInstruction *current = NULL;
    CattleInstruction *previous = NULL;
    CattleInstruction *loop = NULL;
    glong quantity;
    gunichar instruction;
    gunichar next;

    /* Create the first instruction and set it as current */
    first = cattle_instruction_new ();
    current = first;
    g_object_ref (first);

    /* Now loop until we reach the end of input, or until we get an end of
     * loop instruction, whichever comes first */
    do {

        instruction = g_utf8_get_char (*program);

        /* Return to the caller at the end of the input */
        if (instruction == 0) {
            g_object_unref (current);
            return first;
        }

        /* Quit parsing if the current symbol marks the beginning of the
         * program's input, but move the cursor forward before doing so */
        if (instruction == BANG_SYMBOL) {
            *program = g_utf8_next_char (*program);
            g_object_unref (current);
            return first;
        }

        *program = g_utf8_next_char (*program);
        quantity = 0;

        switch (instruction) {

            case CATTLE_INSTRUCTION_LOOP_BEGIN:

                /* Recurse to load the inner loop */
                loop = load_from_string_real (program, error);

                cattle_instruction_set_value (current, CATTLE_INSTRUCTION_LOOP_BEGIN);
                cattle_instruction_set_loop (current, loop);
                g_object_unref (loop);
            break;

            case CATTLE_INSTRUCTION_LOOP_END:

                cattle_instruction_set_value (current, CATTLE_INSTRUCTION_LOOP_END);
                g_object_unref (current);
                return first;
            break;

            case CATTLE_INSTRUCTION_MOVE_LEFT:
            case CATTLE_INSTRUCTION_MOVE_RIGHT:
            case CATTLE_INSTRUCTION_INCREASE:
            case CATTLE_INSTRUCTION_DECREASE:
            case CATTLE_INSTRUCTION_READ:
            case CATTLE_INSTRUCTION_PRINT:
            case CATTLE_INSTRUCTION_DUMP_TAPE:

                do {

                    /* Increase the quantity */
                    quantity++;

                    /* Move to the next character */
                    next = g_utf8_get_char (*program);
                    if (next != 0) {
                        *program = g_utf8_next_char (*program);
                    }
                } while ((next != 0) && (next == instruction));

                /* The last character we red was different, so we step back */
                if (next != 0) {
                    *program = g_utf8_prev_char (*program);
                }

                cattle_instruction_set_value (current, instruction);
                cattle_instruction_set_quantity (current, quantity);
            break;

            default:
                /* Ignore any other character */
            break;
        }

        /* Create a new instruction if needed */
        switch (instruction) {

            case CATTLE_INSTRUCTION_LOOP_BEGIN:
            case CATTLE_INSTRUCTION_MOVE_LEFT:
            case CATTLE_INSTRUCTION_MOVE_RIGHT:
            case CATTLE_INSTRUCTION_INCREASE:
            case CATTLE_INSTRUCTION_DECREASE:
            case CATTLE_INSTRUCTION_READ:
            case CATTLE_INSTRUCTION_PRINT:
            case CATTLE_INSTRUCTION_DUMP_TAPE:

                previous = current;

                current = cattle_instruction_new ();
                cattle_instruction_set_next (previous, current);
                g_object_unref (previous);
            break;

            default:
                /* Ignore any other character */
            break;
        }
    } while (TRUE);

    g_assert_not_reached ();
    return first;
}

/**
 * cattle_program_new:
 *
 * Create a new #CattleProgram. The returned object represent a Brainfuck
 * program and possibly its input.
 *
 * A single instance of a program can be shared between multiple
 * interpreters, as long as the object is not modified after is has been
 * initialized.
 *
 * Return: a new #CattleProgram.
 **/
CattleProgram*
cattle_program_new (void)
{
    return g_object_new (CATTLE_TYPE_PROGRAM, NULL);
}

/**
 * cattle_program_load_from_string:
 * @program: a #CattleProgram
 * @string: the source code of the program
 * @error: a #GError
 *
 * Load @program from @string.
 *
 * The string can optionally contain also the input for the program: in that
 * case, the input must be separated from the code by a bang (!) character.
 *
 * In case of failure, @error is filled with detailed information. The
 * error domain is %CATTLE_PROGRAM_ERROR, and the error code is from the
 * #CattleProgramError enumeration.
 *
 * Return: #TRUE if @program was loaded successfully, #FALSE otherwise.
 */
gboolean
cattle_program_load_from_string (CattleProgram   *self,
                                 const gchar     *program,
                                 GError          **error)
{
    CattleInstruction *instructions;
    gchar *position;
    gunichar temp;
    glong brackets_count = 0;

    g_return_val_if_fail (CATTLE_IS_PROGRAM (self), FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    if (G_LIKELY (!self->priv->disposed)) {

        /* Check the provided string is valid UTF-8 before proceeding */
        if (!g_utf8_validate (program, -1, NULL)) {
            g_set_error (error,
                         CATTLE_PROGRAM_ERROR,
                         CATTLE_PROGRAM_ERROR_BAD_UTF8,
                         "Invalid UTF-8");
            return FALSE;
        }

        /* Check the number of brackets to ensure the loops are balanced */
        position = (gchar *) program;
        do {

            /* If the counter ever gets negative, it means a loop was closed
             * before being opened */
            if (brackets_count < 0) {
                g_set_error (error,
                             CATTLE_PROGRAM_ERROR,
                             CATTLE_PROGRAM_ERROR_UNMATCHED_BRACKET,
                             "Unmatched bracket");
                return FALSE;
            }

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

        /* Report an error to the caller if the number of open brackets is
         * not equal to the number of closed brackets */
        if (brackets_count != 0) {
            g_set_error (error,
                         CATTLE_PROGRAM_ERROR,
                         CATTLE_PROGRAM_ERROR_UNBALANCED_BRACKETS,
                         "Unbalanced brackets");
            return FALSE;
        }

        /* Load the instructions from the string */
        position = (gchar *) program;
        instructions = load_from_string_real ((gchar **) &position, error);

        /* Set the instructions for the program */
        cattle_program_set_instructions (self, instructions);
        g_object_unref (instructions);

        /* Set the input for the program, if present; otherwise, reset it */
        if (g_utf8_strlen (position, -1) > 0) {
            cattle_program_set_input (self, position);
        }
        else {
            cattle_program_set_input (self, NULL);
        }

        return TRUE;
    }

    return FALSE;
}

/**
 * cattle_program_load_from_file:
 * @program: a #CattleProgram
 * @filename: name of the source file
 * @error: a #GError
 *
 * Load @program from a file.
 *
 * The file can optionally contain a sha-bang line, which is used to tell
 * the system how to execute the file. If such a line is present, it is
 * ignored by the code loading routine.
 *
 * In case of failure, @error is filled with detailed information about what
 * went wrong. The error domain can be either #G_FILE_ERROR or
 * #CATTLE_PROGRAM_ERROR, and the error code can be either in the #GFileError
 * or in the #CattleProgramError enumeration.
 *
 * Return: #TRUE if @program was loaded successfully, #FALSE otherwise.
 */
gboolean
cattle_program_load_from_file (CattleProgram   *self,
                               const gchar     *filename,
                               GError          **error)
{
    gchar *content;
    gchar *program;
    gchar *position;
    gunichar temp;

    g_return_val_if_fail (CATTLE_IS_PROGRAM (self), FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    if (G_LIKELY (!self->priv->disposed)) {

        /* Try to load the contents from file */
        if (!g_file_get_contents (filename, &content, NULL, error)) {
            return FALSE;
        }

        /* Validate the file's content as UTF-8. This check is repeated
         * in cattle_program_load_from_string(), which is called by this
         * method, but we can't avoid calling it again here because we have
         * to manipulate the string before passing it over for loading */
        if (!g_utf8_validate (content, -1, NULL)) {
            g_set_error (error,
                         CATTLE_PROGRAM_ERROR,
                         CATTLE_PROGRAM_ERROR_BAD_UTF8,
                         "Invalid UTF-8");
            g_free (content);
            return FALSE;
        }

        program = content;

        /* Now check the first two characters of the file's contents. If
         * they are the "magic bytes", we have to ignore the whole first
         * line to allow the execution as a script */
        position = program;
        temp = g_utf8_get_char (position);

        /* The first character matches */
        if (temp == SHARP_SYMBOL) {

            position = g_utf8_next_char (position);
            temp = g_utf8_get_char (position);

            /* The second character matches as well */
            if (temp == BANG_SYMBOL) {

                /* Skip to the end of the first line */
                do {
                    position = g_utf8_next_char (position);
                    temp = g_utf8_get_char (position);
                } while (temp != NEWLINE_SYMBOL);

                /* Update the cursor's position */
                program = position;
            }
        }

        /* Now that we have cleaned the file's contents, we can pass them
         * over to the loading method proper */
        if (!cattle_program_load_from_string (self, program, error)) {
            g_free (content);
            return FALSE;
        }

        g_free (content);
        return TRUE;
    }

    return FALSE;
}

/**
 * cattle_program_set_instructions:
 * @program: a #CattleProgram
 * @instructions: instructions for the program
 *
 * Set the instructions for @program.
 *
 * You shouldn't usually need to use this: see
 * cattle_program_load_from_string() and cattle_program_load_from_file() for
 * standard ways to load a program.
 */
void
cattle_program_set_instructions (CattleProgram       *self,
                                 CattleInstruction   *instructions)
{
    g_return_if_fail (CATTLE_IS_PROGRAM (self));
    g_return_if_fail (CATTLE_IS_INSTRUCTION (instructions));

    if (G_LIKELY (!self->priv->disposed)) {

        /* Release the reference held on the current instructions */
        g_object_unref (self->priv->instructions);

        self->priv->instructions = instructions;
        g_object_ref (self->priv->instructions);
    }
}

/**
 * cattle_program_get_instructions:
 * @program: a #CattleProgram
 *
 * Get the instructions for @program.
 *
 * The returned object must be unreferenced when no longer needed.
 *
 * Return: a #CattleInstruction.
 */
CattleInstruction*
cattle_program_get_instructions (CattleProgram *self)
{
    CattleInstruction *instructions = NULL;

    g_return_val_if_fail (CATTLE_IS_PROGRAM (self), NULL);

    if (G_LIKELY (!self->priv->disposed)) {

        instructions = self->priv->instructions;
        g_object_ref (instructions);
    }

    return instructions;
}

/**
 * cattle_program_set_input:
 * @program: a #CattleProgram
 * @input: input for the program, or %NULL
 *
 * Set the input for @program.
 */
void
cattle_program_set_input (CattleProgram   *self,
                          const gchar     *input)
{
    g_return_if_fail (CATTLE_IS_PROGRAM (self));

    if (G_LIKELY (!self->priv->disposed)) {

        /* Free the existing input */
        g_free (self->priv->input);

        self->priv->input = g_strdup (input);
    }
}

/**
 * cattle_program_get_input:
 * @program: a #CattleProgram
 *
 * Get the input for the program.
 *
 * The returned string must be freed after use.
 *
 * Return: input for the program, or %NULL.
 */
gchar*
cattle_program_get_input (CattleProgram *self)
{
    gchar *input = NULL;

    g_return_val_if_fail (CATTLE_IS_PROGRAM (self), NULL);

    if (G_LIKELY (!self->priv->disposed)) {

        input = g_strdup (self->priv->input);
    }

    return input;
}

static void
cattle_program_set_property (GObject        *object,
                             guint           property_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
    CattleProgram *self = CATTLE_PROGRAM (object);

    if (G_LIKELY (!self->priv->disposed)) {

        switch (property_id) {

            case PROP_INSTRUCTIONS:
                cattle_program_set_instructions (self, g_value_get_object (value));
                break;

            case PROP_INPUT:
                cattle_program_set_input (self, g_value_get_string (value));
                break;

            default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
                break;
        }
    }
}

static void
cattle_program_get_property (GObject      *object,
                             guint         property_id,
                             GValue       *value,
                             GParamSpec   *pspec)
{
    CattleProgram *self = CATTLE_PROGRAM (object);

    if (G_LIKELY (!self->priv->disposed)) {

        switch (property_id) {

            case PROP_INSTRUCTIONS:
                g_value_set_object (value, cattle_program_get_instructions (self));
                break;

            case PROP_INPUT:
                g_value_set_string (value, cattle_program_get_input (self));
                break;

            default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
                break;
        }
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
     * Instructions to be executed.
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
     * Input for this program, or %NULL if no input is available at the
     * moment of loading.
     *
     * Changes to this property are not notified.
     */
    pspec = g_param_spec_string ("input",
                                 "Input for this program",
                                 "Get/set input",
                                 NULL,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (object_class,
                                     PROP_INPUT,
                                     pspec);

    g_type_class_add_private (object_class, sizeof (CattleProgramPrivate));
}
