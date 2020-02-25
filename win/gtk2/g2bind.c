/* Binding the nethack engine to the gtk2 port
 *
 * $Id: g2bind.c,v 1.2 2004/06/27 00:17:32 miq Exp $
 *
 */


/* define this if you want to see more dialogs to choose from,
 * so more mouse instead of keys */
#define MORE_DIALOGS

#include <gdk/gdkkeysyms.h>

#include "g2map.h"              /* for get_tile */

#include "g2bind.h"
#include "g2main.h"
#include "g2player.h"
#include "g2text.h"
#include "g2i18n.h"

#include "func_tab.h"
#include "decl.h"               /* for inventory and such */
#include "obj.h"                /* for obj_to_glyph */
#include "objclass.h"           /* for gold */

#define G2_OUTPUT_DELAY 40      /* milliseconds to wait between delayed outputs */

static gboolean inventoryReady = FALSE;
static gboolean delayDone = FALSE;

struct window_procs Gtk2_procs = {
  "Gtk2", WC_COLOR | WC_HILITE_PET | WC_INVERSE, 0L,
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},     /* color availability */
  gtk2_init_nhwindows,
  gtk2_player_selection,
  gtk2_askname,
  gtk2_get_nh_event,
  gtk2_exit_nhwindows,
  gtk2_suspend_nhwindows,
  gtk2_resume_nhwindows,
  gtk2_create_nhwindow,
  gtk2_clear_nhwindow,
  gtk2_display_nhwindow,
  gtk2_destroy_nhwindow,
  gtk2_curs,
  gtk2_putstr,
  genl_putmixed,
  gtk2_display_file,
  gtk2_start_menu,
  gtk2_add_menu,
  gtk2_end_menu,
  gtk2_select_menu,
  genl_message_menu,            /* no need for X-specific handling */
  gtk2_update_inventory,
  gtk2_mark_synch,
  gtk2_wait_synch,
#ifdef CLIPPING
  gtk2_cliparound,
#endif
#ifdef POSITIONBAR
  donull,
#endif
  gtk2_print_glyph,
  gtk2_raw_print,
  gtk2_raw_print_bold,
  gtk2_nhgetch,
  gtk2_nh_poskey,
  gtk2_nhbell,
  gtk2_doprev_message,
  gtk2_yn_function,
  gtk2_getlin,
  gtk2_get_ext_cmd,
  gtk2_number_pad,
  gtk2_delay_output,
#ifdef CHANGE_COLOR             /* only a Mac option currently */
  donull,
  donull,
#endif
  /* other defs that really should go away (they're tty specific) */
  gtk2_start_screen,
  gtk2_end_screen,
  gtk2_outrip,
  genl_preference_update,
  genl_getmsghistory,
  genl_putmsghistory,
  genl_status_init,
  genl_status_finish,
  genl_status_enablefield,
  genl_status_update,
  genl_can_suspend_yes,
};

void
gtk2_init_nhwindows (int *argcp, char **argv)
{
  g2_init_main_window (argcp, argv);
  iflags.window_inited = TRUE;
}

void
gtk2_player_selection (void)
{
  g2_do_player_selection (g2_get_main_window ());
}

void
gtk2_askname (void)
{
  GtkWidget *inputDialog;
  GtkWidget *spacerVbox;
  GtkWidget *questionLabel;
  GtkWidget *inputEntry;
  gint response;

  inputDialog = gtk_dialog_new_with_buttons (_("Choose your name?"),
      GTK_WINDOW (g2_get_main_window
          ()),
      GTK_DIALOG_MODAL,
      GTK_STOCK_CANCEL,
      GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
  spacerVbox = gtk_vbox_new (FALSE, 6);

  gtk_container_set_border_width (GTK_CONTAINER (spacerVbox), 5);
  questionLabel = gtk_label_new (_("What's your name?"));
  gtk_misc_set_alignment (GTK_MISC (questionLabel), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (spacerVbox), questionLabel, TRUE, FALSE, 5);

  inputEntry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (inputEntry), PL_NSIZ - 1);
  gtk_entry_set_text (GTK_ENTRY (inputEntry), plname);
  gtk_entry_set_activates_default (GTK_ENTRY (inputEntry), TRUE);
  gtk_box_pack_start (GTK_BOX (spacerVbox), inputEntry, TRUE, FALSE, 5);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (inputDialog)->vbox), spacerVbox,
      TRUE, TRUE, 0);
  gtk_dialog_set_default_response (GTK_DIALOG (inputDialog), GTK_RESPONSE_OK);
  gtk_widget_show_all (inputDialog);

  response = gtk_dialog_run (GTK_DIALOG (inputDialog));
  if (response == GTK_RESPONSE_OK) {
    g_strlcpy (plname, gtk_entry_get_text (GTK_ENTRY (inputEntry)), PL_NSIZ);
  } else {
    // do nothing
  }
  gtk_widget_destroy (inputDialog);

}

