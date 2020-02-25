/* the menu window
 *
 * $Id: g2menu.c,v 1.3 2004/07/20 00:08:56 miq Exp $
 *
 */

#include <string.h>
#include <gdk/gdkkeysyms.h>

#include "config.h"

#include "g2main.h"
#include "g2menu.h"
#include "g2marsh.h"
#include "g2map.h"              /* for g2_get_tile() */
#include "g2i18n.h"
#include "hack.h"

#define G2_MENU_MSG_WIDTH 640
#define G2_MENU_MSG_HEIGHT 300
#define G2_MENU_MENU_WIDTH 640
#define G2_MENU_MENU_HEIGTH 400

static GtkDialogClass *parent_class;
static gint g2_menu_signals[10];
static GtkTreeIter iter;

static void g2_menu_class_init (G2MenuClass * class);
static void g2_menu_init (G2Menu * menu);
static void g2_menu_putstr (G2Menu * win, int attr, const char *text,
    gpointer gp);

enum
{
  G2_MENU_HEADER,
  G2_MENU_ACCEL,
  G2_MENU_GLYPH,
  G2_MENU_STRING,
  G2_MENU_IDENTIFIER,
  G2_MENU_ATTRIBUTES,
  G2_MENU_PRESEL,
  G2_MENU_CATEGORY,
  G2_MENU_COL_NUM
};

enum
{
  INV_NOTHING = 0,
  INV_WEAPON = 1,
  INV_ARMOR = 2,
  INV_COMESTIBLE = 4,
  INV_SCROLL = 8,
  INV_SPELLBOOK = 16,
  INV_POTION = 32,
  INV_RING = 64,
  INV_WAND = 128,
  INV_TOOL = 256,
  INV_GEM = 512,
  INV_ALL = 1023
};

typedef struct
{
  gchar *name;
  guint category;
} ItemCategory;

static ItemCategory inventory_categories[] = {
  {"Weapons", INV_WEAPON},
  {"Armor", INV_ARMOR},
  {"Comestibles", INV_COMESTIBLE},
  {"Scrolls", INV_SCROLL},
  {"Spellbooks", INV_SPELLBOOK},
  {"Potions", INV_POTION},
  {"Rings", INV_RING},
  {"Wands", INV_WAND},
  {"Tools", INV_TOOL},
  {"Gems", INV_GEM},
};

typedef struct
{
  gchar *name;
  guint category;
  gint command;
  gint item;
  gpointer dialog;
} ActionCategory;

static ActionCategory popup_actions[] = {
  {"Drop", INV_ALL, 'd', ' ', NULL},
  {"Eat", INV_COMESTIBLE, 'e', ' ', NULL},
  {"PutOn", INV_RING | INV_TOOL, 'P', ' ', NULL},
  {"Quaff", INV_POTION, 'q', ' ', NULL},
  {"Quiver", INV_ALL, 'Q', ' ', NULL},
  {"Read", INV_SCROLL | INV_SPELLBOOK, 'r', ' ', NULL},
  {"Remove", INV_RING | INV_TOOL, 'R', ' ', NULL},
  {"Throw", INV_ALL, 't', ' ', NULL},
  {"TakeOff", INV_ARMOR, 'T', ' ', NULL},
  {"Wield", INV_WEAPON, 'w', ' ', NULL},
  {"Wear", INV_ARMOR, 'W', ' ', NULL},
  {"Zap", INV_WAND, 'z', ' ', NULL},
  {"Dip", INV_ALL, META_BIT | GDK_d, ' ', NULL},
  {"Offer", INV_COMESTIBLE, META_BIT | GDK_o, ' ', NULL},
  {"Rub", INV_TOOL, META_BIT | GDK_r, ' ', NULL},
};


static const GTypeInfo g2_menu_info = {
  sizeof (G2MenuClass),
  NULL,                         /* base_init */
  NULL,                         /* base_finalize */
  (GClassInitFunc) g2_menu_class_init,
  NULL,                         /* class_finalize */
  NULL,                         /* class_data */
  sizeof (G2Menu),
  0,                            /* n_preallocs */
  (GInstanceInitFunc) g2_menu_init
};

