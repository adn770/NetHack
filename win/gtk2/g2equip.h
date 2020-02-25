/* the equipment window
 *
 * $Id: g2equip.h,v 1.1.1.1 2004/06/23 02:01:44 miq Exp $
 *
 */

#ifndef G2_EQUIPMENT_WINDOW_H
#define G2_EQUIPMENT_WINDOW_H

#include <gtk/gtk.h>

#define TYPE_G2_EQUIPMENT (g2_equipment_get_type())
#define G2_EQUIPMENT(obj)          GTK_CHECK_CAST(obj, g2_equipment_get_type(), G2Equipment)
#define G2_EQUIPMENT_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, g2_equipment_get_type(), G2EquipmentClass)
#define IS_G2_EQUIPMENT(obj)       GTK_CHECK_TYPE(obj, g2_equipment_get_type())

typedef struct
{
  GtkVBox equipContainer;

  GtkWidget *equipWin;
  GdkPixmap *backBuffer;
  GdkPixbuf *equipOutline;
} G2Equipment;

typedef struct
{
  GtkVBoxClass parent_class;

  void (*g2_equipment_update) (G2Equipment * g2Equipment);
  void (*g2_equipment_display) (G2Equipment * g2Equipment);
  void (*g2equipment) (G2Equipment * g2Equipment);
} G2EquipmentClass;

GtkWidget *g2_equipment_new ();

#endif /* G2_EQUIPMENT_WINDOW_H */