void
gtk2_get_nh_event (void)
{
  /* XXX: We do our own eventhandling ?! */
}

void
gtk2_exit_nhwindows (const char *string)
{
  g2_exit_windows (string);
}

void
gtk2_suspend_nhwindows (const char *string)
{
  g_print ("gtk2_suspend_nhwindows: currently unimplemented\n");
}

void
gtk2_resume_nhwindows (void)
{
  g_print ("resume_nhwindows: currently unimplemented\n");
}

winid
gtk2_create_nhwindow (int type)
{
  return g2_create_window (type);
}

void
gtk2_clear_nhwindow (winid wid)
{
  g_signal_emit_by_name (g2_object_for_wid (wid), "clear", NULL);
}

void
gtk2_display_nhwindow (winid wid, BOOLEAN_P block)
{
  g_signal_emit_by_name (g2_object_for_wid (wid), "display", block, NULL);
  /* show the map and wait until user presses a key. This causes the output to
   * block when detecting monsters, gold, food or finding a trap hidden under
   * some item.
   */
  if (block && wid == WIN_MAP) {
    gtk2_nhgetch ();
  }
}

void
gtk2_destroy_nhwindow (winid wid)
{
  /* XXX prevent our main windows from being destroyed! */
  if (wid != WIN_MAP && wid != WIN_STATUS && wid != WIN_MESSAGE) {
    gtk_widget_destroy (GTK_WIDGET (g2_object_for_wid (wid)));
  }
}

void
gtk2_curs (winid wid, int x, int y)
{
  g_signal_emit_by_name (g2_object_for_wid (wid), "curs", x, y, NULL);
}

void
gtk2_putstr (winid wid, int attr, const char *text)
{
  g_signal_emit_by_name (g2_object_for_wid (wid), "putstr", attr, text, NULL);
}

void
gtk2_display_file (const char *filename, BOOLEAN_P must_exist)
{
  dlb *file;

  file = dlb_fopen (filename, "r");
  if (file) {
    GtkWidget *textWindow;
    gchar line[128];

    /* read the contents and display it */
    textWindow = g2_text_new ();
    while (dlb_fgets (line, 128, file)) {
      /* XXX strip newline from buffer, think of changing putstr to not add \n */
      line[strlen (line) - 1] = '\0';
      g_signal_emit_by_name (G_OBJECT (textWindow), "putstr", 0, line, NULL);
    }
    dlb_fclose (file);
    g_signal_emit_by_name (G_OBJECT (textWindow), "display", TRUE, NULL);
    gtk_widget_destroy (textWindow);
  } else if (!file && must_exist) {
    /* show error */
    GtkWidget *errorDialog;
    errorDialog =
        gtk_message_dialog_new (GTK_WINDOW (g2_get_main_window ()),
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
        "Error! Could not find file: %s\n", filename);
    gtk_dialog_run (GTK_DIALOG (errorDialog));
    gtk_widget_destroy (errorDialog);
  }
}

void
gtk2_start_menu (winid wid)
{
  g_signal_emit_by_name (g2_object_for_wid (wid), "start_menu", NULL);
}

void
gtk2_add_menu (winid wid, int glyph, const ANY_P * identifier,
    CHAR_P accelerator, CHAR_P group_accel, int attr,
    const char *str, BOOLEAN_P presel)
{
  g_signal_emit_by_name (g2_object_for_wid (wid),
      "add_menu", glyph, identifier, accelerator,
      group_accel, attr, str, presel, NULL);
}

