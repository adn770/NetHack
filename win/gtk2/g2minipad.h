/* minipad widget
 *
 * g2minipad.h
 *
 * The mini game pad control.
 */

#ifndef __GTK_MINIPAD_H__
#define __GTK_MINIPAD_H__

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>

#define GTK_MINIPAD(obj)          GTK_CHECK_CAST (obj, gtk_minipad_get_type (), GtkMinipad)
#define GTK_MINIPAD_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_minipad_get_type (), GtkMinipadClass)
#define GTK_IS_MINIPAD(obj)       GTK_CHECK_TYPE (obj, gtk_minipad_get_type ())


  typedef struct _GtkMinipad GtkMinipad;
  typedef struct _GtkMinipadClass GtkMinipadClass;

  struct _GtkMinipad
  {
    GtkWidget widget;

    /* Button currently pressed or 0 if none */
    guint8 button;

    /* Hotpoints */
    GdkRectangle hotzone[11];
    gint current_dir;

    guint32 timer;

  };

  struct _GtkMinipadClass
  {
    GtkWidgetClass parent_class;
  };

  GtkWidget *gtk_minipad_new (void);
  GType gtk_minipad_get_type (void);

#endif                          /* __GTK_MINIPAD_H__ */
