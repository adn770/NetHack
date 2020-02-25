/* the main window
 *
 * $Id: g2main.c,v 1.12 2005/04/26 23:26:41 miq Exp $
 *
 */

#ifdef HILDON
#if defined(MAEMO1)
#include <hildon-widgets/hildon-app.h>
#include <hildon-widgets/hildon-appview.h>
#elif defined(MAEMO2)
#include <hildon-widgets/hildon-program.h>
#elif defined(MAEMO4)
#include <hildon/hildon-program.h>
#endif

#include <libosso.h>
#endif

#define PACKAGE "nethack"
#define VERSION "3.6.5"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "g2main.h"
#include "g2mesg.h"
#include "g2status.h"
#include "g2map.h"
#include "g2menu.h"
#include "g2text.h"
#include "g2equip.h"
#include "g2i18n.h"
#include "g2minipad.h"
#include "hack.h"

#include "config.h"

#define RC_FILE "gtk2hackrc"

#define G2_MAXWINDOWS   20

typedef struct
{
  gpointer ptr;
  gint type;
} G2Window;

static G2Window g2_windows[G2_MAXWINDOWS] = { 0, };

static void g2_zoom_in (GtkWidget * app, gpointer data);
static void g2_zoom_out (GtkWidget * app, gpointer data);
static void g2_exit (GtkWidget * app, gpointer data);
static void g2_show_about (GtkWidget * app, gpointer data);

static GtkWidget *mainWin;
static GtkWidget *mapWin;
static GtkWidget *messageWin;
static GtkWidget *equipWin;
static GtkWidget *statusWin;

#ifdef MINIPAD
static GtkWidget *padWin;
#endif

static char *translators[] = {
  "Mihael Vrbanec <miq@users.sourceforge.net>",
  "Paolo M. <qualsiasi@qualsiasi.net>",
  "Paulo Henrique Cabral <paulo@mudvayne.com.br>",
};

static char *linguas[] = {
  N_("german"),
  N_("italian"),
  N_("brazilian portuguese"),
};

#if GTK_CHECK_VERSION(2,4,0)

/** Need to create some icons on my own */
/** The following is copied from gimp gimpstock.c */

#include <gtk/gtkiconfactory.h>
#include "icons/hack-stock-pixbufs.h"

static GtkIconFactory *hack_stock_factory = NULL;

static void
icon_set_from_inline (GtkIconSet * set,
    const guchar * inline_data, GtkIconSize size, gboolean fallback)
{
  GtkIconSource *source;
  GdkPixbuf *pixbuf;

  source = gtk_icon_source_new ();
  gtk_icon_source_set_size (source, size);
  gtk_icon_source_set_size_wildcarded (source, FALSE);

  pixbuf = gdk_pixbuf_new_from_inline (-1, inline_data, FALSE, NULL);

  g_assert (pixbuf);

  gtk_icon_source_set_pixbuf (source, pixbuf);

  g_object_unref (pixbuf);

  gtk_icon_set_add_source (set, source);

  if (fallback) {
    gtk_icon_source_set_size_wildcarded (source, TRUE);
    gtk_icon_set_add_source (set, source);
  }

  gtk_icon_source_free (source);
}

/** Adds an icon to the icon factory.
 *  Adds an icon to the icon factory and setting an already existing
 *  icon as fallback.
 */
static void
add_sized_with_same_fallback (GtkIconFactory * factory,
    const guchar * inline_data, GtkIconSize size, const gchar * stock_id)
{
  GtkIconSet *set;
  gboolean fallback = FALSE;

  set = gtk_icon_factory_lookup (factory, stock_id);

  if (!set) {
    set = gtk_icon_set_new ();
    gtk_icon_factory_add (factory, stock_id, set);
    gtk_icon_set_unref (set);

    fallback = TRUE;
  }

  icon_set_from_inline (set, inline_data, size, fallback);
}

static GtkStockItem hack_stock_items[] = {
  {HACK_STOCK_DIR_NW, N_(""), 0, '7', GTK_HACK_DOMAIN},
  {HACK_STOCK_DIR_N, N_(""), 0, '8', GTK_HACK_DOMAIN},
  {HACK_STOCK_DIR_NE, N_(""), 0, '6', GTK_HACK_DOMAIN},
  {HACK_STOCK_DIR_W, N_(""), 0, '4', GTK_HACK_DOMAIN},
  {HACK_STOCK_DIR_E, N_(""), 0, '6', GTK_HACK_DOMAIN},
  {HACK_STOCK_DIR_SW, N_(""), 0, '1', GTK_HACK_DOMAIN},
  {HACK_STOCK_DIR_S, N_(""), 0, '2', GTK_HACK_DOMAIN},
  {HACK_STOCK_DIR_SE, N_(""), 0, '3', GTK_HACK_DOMAIN},
  {HACK_STOCK_DIR_SELF, N_(""), 0, '.', GTK_HACK_DOMAIN},
};

