/* common -- Useful, general purpose routines
 * Copyright (C) 2008-2011  Andrea Bolognani <eof@kiyuko.org>
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
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Homepage: http://kiyuko.org/software/cattle
 */

#include <gio/gio.h>
#include "common.h"

gchar*
read_file_contents (const gchar  *path,
                    GError      **error)
{
	GFile *file;
	GError *inner_error;
	gchar *contents;
	gchar *temp;
	gboolean success;

	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	file = g_file_new_for_commandline_arg (path);

	inner_error = NULL;
	success = g_file_load_contents (file,
	                                NULL,
	                                &contents,
	                                NULL,
	                                NULL, /* No etag */
	                                &inner_error);

	if (!success) {

		g_propagate_error (error,
		                   inner_error);

		g_object_unref (file);

		return NULL;
	}

	/* Detect magic bytes and strip the first line if present */
	if (g_str_has_prefix (contents, "#!")) {

		temp = contents;

		while (g_utf8_get_char (temp) != '\n') {
			*temp = ' ';
			temp = g_utf8_next_char (temp);
		}

		contents = g_strchug (contents);
	}

	g_object_unref (file);

	return contents;
}