void
gtk2_end_menu (winid wid, const char *prompt)
{
  g_signal_emit_by_name (g2_object_for_wid (wid), "end_menu", prompt, NULL);
}

int
gtk2_select_menu (winid wid, int how, MENU_ITEM_P ** selected)
{
  gint returnValue;

  g_signal_emit_by_name (g2_object_for_wid (wid),
      "select_menu", how, selected, &returnValue);

  return returnValue;
}

void
gtk2_update_inventory (void)
{
  /* XXX we need to investigate segfault when restoring game
   * and not delaying inventory update */
#ifndef BUILD_MAEMO
  if (iflags.perm_invent && inventoryReady) {
    display_inventory (NULL, 0);
  }
#endif
}

void
gtk2_mark_synch (void)
{
  /* XXX: do we have to do to something? */
  g_print ("mark_synch, unimplemented\n");
}

void
gtk2_wait_synch (void)
{
  /* XXX: do we have to do to something? */
  g_print ("wait_synch, unimplemented\n");
}

void
gtk2_cliparound (int x, int y)
{
  /* XXX: Currently we assume that only the map can be clipped */
  g_signal_emit_by_name (g2_object_for_wid (WIN_MAP), "cliparound", x, y, NULL);
}

void
gtk2_print_glyph (winid wid, XCHAR_P x, XCHAR_P y, int glyph, int bkglyph)
{
  g_signal_emit_by_name (g2_object_for_wid (wid), "print_glyph", x, y, glyph,
      NULL);
}

void
gtk2_raw_print (const char *str)
{
  /* g_print("%s\n", str); */
}

void
gtk2_raw_print_bold (const char *str)
{
  /* g_print("%s\n", str); */
  /* tty_raw_print_bold(str); */
}

/* XXX: This is our event-loop thingy, that checks the keybuffer */
int
gtk2_nhgetch (void)
{
  int key;

  do {
    while (keyBuffer == NULL) {
      gtk_main_iteration ();
    }
    key = GPOINTER_TO_INT (keyBuffer->data);
    keyBuffer = g_slist_delete_link (keyBuffer, keyBuffer);
  }
  while (key == 0);             /* key == 0 would be a pos event */

  return key;
}


/** Nethack callback function.
 *  Nethack is waiting for a key input or a position.
 *  we call gtk_main_iteration until we have something.
 */
int
gtk2_nh_poskey (int *x, int *y, int *mod)
{
  gint key;

  /* XXX hack: we call set a flag for perm_invent and
   * call display_inventory the first time */
#ifndef HILDON
  if (!inventoryReady && iflags.perm_invent) {
    inventoryReady = TRUE;
    display_inventory (NULL, 0);
  }
  g_signal_emit_by_name (G_OBJECT (g2_get_equipment_window ()), "update", NULL);
#endif
  while (keyBuffer == NULL) {
    gtk_main_iteration ();
  }
  g_signal_emit_by_name (g2_object_for_wid (WIN_STATUS), "player_acted", NULL);
  key = GPOINTER_TO_INT (keyBuffer->data);
  keyBuffer = g_slist_delete_link (keyBuffer, keyBuffer);

  /* key == 0 means that we return a position event */
  if (key == 0) {
    *x = clickX;
    *y = clickY;
    *mod = clickMod;
  }
  return key;
}

void
gtk2_nhbell (void)
{
  gdk_beep ();
}

int
gtk2_doprev_message (void)
{
  /* do nothing, we have a nice textarea with a scrollbar */
  return 0;
}


/** An event handler for key events.
 *  Closes the dialog and returns the pressed key as result.
 *  The user doesn't have to press the buttons to make actions.
 */
static gboolean
g2_dialog_key_press_event (GtkWidget * widget, GdkEventKey * event,
    gpointer data)
{
  gint key = event->keyval;

  /* convert movement keys */
  if (iflags.num_pad && g2_move_keys_with_numpad (event->keyval, &key)) {
    gtk_dialog_response (GTK_DIALOG (widget), key);

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

    gtk_dialog_response (GTK_DIALOG (widget), key);
  }


  return TRUE;
}