static struct
{
  const gchar *stock_id;
  gconstpointer inline_data;
} hack_stock_button_pixbufs[] = {
  {HACK_STOCK_EAT, stock_eat_24},
  {HACK_STOCK_INV, stock_inv_24},
  {HACK_STOCK_KICK, stock_kick_24},
  {HACK_STOCK_OPEN, stock_open_24},
  {HACK_STOCK_THROW, stock_throw_24},
  {HACK_STOCK_ZAP, stock_zap_24},
  {HACK_STOCK_SPELLBOOK, stock_spellbook_24},
  {HACK_STOCK_DIR_NW, stock_up_left_arrow_24},
  {HACK_STOCK_DIR_N, stock_up_arrow_24},
  {HACK_STOCK_DIR_NE, stock_up_right_arrow_24},
  {HACK_STOCK_DIR_W, stock_left_arrow_24},
  {HACK_STOCK_DIR_E, stock_right_arrow_24},
  {HACK_STOCK_DIR_SW, stock_down_left_arrow_24},
  {HACK_STOCK_DIR_S, stock_down_arrow_24},
  {HACK_STOCK_DIR_SE, stock_down_right_arrow_24},
  {HACK_STOCK_DIR_SELF, stock_self_24},
  {HACK_STOCK_WHATSHERE, stock_whatshere_24},
  {HACK_STOCK_WHATSTHERE, stock_whatsthere_24},
  {HACK_STOCK_SEARCH, stock_search_24},
};

static struct
{
  const gchar *stock_id;
  gconstpointer inline_data;
} hack_stock_menu_pixbufs[] = {
  {HACK_STOCK_EAT, stock_eat_16},
  {HACK_STOCK_INV, stock_inv_16},
  {HACK_STOCK_KICK, stock_kick_16},
  {HACK_STOCK_OPEN, stock_open_16},
  {HACK_STOCK_THROW, stock_throw_16},
  {HACK_STOCK_ZAP, stock_zap_16},
  {HACK_STOCK_SPELLBOOK, stock_spellbook_16},
  {HACK_STOCK_WHATSHERE, stock_whatshere_16},
  {HACK_STOCK_WHATSTHERE, stock_whatsthere_16},
  {HACK_STOCK_SEARCH, stock_search_16},
};

/**
 * hack_stock_init:
 *
 * Initializes the HACK stock icon factory.
 *
 * You don't need to call this function as hack_ui_init() already does
 * this for you.
 */
void
hack_stock_init (void)
{
  static gboolean initialized = FALSE;

  gint i;

  if (initialized)
    return;

  hack_stock_factory = gtk_icon_factory_new ();

  for (i = 0; i < G_N_ELEMENTS (hack_stock_button_pixbufs); i++) {
    add_sized_with_same_fallback (hack_stock_factory,
        hack_stock_button_pixbufs[i].inline_data,
        GTK_ICON_SIZE_BUTTON, hack_stock_button_pixbufs[i].stock_id);
  }

  for (i = 0; i < G_N_ELEMENTS (hack_stock_menu_pixbufs); i++) {
    add_sized_with_same_fallback (hack_stock_factory,
        hack_stock_menu_pixbufs[i].inline_data,
        GTK_ICON_SIZE_MENU, hack_stock_menu_pixbufs[i].stock_id);
  }

  gtk_icon_factory_add_default (hack_stock_factory);

  gtk_stock_add_static (hack_stock_items, G_N_ELEMENTS (hack_stock_items));

  initialized = TRUE;
}

typedef struct
{
  gchar *name;
  gchar *label;
  gchar *tooltip;
  gchar *accel;
  gchar command;
  gchar *stock_id;
} G2ActionEntry;

/* Normal items */
static GtkActionEntry entries[] = {
  {"FileMenu", NULL, N_("_File")},
  {"EditMenu", NULL, N_("_Edit")},
  {"ApparelMenu", NULL, N_("Appa_rel")},
  {"ActionMenu", NULL, N_("_Action")},
  {"SpecialMenu", NULL, N_("_Special")},
  {"MoveMenu", NULL, N_("Move")},
  {"HelpMenu", NULL, N_("_Help")},
  {"TilesetMenu", NULL, N_("_TileSet")},
  {"Quit", GTK_STOCK_QUIT, N_("_Quit"), NULL, N_("Quit Gtk2Hack"),
      G_CALLBACK (g2_exit)},
  {"Ride", NULL, N_("Ride"), NULL, NULL, G_CALLBACK (doride)},
  {"About", NULL, N_("_About"), NULL, N_("About Gtk2Hack"),
      G_CALLBACK (g2_show_about)},
  {"ZoomIn", GTK_STOCK_ZOOM_IN, N_("_Zoom In"), "F7", N_("Zoom in"),
      G_CALLBACK (g2_zoom_in)},
  {"ZoomOut", GTK_STOCK_ZOOM_OUT, N_("_Zoom Out"), "F8", N_("Zoom out"),
      G_CALLBACK (g2_zoom_out)},
};