GType
g2_menu_get_type ()
{
  static GType g2_menu_type = 0;

  if (g2_menu_type == 0) {
    g2_menu_type = g_type_register_static (GTK_TYPE_DIALOG,
        "G2Menu", &g2_menu_info, 0);
  }
  return g2_menu_type;
}

static void
g2_menu_class_init (G2MenuClass * class)
{
  parent_class = gtk_type_class (gtk_dialog_get_type ());

  g2_menu_signals[0] =
      g_signal_new ("start_menu",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MenuClass, g2_menu_start_menu),
      NULL, NULL, gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);
  g2_menu_signals[1] =
      g_signal_new ("add_menu",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MenuClass, g2_menu_add_menu),
      NULL, NULL,
      g2_marshal_VOID__INT_POINTER_CHAR_CHAR_INT_STRING_BOOLEAN,
      G_TYPE_NONE, 7, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_CHAR,
      G_TYPE_CHAR, G_TYPE_INT, G_TYPE_STRING, G_TYPE_BOOLEAN);
  g2_menu_signals[2] =
      g_signal_new ("end_menu",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MenuClass, g2_menu_end_menu),
      NULL, NULL, gtk_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
  g2_menu_signals[3] =
      g_signal_new ("select_menu",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_ACTION,
      G_STRUCT_OFFSET (G2MenuClass, g2_menu_select_menu),
      NULL, NULL,
      g2_marshal_INT__INT_POINTER, G_TYPE_INT, 2, G_TYPE_INT, G_TYPE_POINTER);
  g2_menu_signals[4] =
      g_signal_new ("putstr",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MenuClass, g2_menu_putstr),
      NULL, NULL,
      g2_marshal_VOID__INT_STRING, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
  g2_menu_signals[5] =
      g_signal_new ("display",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2MenuClass, g2_menu_display),
      NULL, NULL, gtk_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
g2_menu_init (G2Menu * menu)
{
}

static gboolean
can_select_row (GtkTreeSelection * selection, GtkTreeModel * model,
    GtkTreePath * path, gboolean path_currently_selected, gpointer data)
{
  gpointer identifier;

  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
      G2_MENU_IDENTIFIER, &identifier, -1);
  if (identifier == NULL) {
    return FALSE;
  } else {
    return TRUE;
  }
}

static void
g2_menu_setup_text_message (G2Menu * win)
{
  GtkWidget *okButton;

//    PangoFontDescription *fontDescription;

  win->messageBuffer = gtk_text_buffer_new (NULL);
  gtk_window_set_title (GTK_WINDOW (win), _("Gtk2Hack - Message"));
  gtk_window_set_default_size (GTK_WINDOW (win), G2_MENU_MSG_WIDTH,
      G2_MENU_MSG_HEIGHT);
  win->menuWin = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (win->menuWin), 5);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (win->menuWin),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  win->view = gtk_text_view_new ();

//    fontDescription = pango_font_description_from_string("Sans");
//    gtk_widget_modify_font(win->view, fontDescription);
//    pango_font_description_free (fontDescription);

  gtk_text_view_set_buffer (GTK_TEXT_VIEW (win->view), win->messageBuffer);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (win->view), FALSE);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (win->view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (win->view), GTK_WRAP_WORD);
  gtk_container_add (GTK_CONTAINER (win->menuWin), win->view);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (win)->vbox), win->menuWin, TRUE,
      TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (GTK_DIALOG (win)->vbox));
  okButton = gtk_dialog_add_button (GTK_DIALOG (win), GTK_STOCK_OK,
      GTK_RESPONSE_OK);
  gtk_window_set_default (GTK_WINDOW (win), okButton);
}



