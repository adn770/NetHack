/* the menu window as a GObject
 *
 * we derive from <gtk_dialog> to build our own menu window widget with our own signals which
 * g2bind will emit on calls from the nethack engine
 *
 * $Id: g2menu.h,v 1.1.1.1 2004/06/23 02:01:44 miq Exp $
 *
 */

#ifndef G2_MENU_DIALOG_H
#define G2_MENU_DIALOG_H

#include <gtk/gtk.h>

#define TYPE_G2_MENU (g2_menu_get_type())
#define G2_MENU(obj)          GTK_CHECK_CAST(obj, g2_menu_get_type(), G2Menu)
#define G2_MENU_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, g2_menu_get_type(), G2MenuClass)
#define IS_G2_MENU(obj)       GTK_CHECK_TYPE(obj, g2_menu_get_type())

typedef struct
{
  GtkDialog menuDialog;

  GtkWidget *menuWin;
  GtkWidget *view;
  GtkTextBuffer *messageBuffer;
  GtkListStore *menuTree;
  gboolean isSetUp;
  gchar nextAccelerator;
  guint currentCategory;
  guint wearing;
  GtkWidget *context_menu;
} G2Menu;

typedef struct
{
  GtkDialogClass parent_class;

  void (*g2_menu_start_menu) (G2Menu * g2Menu);
  void (*g2_menu_add_menu) (G2Menu * g2Menu);
  void (*g2_menu_end_menu) (G2Menu * g2Menu);
    gint (*g2_menu_select_menu) (G2Menu * g2Menu);
  void (*g2_menu_putstr) (G2Menu * g2Menu);
  void (*g2_menu_display) (G2Menu * g2Menu);
  void (*g2menu) (G2Menu * g2Menu);
} G2MenuClass;

GType g2_menu_get_type (void);
GtkWidget *g2_menu_new (void);

#endif /* G2_MENU_DIALOG_H */
