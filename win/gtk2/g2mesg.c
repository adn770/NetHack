/* the message window
 *
 * $Id: g2mesg.c,v 1.1.1.1 2004/06/23 02:01:44 miq Exp $
 *
 */

#include <string.h>

#include "config.h"

#include "g2mesg.h"
#include "g2marsh.h"

#define SCROLL_BACK_LINES 200
#define G2_MESSAGE_WIDTH 70
#define G2_MESSAGE_HEIGTH 60

static GtkVBoxClass *parent_class;
static gint g2_message_signals[4];

static void g2_message_class_init (G2MessageClass * class);
static void g2_message_init (G2Message * message);


static const GTypeInfo g2_message_info = {
  sizeof (G2MessageClass),
  NULL,                         /* base_init */
  NULL,                         /* base_finalize */
  (GClassInitFunc) g2_message_class_init,
  NULL,                         /* class_finalize */
  NULL,                         /* class_data */
  sizeof (G2Message),
  0,                            /* n_preallocs */
  (GInstanceInitFunc) g2_message_init
};

GType
g2_message_get_type ()
{
  static GType g2_message_type = 0;

  if (g2_message_type == 0) {
    g2_message_type = g_type_register_static (GTK_TYPE_VBOX,
        "G2Message", &g2_message_info, 0);
  }
  return g2_message_type;
}

static void
g2_message_class_init (G2MessageClass * class)
{
  parent_class = gtk_type_class (gtk_vbox_get_type ());

/*    g2_message_signals[0] =
        g_signal_new("curs",
                     G_OBJECT_CLASS_TYPE(class),
                     G_SIGNAL_RUN_FIRST,
                     G_STRUCT_OFFSET(G2StatusClass, g2_status_curs),
                     NULL, NULL,
                     gtk_marshal_VOID__INT_INT, G_TYPE_NONE, 2, G_TYPE_INT,
                     G_TYPE_INT);*/
  g2_message_signals[0] =
      g_signal_new ("putstr",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MessageClass, g2_message_putstr),
      NULL, NULL,
      g2_marshal_VOID__INT_STRING, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
  g2_message_signals[1] =
      g_signal_new ("clear",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MessageClass, g2_message_clear),
      NULL, NULL, gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);
  g2_message_signals[2] =
      g_signal_new ("display",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MessageClass, g2_message_display),
      NULL, NULL, gtk_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
  g2_message_signals[3] =
      g_signal_new ("show_ext_command",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MessageClass,
          g2_message_show_ext_command), NULL, NULL,
      gtk_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);

}

/* XXX: attr currently ignored */
static void
g2_message_putstr (GtkWidget * win, int attr, gchar * text, gpointer gp)
{
  int lineCount;
  GtkTextIter start;
  GtkTextIter deletionEnd;
  GtkTextIter end;
  G2Message *g2Message = G2_MESSAGE (win);

  gtk_text_buffer_get_end_iter (g2Message->messageBuffer, &end);
  gtk_text_buffer_place_cursor (g2Message->messageBuffer, &end);
  gtk_text_buffer_insert_at_cursor (g2Message->messageBuffer, text,
      strlen (text));
  gtk_text_buffer_insert_at_cursor (g2Message->messageBuffer, "\n", 1);
  lineCount = gtk_text_buffer_get_line_count (g2Message->messageBuffer);
  if (lineCount > SCROLL_BACK_LINES) {
    gtk_text_buffer_get_start_iter (g2Message->messageBuffer, &start);
    gtk_text_buffer_get_iter_at_line (g2Message->messageBuffer,
        &deletionEnd, lineCount - SCROLL_BACK_LINES);
    gtk_text_buffer_delete (g2Message->messageBuffer, &start, &deletionEnd);
  }
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (g2Message->messageView),
      gtk_text_buffer_get_insert (g2Message->messageBuffer));
}

static void
g2_message_clear (GtkWidget * win, gpointer gp)
{
  /* we do nothing since we have a nice buffer */
}

static void
g2_message_display (GtkWidget * win, gboolean block, gpointer gp)
{
  gtk_widget_show_all (win);
}

static void
g2_message_show_ext_command (GtkWidget * win, gchar * command, gpointer gp)
{
  GtkTextIter start;
  GtkTextIter end;
  gchar commandString[12];

  g_strlcpy (commandString, "# ", 12);
  g_strlcat (commandString, command, 12);
  gtk_text_buffer_get_end_iter (G2_MESSAGE (win)->messageBuffer, &end);
  start = end;
  gtk_text_iter_set_line_offset (&start, 0);
  gtk_text_buffer_delete (G2_MESSAGE (win)->messageBuffer, &start, &end);
  gtk_text_buffer_insert_at_cursor (G2_MESSAGE (win)->messageBuffer,
      commandString, strlen (commandString));
}

static void
g2_message_init (G2Message * message)
{
  gtk_widget_set_name (GTK_WIDGET (message), "message window");

  message->messageWin = gtk_scrolled_window_new (NULL, NULL);
  message->messageView = gtk_text_view_new ();
  message->messageBuffer =
      gtk_text_view_get_buffer (GTK_TEXT_VIEW (message->messageView));
  /* XXX: cursor is visible for debugging purposes only */
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (message->messageView), TRUE);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (message->messageView), FALSE);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (message->messageView), 5);
  g_object_set (G_OBJECT (message->messageView), "can-focus", FALSE, NULL);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (message->messageView),
      GTK_WRAP_WORD);
  gtk_widget_set_size_request (message->messageWin, G2_MESSAGE_WIDTH,
      G2_MESSAGE_HEIGTH);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (message->messageWin),
      GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_container_add (GTK_CONTAINER (message->messageWin), message->messageView);
  gtk_box_pack_start (GTK_BOX (message), message->messageWin, TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (message));
}


GtkWidget *
g2_message_new ()
{
  G2Message *g2Message;

  g2Message = G2_MESSAGE (g_object_new (TYPE_G2_MESSAGE, NULL));
  g_signal_connect (G_OBJECT (g2Message), "putstr",
      G_CALLBACK (g2_message_putstr), NULL);
  g_signal_connect (G_OBJECT (g2Message), "clear",
      G_CALLBACK (g2_message_clear), NULL);
  g_signal_connect (G_OBJECT (g2Message), "display",
      G_CALLBACK (g2_message_display), NULL);
  g_signal_connect (G_OBJECT (g2Message), "show_ext_command",
      G_CALLBACK (g2_message_show_ext_command), NULL);

  return GTK_WIDGET (g2Message);
}