static void
view_popup_menu_onDoAction (GtkWidget * menuitem, gpointer userdata)
{
  ActionCategory *ac = (ActionCategory *) userdata;
  G2Menu *win = G2_MENU (ac->dialog);

  skip_question = TRUE;

  keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (ac->command));
  if (ac->command == 'T') {
    if (win->wearing > 1)
      keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (ac->item));
  } else {
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (ac->item));
  }

  gtk_widget_hide (GTK_WIDGET (win));
}

static gboolean
setup_popup_menu (GtkWidget * treeview, gpointer userdata)
{
  GtkWidget *menuitem;
  gint i;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  guint category = INV_NOTHING;
  gchar *itemAccel;

  G2Menu *win = G2_MENU (userdata);
  GtkWidget *context_menu = win->context_menu;

  /* Destroy previous options */
  gtk_container_foreach (GTK_CONTAINER (context_menu),
      (GtkCallback) gtk_widget_destroy, NULL);

  /* Crate a new set of options */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get (model, &iter, G2_MENU_CATEGORY, &category,
        G2_MENU_ACCEL, &itemAccel, -1);
  }
  if (!category)
    return FALSE;

  for (i = 0; i < G_N_ELEMENTS (popup_actions); i++) {
    if (popup_actions[i].category & category) {
      if ((popup_actions[i].command == 'T') && (win->wearing == 0))
        continue;

      popup_actions[i].item = itemAccel[0];
      popup_actions[i].dialog = userdata;
      menuitem = gtk_menu_item_new_with_label (popup_actions[i].name);
      g_signal_connect (menuitem, "activate",
          (GCallback) view_popup_menu_onDoAction,
          (gpointer) & popup_actions[i]);
      gtk_menu_shell_append (GTK_MENU_SHELL (context_menu), menuitem);
    }
  }

  gtk_widget_show_all (context_menu);
  return TRUE;
}

static gboolean
g2_menu_view_key_press (GtkWidget * view, GdkEventKey * event, gpointer dialog)
{
  GtkTreeIter iter;
  gboolean valid;
  gboolean isValidAccel;
  gchar *itemAccel;
  GtkTreeSelection *selection;
  G2Menu *menuDialog = G2_MENU (dialog);

  g_assert (menuDialog != NULL);

  if ((dialog == g2_object_for_wid (WIN_INVEN)) && (event->keyval == GDK_F4)) {
    /* Note: event can be NULL here when called from view_onPopupMenu;
     *  gdk_event_get_time() accepts a NULL argument */
    if (setup_popup_menu (view, dialog)) {
      gtk_menu_popup (GTK_MENU (menuDialog->context_menu), \
          NULL, NULL, NULL, NULL, \
          0, gdk_event_get_time ((GdkEvent *) event));
    }

    return TRUE;                /* we handled this */
  }

  if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter) {
    gtk_window_activate_default (GTK_WINDOW (dialog));
    return TRUE;
  }
  isValidAccel = (event->keyval >= 'A' && event->keyval <= 'Z') ||
      (event->keyval >= 'a' && event->keyval <= 'z');
  if (isValidAccel && !(event->state & GDK_CONTROL_MASK)) {
    gchar *accelString = gdk_keyval_name (event->keyval);

    valid =
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (menuDialog->menuTree),
        &iter);
    while (valid) {
      gtk_tree_model_get (GTK_TREE_MODEL (menuDialog->menuTree), &iter,
          G2_MENU_ACCEL, &itemAccel, -1);
      if (!strncmp (accelString, itemAccel, 1)) {
        selection =
            gtk_tree_view_get_selection (GTK_TREE_VIEW (menuDialog->view));
        if (gtk_tree_selection_iter_is_selected (selection, &iter)) {
          gtk_tree_selection_unselect_iter (selection, &iter);
        } else {
          gtk_tree_selection_select_iter (selection, &iter);
        }
        return TRUE;
      }
      valid =
          gtk_tree_model_iter_next (GTK_TREE_MODEL (menuDialog->menuTree),
          &iter);
    }
  }
  return FALSE;
}

