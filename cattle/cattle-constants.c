/* Cattle - Brainfuck language toolkit
 * Copyright (C) 2008-2020  Andrea Bolognani <eof@kiyuko.org>
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

#include "cattle-version.h"

/**
 * SECTION:cattle-constants
 * @short_description: Useful constants
 */

/**
 * CATTLE_EOF:
 *
 * End-of-file value. Defined as a #gint8 with value -1 on every platform.
 *
 * The value of EOF is not guaranteed to be consistent across C libraries,
 * so this value is used instead.
 */
const gint8 CATTLE_EOF = -1;
