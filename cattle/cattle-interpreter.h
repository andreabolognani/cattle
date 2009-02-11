/* Cattle -- Flexible Brainfuck interpreter library
 * Copyright (C) 2008-2009  Andrea Bolognani <eof@kiyuko.org>
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
 * Homepage: http://www.kiyuko.org/software/cattle
 */

#ifndef __CATTLE_INTERPRETER_H__
#define __CATTLE_INTERPRETER_H__

#include <glib-object.h>
#include <cattle/cattle-configuration.h>
#include <cattle/cattle-program.h>
#include <cattle/cattle-tape.h>

G_BEGIN_DECLS

#define CATTLE_TYPE_INTERPRETER                (cattle_interpreter_get_type ())
#define CATTLE_INTERPRETER(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), CATTLE_TYPE_INTERPRETER, CattleInterpreter))
#define CATTLE_INTERPRETER_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), CATTLE_TYPE_INTERPRETER, CattleInterpreterClass))
#define CATTLE_IS_INTERPRETER(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), CATTLE_TYPE_INTERPRETER))
#define CATTLE_IS_INTERPRETER_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), CATTLE_TYPE_INTERPRETER))
#define CATTLE_INTERPRETER_GET_CLASS(object)   (G_TYPE_INSTANCE_GET_CLASS ((object), CATTLE_TYPE_INTERPRETER, CattleInterpreterClass))

typedef struct _CattleInterpreter          CattleInterpreter;
typedef struct _CattleInterpreterClass     CattleInterpreterClass;
typedef struct _CattleInterpreterPrivate   CattleInterpreterPrivate;

struct _CattleInterpreter {
    GObject parent;
    CattleInterpreterPrivate *priv;
};

struct _CattleInterpreterClass {
    GObjectClass parent;

    gboolean   (*output_request)   (CattleInterpreter    *interpreter,
                                    gchar                 output,
                                    GError              **error);
    gboolean   (*input_request)    (CattleInterpreter    *interpreter,
                                    gchar               **input,
                                    GError              **error);
    gboolean   (*debug_request)    (CattleInterpreter    *interpreter,
                                    GError              **error);
};

CattleInterpreter*     cattle_interpreter_new                 (void);
gboolean               cattle_interpreter_run                 (CattleInterpreter      *interpreter,
                                                               GError                **error);
void                   cattle_interpreter_set_configuration   (CattleInterpreter      *interpreter,
                                                               CattleConfiguration    *configuration);
CattleConfiguration*   cattle_interpreter_get_configuration   (CattleInterpreter      *interpreter);
void                   cattle_interpreter_set_program         (CattleInterpreter      *interpreter,
                                                               CattleProgram          *program);
CattleProgram*         cattle_interpreter_get_program         (CattleInterpreter      *interpreter);
void                   cattle_interpreter_set_tape            (CattleInterpreter      *interpreter,
                                                               CattleTape             *tape);
CattleTape*            cattle_interpreter_get_tape            (CattleInterpreter      *interpreter);

GType                  cattle_interpreter_get_type            (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __CATTLE_INTERPRETER_H__ */
