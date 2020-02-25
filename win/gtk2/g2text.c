/* the text window for larger amounts of text. It is used for displaying files e.g.
 * the game instructions, command explanation and the rip window
 *
 * $Id: g2text.c,v 1.1.1.1 2004/06/23 02:01:46 miq Exp $
 *
 */

#include <string.h>

#include "g2text.h"
#include "g2marsh.h"

#define G2_TEXT_WIDTH 700
#define G2_TEXT_HEIGHT 400

G2Text *g2Text;
static GtkWindowClass *parent_class;
static gint g2_text_signals[5];

static void g2_text_class_init (G2TextClass * class);
static void g2_text_init (G2Text * win);
static void g2_text_putstr (G2Text * win, int attr, const char *text,
    gpointer gp);


static const GTypeInfo g2_text_info = {
  sizeof (G2TextClass),
  NULL,                         /* base_init */
  NULL,                         /* base_finalize */
  (GClassInitFunc) g2_text_class_init,
  NULL,                         /* class_finalize */
  NULL,                         /* class_data */
  sizeof (G2Text),
  0,                            /* n_preallocs */
  (GInstanceInitFunc) g2_text_init
};

GType
g2_text_get_type ()
{
  static GType g2_text_type = 0;

  if (g2_text_type == 0) {
    g2_text_type = g_type_register_static (GTK_TYPE_DIALOG,
        "G2Text", &g2_text_info, 0);
  }
  return g2_text_type;
}

static void
g2_text_class_init (G2TextClass * class)
{
  parent_class = gtk_type_class (gtk_window_get_type ());

  g2_text_signals[0] =
      g_signal_new ("putstr",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2TextClass, g2_text_putstr),
      NULL, NULL,
      g2_marshal_VOID__INT_STRING, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
  g2_text_signals[1] =
      g_signal_new ("display",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2TextClass, g2_text_display),
      NULL, NULL, gtk_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
  g2_text_signals[2] =
      g_signal_new ("outrip",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2TextClass, g2_text_outrip),
      NULL, NULL, gtk_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

static void
g2_text_init (G2Text * win)
{
  GtkWidget *okButton;
  PangoFontDescription *fontDescription;

  win->messageBuffer = gtk_text_buffer_new (NULL);
  gtk_window_set_title (GTK_WINDOW (win), "G2Hack - Text window");
  win->messageWin = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (win->messageWin, "text window");

  gtk_container_set_border_width (GTK_CONTAINER (win->messageWin), 5);
  gtk_widget_set_size_request (win->messageWin, G2_TEXT_WIDTH, G2_TEXT_HEIGHT);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (win->messageWin),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  win->messageView = gtk_text_view_new ();
  fontDescription = pango_font_description_from_string ("monospace");
  gtk_widget_modify_font (win->messageView, fontDescription);
  pango_font_description_free (fontDescription);
  gtk_text_view_set_buffer (GTK_TEXT_VIEW (win->messageView),
      win->messageBuffer);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (win->messageView), FALSE);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (win->messageView), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (win->messageView), GTK_WRAP_NONE);
  gtk_container_add (GTK_CONTAINER (win->messageWin), win->messageView);
  gtk_box_pack_end (GTK_BOX (GTK_DIALOG (win)->vbox), win->messageWin, TRUE,
      TRUE, 0);
  okButton =
      gtk_dialog_add_button (GTK_DIALOG (win), GTK_STOCK_OK,
      GTK_RESPONSE_ACCEPT);
  gtk_window_set_default (GTK_WINDOW (win), okButton);
}

static void
g2_text_putstr (G2Text * win, int attr, const char *text, gpointer gp)
{
  gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER (win->messageBuffer),
      text, strlen (text));
  gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER (win->messageBuffer),
      "\n", 1);
}

static void
g2_text_display (GtkWidget * win, gboolean block, gpointer gp)
{
  gint result;

  gtk_widget_show_all (GTK_WIDGET (win));
  gtk_window_set_modal (GTK_WINDOW (win), TRUE);
  result = gtk_dialog_run (GTK_DIALOG (win));
}

static void
g2_text_outrip (GtkWidget * win, int how, gpointer gp)
{
  GtkWidget *tombstone;

  tombstone = gtk_image_new_from_file ("rip.xpm");
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (win)->vbox), tombstone, FALSE,
      FALSE, 0);
  gtk_widget_set_size_request (G2_TEXT (win)->messageWin, G2_TEXT_WIDTH,
      G2_TEXT_HEIGHT / 2);
}

GtkWidget *
g2_text_new ()
{
  g2Text = G2_TEXT (g_object_new (TYPE_G2_TEXT, NULL));
  g_signal_connect (G_OBJECT (g2Text), "putstr",
      G_CALLBACK (g2_text_putstr), NULL);
  g_signal_connect (G_OBJECT (g2Text), "display",
      G_CALLBACK (g2_text_display), NULL);
  g_signal_connect (G_OBJECT (g2Text), "outrip",
      G_CALLBACK (g2_text_outrip), NULL);
  return GTK_WIDGET (g2Text);
}
