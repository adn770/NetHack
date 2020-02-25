/* the text window as a GObject
 *
 * we derive from <gtk_dialog> to build our own text window widget with our own signals which
 * g2bind will emit on calls from the nethack engine
 *
 * $Id: g2text.h,v 1.1.1.1 2004/06/23 02:01:46 miq Exp $
 *
 */

#ifndef G2_TEXT_DIALOG_H
#define G2_TEXT_DIALOG_H

#include <gtk/gtk.h>

#define TYPE_G2_TEXT (g2_text_get_type())
#define G2_TEXT(obj)          GTK_CHECK_CAST(obj, g2_text_get_type(), G2Text)
#define G2_TEXT_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, g2_text_get_type(), G2TextClass)
#define IS_G2_TEXT(obj)       GTK_CHECK_TYPE(obj, g2_text_get_type())

typedef struct
{
  GtkDialog textDialog;

  GtkWidget *messageWin;
  GtkWidget *messageView;
  GtkTextBuffer *messageBuffer;
} G2Text;

typedef struct
{
  GtkDialogClass parent_class;

  void (*g2_text_putstr) (G2Text * g2Text);
  void (*g2_text_display) (G2Text * g2Text);
  void (*g2_text_outrip) (G2Text * g2Text);
  void (*g2text) (G2Text * g2Text);
} G2TextClass;

GType g2_text_get_type (void);
GtkWidget *g2_text_new (void);

#endif /* G2_TEXT_DIALOG_H */