/* XXX perhaps add more commands, tooltips and memnonics; check for sanity with other options */
static G2ActionEntry commands[] = {
  {"Version", N_("_Version"), N_("Show Nethack version"), "v", 'v', NULL},
  {"Slash", N_("Identify a map symbol"), NULL, NULL, '/', NULL},
  {"WhatsHere", N_("What's here?"), NULL, ":", ':', HACK_STOCK_WHATSHERE},
  {"WhatsThere", N_("What's there?"), NULL, ";", ';', HACK_STOCK_WHATSTHERE},
  {"Backslash", N_("Discoveries"), NULL, NULL, GDK_backslash, NULL},
  {"History", N_("_History"), NULL, "<shift>v", 'V', NULL},
  {"Ampersand", N_("Explain command"), NULL, NULL, '&', NULL},
  {"Smaller", N_("Go up"), NULL, NULL, '<', GTK_STOCK_GO_UP},
  {"Greater", N_("Go down"), NULL, NULL, '>', GTK_STOCK_GO_DOWN},
  {"Fight", N_("Attack <direction>"), NULL, "F", 'F', NULL},
  {"Underscore", N_("Travel"), NULL, NULL, '_', NULL},
  {"Rest", N_("Rest"), NULL, NULL, '.', NULL},
  {"Apply", N_("Apply"), NULL, "a", 'a', GTK_STOCK_EXECUTE},
  {"Close", N_("Close"), NULL, "c", 'c', GTK_STOCK_CLOSE},
  {"Name", N_("Name monster"), NULL, "<shift>c", 'C', NULL},
  {"Drop", N_("Drop"), NULL, "d", 'd', GTK_STOCK_GOTO_BOTTOM},
  {"DropMany", N_("Drop many"), NULL, "<shift>d", 'D', NULL},
  {"Kick", N_("Kick"), NULL, "<control>d", CTRL_BITS & GDK_d,
      HACK_STOCK_KICK},
  {"Eat", N_("Eat"), NULL, "e", 'e', HACK_STOCK_EAT},
  {"Engrave", N_("Engrave"), NULL, "<shift>e", 'E', NULL},
  {"Fire", N_("Fire"), NULL, "f", 'f', NULL},
  {"Inventory", N_("Inventory"), NULL, "i", 'i', HACK_STOCK_INV},
  {"Move", N_("Move"), NULL, "m", 'm', NULL},
  {"Open", N_("Open"), NULL, "o", 'o', HACK_STOCK_OPEN},
  {"Pay", N_("Pay"), NULL, "p", 'p', NULL},
  {"PutOn", N_("Put on accessory"), NULL, "<shift>p", 'P', NULL},
  {"Quaff", N_("Quaff"), NULL, "q", 'q', NULL},
  {"Quiver", N_("Put in quiver"), NULL, "<shift>q", 'Q', NULL},
  {"Read", N_("Read"), NULL, "r", 'r', GTK_STOCK_JUSTIFY_LEFT},
  {"Remove", N_("Remove accessory"), NULL, "<shift>r", 'R', NULL},
  {"Search", N_("Search"), NULL, "s", 's', HACK_STOCK_SEARCH},
  {"Throw", N_("Throw/Shoot"), NULL, "t", 't', HACK_STOCK_THROW},
  {"TakeOff", N_("Take off armor"), NULL, "<shift>t", 'T', NULL},
  {"Teleport", N_("Teleport"), NULL, "<control>t", CTRL_BITS & GDK_t,
      GTK_STOCK_JUMP_TO},
  {"Wield", N_("Wield weapon"), NULL, "w", 'w', GTK_STOCK_CUT},
  {"Wear", N_("Wear armor"), NULL, "<shift>w", 'W', NULL},
  {"Exchange", N_("Exchange weapons"), NULL, "x", 'x', NULL},
  {"Explore", N_("Explore mode"), NULL, "<shift>x", 'X', NULL},
  {"Zap", N_("Zap a wand"), NULL, "z", 'z', HACK_STOCK_ZAP},
  {"Cast", N_("Cast a spell"), NULL, "<shift>z", 'Z', HACK_STOCK_SPELLBOOK},
  {"PickUp", N_("Pick up"), NULL, NULL, ',', GTK_STOCK_GOTO_TOP},
  {"Twoweapons", N_("Twoweapons"), NULL, "<alt>2", META_BIT | GDK_2, NULL},
  {"Adjust", N_("Adjust letters"), NULL, "<alt>a", META_BIT | GDK_a, NULL},
  {"Chat", N_("Chat"), NULL, "<alt>c", META_BIT | GDK_c, NULL},
  {"Dip", N_("Dip"), NULL, "<alt>d", META_BIT | GDK_d, NULL},
  {"Enhance", N_("Enhance skills"), NULL, "<alt>e", META_BIT | GDK_e, NULL},
  {"Force", N_("Force"), NULL, "<alt>f", META_BIT | GDK_f, GTK_STOCK_PREFERENCES},
  {"Invoke", N_("Invoke"), NULL, "<alt>i", META_BIT | GDK_i, NULL},
  {"Jump", N_("Jump"), NULL, "<alt>j", META_BIT | GDK_j, NULL},
  {"Loot", N_("Loot"), NULL, "<alt>l", META_BIT | GDK_l, NULL},
  {"Monster", N_("Monster action"), NULL, "<alt>m", META_BIT | GDK_m, NULL},
  {"NameIndividual", N_("Name object"), NULL, "<alt>n", META_BIT | GDK_n,
      NULL},
  {"Offer", N_("Offer"), NULL, "<alt>o", META_BIT | GDK_o, NULL},
  {"Pray", N_("Pray"), NULL, "<alt>p", META_BIT | GDK_p, NULL},
  {"Rub", N_("Rub"), NULL, "<alt>r", META_BIT | GDK_r, NULL},
  {"Sit", N_("Sit"), NULL, "<alt>s", META_BIT | GDK_s, NULL},
  {"Turn", N_("Turn undead"), NULL, "<alt>t", META_BIT | GDK_t, NULL},
  {"Untrap", N_("Untrap"), NULL, "<alt>u", META_BIT | GDK_u, GTK_STOCK_HELP},
  {"Wipe", N_("Wipe"), NULL, "<alt>w", META_BIT | GDK_w, NULL},
  {"Edition", N_("Show build options"), NULL, "<alt>v", META_BIT | GDK_v,
      NULL},
  {"CharacterInfo", N_("Show character info"), NULL, "<control>x",
      CTRL_BITS & GDK_x, NULL},

  {"Redo", N_("Redo"), NULL, "<control>A", CTRL_BITS & GDK_A, GTK_STOCK_REDO},
  {"Save", N_("Save game"), NULL, "<shift>s", GDK_S, GTK_STOCK_SAVE},
  {"Options", N_("Options"), NULL, "<shift>o", GDK_O, GTK_STOCK_PREFERENCES},
  {"Help", N_("Show Help Menu"), NULL, NULL, GDK_question, GTK_STOCK_HELP},
};


