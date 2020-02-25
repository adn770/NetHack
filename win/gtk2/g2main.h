/* the main window
 *
 * $Id: g2main.h,v 1.1.1.1 2004/06/23 02:01:44 miq Exp $
 *
 */

#ifndef G2_MAIN_WINDOW_H
#define G2_MAIN_WINDOW_H

#include <gtk/gtk.h>

#define MINIPAD

#define GTK_HACK_DOMAIN    "gtk-hack"
#define HACK_TOILET_PAPER  "hack-toilet-paper"


#define HACK_STOCK_DIR_NW                 "hack-dir-nw"
#define HACK_STOCK_DIR_N                  "hack-dir-n"
#define HACK_STOCK_DIR_NE                 "hack-dir-ne"
#define HACK_STOCK_DIR_W                  "hack-dir-w"
#define HACK_STOCK_DIR_E                  "hack-dir-e"
#define HACK_STOCK_DIR_SW                 "hack-dir-sw"
#define HACK_STOCK_DIR_S                  "hack-dir-s"
#define HACK_STOCK_DIR_SE                 "hack-dir-se"
#define HACK_STOCK_DIR_SELF               "hack-dir-self"

#define HACK_STOCK_EAT                    "hack-eat"
#define HACK_STOCK_INV                    "hack-inv"
#define HACK_STOCK_KICK                   "hack-kick"
#define HACK_STOCK_OPEN                   "hack-open"
#define HACK_STOCK_THROW                  "hack-throw"
#define HACK_STOCK_ZAP                    "hack-zap"
#define HACK_STOCK_SPELLBOOK              "hack-spellbook"
#define HACK_STOCK_WHATSHERE              "hack-whatshere"
#define HACK_STOCK_WHATSTHERE             "hack-whatsthere"
#define HACK_STOCK_SEARCH                 "hack-search"

/* used for the keyboard functions */
enum
{
  META_BIT = 1 << 7,
  CTRL_BITS = 0x1f
};

enum
{
  TILESET_DEFAULT,
  TILESET_STANDARD,
  TILESET_ASCII
};

GSList *keyBuffer;
gint clickX;
gint clickY;
gint clickMod;

gboolean skip_question;

void g2_init_main_window (int *argcp, char **argv);
GtkWidget *g2_get_main_window ();
GtkWidget *g2_get_equipment_window ();
int g2_create_window (gint type);
void g2_exit_windows (const gchar * string);

GObject *g2_object_for_wid (gint id);

/* utility functions */
gboolean g2_move_keys_with_numpad (guint keyval, gint * key);
gboolean g2_key_is_valid_input (guint keyval);

#endif /* G2_MAIN_WINDOW_H */