static gboolean
view_onButtonPressed (GtkWidget * treeview, GdkEventButton * event,
    gpointer userdata)
{
  G2Menu *win = G2_MENU (userdata);
  GtkTreeSelection *selection;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

  if (gtk_tree_selection_count_selected_rows (selection) <= 1) {
    GtkTreePath *path;

    /* Get tree path for row that was clicked */
    if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (treeview),
            (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) {
      gtk_tree_selection_unselect_all (selection);
      gtk_tree_selection_select_path (selection, path);
      gtk_tree_path_free (path);
    }
  }

  /* single click with the right mouse button? */
  if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
    /* Note: event can be NULL here when called from view_onPopupMenu;
     *  gdk_event_get_time() accepts a NULL argument */
    if (setup_popup_menu (treeview, userdata)) {
      gtk_menu_popup (GTK_MENU (win->context_menu), \
          NULL, NULL, NULL, NULL, \
          (event != NULL) ? event->button : 0, \
          gdk_event_get_time ((GdkEvent *) event));
    }

    return TRUE;                /* we handled this */
  }

  return FALSE;                 /* we did not handle this */
}

static void
g2_menu_setup_menu (G2Menu * win)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;

  gtk_window_set_default_size (GTK_WINDOW (win), G2_MENU_MENU_WIDTH,
      G2_MENU_MENU_HEIGTH);
  win->menuTree =
      gtk_list_store_new (G2_MENU_COL_NUM, G_TYPE_STRING, G_TYPE_STRING,
      GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_INT,
      G_TYPE_INT, G_TYPE_BOOLEAN, G_TYPE_UINT);
  win->menuWin = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (win->menuWin), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (win->menuWin),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  win->view = gtk_tree_view_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (win->view), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (win->view), TRUE);
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
      "text", G2_MENU_ACCEL, "underline", G2_MENU_ATTRIBUTES, NULL);
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
      "pixbuf", G2_MENU_GLYPH, NULL);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer,
      "text", G2_MENU_STRING, "underline", G2_MENU_ATTRIBUTES, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (win->view), column);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (win->view));
  gtk_tree_selection_set_select_function (selection, &can_select_row, NULL,
      NULL);
  g_signal_connect (G_OBJECT (win->view), "key-press-event",
      G_CALLBACK (g2_menu_view_key_press), win);


  if (win == (G2Menu *) g2_object_for_wid (WIN_INVEN)) {
    g_signal_connect (G_OBJECT (win->view), "button_press_event",
        G_CALLBACK (view_onButtonPressed), win);
  }
  win->context_menu = gtk_menu_new ();
#ifdef HILDON
  gtk_widget_tap_and_hold_setup (win->view, win->context_menu, NULL, 0);
#endif

  gtk_widget_set_name (GTK_WIDGET (win), "menu dialog");
  gtk_container_add (GTK_CONTAINER (win->menuWin), win->view);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (win)->vbox),
      win->menuWin, TRUE, TRUE, 0);
  gtk_dialog_add_buttons (GTK_DIALOG (win),
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (win), GTK_RESPONSE_OK);
  g_signal_connect (G_OBJECT (win), "delete-event", G_CALLBACK (gtk_true),
      NULL);

}

static void
g2_menu_start_menu (G2Menu * win, gpointer gp)
{
  if (!win->isSetUp) {
    g2_menu_setup_menu (win);
    win->isSetUp = TRUE;
  }
  gtk_list_store_clear (win->menuTree);
  win->nextAccelerator = 'a';
  win->currentCategory = INV_NOTHING;
  win->wearing = 0;
}

static guint
categoryParser (gchar * string)
{
  guint retval = INV_NOTHING;
  gint i;

  for (i = 0; i < G_N_ELEMENTS (inventory_categories); i++) {
    if (strcmp (string, inventory_categories[i].name) == 0) {
      retval = inventory_categories[i].category;
      break;
    }
  }

  return retval;
}