/** Displays a yes/no/cancel dialog and returns the result */
gchar
gtk2_yn_dialog (const gchar * question, const gchar * choices, CHAR_P def)
{
  gchar res;
  gint dialogResult;
  GtkWidget *dialog =
      gtk_message_dialog_new (GTK_WINDOW (g2_get_main_window ()),
      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_NONE,
      "%s", question);

  /* -- add buttons */
  if (g_strrstr (choices, "q") != NULL || g_strrstr (choices, "n") == NULL) {   /* cancel should be always allowed */
    gtk_dialog_add_button (GTK_DIALOG (dialog),
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  }
  if (g_strrstr (choices, "n") != NULL) {
    gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_NO, GTK_RESPONSE_NO);
  }

  if (g_strrstr (choices, "y") != NULL) {
    gtk_dialog_add_button (GTK_DIALOG (dialog),
        GTK_STOCK_YES, GTK_RESPONSE_YES);
  }

  if (g_strrstr (choices, "a") != NULL) {
    gtk_dialog_add_button (GTK_DIALOG (dialog), _("All"), 'a');
  }

  if (g_strrstr (choices, "l") != NULL) {
    gtk_dialog_add_button (GTK_DIALOG (dialog), _("Left"), 'l');
  }

  if (g_strrstr (choices, "r") != NULL) {
    gtk_dialog_add_button (GTK_DIALOG (dialog), _("Right"), 'r');
  }

  /* -- set default action */
  switch (def) {
    case 'y':
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
      break;
    case 'n':
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_NO);
      break;
    case 'a':
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), 'a');
      break;
    case 'l':
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), 'l');
      break;
    case 'r':
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), 'r');
      break;
    case 'q':
      gtk_dialog_set_default_response (GTK_DIALOG (dialog),
          GTK_RESPONSE_CANCEL);
      break;
  }

  g_signal_connect (G_OBJECT (dialog), "key_press_event",
      G_CALLBACK (g2_dialog_key_press_event), dialog);

  res = 0;
  do {
    dialogResult = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (dialogResult) {
      case GTK_RESPONSE_YES:
        res = 'y';
        break;
      case GTK_RESPONSE_NO:
        res = 'n';
        break;
      default:
        if (dialogResult > ' ')
          res = dialogResult;

        else if (choices != NULL && g_strrstr (choices, "q") != NULL) {
          res = 'q';
        }
        break;
    }

  }
  while (res == 0);

  gtk_widget_destroy (dialog);

  return res;
}


/** a convinience function for adding something to a table */
void
table_attach_simple (GtkTable * table, GtkWidget * widget, gint x, gint y)
{
  gtk_table_attach (table, widget,
      x, x + 1, y, y + 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
}



typedef struct
{
  const gchar *name;
  const gchar key;
  const gchar num_key;
  const gint xpos;
  const gint ypos;
} DirectionButton;

static DirectionButton direction_buttons[] = {
  {HACK_STOCK_DIR_NW, 'y', '7', 0, 0},
  {HACK_STOCK_DIR_N, 'k', '8', 1, 0},
  {HACK_STOCK_DIR_NE, 'u', '9', 2, 0},
  {HACK_STOCK_DIR_W, 'h', '4', 0, 1},
  {HACK_STOCK_DIR_SELF, '.', '.', 1, 1},
  {HACK_STOCK_DIR_E, 'l', '6', 2, 1},
  {HACK_STOCK_DIR_SW, 'b', '1', 0, 2},
  {HACK_STOCK_DIR_S, 'j', '2', 1, 2},
  {HACK_STOCK_DIR_SE, 'n', '3', 2, 2},
  {GTK_STOCK_GO_UP, '<', '<', 3, 1},
  {GTK_STOCK_GO_DOWN, '>', '>', 3, 2},
};

/* Callback activated by direction button press.
 * just closes the dialog with the correct response */
static void
gtk2_direction_button_callback (GtkWidget * widget, gpointer data)
{
  GtkWidget *dialog = gtk_widget_get_ancestor (widget, GTK_TYPE_DIALOG);

  if (dialog) {
    if (iflags.num_pad)
      gtk_dialog_response (GTK_DIALOG (dialog),
          ((DirectionButton *) data)->num_key);
    else
      gtk_dialog_response (GTK_DIALOG (dialog),
          ((DirectionButton *) data)->key);
  }
}


gchar
gtk2_yn_direction_dialog ()
{

  int i;
  GtkWidget *directionDialog;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *button;
  gint response;
  gchar res;

//  gint window_width;
//  gint window_height;

  directionDialog = gtk_dialog_new_with_buttons ("In what direction?",
      GTK_WINDOW
      (g2_get_main_window ()),
      GTK_DIALOG_MODAL |
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
  gtk_dialog_set_has_separator (GTK_DIALOG (directionDialog), FALSE);

  /* -- create the table and the direction buttons */
  table = gtk_table_new (4, 3, FALSE);
  for (i = 0; i < G_N_ELEMENTS (direction_buttons); i++) {
    button = gtk_button_new_from_stock (direction_buttons[i].name);
    table_attach_simple (GTK_TABLE (table), button,
        direction_buttons[i].xpos, direction_buttons[i].ypos);
    g_signal_connect (G_OBJECT (button), "clicked",
        G_CALLBACK (gtk2_direction_button_callback), &(direction_buttons[i]));
  }

  /* -- and now pack everything together */
  vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);


  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (directionDialog)->vbox), vbox,
      FALSE, FALSE, 0);
  gtk_dialog_set_default_response (GTK_DIALOG (directionDialog),
      GTK_RESPONSE_OK);

  g_signal_connect (G_OBJECT (directionDialog), "key_press_event",
      G_CALLBACK (g2_dialog_key_press_event), directionDialog);

  gtk_widget_show_all (directionDialog);
  gtk_window_move (GTK_WINDOW (directionDialog), 10, 300);
  response = gtk_dialog_run (GTK_DIALOG (directionDialog));

  if (response > 0) {
    res = response;
  } else {
    res = 0;
  }
  gtk_widget_destroy (directionDialog);

  return res;
}

