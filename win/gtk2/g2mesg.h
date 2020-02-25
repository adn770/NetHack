/* the message window
 *
 * $Id: g2mesg.h,v 1.1.1.1 2004/06/23 02:01:44 miq Exp $
 *
 */

#ifndef G2_MESSAGE_WINDOW_H
#define G2_MESSAGE_WINDOW_H

#include <gtk/gtk.h>

#define TYPE_G2_MESSAGE (g2_message_get_type())
#define G2_MESSAGE(obj)          GTK_CHECK_CAST(obj, g2_message_get_type(), G2Message)
#define G2_MESSAGE_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, g2_message_get_type(), G2MessageClass)
#define IS_G2_MESSAGE(obj)       GTK_CHECK_TYPE(obj, g2_message_get_type())

typedef struct
{
  GtkVBox messageContainer;

  GtkWidget *messageWin;
  GtkWidget *messageView;
  GtkTextBuffer *messageBuffer;
} G2Message;

typedef struct
{
  GtkVBoxClass parent_class;

  void (*g2_message_putstr) (G2Message * g2Message);
  void (*g2_message_clear) (G2Message * g2Message);
  void (*g2_message_display) (G2Message * g2Message);
  void (*g2_message_show_ext_command) (G2Message * g2Message);
  void (*g2message) (G2Message * g2Message);
} G2MessageClass;

GtkWidget *g2_message_new ();

#endif /* G2_MESSAGE_WINDOW_H */
