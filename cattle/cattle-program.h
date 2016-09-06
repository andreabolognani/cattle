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

#ifndef __CATTLE_PROGRAM_H__
#define __CATTLE_PROGRAM_H__

#include <glib.h>
#include <glib-object.h>
#include <cattle/cattle-buffer.h>
#include <cattle/cattle-instruction.h>

G_BEGIN_DECLS

#define CATTLE_TYPE_PROGRAM              (cattle_program_get_type ())
#define CATTLE_PROGRAM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CATTLE_TYPE_PROGRAM, CattleProgram))
#define CATTLE_PROGRAM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CATTLE_TYPE_PROGRAM, CattleProgramClass))
#define CATTLE_IS_PROGRAM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CATTLE_TYPE_PROGRAM))
#define CATTLE_IS_PROGRAM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CATTLE_TYPE_PROGRAM))
#define CATTLE_PROGRAM_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), CATTLE_TYPE_PROGRAM, CattleProgramClass))

typedef struct _CattleProgram        CattleProgram;
typedef struct _CattleProgramClass   CattleProgramClass;
typedef struct _CattleProgramPrivate CattleProgramPrivate;

struct _CattleProgram
{
	GObject parent;
	CattleProgramPrivate *priv;
};

struct _CattleProgramClass
{
	GObjectClass parent;
};

CattleProgram*     cattle_program_new              (void);
gboolean           cattle_program_load             (CattleProgram      *program,
                                                    CattleBuffer       *buffer,
                                                    GError            **error);
void               cattle_program_set_instructions (CattleProgram      *program,
                                                    CattleInstruction  *instructions);
CattleInstruction* cattle_program_get_instructions (CattleProgram      *program);
void               cattle_program_set_input        (CattleProgram      *program,
                                                    CattleBuffer       *input);
CattleBuffer*      cattle_program_get_input        (CattleProgram      *program);

GType              cattle_program_get_type         (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __CATTLE_PROGRAM_H__ */
