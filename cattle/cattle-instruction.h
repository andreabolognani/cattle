/* Cattle -- Brainfuck language toolkit
 * Copyright (C) 2008-2011  Andrea Bolognani <eof@kiyuko.org>
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

#ifndef __CATTLE_INSTRUCTION_H__
#define __CATTLE_INSTRUCTION_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define CATTLE_TYPE_INSTRUCTION              (cattle_instruction_get_type ())
#define CATTLE_INSTRUCTION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), CATTLE_TYPE_INSTRUCTION, CattleInstruction))
#define CATTLE_INSTRUCTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CATTLE_TYPE_INSTRUCTION, CattleInstructionClass))
#define CATTLE_IS_INSTRUCTION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), CATTLE_TYPE_INSTRUCTION))
#define CATTLE_IS_INSTRUCTION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CATTLE_TYPE_INSTRUCTION))
#define CATTLE_INSTRUCTION_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), CATTLE_TYPE_INSTRUCTION, CattleInstructionClass))

typedef enum
{
	CATTLE_INSTRUCTION_NONE        = 0x5F,
	CATTLE_INSTRUCTION_MOVE_LEFT   = 0x3C,
	CATTLE_INSTRUCTION_MOVE_RIGHT  = 0x3E,
	CATTLE_INSTRUCTION_INCREASE    = 0x2B,
	CATTLE_INSTRUCTION_DECREASE    = 0x2D,
	CATTLE_INSTRUCTION_LOOP_BEGIN  = 0x5B,
	CATTLE_INSTRUCTION_LOOP_END    = 0x5D,
	CATTLE_INSTRUCTION_READ        = 0x2C,
	CATTLE_INSTRUCTION_PRINT       = 0x2E,
	CATTLE_INSTRUCTION_DEBUG       = 0x23
} CattleInstructionValue;

typedef struct _CattleInstruction        CattleInstruction;
typedef struct _CattleInstructionClass   CattleInstructionClass;
typedef struct _CattleInstructionPrivate CattleInstructionPrivate;

struct _CattleInstruction
{
	GObject parent;
	CattleInstructionPrivate *priv;
};

struct _CattleInstructionClass
{
	GObjectClass parent;
};

CattleInstruction*      cattle_instruction_new         (void);
void                    cattle_instruction_set_value   (CattleInstruction      *instruction,
                                                        CattleInstructionValue  value);
CattleInstructionValue cattle_instruction_get_value    (CattleInstruction      *instruction);
void                   cattle_instruction_set_quantity (CattleInstruction      *instruction,
                                                        gint                    quantity);
gint                   cattle_instruction_get_quantity (CattleInstruction      *instruction);
void                   cattle_instruction_set_next     (CattleInstruction      *instruction,
                                                        CattleInstruction      *next);
CattleInstruction*     cattle_instruction_get_next     (CattleInstruction      *instruction);
void                   cattle_instruction_set_loop     (CattleInstruction      *instruction,
                                                        CattleInstruction      *loop);
CattleInstruction*     cattle_instruction_get_loop     (CattleInstruction      *instruction);

GType                  cattle_instruction_get_type     (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __CATTLE_INSTRUCTION_H__ */
