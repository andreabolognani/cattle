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

#define BUFFER_SIZE 1024

gchar*
read_file_contents (const gchar  *path,
                    GError      **error)
{
	GFile *file;
	GFileInputStream *stream;
	GString *contents;
	GError *inner_error;
	gchar buffer[BUFFER_SIZE];
	gchar *contents_str;
	gchar *temp;
	gssize count;

	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	file = g_file_new_for_commandline_arg (path);

	inner_error = NULL;
	stream = g_file_read (file, NULL, &inner_error);

	if (stream == NULL) {
		g_propagate_error (error, inner_error);
		g_object_unref (file);
		return NULL;
	}

	contents = g_string_new ("");

	do {
		inner_error = NULL;
		count = g_input_stream_read (G_INPUT_STREAM (stream),
		                             buffer,
		                             BUFFER_SIZE,
		                             NULL,
		                             &inner_error);

		if (count < 0) {
			g_propagate_error (error, inner_error);
			g_string_free (contents, TRUE);
			g_object_unref (stream);
			g_object_unref (file);
			return NULL;
		}

		g_string_append_len (contents, buffer, count);

	} while (count > 0);

	contents_str = contents->str;

	g_string_free (contents, FALSE);
	g_object_unref (stream);
	g_object_unref (file);

	/* Detect magic bytes and strip the first line if present */
	if (g_str_has_prefix (contents_str, "#!")) {

		temp = contents_str;

		while (g_utf8_get_char (temp) != '\n') {
			*temp = ' ';
			temp = g_utf8_next_char (temp);
		}

		contents_str = g_strchug (contents_str);
	}

	return contents_str;
}
