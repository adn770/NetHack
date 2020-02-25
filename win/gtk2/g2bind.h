/*
 * This header files defines the interface between the window port specific
 * code in the Gnome port and the rest of the nethack game engine.
 *
 * $Id: g2bind.h,v 1.1.1.1 2004/06/23 02:01:43 miq Exp $
 *
 */

#ifndef G2_BIND_H
#define G2_BIND_H

/* this *must* go before hack.h, else there is strange parse error in pango-headers */
#include <gtk/gtk.h>

#include "hack.h"
#include "decl.h"               /* for inventory and such */
#include "dlb.h"
#include "patchlevel.h"


/* Some prototypes */
void gtk2_init_nhwindows (int *argcp, char **argv);
void gtk2_player_selection (void);
void gtk2_askname (void);
void gtk2_get_nh_event (void);
void gtk2_exit_nhwindows (const char *string);
void gtk2_suspend_nhwindows (const char *string);
void gtk2_resume_nhwindows (void);
winid gtk2_create_nhwindow (int type);
void gtk2_create_nhwindow_by_id (int type, winid i);
void gtk2_clear_nhwindow (winid wid);
void gtk2_display_nhwindow (winid wid, BOOLEAN_P block);
void gtk2_destroy_nhwindow (winid wid);
void gtk2_curs (winid wid, int x, int y);
void gtk2_putstr (winid wid, int attr, const char *text);
void gtk2_display_file (const char *filename, BOOLEAN_P must_exist);
void gtk2_start_menu (winid wid);
void gtk2_add_menu (winid wid, int glyph, const ANY_P * identifier,
    CHAR_P accelerator, CHAR_P group_accel, int attr,
    const char *str, BOOLEAN_P presel);
void gtk2_end_menu (winid wid, const char *prompt);
int gtk2_select_menu (winid wid, int how, MENU_ITEM_P ** selected);

/* No need for message_menu -- we'll use genl_message_menu instead */
void gtk2_update_inventory (void);
void gtk2_mark_synch (void);
void gtk2_wait_synch (void);
void gtk2_cliparound (int x, int y);
void gtk2_print_glyph (winid wid, XCHAR_P x, XCHAR_P y, int glyph, int bkglyph);
void gtk2_raw_print (const char *str);
void gtk2_raw_print_bold (const char *str);
int gtk2_nhgetch (void);
int gtk2_nh_poskey (int *x, int *y, int *mod);
void gtk2_nhbell (void);
int gtk2_doprev_message (void);
char gtk2_yn_function (const char *question, const char *choices, CHAR_P def);
void gtk2_getlin (const char *question, char *input);
int gtk2_get_ext_cmd (void);
void gtk2_number_pad (int state);
void gtk2_delay_output (void);
void gtk2_start_screen (void);
void gtk2_end_screen (void);
void gtk2_outrip (winid wid, int how, time_t when);
void gtk2_delete_nhwindow_by_reference (GtkWidget * menuWin);

#endif /* G2_BIND_H */