/** my own isAlpha function.
 *  @returns TRUE if the character is a alpha character
 */
gchar
isAlpha (gchar c)
{
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ? TRUE : FALSE;
}


/* Callback activated by item button press.
 * just closes the dialog with the correct response */
static void
gtk2_item_button_callback (GtkWidget * widget, gpointer data)
{
  GtkWidget *dialog = gtk_widget_get_ancestor (widget, GTK_TYPE_DIALOG);

  if (dialog) {
    gtk_dialog_response (GTK_DIALOG (dialog), GPOINTER_TO_INT (data));
  }
}



/** Produces a new item button for the gtk2_yn_item_dialog() */
GtkWidget *
newItemButton (gchar id, int glyph)
{
  gchar text[2];
  GtkWidget *button;

  text[0] = id;
  text[1] = '\0';

  button = gtk_button_new_from_stock (text);
  // gtk_container_set_border_width(GTK_CONTAINER(button), 0);

  if (glyph != NO_GLYPH) {
    GdkPixbuf *tile = g2_get_tile_scaled (glyph, 16);
    GtkWidget *image = gtk_image_new_from_pixbuf (tile);
    GValue value = { 0, };
    g_value_init (&value, G_TYPE_OBJECT);
    g_value_set_object (&value, G_OBJECT (image));
    g_object_set_property (G_OBJECT (button), "image", &value);
    g_value_unset (&value);
  }

  g_signal_connect (G_OBJECT (button), "clicked",
      G_CALLBACK (gtk2_item_button_callback), GINT_TO_POINTER ((int) id));

  return button;
}


/** Returns the object for the given inventory character */
struct obj *
getObject (gchar c)
{
  struct obj *otmp;

  for (otmp = invent; otmp; otmp = otmp->nobj)
    if (otmp->invlet == c)
      return otmp;

  return NULL;
}


