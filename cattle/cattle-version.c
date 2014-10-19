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

#include "cattle-version.h"

/**
 * SECTION:cattle-version
 * @short_description: Variables and functions to check Cattle's version
 *
 * The following variables, functions and macros allow one to check
 * both the version of Cattle being used for compilation and the one
 * used at runtime.
 *
 * Cattle follows the MAJOR.MINOR.MICRO versioning scheme, where all
 * releases sharing the same major number are ABI compatible and releases
 * with and odd minor number are unstable releases targeted at developers
 * only.
 */

/**
 * CATTLE_MAJOR_VERSION:
 *
 * Major version of the Cattle library used for compilation.
 */

/**
 * CATTLE_MINOR_VERSION:
 *
 * Minor version of the Cattle library used for compilation.
 */

/**
 * CATTLE_MICRO_VERSION:
 *
 * Micro version of the Cattle library used for compilation.
 */

/**
 * CATTLE_CHECK_VERSION:
 * @major: required major version
 * @minor: required minor version
 * @micro: required micro version
 *
 * Check the Cattle library used for compilation is compatible with
 * the required version described by @major, @minor and @micro.
 *
 * Returns: %TRUE if the library used for compilation is compatible,
 *          %FALSE otherwise
 */

/**
 * cattle_major_version:
 *
 * Major version of the Cattle library used at runtime.
 */
const guint cattle_major_version = CATTLE_MAJOR_VERSION;

/**
 * cattle_minor_version:
 *
 * Minor version of the Cattle library used at runtime.
 */
const guint cattle_minor_version = CATTLE_MINOR_VERSION;

/**
 * cattle_micro_version:
 *
 * Micro version of the Cattle library used at runtime.
 */
const guint cattle_micro_version = CATTLE_MICRO_VERSION;

/**
 * cattle_check_version:
 * @required_major: required major version
 * @required_minor: required minor version
 * @required_micro: required micro version
 *
 * Check the Cattle library used at runtime is compatible with the
 * required version described by @required_major, @required_minor and
 * @required_micro.
 *
 * Returns: %TRUE if the runtime library is compatible, %FALSE otherwise
 */
gboolean
cattle_check_version (guint required_major,
                      guint required_minor,
                      guint required_micro)
{
	if (cattle_major_version > required_major) {
		return TRUE;
	}
	if (cattle_major_version == required_major &&
        cattle_minor_version > required_minor)
	{
		return TRUE;
	}
	if (cattle_major_version == required_major &&
	    cattle_minor_version == required_minor &&
	    cattle_micro_version >= required_micro)
	{
		return TRUE;
	}

	return FALSE;
}