static GtkRadioActionEntry tileset_entries[] = {
  {"Default", NULL, N_("_Default"), NULL, "Default",
      TILESET_DEFAULT},
  {"Standard", NULL, N_("_Standard"), NULL, "Standard",
      TILESET_STANDARD},
  {"ASCII", NULL, N_("_ASCII"), NULL, "ASCII",
      TILESET_ASCII},
};

static guint n_tileset_entries = G_N_ELEMENTS (tileset_entries);

static const char *uiDescription =
    "<ui>"
    "  <menubar name='MainMenu'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='Save'/>"
    "      <separator/>"
/*    "      <menuitem action='Redo'/>" */
    "      <menuitem action='ZoomIn'/>"
    "      <menuitem action='ZoomOut'/>"
    "      <separator/>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
    "    <menu action='EditMenu'>"
    "      <menuitem action='Adjust'/>"
/*    "      <menuitem action='Inventory'/>" */
    "      <separator/>"
    "      <menuitem action='CharacterInfo'/>"
    "      <menuitem action='Enhance'/>"
    "      <menuitem action='Name'/>"
    "      <menuitem action='NameIndividual'/>"
    "      <menuitem action='Backslash'/>"
    "      <separator/>"
    "      <menuitem action='Options'/>"
    "      <menuitem action='Explore'/>"
    "    </menu>"
    "    <menu action='ApparelMenu'>"
    "      <menuitem action='Wield'/>"
    "      <menuitem action='Exchange'/>"
    "      <menuitem action='Twoweapons'/>"
    "      <menuitem action='Quiver'/>"
    "      <separator/>"
    "      <menuitem action='Wear'/>"
    "      <menuitem action='TakeOff'/>"
    "      <menuitem action='PutOn'/>"
    "      <menuitem action='Remove'/>"
    "    </menu>"
    "    <menu action='ActionMenu'>"
    "      <menuitem action='Fight'/>"
/*    "      <menuitem action='Throw'/>" */
    "      <menuitem action='Fire'/>"
    "      <separator/>"
    "      <menuitem action='Force'/>"
    "      <menuitem action='Untrap'/>"
/*    "      <menuitem action='Kick'/>"
    "      <menuitem action='Apply'/>" */
    "      <menuitem action='Engrave'/>"
/*
    "      <menuitem action='Search'/>"
    "      <menuitem action='Eat'/>"
    "      <menuitem action='Open'/>"
 */
    "      <menuitem action='Close'/>"
    "      <separator/>"
    "      <menuitem action='Loot'/>"
/*
    "      <menuitem action='PickUp'/>"
    "      <menuitem action='Drop'/>"
 */
    "      <menuitem action='DropMany'/>"
    "    </menu>"
    "    <menu action='SpecialMenu'>"
    "      <menuitem action='Quaff'/>"
/*    "      <menuitem action='Zap'/>"
    "      <menuitem action='Read'/>" */
    "      <menuitem action='Rub'/>"
    "      <menuitem action='Dip'/>"
    "      <separator/>"
/*    "      <menuitem action='Cast'/>" */
    "      <menuitem action='Invoke'/>"
    "      <menuitem action='Teleport'/>"
    "      <separator/>"
    "      <menuitem action='Pay'/>"
    "      <menuitem action='Offer'/>"
    "      <menuitem action='Pray'/>"
    "      <menuitem action='Chat'/>"
    "      <separator/>"
    "      <menuitem action='Wipe'/>"
    "      <menuitem action='Turn'/>"
    "      <menuitem action='Monster'/>"
    "    </menu>"
    "    <menu action='MoveMenu'>"
    "      <menuitem action='Move'/>"
    "      <menuitem action='Jump'/>"
    "      <menuitem action='Ride'/>"
    "      <menuitem action='Sit'/>"
    "      <menuitem action='Smaller'/>"
    "      <menuitem action='Greater'/>"
    "      <menuitem action='Underscore'/>"
/*    "      <menuitem action='Rest'/>" */
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='About'/>"
/*
    "      <separator/>"
    "      <menuitem action='WhatsHere'/>"
    "      <menuitem action='WhatsThere'/>"
 */
    "      <separator/>"
    "        <menu action='TilesetMenu'>"
    "          <menuitem action='Default'/>"
    "          <menuitem action='Standard'/>"
    "          <menuitem action='ASCII'/>"
    "        </menu>"
    "      <separator/>"
    "      <menuitem action='Version'/>"
    "      <menuitem action='Edition'/>"
    "      <menuitem action='History'/>"
    "      <menuitem action='Help'/>"
    "      <separator/>"
    "      <menuitem action='Slash'/>"
    "      <menuitem action='Ampersand'/>"
    "    </menu>"
    "  </menubar>"
    "  <toolbar  name='ToolBar'>"
    "    <toolitem action='WhatsHere'/>"
    "    <toolitem action='WhatsThere'/>"
    "    <toolitem action='Inventory'/>"
    "    <toolitem action='Redo'/>"
    "    <separator action='Sep1'/>"
    "    <toolitem action='PickUp'/>"
    "    <toolitem action='Drop'/>"
    "    <toolitem action='Search'/>"
    "    <toolitem action='Open'/>"
    "    <toolitem action='Kick'/>"
    "    <toolitem action='Throw'/>"
    "    <separator action='Sep2'/>"
    "    <toolitem action='Wield'/>"
    "    <toolitem action='Apply'/>"
    "    <toolitem action='Eat'/>"
    "    <toolitem action='Cast'/>"
    "    <toolitem action='Zap'/>"
    "    <toolitem action='Read'/>"
