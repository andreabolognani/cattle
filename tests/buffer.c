/* buffer -- Tests for CattleBuffer
 * Copyright (C) 2008-2014  Andrea Bolognani <eof@kiyuko.org>
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
 * test_buffer_create:
 *
 * Check the initial position is reported correctly: a newly-created tape
 * is made of a single cell, so the current position is both at the
 * beginning and at the end.
 */
static void
test_buffer_create (void)
{
	CattleBuffer *buffer;

	buffer = cattle_buffer_new (42);

	g_assert (cattle_buffer_get_size (buffer) == 42);

	g_object_unref (buffer);
}

gint
main (gint argc, gchar **argv)
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
	g_type_init ();
#endif

	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/buffer/create",
	                 test_buffer_create);

	return g_test_run ();
}
