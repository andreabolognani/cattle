/* common - Useful, general purpose routines
 * Copyright (C) 2008-2020  Andrea Bolognani <eof@kiyuko.org>
 * This file is part of Cattle
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
 * with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Homepage: https://kiyuko.org/software/cattle
 */

#include <gio/gio.h>
#include "common.h"

CattleBuffer*
read_file_contents (const gchar  *path,
                    GError      **error)
{
    CattleBuffer      *buffer;
    g_autoptr (GFile)  file = NULL;
    g_autofree gchar  *contents = NULL;
    GError            *inner_error;
    gint8             *start;
    gsize              length;
    gboolean           success;

    g_return_val_if_fail (path != NULL, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    file = g_file_new_for_commandline_arg (path);

    inner_error = NULL;
    success = g_file_load_contents (file,
                                    NULL,
                                    &contents,
                                    &length,
                                    NULL,
                                    &inner_error);

    if (!success)
    {
        g_propagate_error (error,
                           inner_error);

        return NULL;
    }

    start = (gint8 *) contents;

    /* Skip the sha-bang line if present */
    if (length >= 2 && contents[0] == '#' && contents[1] == '!')
    {
        while (length > 0 && start[0] != '\n')
        {
            start++;
            length--;
        }
    }

    buffer = cattle_buffer_new (length);

    if (length > 0)
    {
        cattle_buffer_set_contents (buffer, start);
    }

    return buffer;
}