#if 0
    "    <separator action='Sep3'/>"
    "    <toolitem action='Close'/>"
    "    <toolitem action='Force'/>"
    "    <toolitem action='Untrap'/>"
    "    <toolitem action='Loot'/>"
    "    <separator action='Sep4'/>"
    "    <toolitem action='Exchange'/>"
    "    <toolitem action='Twoweapons'/>"
    "    <separator action='Sep5'/>"
    "    <toolitem action='Pay'/>"
    "    <toolitem action='Offer'/>"
    "    <toolitem action='Pray'/>"
    "    <toolitem action='Chat'/>"
    "    <separator action='Sep6'/>"
    "    <toolitem action='Wipe'/>"
    "    <toolitem action='Turn'/>"
    "    <toolitem action='Monster'/>"
    "    <separator action='Sep7'/>"
    "    <toolitem action='ZoomIn'/>"
    "    <toolitem action='ZoomOut'/>"
#endif
    "  </toolbar>" "</ui>";


static void
command_activated (GtkWidget * action, gpointer command)
{
  if (action->name && g_str_has_prefix (action->name, "Twoweapons")) {
    /* Workaround to handle Twoweapons extended command */
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER ('#'));
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER ('t'));
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER ('w'));
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (GDK_Return));
  } else {
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (command));
  }
}

static void
add_new_action_to_group (GtkActionGroup * group, const gchar * name,
    const gchar * label, const gchar * tooltip,
    const gchar * accel, const gchar command, const gchar * stock_id)
{
  GtkAction *action;

  action = gtk_action_new (name, label, tooltip, stock_id);
  g_signal_connect (G_OBJECT (action), "activate",
      G_CALLBACK (command_activated), GINT_TO_POINTER ((gint) command));
  gtk_action_group_add_action_with_accel (group, action, accel);
}
#endif

static void
activate_radio_action (GtkAction * action, GtkRadioAction * current)
{
  gboolean active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (current));
  gint value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (current));

  if (active) {
    g_signal_emit_by_name (G_OBJECT (mapWin), "reload_tiles", value, NULL);
  }
}

static void
g2_zoom_in (GtkWidget * app, gpointer data)
{
  gdouble res;

  g_signal_emit_by_name (G_OBJECT (mapWin), "rescale", +0.25, &res, NULL);
}

static void
g2_zoom_out (GtkWidget * app, gpointer data)
{
  gdouble res;

  g_signal_emit_by_name (G_OBJECT (mapWin), "rescale", -0.25, &res, NULL);
}

void
g2_exit_windows (const gchar * string)
{
  clearlocks ();
  gtk_widget_destroy (g2_get_main_window ());
  exit (0);
}

static void
g2_exit (GtkWidget * app, gpointer data)
{
  gint response;
  GtkWidget *saveDialog;

  saveDialog = gtk_message_dialog_new (GTK_WINDOW (mainWin), GTK_DIALOG_MODAL,
      GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_YES_NO,
      _("Do you want to save the "
          "current game?\nIf not your current game will be lost!"));
  gtk_dialog_set_default_response (GTK_DIALOG (saveDialog), GTK_RESPONSE_YES);
  response = gtk_dialog_run (GTK_DIALOG (saveDialog));
  if (response == GTK_RESPONSE_YES) {
    dosave0 ();
  }
  g2_exit_windows (_("Exiting...\n"));
}


static void
g2_show_about (GtkWidget * app, gpointer data)
{
  gint response;
  gint i;
  gchar *trans;
  GtkWidget *aboutDialog;
  GtkWidget *vbox;
  GtkWidget *tabContent;
  GtkWidget *notebook;
  GtkWidget *label;

  aboutDialog = gtk_dialog_new_with_buttons (_("About Gtk2Hack"),
      GTK_WINDOW (mainWin),
      GTK_DIALOG_MODAL |
      GTK_DIALOG_NO_SEPARATOR, GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
  vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  notebook = gtk_notebook_new ();
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label),
      "<b><big>nethack " VERSION "</big></b>");
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 10);
  label = gtk_label_new (_("Gtk2Hack was developed by Mihael Vrbanec.\n"
          "Window icon and gold icon by David Theis.\n"
          "Other artwork is borrowed from the Qt, X11 and Gnome window ports.\n\n"
          "Thanks for trying Gtk2Hack."));
  tabContent = gtk_vbox_new (FALSE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (tabContent), 5);
  gtk_box_pack_start (GTK_BOX (tabContent), label, FALSE, FALSE, 0);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), tabContent,
      gtk_label_new (_("Credits")));
  tabContent = gtk_vbox_new (FALSE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (tabContent), 5);

  trans = g_strdup ("");
  for (i = 0; linguas[i] != NULL; i++) {
    gchar *temp =
        g_strconcat (trans, translators[i], " (", _(linguas[i]), ")\n", NULL);
    g_free (trans);
    trans = temp;
  }

  label = gtk_label_new (trans);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.0f);
  gtk_box_pack_start (GTK_BOX (tabContent), label, FALSE, FALSE, 0);
  g_free (trans);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), tabContent,
      gtk_label_new (_("Translations")));

  gtk_box_pack_start (GTK_BOX (vbox), notebook, FALSE, FALSE, 10);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (aboutDialog)->vbox), vbox);
  gtk_widget_show_all (aboutDialog);
  response = gtk_dialog_run (GTK_DIALOG (aboutDialog));
  gtk_widget_destroy (aboutDialog);
}

