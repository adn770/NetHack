/* the inventory window as a GObject
 *
 * we derive from <gtk_dialog> to build our own widget with our own signals
 * which g2bind will emit on calls from the nethack engine
 *
 * $Id: g2invent.h,v 1.1.1.1 2004/06/23 02:01:44 miq Exp $
 *
 */

#ifndef G2_INVENTORY_WINDOW_H
#define G2_INVENTORY_WINDOW_H

#include <gtk/gtk.h>

#define TYPE_G2_INVENTORY (g2_inventory_get_type())
#define G2_INVENTORY(obj)          GTK_CHECK_CAST(obj, g2_inventory_get_type(), G2Inventory)
#define G2_INVENTORY_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, g2_inventory_get_type(), G2InventoryClass)
#define IS_G2_INVENTORY(obj)       GTK_CHECK_TYPE(obj, g2_inventory_get_type())

typedef struct
{
  GtkWindow inventoryWindow;

  GtkTreeStore *invTree;
  GtkWidget *invView;
} G2Inventory;

typedef struct
{
  GtkWindowClass parent_class;

  void (*g2_inventory_start_menu) (G2Inventory * g2Inventory);
  void (*g2_inventory_add_menu) (G2Inventory * g2Inventory);
  void (*g2_inventory_end_menu) (G2Inventory * g2Inventory);
  void (*g2inventory) (G2Inventory * g2Inventory);
} G2InventoryClass;

GType g2_inventory_get_type (void);
GtkWidget *g2_inventory_new (void);

void g2_display_inventory_window (int block);

#endif /* G2_INVENTORY_WINDOW_H */