/* XXX: preselection, attr and group_accel currently ignored */
static void
g2_menu_add_menu (G2Menu * win, gint glyph,
    anything * identifier, gchar accelerator,
    gchar group_accel, gint attr, gchar * str, gboolean presel, gpointer gp)
{
  gchar accelString[2];
  GdkPixbuf *tile = NULL;
  PangoUnderline underline = PANGO_UNDERLINE_NONE;

  g_assert (IS_G2_MENU (win));

  if (glyph != NO_GLYPH) {
    tile = g2_get_tile_scaled (glyph, 20);
    // tile = g2_get_tile(glyph);
  }
  if (attr != ATR_NONE) {
    underline = PANGO_UNDERLINE_SINGLE;
  }
  gtk_list_store_append (win->menuTree, &iter);

  if (identifier->a_obj == NULL) {
    accelString[0] = '\0';
    presel = FALSE;
    if (win == (G2Menu *) g2_object_for_wid (WIN_INVEN))
      win->currentCategory = categoryParser (str);
  } else if (accelerator != '\0') {
    accelString[0] = accelerator;
    accelString[1] = '\0';
    if (strstr (str, "being worn") != NULL)
      win->wearing++;
  } else {

#ifdef HILDON
    accelString[0] = '\0';      /* inventing accelerators is irrelevant for hildon */
#else
    accelString[0] = win->nextAccelerator;
    accelString[1] = '\0';
    /* XXX: what should we do if we have more than 52 items? */
    if (win->nextAccelerator >= 'a' && win->nextAccelerator < 'z') {
      win->nextAccelerator++;
    } else if (win->nextAccelerator == 'z') {
      win->nextAccelerator = 'A';
    } else if (win->nextAccelerator < 'Z') {
      win->nextAccelerator++;
    } else {
      win->nextAccelerator = ' ';
    }
#endif

  }
  gtk_list_store_set (win->menuTree, &iter,
      G2_MENU_ACCEL, &accelString,
      G2_MENU_GLYPH, tile,
      G2_MENU_STRING, str,
      G2_MENU_ATTRIBUTES, underline,
      G2_MENU_IDENTIFIER, identifier->a_obj,
      G2_MENU_PRESEL, presel, G2_MENU_CATEGORY, win->currentCategory, -1);
}

static void
g2_menu_end_menu (G2Menu * win, const gchar * prompt, gpointer gp)
{
  g_assert (IS_G2_MENU (win));

  gtk_tree_view_set_model (GTK_TREE_VIEW (win->view),
      GTK_TREE_MODEL (win->menuTree));
  gtk_container_check_resize (GTK_CONTAINER (win));
}