gboolean
g2_move_keys_with_numpad (guint keyval, gint * key)
{
  switch (keyval) {
    case GDK_Right:
    case GDK_rightarrow:
    case GDK_KP_Right:
    case GDK_KP_6:
      if (iflags.num_pad)
        *key = '6';
      else
        *key = 'l';
      break;
    case GDK_Left:
    case GDK_leftarrow:
    case GDK_KP_Left:
    case GDK_KP_4:
      if (iflags.num_pad)
        *key = '4';
      else
        *key = 'h';
      break;
    case GDK_Up:
    case GDK_uparrow:
    case GDK_KP_Up:
    case GDK_KP_8:
      if (iflags.num_pad)
        *key = '8';
      else
        *key = 'k';
      break;
    case GDK_Down:
    case GDK_downarrow:
    case GDK_KP_Down:
    case GDK_KP_2:
      if (iflags.num_pad)
        *key = '2';
      else
        *key = 'j';
      break;
    case GDK_Home:
    case GDK_KP_Home:
    case GDK_KP_7:
      if (iflags.num_pad)
        *key = '7';
      else
        *key = 'y';
      break;
    case GDK_End:
    case GDK_KP_End:
    case GDK_KP_1:
      if (iflags.num_pad)
        *key = '1';
      else
        *key = 'b';
      break;
    case GDK_Page_Down:
    case GDK_KP_Page_Down:
    case GDK_KP_3:
      if (iflags.num_pad)
        *key = '3';
      else
        *key = 'n';
      break;
    case GDK_Page_Up:
    case GDK_KP_Page_Up:
    case GDK_KP_9:
      if (iflags.num_pad)
        *key = '9';
      else
        *key = 'u';
      break;
    default:
      return FALSE;
      break;
  }
  return TRUE;
}


/** Returns true if this key is a valid input character.
 *  Valid are all ascii characters plus some.
 */
gboolean
g2_key_is_valid_input (guint keyval)
{
  if (g_ascii_isprint (keyval))
    return TRUE;

  switch (keyval) {
    case GDK_Return:           /* since when is this valid? (Ralf) */
    case GDK_KP_Enter:
    case GDK_Escape:
    case GDK_BackSpace:
      return TRUE;
      break;
    default:
      return FALSE;
  }
}

static gboolean
g2_main_key_press_event (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
#if defined(HILDON) && (defined(MAEMO2) || defined(MAEMO4))
  /* Fullscreen mode is on (TRUE) or off (FALSE) */
  static gboolean fullscreen = FALSE;
#endif

  gint key;
  gdouble res;

  key = event->keyval;
  /* convert movement keys */
  if (key == GDK_F7) {
    g2_zoom_in (widget, data);
  } else if (key == GDK_F8) {
    g2_zoom_out (widget, data);
  } else if (key == GDK_F6) {
#if defined(HILDON) && (defined(MAEMO2) || defined(MAEMO4))
    /* toggle fullscreen on<->off */
    fullscreen = !fullscreen;
    if (fullscreen) {
      gtk_window_fullscreen (GTK_WINDOW (mainWin));
    } else {
      gtk_window_unfullscreen (GTK_WINDOW (mainWin));
    }
    g_signal_emit_by_name (G_OBJECT (mapWin), "rescale", 0.0, &res, NULL);
#endif
  } else if (key == GDK_Return) {
    /* Return in Maemo is binded to pickup */
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (','));
  } else if (g2_move_keys_with_numpad (event->keyval, &key)) {
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (key));
  } else if (g2_key_is_valid_input (key)) {
    if (event->state & GDK_MOD1_MASK) {
      key = key | META_BIT;
    } else if (event->state & GDK_CONTROL_MASK) {
      /* do not handle some menu accelerators which are not needed by nethack */
      switch (key) {
        case GDK_q:
          return FALSE;
        default:
          break;
      }
      key = CTRL_BITS & key;
    }
    keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (key));
  }

  return TRUE;
}

static void
set_expand (GtkWidget * child, GtkWidget * parent)
{
  gtk_container_child_set (GTK_CONTAINER (parent), child, "expand", TRUE, NULL);
}