/** This dialog let's you choose some objects. (maybe to drop, e.g)*/
gchar
gtk2_yn_item_dialog (const gchar * question, const gchar * choice)
{
  gchar lastChar = 0;
  const gchar *pos;
  int itemCount;
  int itemWidth;
  int itemNum;
  GtkWidget *itemDialog;
  GtkWidget *vbox;
  GtkWidget *questionLabel;
  GtkWidget *table;
  gint response;
  gchar res;

  itemDialog = gtk_dialog_new_with_buttons (_("What item?"),
      GTK_WINDOW (g2_get_main_window
          ()),
      GTK_DIALOG_MODAL |
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
  gtk_dialog_set_has_separator (GTK_DIALOG (itemDialog), FALSE);

  gtk_dialog_add_button (GTK_DIALOG (itemDialog), _("Details"), '?');
  gtk_dialog_add_button (GTK_DIALOG (itemDialog), _("All Items"), '*');

  /* -- add a label */
  questionLabel = gtk_label_new (question);
  gtk_label_set_line_wrap (GTK_LABEL (questionLabel), TRUE);
  gtk_label_set_selectable (GTK_LABEL (questionLabel), TRUE);
  gtk_misc_set_alignment (GTK_MISC (questionLabel), 0.0, 0.0);


  /* - start parsing the choice string */

  /* - first count the item */
  itemCount = 0;
  for (pos = choice; *pos; pos++) {
    if (*pos == '-' && itemCount == 0) {        /* hands */
      itemCount++;

    } else if (*pos == '-' && isAlpha (lastChar) && isAlpha (*(pos + 1))) {
      gchar c;

      for (c = lastChar + 1; c < *(pos + 1); c++) {
        itemCount++;
      }
      lastChar = c;

    } else if (isAlpha (*pos)) {
      itemCount++;
      lastChar = *pos;

    } else if (*pos == '$') {
      itemCount++;
      lastChar = *pos;

    } else if (strstr (pos, " or ?*") == pos || *pos == ']') {
      break;

    } else {
      lastChar = *pos;
    }
  }

  if (itemCount <= 0)
    itemCount = 1;              /* prevent division by zero with parsing errors */

  /* - figure out a width for our table */
  if (itemCount < 5) {
    itemWidth = itemCount;
  } else if (itemCount < 20) {
    itemWidth = 5;
  } else {
    itemWidth = 8;
  }
  table = gtk_table_new (itemWidth, itemCount / itemWidth, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);

  /* - now add them to our table */
  itemNum = 0;
  for (pos = choice; *pos; pos++) {
    if (*pos == '-' && itemNum == 0) {  /* hands */
      table_attach_simple (GTK_TABLE (table),
          newItemButton ('-',
              LEATHER_GLOVES + GLYPH_OBJ_OFF),
          itemNum % itemWidth, itemNum / itemWidth);
      itemNum++;

    } else if (*pos == '-' && isAlpha (lastChar) && isAlpha (*(pos + 1))) {
      gchar c;

      for (c = lastChar + 1; c < *(pos + 1); c++) {
        table_attach_simple (GTK_TABLE (table),
            newItemButton (c,
                obj_to_glyph (getObject
                    (c), rn2_on_display_rng)), itemNum % itemWidth,
            itemNum / itemWidth);
        itemNum++;
      }
      lastChar = c;

    } else if (isAlpha (*pos)) {

      table_attach_simple (GTK_TABLE (table),
          newItemButton (*pos,
              obj_to_glyph (getObject
                  (*pos), rn2_on_display_rng)), itemNum % itemWidth,
          itemNum / itemWidth);
      itemNum++;
      lastChar = *pos;

    } else if (*pos == '$') {
      table_attach_simple (GTK_TABLE (table),
          newItemButton ('$',
              GOLD_PIECE + GLYPH_OBJ_OFF),
          itemNum % itemWidth, itemNum / itemWidth);
      itemNum++;
      lastChar = *pos;

    } else if (strstr (pos, " or ?*") == pos || *pos == ']') {
      break;

    } else {
      lastChar = *pos;
    }
  }



  /* -- and now pack everything together */
  vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);

  gtk_box_pack_start (GTK_BOX (vbox), questionLabel, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);


  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (itemDialog)->vbox), vbox, FALSE,
      FALSE, 0);

  g_signal_connect (G_OBJECT (itemDialog), "key_press_event",
      G_CALLBACK (g2_dialog_key_press_event), itemDialog);

  gtk_widget_show_all (itemDialog);
  response = gtk_dialog_run (GTK_DIALOG (itemDialog));

  if (response > 0) {
    res = response;

  } else {
    res = 0;
  }
  gtk_widget_destroy (itemDialog);

  return res;
}


