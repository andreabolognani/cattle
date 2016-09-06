/* buffer - Tests for CattleBuffer
 * Copyright (C) 2008-2016  Andrea Bolognani <eof@kiyuko.org>
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

#include <glib.h>
#include <glib-object.h>
#include <cattle/cattle.h>

/**
 * test_buffer_empty:
 *
 * Ensure an empty buffer can be created.
 */
static void
test_buffer_empty (void)
{
	CattleBuffer *buffer;

	buffer = cattle_buffer_new (0);
	g_assert (cattle_buffer_get_size (buffer) == 0);

	g_object_unref (buffer);
}

/**
 * test_buffer_create:
 *
 * Ensure the size of a newly-created buffer is reported correctly.
 */
static void
test_buffer_create (void)
{
	CattleBuffer *buffer;

	buffer = cattle_buffer_new (42);
	g_assert (cattle_buffer_get_size (buffer) == 42);

	g_object_unref (buffer);
}

/**
 * test_buffer_get_value:
 *
 * Ensure cattle_buffer_get_value() works as expected.
 */
void
test_buffer_get_value (void)
{
	CattleBuffer *buffer;

	buffer = cattle_buffer_new (3);
	g_assert (cattle_buffer_get_size (buffer) == 3);

	g_assert (cattle_buffer_get_value (buffer, 0) == 0);
	g_assert (cattle_buffer_get_value (buffer, 1) == 0);
	g_assert (cattle_buffer_get_value (buffer, 2) == 0);

	g_object_unref (buffer);
}

/**
 * test_buffer_set_contents_array:
 *
 * Ensure setting contents using an array of gint8 values works.
 */
void
test_buffer_set_contents_array (void)
{
	CattleBuffer *buffer;
	gint8 values[5];
	gint i;

	buffer = cattle_buffer_new (3);
	g_assert (cattle_buffer_get_size (buffer) == 3);

	values[0] = G_MININT8;
	values[1] = 0;
	values[2] = 10;
	values[3] = -5;
	values[4] = G_MAXINT8;

	cattle_buffer_set_contents (buffer, values);

	for (i = 0; i < 3; i++)
	{
		g_assert (cattle_buffer_get_value (buffer, i) == values[i]);
	}

	g_object_unref (buffer);
}

/**
 * test_buffer_set_contents_string:
 *
 * Ensure setting contents using a string works.
 */
void
test_buffer_set_contents_string (void)
{
	CattleBuffer *buffer;

	buffer = cattle_buffer_new (3);
	g_assert (cattle_buffer_get_size (buffer) == 3);

	cattle_buffer_set_contents (buffer, (gint8 *) "abcd");

	g_assert (cattle_buffer_get_value (buffer, 0) == 'a');
	g_assert (cattle_buffer_get_value (buffer, 1) == 'b');
	g_assert (cattle_buffer_get_value (buffer, 2) == 'c');

	g_object_unref (buffer);
}

/**
 * test_buffer_set_value:
 *
 * Ensure that changing the value of a single byte inside the memory
 * buffer works.
 */
void
test_buffer_set_value (void)
{
	CattleBuffer *buffer;
	gint8 values[3];
	gint i;

	buffer = cattle_buffer_new (3);
	g_assert (cattle_buffer_get_size (buffer) == 3);

	values[0] = 0;
	values[1] = G_MAXINT8;
	values[2] = 0;

	cattle_buffer_set_value (buffer, 1, G_MAXINT8);

	for (i = 0; i < 3; i++)
	{
		g_assert (cattle_buffer_get_value (buffer, i) == values[i]);
	}

	g_object_unref (buffer);
}

gint
main (gint argc, gchar **argv)
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init ();
#endif

	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/buffer/empty",
	                 test_buffer_empty);
	g_test_add_func ("/buffer/create",
	                 test_buffer_create);
	g_test_add_func ("/buffer/get-value",
	                 test_buffer_get_value);
	g_test_add_func ("/buffer/set-contents-array",
	                 test_buffer_set_contents_array);
	g_test_add_func ("/buffer/set-contents-string",
	                 test_buffer_set_contents_string);
	g_test_add_func ("/buffer/set-value",
	                 test_buffer_set_value);

	return g_test_run ();
}