#if GTK_CHECK_VERSION(2,4,0)
#if defined(HILDON) && (defined(MAEMO2) || defined(MAEMO4))
static void
add_menu_and_toolbar (HildonWindow * view)
#else
static void
add_menu_and_toolbar (GtkWidget * box)
#endif
{
  GtkWidget *toolbar;
  GtkActionGroup *actionGroup;
  GtkUIManager *uiManager;
#if defined(HILDON) && (defined(MAEMO2) || defined(MAEMO4))
  GtkMenu *menubar;
#else
  GtkAccelGroup *accelGroup;
  GtkWidget *menubar;
#endif
  GError *error;
  gint i;

  hack_stock_init ();

  actionGroup = gtk_action_group_new ("MenuActions");
  gtk_action_group_set_translation_domain (actionGroup, "gtk2hack");
  gtk_action_group_add_actions (actionGroup, entries, G_N_ELEMENTS (entries),
      mainWin);
  for (i = 0; i < G_N_ELEMENTS (commands); i++) {
    add_new_action_to_group (actionGroup,
        commands[i].name, _(commands[i].label), _(commands[i].tooltip),
#ifdef HILDON
        NULL,
#else
        commands[i].accel,
#endif
        commands[i].command, commands[i].stock_id);
  }

  gtk_action_group_add_radio_actions (actionGroup,
      tileset_entries, n_tileset_entries,
      TILESET_DEFAULT, G_CALLBACK (activate_radio_action), NULL);

  uiManager = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (uiManager, actionGroup, 0);

#ifndef HILDON
  accelGroup = gtk_ui_manager_get_accel_group (uiManager);
  gtk_window_add_accel_group (GTK_WINDOW (mainWin), accelGroup);
#endif

  error = NULL;
  if (!gtk_ui_manager_add_ui_from_string (uiManager, uiDescription, -1, &error)) {
    g_message ("building menus failed: %s", error->message);
    g_error_free (error);
    exit (EXIT_FAILURE);
  }
#ifdef HILDON
#if defined(MAEMO1)
  menubar = GTK_WIDGET (hildon_appview_get_menu (HILDON_APPVIEW (box)));
#elif (defined(MAEMO2) || defined(MAEMO4))
  menubar = GTK_MENU (gtk_menu_new ());
#endif

  gtk_widget_reparent (gtk_ui_manager_get_widget
      (uiManager, "/MainMenu/FileMenu"), GTK_WIDGET (menubar));
  gtk_widget_reparent (gtk_ui_manager_get_widget
      (uiManager, "/MainMenu/EditMenu"), GTK_WIDGET (menubar));
  gtk_widget_reparent (gtk_ui_manager_get_widget
      (uiManager, "/MainMenu/ApparelMenu"), GTK_WIDGET (menubar));
  gtk_widget_reparent (gtk_ui_manager_get_widget
      (uiManager, "/MainMenu/ActionMenu"), GTK_WIDGET (menubar));
  gtk_widget_reparent (gtk_ui_manager_get_widget
      (uiManager, "/MainMenu/SpecialMenu"), GTK_WIDGET (menubar));
  gtk_widget_reparent (gtk_ui_manager_get_widget
      (uiManager, "/MainMenu/MoveMenu"), GTK_WIDGET (menubar));
  gtk_widget_reparent (gtk_ui_manager_get_widget
      (uiManager, "/MainMenu/HelpMenu"), GTK_WIDGET (menubar));

  toolbar = gtk_ui_manager_get_widget (uiManager, "/ToolBar");
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
#ifdef HILDON
  gtk_rc_parse_string ("style \"nethack-toolbutton\" {\n"
      "  GtkButton::minimum_width = 40\n"
      "}\n"
      "widget_class \"*.GtkToolButton.GtkButton\" style \"nethack-toolbutton\"\n");
  gtk_container_foreach (GTK_CONTAINER (toolbar), (GtkCallback) set_expand,
      (gpointer) toolbar);
#endif
#if defined(MAEMO1)
  hildon_appview_set_toolbar (HILDON_APPVIEW (box), GTK_TOOLBAR (toolbar));
#elif (defined(MAEMO2) || defined(MAEMO4))
  hildon_window_add_toolbar (HILDON_WINDOW (view), GTK_TOOLBAR (toolbar));
  hildon_window_set_menu (HILDON_WINDOW (view), GTK_MENU (menubar));
#endif
#else
  menubar = gtk_ui_manager_get_widget (uiManager, "/MainMenu");
  gtk_box_pack_start (GTK_BOX (box), menubar, FALSE, FALSE, 0);

  toolbar = gtk_ui_manager_get_widget (uiManager, "/ToolBar");
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_box_pack_end (GTK_BOX (box), toolbar, FALSE, FALSE, 0);
#endif
}
#endif

void
g2_init_main_window (int *argcp, char **argv)
{
  GtkWidget *mainBox = NULL;
  GtkWidget *topBox = NULL;
  GtkWidget *bottomBox = NULL;
  GtkWidget *pane1 = NULL;
  GtkWidget *pane2 = NULL;
#ifdef HILDON
#if defined(MAEMO1)
  HildonApp *app = NULL;
  HildonAppView *view = NULL;
#elif (defined(MAEMO2) || defined(MAEMO4))
  HildonProgram *app = NULL;
  HildonWindow *view = NULL;
#endif
  osso_context_t *osso_context = NULL;
#else
  GdkPixbuf *icon = NULL;
  GError *error = NULL;
#endif

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, HACKDIR "/locale");
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain ("gtk2hack");

  gtk_rc_parse (RC_FILE);

#ifdef HILDON
  /* Initialize maemo application */
  osso_context = osso_initialize (PACKAGE, VERSION, TRUE, NULL);

  /* Check that initialization was ok */
  if (osso_context == NULL) {
    fprintf (stderr, "osso_initialize failed.\n");
    exit (1);
  }
#endif

  /* Init the gtk - must be called before any hildon stuff */
  gtk_init (argcp, &argv);

  mainBox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (mainBox), 0);
  topBox = gtk_vbox_new (FALSE, 1);

