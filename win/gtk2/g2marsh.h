
#ifndef __g2_marshal_MARSHAL_H__
#define __g2_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS
/* VOID:INT,INT,INT (gen_marsh_input:1) */
extern void g2_marshal_VOID__INT_INT_INT (GClosure * closure,
    GValue * return_value,
    guint n_param_values,
    const GValue * param_values,
    gpointer invocation_hint, gpointer marshal_data);

/* VOID:INT,STRING (gen_marsh_input:2) */
extern void g2_marshal_VOID__INT_STRING (GClosure * closure,
    GValue * return_value,
    guint n_param_values,
    const GValue * param_values,
    gpointer invocation_hint, gpointer marshal_data);

/* VOID:INT,POINTER,CHAR,CHAR,INT,STRING,BOOLEAN (gen_marsh_input:3) */
extern void g2_marshal_VOID__INT_POINTER_CHAR_CHAR_INT_STRING_BOOLEAN (GClosure
    * closure, GValue * return_value, guint n_param_values,
    const GValue * param_values, gpointer invocation_hint,
    gpointer marshal_data);

/* INT:INT,POINTER (gen_marsh_input:4) */
extern void g2_marshal_INT__INT_POINTER (GClosure * closure,
    GValue * return_value,
    guint n_param_values,
    const GValue * param_values,
    gpointer invocation_hint, gpointer marshal_data);

/* DOUBLE:DOUBLE (gen_marsh_input:5) */
extern void g2_marshal_DOUBLE__DOUBLE (GClosure * closure,
    GValue * return_value,
    guint n_param_values,
    const GValue * param_values,
    gpointer invocation_hint, gpointer marshal_data);

G_END_DECLS
#endif /* __g2_marshal_MARSHAL_H__ */
