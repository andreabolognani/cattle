/* Cattle - Brainfuck language toolkit
 * Copyright (C) 2008-2016  Andrea Bolognani <eof@kiyuko.org>
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

#if !defined (__CATTLE_H_INSIDE__) && !defined (CATTLE_COMPILATION)
#error "Only <cattle/cattle.h> can be included directly."
#endif

#ifndef __CATTLE_ERROR_H__
#define __CATTLE_ERROR_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef enum
{
	CATTLE_ERROR_IO,
	CATTLE_ERROR_UNBALANCED_BRACKETS,
	CATTLE_ERROR_INPUT_OUT_OF_RANGE
} CattleError;

#define CATTLE_ERROR cattle_error_quark()

GQuark cattle_error_quark (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __CATTLE_ERROR_H__ */