gchar
gtk2_yn_edit_dialog (const gchar * question, const gchar * choices, CHAR_P def)
{
  GtkWidget *inputDialog;
  GtkWidget *spacerVbox;
  GtkWidget *label;
  GtkWidget *inputEntry;
  gint response;
  gchar res;

  inputDialog = gtk_dialog_new_with_buttons ("Gtk2Hack - Question",
      GTK_WINDOW (g2_get_main_window
          ()),
      GTK_DIALOG_MODAL,
      GTK_STOCK_CANCEL,
      GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
  spacerVbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (spacerVbox), 5);

  label = gtk_label_new (question);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (spacerVbox), label, TRUE, FALSE, 5);

  if (choices) {
    label = gtk_label_new (choices);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start (GTK_BOX (spacerVbox), label, TRUE, FALSE, 5);
  }

  inputEntry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (inputEntry), 1);
  if (def >= 'a') {
    char scratch[3];

    scratch[0] = def;
    scratch[1] = '\0';
    gtk_entry_set_text (GTK_ENTRY (inputEntry), scratch);
  }
  gtk_entry_set_activates_default (GTK_ENTRY (inputEntry), TRUE);
  gtk_box_pack_start (GTK_BOX (spacerVbox), inputEntry, TRUE, FALSE, 5);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (inputDialog)->vbox), spacerVbox,
      TRUE, TRUE, 0);
  gtk_dialog_set_default_response (GTK_DIALOG (inputDialog), GTK_RESPONSE_OK);

  /* We have a text entry, so the key press event should be left for the entry
     g_signal_connect(G_OBJECT(inputDialog), "key_press_event",
     G_CALLBACK(g2_dialog_key_press_event), inputDialog);
   */

  gtk_widget_show_all (inputDialog);
  response = gtk_dialog_run (GTK_DIALOG (inputDialog));

  if (response == GTK_RESPONSE_OK) {
    res = gtk_entry_get_text (GTK_ENTRY (inputEntry))[0];

  } else if (response > 0) {
    res = response;

  } else if (choices != NULL && g_strrstr (choices, "q") != NULL) {
    res = 'q';

  } else {
    res = 0;
  }
  gtk_widget_destroy (inputDialog);

  return res;
}


gchar
gtk2_yn_function (const gchar * question, const gchar * choices, CHAR_P def)
{

  if (skip_question) {
    int key;

    skip_question = FALSE;
    key = GPOINTER_TO_INT (keyBuffer->data);
    keyBuffer = g_slist_delete_link (keyBuffer, keyBuffer);
    return key;
  }

  /* a direction question? */
  if (choices == NULL && strstr (question, "direction")) {
#ifdef MINIPAD
    int x, y, mod;
    g_signal_emit_by_name (g2_object_for_wid (WIN_MESSAGE), "putstr", 0,
        question, NULL);
    return gtk2_nh_poskey (&x, &y, &mod);
#else
    return gtk2_yn_direction_dialog ();
#endif
    /* a item question? */
  } else if (choices == NULL && strstr (question, "or ?*]")) {
    return gtk2_yn_item_dialog (question, strstr (question, "["));
    /* Or a simple choice? */
  } else if (choices != NULL &&
      (strcmp (choices, "yn") == 0 ||
          strcmp (choices, "ynq") == 0 ||
          strcmp (choices, "ynaq") == 0 || strcmp (choices, "rl") == 0)) {
    return gtk2_yn_dialog (question, choices, def);

  } else {
    return gtk2_yn_edit_dialog (question, choices, def);
  }
}

void
gtk2_getlin (const char *question, char *input)
{
  GtkWidget *inputDialog;
  GtkWidget *spacerVbox;
  GtkWidget *questionLabel;
  GtkWidget *inputEntry;
  gint response;

  inputDialog = gtk_dialog_new_with_buttons (_("Gtk2Hack - Question"),
      GTK_WINDOW (g2_get_main_window
          ()),
      GTK_DIALOG_MODAL,
      GTK_STOCK_CANCEL,
      GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
  spacerVbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (spacerVbox), 5);
  questionLabel = gtk_label_new (question);
  gtk_misc_set_alignment (GTK_MISC (questionLabel), 0.0, 0.5);
  inputEntry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (inputEntry), BUFSIZ);
  gtk_entry_set_activates_default (GTK_ENTRY (inputEntry), TRUE);
  gtk_box_pack_start (GTK_BOX (spacerVbox), questionLabel, TRUE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (spacerVbox), inputEntry, TRUE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (inputDialog)->vbox), spacerVbox,
      TRUE, TRUE, 0);
  gtk_dialog_set_default_response (GTK_DIALOG (inputDialog), GTK_RESPONSE_OK);
  gtk_widget_show_all (inputDialog);
  response = gtk_dialog_run (GTK_DIALOG (inputDialog));
  if (response == GTK_RESPONSE_OK) {
    g_strlcpy (input, gtk_entry_get_text (GTK_ENTRY (inputEntry)), BUFSIZ);
  } else {
    g_strlcpy (input, "\033\000", 2);
  }
  gtk_widget_destroy (inputDialog);
}

