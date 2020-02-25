/* the inventory window
 *
 * $Id: g2invent.c,v 1.3 2004/07/20 00:08:56 miq Exp $
 *
 */

#include "config.h"

#include "g2invent.h"
#include "g2marsh.h"
#include "g2map.h"
#include "g2i18n.h"
#include "hack.h"

G2Inventory *g2Inventory;
static GtkWindowClass *parent_class;
GtkWidget *scrolledWin;
static gint g2_inventory_signals[5];
static GtkTreeIter parent_iter;
static GtkTreeIter child_iter;

static void g2_inventory_class_init (G2InventoryClass * class);
static void g2_inventory_init (G2Inventory * inventory);
static void g2_inventory_start_menu (GtkWidget * win, gpointer gp);
static void g2_inventory_add_menu (GtkWidget * win, int glyph,
    gpointer * identifier, gchar accelerator,
    gchar group_accel, int attr, gchar * str, gboolean presel, gpointer gp);

enum
{
  G2_INVENTORY_HEADER,
  G2_INVENTORY_ACCEL,
  G2_INVENTORY_GLYPH,
  G2_INVENTORY_STRING,
  G2_INVENTORY_IDENTIFIER,
  G2_INVENTORY_IS_HEADER,
  G2_INVENTORY_PRESEL,
  G2_INVENTORY_COL_NUM
};



static const GTypeInfo g2_inventory_info = {
  sizeof (G2InventoryClass),
  NULL,                         /* base_init */
  NULL,                         /* base_finalize */
  (GClassInitFunc) g2_inventory_class_init,
  NULL,                         /* class_finalize */
  NULL,                         /* class_data */
  sizeof (G2Inventory),
  0,                            /* n_preallocs */
  (GInstanceInitFunc) g2_inventory_init
};

GType
g2_inventory_get_type ()
{
  static GType g2_inventory_type = 0;

  if (g2_inventory_type == 0) {
    g2_inventory_type = g_type_register_static (GTK_TYPE_WINDOW,
        "G2Inventory", &g2_inventory_info, 0);
  }
  return g2_inventory_type;
}

static void
g2_inventory_class_init (G2InventoryClass * class)
{
  parent_class = gtk_type_class (gtk_window_get_type ());

  g2_inventory_signals[0] =
      g_signal_new ("start_menu",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2InventoryClass, g2_inventory_start_menu),
      NULL, NULL, gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);
  g2_inventory_signals[1] =
      g_signal_new ("add_menu",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2InventoryClass, g2_inventory_add_menu),
      NULL, NULL,
      g2_marshal_VOID__INT_POINTER_CHAR_CHAR_INT_STRING_BOOLEAN,
      G_TYPE_NONE, 7, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_CHAR,
      G_TYPE_CHAR, G_TYPE_INT, G_TYPE_STRING, G_TYPE_BOOLEAN);
  g2_inventory_signals[0] =
      g_signal_new ("end_menu",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2InventoryClass, g2_inventory_end_menu),
      NULL, NULL, gtk_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
g2_inventory_init (G2Inventory * inventory)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  gtk_window_set_title (GTK_WINDOW (inventory), _("Inventory"));
  scrolledWin = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (scrolledWin, 600, 600);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledWin),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  inventory->invView = gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (inventory->invView), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (inventory->invView), TRUE);
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
      "text", G2_INVENTORY_ACCEL, "underline", G2_INVENTORY_IS_HEADER, NULL);
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
      "pixbuf", G2_INVENTORY_GLYPH, NULL);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer,
      "text", G2_INVENTORY_STRING, "underline", G2_INVENTORY_IS_HEADER, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->invView), column);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolledWin),
      inventory->invView);
  gtk_container_add (GTK_CONTAINER (inventory), scrolledWin);
  gtk_widget_show (scrolledWin);
}

static void
g2_inventory_start_menu (GtkWidget * win, gpointer gp)
{
  g2Inventory->invTree = gtk_tree_store_new (G2_INVENTORY_COL_NUM,
      G_TYPE_STRING,
      G_TYPE_STRING,
      GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_BOOLEAN);
}

/* XXX: preselection, attr and group_accel currently ignored */
static void
g2_inventory_add_menu (GtkWidget * win, gint glyph,
    gpointer * identifier, gchar accelerator,
    gchar group_accel, gint attr, gchar * str, gboolean presel, gpointer gp)
{
  gchar accelString[2];
  GdkPixbuf *tile;

  if (glyph == NO_GLYPH) {
    tile = NULL;
  } else {
    tile = g2_get_tile_scaled (glyph, 20);
  }
  if (((int) *identifier) == 0) {
    gtk_tree_store_append (g2Inventory->invTree, &parent_iter, NULL);
    gtk_tree_store_set (g2Inventory->invTree, &parent_iter,
        G2_INVENTORY_GLYPH, tile,
        G2_INVENTORY_STRING, str,
        G2_INVENTORY_IS_HEADER, PANGO_UNDERLINE_SINGLE,
        G2_INVENTORY_IDENTIFIER, *identifier, -1);
  } else {
    accelString[0] = accelerator;
    accelString[1] = '\0';
    gtk_tree_store_append (g2Inventory->invTree, &child_iter, &parent_iter);
    gtk_tree_store_set (g2Inventory->invTree, &child_iter,
        G2_INVENTORY_ACCEL, &accelString,
        G2_INVENTORY_GLYPH, tile,
        G2_INVENTORY_STRING, str,
        G2_INVENTORY_IS_HEADER, PANGO_UNDERLINE_NONE,
        G2_INVENTORY_IDENTIFIER, *identifier, G2_INVENTORY_PRESEL, presel, -1);
  }
}

static void
g2_inventory_end_menu (GtkWidget * win, const gchar * prompt, gpointer gp)
{
  gtk_tree_view_set_model (GTK_TREE_VIEW (g2Inventory->invView),
      GTK_TREE_MODEL (g2Inventory->invTree));
  gtk_tree_view_expand_all (GTK_TREE_VIEW (g2Inventory->invView));
  gtk_widget_show (g2Inventory->invView);
}

GtkWidget *
g2_inventory_new ()
{
  /* XXX singleton */
  if (!g2Inventory) {
    g2Inventory = G2_INVENTORY (g_object_new (TYPE_G2_INVENTORY, NULL));
    g_signal_connect (G_OBJECT (g2Inventory), "start_menu",
        G_CALLBACK (g2_inventory_start_menu), NULL);
    g_signal_connect (G_OBJECT (g2Inventory), "add_menu",
        G_CALLBACK (g2_inventory_add_menu), NULL);
    g_signal_connect (G_OBJECT (g2Inventory), "end_menu",
        G_CALLBACK (g2_inventory_end_menu), NULL);
  }
  return GTK_WIDGET (g2Inventory);
}

void
g2_display_inventory_window (int block)
{
  gtk_widget_show (GTK_WIDGET (g2Inventory));
}
