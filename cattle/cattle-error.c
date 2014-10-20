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

#include "cattle-error.h"

/**
 * SECTION:cattle-error
 * @short_description: Load time and runtime errors
 *
 * Cattle uses the facilities provided by GLib for error reporting.
 *
 * Functions that can fail take a #GError as last argument; errors raised
 * are in the %CATTLE_ERROR domain with error codes from the #CattleError
 * enumeration.
 */

/**
 * CattleError:
 * @CATTLE_ERROR_IO: Generic I/O error
 * @CATTLE_ERROR_UNBALANCED_BRACKETS: The number of open and closed
 * brackets don't match
 * @CATTLE_ERROR_INPUT_OUT_OF_RANGE: The input cannot be stored in a
 * tape cell
 *
 * Errors detected either on code loading or at runtime.
 */

/**
 * CATTLE_ERROR:
 *
 * Error domain for Cattle. Errors in this domain will be from the
 * #CattleError enumeration.
 */
GQuark
cattle_error_quark (void)
{
	return g_quark_from_static_string ("cattle-error-quark");
}