static gint
g2_menu_select_menu (G2Menu * win, gint how, gpointer selected, gpointer gp)
{
  gint response;
  GList *selectedRows;
  GtkTreeIter iter;
  gint selectedCount;
  gpointer identifier;
  MENU_ITEM_P *items;
  gint currentItem = 0;
  GtkTreeSelection *selection;

  g_assert (IS_G2_MENU (win));

  if (win == (G2Menu *) g2_object_for_wid (WIN_INVEN)) {
    gtk_window_set_title (GTK_WINDOW (win), _("Inventory"));
  }
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (win->view));
  gtk_widget_show_all (GTK_WIDGET (win));
  if (how == PICK_NONE) {
    if (win == (G2Menu *) g2_object_for_wid (WIN_INVEN))
      gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
    else
      gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

    if (iflags.perm_invent && win == (G2Menu *) g2_object_for_wid (WIN_INVEN)) {
      gtk_dialog_set_response_sensitive (GTK_DIALOG (win), GTK_RESPONSE_OK,
          FALSE);
      gtk_dialog_set_response_sensitive (GTK_DIALOG (win), GTK_RESPONSE_CANCEL,
          FALSE);
    } else {
      gtk_dialog_run (GTK_DIALOG (win));
      gtk_widget_hide (GTK_WIDGET (win));
    }
    return 0;

  } else {
    gtk_dialog_set_response_sensitive (GTK_DIALOG (win), GTK_RESPONSE_OK, TRUE);
    gtk_dialog_set_response_sensitive (GTK_DIALOG (win),
        GTK_RESPONSE_CANCEL, TRUE);
    if (how == PICK_ONE) {
      gtk_window_set_title (GTK_WINDOW (win), _("Pick one"));
      gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
    } else {
      gtk_window_set_title (GTK_WINDOW (win), _("Pick any"));
      gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
    }
    response = gtk_dialog_run (GTK_DIALOG (win));
    /* XXX perhaps refactor into handleDialogResponse function */
    if (iflags.perm_invent && win == (G2Menu *) g2_object_for_wid (WIN_INVEN)) {
      gtk_dialog_set_response_sensitive (GTK_DIALOG (win), GTK_RESPONSE_OK,
          FALSE);
      gtk_dialog_set_response_sensitive (GTK_DIALOG (win), GTK_RESPONSE_CANCEL,
          FALSE);
    } else {
      gtk_widget_hide (GTK_WIDGET (win));
    }
    if (response == GTK_RESPONSE_CANCEL
        || response == GTK_RESPONSE_DELETE_EVENT) {
      return -1;
    } else if (response == GTK_RESPONSE_OK) {
      selectedRows =
          gtk_tree_selection_get_selected_rows (GTK_TREE_SELECTION
          (selection), NULL);
      selectedCount = gtk_tree_selection_count_selected_rows (selection);
      if (selectedCount == 0) {
        return 0;
      }
      items = (MENU_ITEM_P *) malloc (selectedCount * sizeof (MENU_ITEM_P));
      while (selectedRows) {
        gtk_tree_model_get_iter (GTK_TREE_MODEL (win->menuTree), &iter,
            (GtkTreePath *) selectedRows->data);
        gtk_tree_model_get (GTK_TREE_MODEL (win->menuTree), &iter,
            G2_MENU_IDENTIFIER, &identifier, -1);
        items[currentItem].item.a_obj = identifier;
        items[currentItem].count = -1;
        currentItem++;
        selectedRows = selectedRows->next;
      }
      *(MENU_ITEM_P **) selected = items;

      return currentItem;
    }
  }

  /* should not be reached */
  return 0;
}


static void
g2_menu_putstr (G2Menu * win, int attr, const char *text, gpointer gp)
{
  if (!win->isSetUp) {
    g2_menu_setup_text_message (win);
    win->isSetUp = TRUE;
  }
  gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER (win->messageBuffer),
      text, strlen (text));
  gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER (win->messageBuffer),
      "\n", 1);
}

static void
g2_menu_display (GtkWidget * win, gboolean block, gpointer gp)
{
  gint result;

  /* XXX perhaps we need to be always modal because else the window will
   * get destroyed to soon by nh
   */
  gtk_window_set_modal (GTK_WINDOW (G2_MENU (win)), TRUE);
  result = gtk_dialog_run (GTK_DIALOG (G2_MENU (win)));
}

GtkWidget *
g2_menu_new ()
{
  G2Menu *g2Menu;

  g2Menu = G2_MENU (g_object_new (TYPE_G2_MENU, NULL));
  g_signal_connect (G_OBJECT (g2Menu), "start_menu",
      G_CALLBACK (g2_menu_start_menu), NULL);
  g_signal_connect (G_OBJECT (g2Menu), "add_menu",
      G_CALLBACK (g2_menu_add_menu), NULL);
  g_signal_connect (G_OBJECT (g2Menu), "end_menu",
      G_CALLBACK (g2_menu_end_menu), NULL);
  g_signal_connect (G_OBJECT (g2Menu), "select_menu",
      G_CALLBACK (g2_menu_select_menu), NULL);
  g_signal_connect (G_OBJECT (g2Menu), "putstr",
      G_CALLBACK (g2_menu_putstr), NULL);
  g_signal_connect (G_OBJECT (g2Menu), "display",
      G_CALLBACK (g2_menu_display), NULL);
  return GTK_WIDGET (g2Menu);
}