#ifdef HILDON
#if defined(MAEMO1)
  /* Create the hildon application and setup the title */
  app = HILDON_APP (hildon_app_new ());
  hildon_app_set_title (app, _("Nethack"));
  hildon_app_set_two_part_title (app, TRUE);
#elif (defined(MAEMO2) || defined(MAEMO4))
  app = HILDON_PROGRAM (hildon_program_get_instance ());
  g_set_application_name (_("Nethack"));
#endif

#if defined(MAEMO1)
  view = HILDON_APPVIEW (hildon_appview_new (_("Nethack")));
  hildon_appview_set_fullscreen_key_allowed (view, TRUE);
  add_menu_and_toolbar (GTK_WIDGET (view));
  mainWin = GTK_WIDGET (app);
#elif (defined(MAEMO2) || defined(MAEMO4))
  view = HILDON_WINDOW (hildon_window_new ());
  add_menu_and_toolbar (view);
  mainWin = GTK_WIDGET (view);
#endif
#else
  mainWin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (mainWin), "Nethack " VERSION);
  icon = gdk_pixbuf_new_from_file ("gtk2hack.png", &error);
  if (icon != NULL) {
    gtk_window_set_icon (GTK_WINDOW (mainWin), icon);
  }
#if GTK_CHECK_VERSION(2,4,0)
  add_menu_and_toolbar (GTK_WIDGET (mainBox));
#endif
#endif
  g_signal_connect (G_OBJECT (mainWin), "delete_event",
      G_CALLBACK (g2_exit), NULL);

  messageWin = g2_message_new ();
  statusWin = g2_status_new ();
  mapWin = g2_map_new ();

  gtk_box_pack_start (GTK_BOX (topBox), statusWin, FALSE, FALSE, 0);
#ifndef HILDON                  /* save space for HILDON by removing the Equipment window */
  {
    GtkWidget *equipBox = NULL;
    equipWin = g2_equipment_new ();
    equipBox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (equipBox), equipWin, FALSE, FALSE, 20);
    gtk_box_pack_start (GTK_BOX (topBox), equipBox, TRUE, FALSE, 10);
  }
#endif

#ifdef MINIPAD
  padWin = gtk_minipad_new ();
#endif

  pane1 = gtk_hpaned_new ();

  gtk_paned_pack1 (GTK_PANED (pane1), topBox, FALSE, FALSE);

  gtk_paned_pack2 (GTK_PANED (pane1), mapWin, TRUE, FALSE);

  pane2 = gtk_vpaned_new ();

  gtk_paned_pack1 (GTK_PANED (pane2), pane1, TRUE, FALSE);

  bottomBox = gtk_hbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (bottomBox), messageWin, TRUE, TRUE, 0);

#ifdef MINIPAD
  gtk_box_pack_end (GTK_BOX (bottomBox), padWin, FALSE, FALSE, 0);
#endif

  gtk_paned_pack2 (GTK_PANED (pane2), bottomBox, FALSE, FALSE);
  gtk_box_pack_start (GTK_BOX (mainBox), pane2, TRUE, TRUE, 0);

//  gtk_paned_set_position( GTK_PANED(pane1), 600 );
//  gtk_paned_set_position( GTK_PANED(pane2), 50 );
#if defined(HILDON) && (defined(MAEMO2) || defined(MAEMO4))
  g_signal_connect (G_OBJECT (mapWin), "key_press_event",
      G_CALLBACK (g2_main_key_press_event), NULL);
#else
  g_signal_connect (G_OBJECT (mainWin), "key_press_event",
      G_CALLBACK (g2_main_key_press_event), NULL);
#endif

#ifdef HILDON
  gtk_container_add (GTK_CONTAINER (view), mainBox);
#if defined(MAEMO1)
  hildon_app_set_appview (app, view);
  gtk_widget_show_all (GTK_WIDGET (app));
#elif (defined(MAEMO2) || defined(MAEMO4))
  hildon_program_add_window (app, view);
  gtk_widget_show_all (GTK_WIDGET (view));
#endif
#else
  gtk_container_add (GTK_CONTAINER (mainWin), mainBox);
  gtk_window_set_default_size (GTK_WINDOW (mainWin), 1280, 700);
  gtk_widget_show_all (GTK_WIDGET (mainWin));
#endif
}

GtkWidget *
g2_get_main_window ()
{
  return mainWin;
}

GtkWidget *
g2_get_equipment_window ()
{
  return equipWin;
}

gint
g2_create_window (gint type)
{
  gpointer win = NULL;
  gint i = 0;

  switch (type) {
    case NHW_MAP:
      win = mapWin;
      break;
    case NHW_MESSAGE:
      win = messageWin;
      break;
    case NHW_STATUS:
      win = statusWin;
      break;
    case NHW_MENU:
      win = g2_menu_new ();
      break;
    case NHW_TEXT:
      win = g2_text_new ();
      break;
    default:
      g_print ("create unknown window=%d\n", type);
  }

  /* Return the next available winid */
  for (i = 0; i < G2_MAXWINDOWS; i++)
    if (g2_windows[i].ptr == NULL)
      break;
  if (i == G2_MAXWINDOWS)
    g_error ("ERROR: Window list full\n");
  g2_windows[i].ptr = win;
  g2_windows[i].type = type;

  return i;
}

GObject *
g2_object_for_wid (gint id)
{
  G2Window *g2win = NULL;

  g_assert (id < G2_MAXWINDOWS);
  g2win = g2_windows + id;
  g_assert (g2win->ptr != NULL);

  return G_OBJECT (g2win->ptr);
}