static gint
g2_get_command_index (gchar * cmd)
{
  gint possibleCommands = 0;
  gint commandIndex = -1;
  gint i = 0;

  for (i = 0; extcmdlist[i].ef_txt; i++) {
    if (g_str_has_prefix (extcmdlist[i].ef_txt, cmd)) {
      commandIndex = i;
      possibleCommands++;
    }
  }
  if (possibleCommands == 1) {
    return commandIndex;
  } else {
    return -1;
  }
}

static gint
g2_complete_ext_cmd (gchar * commandBuffer)
{
  gint commandIndex;

  commandIndex = g2_get_command_index (commandBuffer);
  if (commandIndex == -1) {
    g_signal_emit_by_name (g2_object_for_wid (WIN_MESSAGE), "show_ext_command",
        commandBuffer, NULL);
  } else {
    g_signal_emit_by_name (g2_object_for_wid (WIN_MESSAGE), "show_ext_command",
        extcmdlist[commandIndex].ef_txt, NULL);
  }
  return commandIndex;
}

int
gtk2_get_ext_cmd (void)
{
  gint key;
  gchar commandBuffer[10];
  gint bufferIndex = 0;
  gint commandIndex = -1;

  g_signal_emit_by_name (g2_object_for_wid (WIN_MESSAGE), "show_ext_command",
      "", NULL);
  do {
    while (keyBuffer == NULL) {
      gtk_main_iteration ();
    }
    key = GPOINTER_TO_INT (keyBuffer->data);
    keyBuffer = g_slist_delete_link (keyBuffer, keyBuffer);
    if (key == GDK_Escape) {
      commandIndex = -1;
    } else if (bufferIndex > 0 && key == GDK_BackSpace) {
      bufferIndex--;
      commandBuffer[bufferIndex] = '\0';
      commandIndex = g2_complete_ext_cmd (commandBuffer);
    } else if (bufferIndex < 9 && g_ascii_isalnum ((guchar) key)) {
      commandBuffer[bufferIndex] = (gchar) key;
      commandBuffer[bufferIndex + 1] = '\0';
      bufferIndex++;
      commandIndex = g2_complete_ext_cmd (commandBuffer);
    }
  } while (key != GDK_Return && key != GDK_KP_Enter && key != GDK_Escape);
  g_signal_emit_by_name (g2_object_for_wid (WIN_MESSAGE), "putstr", 0, "",
      NULL);

  return commandIndex;
}

void
gtk2_number_pad (int state)
{
  /* XXX: do we have to do to something? */
  g_print ("gtk2_number_pad: currently unimplemented\n");
}

static gint
timeout (gpointer data)
{
  delayDone = TRUE;
  return FALSE;
}

void
gtk2_delay_output (void)
{
  delayDone = FALSE;
  g_timeout_add (G2_OUTPUT_DELAY, timeout, NULL);
  while (delayDone == FALSE) {
    gtk_main_iteration ();
  }
}

void
gtk2_start_screen (void)
{
  /* XXX: do we have to do to something? */
}

void
gtk2_end_screen (void)
{
  /* XXX: do we have to do to something? */
}

void
gtk2_outrip (winid wid, int how, time_t when)
{
  g_signal_emit_by_name (g2_object_for_wid (wid), "outrip", how, NULL);
  g_print ("gtk2_outrip: weakly implemented\n");
}

void
gtk2_delete_nhwindow_by_reference (GtkWidget * menuWin)
{
  g_print ("gtk2_delete_nhwindow_by_reference: currently unimplemented\n");
}
