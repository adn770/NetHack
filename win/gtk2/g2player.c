/* the player selection dialog
 *
 * $Id: g2player.c,v 1.4 2005/04/26 23:30:07 miq Exp $
 *
 */

#include <gdk/gdkkeysyms.h>

#include "config.h"

#include "g2player.h"
#include "g2main.h"
#include "g2map.h"
#include "g2i18n.h"

enum
{
  G2_ID_COL,
  G2_PIXBUF_COL,
  G2_DESCRIPTION_COL,
  G2_VALID_COL,
  G2_TEXT_FOREGROUND,
  G2_COL_COUNT
};

enum
{
  G2_MALE,
  G2_FEMALE,
  G2_SEX_NONE = -1
};

enum
{
  G2_LAWFUL,
  G2_NEUTRAL,
  G2_CHAOTIC,
  G2_ALIGN_NONE = -1
};

static gint roleTilesMale[] = {
  328,                          /* Archeologist */
  329,                          /* Barbarian */
  330,                          /* Caveman */
  332,                          /* Healer */
  333,                          /* Knight */
  334,                          /* Monk */
  335,                          /* Priest */
  338,                          /* Ranger */
  337,                          /* Rogue */
  339,                          /* Samurai */
  340,                          /* Tourist */
  341,                          /* Valkyrie */
  342,                          /* Wizard */
};

static gint roleTilesFemale[] = {
  328,                          /* Archeologist */
  329,                          /* Barbarian */
  331,                          /* Cavewoman */
  332,                          /* Healer */
  333,                          /* Knight */
  334,                          /* Monk */
  336,                          /* Priestess */
  338,                          /* Ranger */
  337,                          /* Rogue */
  339,                          /* Samurai */
  340,                          /* Tourist */
  341,                          /* Valkyrie */
  342,                          /* Wizard */
};

static gint raceTiles[] = {
  256,                          /* human */
  260,                          /* elf */
  43,                           /* dwarf */
  162,                          /* gnome */
  71,                           /* orc */
};

static GtkWidget *dialog;
static GtkWidget *roleView;
static GtkListStore *roleStore;
static GtkWidget *raceView;
static GtkListStore *raceStore;
static GtkWidget *noSexRadio;
static GtkWidget *maleRadio;
static GtkWidget *femaleRadio;
static GtkWidget *noAlignRadio;
static GtkWidget *lawfulRadio;
static GtkWidget *neutralRadio;
static GtkWidget *chaoticRadio;

static GdkColor *fgInsensitive;
static GdkColor *fgNormal;

static gboolean
can_select_row (GtkTreeSelection * selection, GtkTreeModel * model,
    GtkTreePath * path, gboolean path_currently_selected, gpointer data)
{
  GtkTreeIter iter;
  gboolean valid;

  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
  gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, G2_VALID_COL, &valid, -1);
  return valid;
}

static gboolean
g2_player_key_pess (GtkWidget * widget, GdkEventKey * event, gpointer dialog)
{
  if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter) {
    gtk_window_activate_default (GTK_WINDOW (dialog));
    return TRUE;
  } else {
    return FALSE;
  }
}

static gboolean
is_selection_complete ()
{
  return flags.initrole > -1 && flags.initrace > -1
      && ok_role (flags.initrole, flags.initrace, flags.initgend,
      flags.initalign)
      && ok_race (flags.initrole, flags.initrace, flags.initgend,
      flags.initalign);

//      && flags.initgend > -1
//        && flags.initalign > -1


//       && ok_align(flags.initrole, flags.initrace, flags.initgend, flags.initalign)
//        && ok_gend(flags.initrole, flags.initrace, flags.initgend, flags.initalign);
}

static void
check_selection_complete ()
{
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
      GTK_RESPONSE_ACCEPT, is_selection_complete ());
}

static void
compute_valid_roles ()
{
  GtkTreeIter iter;
  gint i;
  gboolean valid;

  valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (roleStore), &iter);
  while (valid) {
    gtk_tree_model_get (GTK_TREE_MODEL (roleStore), &iter, G2_ID_COL, &i, -1);
    if (ok_role (i, flags.initrace, flags.initgend, flags.initalign)) {
      /* XXX: we *know* that currently flags.initgend == 1 means female */
      if (flags.initgend == 1 && roles[i].name.f != NULL) {
        gtk_list_store_set (roleStore, &iter,
            G2_DESCRIPTION_COL, roles[i].name.f,
            G2_VALID_COL, TRUE, G2_TEXT_FOREGROUND, fgNormal, -1);
      } else {
        gtk_list_store_set (roleStore, &iter,
            G2_DESCRIPTION_COL, roles[i].name.m,
            G2_VALID_COL, TRUE, G2_TEXT_FOREGROUND, fgNormal, -1);
      }
    } else {
      if (flags.initgend == 1 && roles[i].name.f != NULL) {
        gtk_list_store_set (roleStore, &iter,
            G2_DESCRIPTION_COL, roles[i].name.f,
            G2_VALID_COL, FALSE, G2_TEXT_FOREGROUND, fgInsensitive, -1);
      } else {
        gtk_list_store_set (roleStore, &iter,
            G2_DESCRIPTION_COL, roles[i].name.m,
            G2_VALID_COL, FALSE, G2_TEXT_FOREGROUND, fgInsensitive, -1);
      }
    }
    valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (roleStore), &iter);
  }
}


static void
compute_valid_races ()
{
  GtkTreeIter iter;
  gint i;
  gboolean valid;

  valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (raceStore), &iter);
  while (valid) {
    gtk_tree_model_get (GTK_TREE_MODEL (raceStore), &iter, G2_ID_COL, &i, -1);
    if (ok_race (flags.initrole, i, flags.initgend, flags.initalign)) {
      gtk_list_store_set (raceStore, &iter, G2_VALID_COL, TRUE,
          G2_TEXT_FOREGROUND, fgNormal, -1);
    } else {
      gtk_list_store_set (raceStore, &iter, G2_VALID_COL, FALSE,
          G2_TEXT_FOREGROUND, fgInsensitive, -1);
    }
    valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (raceStore), &iter);
  }
}

static void
compute_valid_genders ()
{
  gtk_widget_set_sensitive (maleRadio,
      ok_gend (flags.initrole, flags.initrace, G2_MALE, flags.initalign));
  gtk_widget_set_sensitive (femaleRadio,
      ok_gend (flags.initrole, flags.initrace, G2_FEMALE, flags.initalign));
}

static void
compute_valid_alignments ()
{
  gtk_widget_set_sensitive (lawfulRadio,
      ok_align (flags.initrole, flags.initrace, flags.initgend, G2_LAWFUL));
  gtk_widget_set_sensitive (neutralRadio,
      ok_align (flags.initrole, flags.initrace, flags.initgend, G2_NEUTRAL));
  gtk_widget_set_sensitive (chaoticRadio,
      ok_align (flags.initrole, flags.initrace, flags.initgend, G2_CHAOTIC));
}

static void
role_changed (GtkTreeSelection * selection, gpointer data)
{
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
    gtk_tree_model_get (GTK_TREE_MODEL (roleStore), &iter, G2_ID_COL,
        &flags.initrole, -1);
  } else {
    flags.initrole = -1;
  }
  compute_valid_races ();
  compute_valid_genders ();
  compute_valid_alignments ();
  check_selection_complete ();
}

static void
race_changed (GtkTreeSelection * selection, gpointer data)
{
  GtkTreeIter iter;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
    gtk_tree_model_get (GTK_TREE_MODEL (raceStore), &iter, G2_ID_COL,
        &flags.initrace, -1);
  } else {
    flags.initrace = -1;
  }
  compute_valid_roles ();
  compute_valid_genders ();
  compute_valid_alignments ();
  check_selection_complete ();
}

static void
gender_changed (GtkWidget * dialog, gpointer data)
{
  GtkTreeIter iter;
  gint i = 0;
  gboolean valid;

  flags.initgend = GPOINTER_TO_INT (data);
  valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (roleStore), &iter);
  while (valid) {
    if (GPOINTER_TO_INT (data) == G2_FEMALE) {
      gtk_list_store_set (roleStore, &iter,
          G2_PIXBUF_COL, g2_get_tile (roleTilesFemale[i]), -1);
    } else {
      gtk_list_store_set (roleStore, &iter,
          G2_PIXBUF_COL, g2_get_tile (roleTilesMale[i]), -1);
    }
    valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (roleStore), &iter);
    i++;
  }
  compute_valid_roles ();
  compute_valid_races ();
  check_selection_complete ();
}

static void
alignment_changed (GtkWidget * dialog, gpointer data)
{
  flags.initalign = GPOINTER_TO_INT (data);
  compute_valid_roles ();
  compute_valid_races ();
  check_selection_complete ();
}


static void
init_choices ()
{
  GtkTreeIter iter;
  gint i;
  gint id;
  gboolean valid;

  for (i = 0; roles[i].name.m; i++) {
    gtk_list_store_append (roleStore, &iter);
    gtk_list_store_set (roleStore, &iter, G2_ID_COL, i,
        G2_PIXBUF_COL, g2_get_tile (roleTilesMale[i]),
        G2_DESCRIPTION_COL, roles[i].name.m, G2_VALID_COL, TRUE, -1);
  }

  for (i = 0; races[i].noun; i++) {
    gtk_list_store_append (raceStore, &iter);
    gtk_list_store_set (raceStore, &iter, G2_ID_COL, i,
        G2_PIXBUF_COL, g2_get_tile (raceTiles[i]),
        G2_DESCRIPTION_COL, races[i].noun, G2_VALID_COL, TRUE, -1);
  }

  if (flags.initrole > -1) {
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (roleStore), &iter);
    while (valid) {
      gtk_tree_model_get (GTK_TREE_MODEL (roleStore), &iter, G2_ID_COL,
          &id, -1);
      if (flags.initrole == id) {
        gtk_tree_selection_select_iter (gtk_tree_view_get_selection
            (GTK_TREE_VIEW (roleView)), &iter);
      }
      valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (roleStore), &iter);
    }
  } else {
    flags.initrole = -1;
  }

  if (flags.initrace > -1 &&
      ok_race (flags.initrole, flags.initrace, G2_SEX_NONE, G2_ALIGN_NONE)) {
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (raceStore), &iter);
    while (valid) {
      gtk_tree_model_get (GTK_TREE_MODEL (raceStore), &iter, G2_ID_COL,
          &id, -1);
      if (flags.initrace == id) {
        gtk_tree_selection_select_iter (gtk_tree_view_get_selection
            (GTK_TREE_VIEW (raceView)), &iter);
      }
      valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (raceStore), &iter);
    }
  } else {
    flags.initrace = -1;
  }

  if (flags.initgend > -1 &&
      ok_gend (flags.initrole, flags.initrace, flags.initgend, G2_ALIGN_NONE)) {
    switch (flags.initgend) {
      case G2_MALE:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (maleRadio), TRUE);
        break;
      case G2_FEMALE:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (femaleRadio), TRUE);
        break;
    }
  } else {
    flags.initgend = G2_SEX_NONE;
  }
  if (flags.initalign > -1 &&
      ok_align (flags.initrole, flags.initrace, flags.initgend,
          flags.initalign)) {
    switch (flags.initalign) {
      case G2_LAWFUL:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (lawfulRadio), TRUE);
        break;
      case G2_NEUTRAL:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (neutralRadio), TRUE);
        break;
      case G2_CHAOTIC:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chaoticRadio), TRUE);
        break;
    }
  } else {
    flags.initalign = G2_ALIGN_NONE;
  }
  check_selection_complete ();
}


#ifndef HILDON
static void
name_changed_event (GtkWidget * entry, gpointer datas)
{
  g_strlcpy (plname, gtk_entry_get_text (GTK_ENTRY (entry)), PL_NSIZ);
}

static GtkWidget *
create_name_entry ()
{
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_set_homogeneous (GTK_BOX (hbox), FALSE);

  label = gtk_label_new (_("Name:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (entry), PL_NSIZ - 1);
  gtk_entry_set_text (GTK_ENTRY (entry), plname);
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (entry), "state_changed",
      G_CALLBACK (name_changed_event), entry);


  return hbox;
}
#endif

static GtkWidget *
create_list_view (const gchar * title, GtkWidget ** view,
    GtkListStore ** model, GCallback cb)
{
  GtkWidget *vbox;
  GtkWidget *sw;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;

#ifndef HILDON
  GtkWidget *frame = gtk_frame_new (title);
#endif

  vbox = gtk_vbox_new (FALSE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER,
      GTK_POLICY_AUTOMATIC);

  *model = gtk_list_store_new (G2_COL_COUNT, G_TYPE_INT, GDK_TYPE_PIXBUF,
      G_TYPE_STRING, G_TYPE_BOOLEAN, GDK_TYPE_COLOR);
  *view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (*model));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (*view), TRUE);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (*view));
  gtk_tree_selection_set_select_function (selection, &can_select_row, NULL,
      NULL);
  g_signal_connect (G_OBJECT (selection), "changed", cb, NULL);
  column = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
      "pixbuf", G2_PIXBUF_COL, NULL);
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (*view), FALSE);
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer,
      "text", G2_DESCRIPTION_COL, "foreground-gdk", G2_TEXT_FOREGROUND, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (*view), column);
  g_signal_connect (G_OBJECT (*view), "key-press-event",
      G_CALLBACK (g2_player_key_pess), dialog);

  gtk_container_add (GTK_CONTAINER (sw), *view);

#ifndef HILDON
  gtk_container_add (GTK_CONTAINER (frame), sw);
  return frame;
#else
  return sw;
#endif
}

static GtkWidget *
create_radio_row (gchar * label, GtkWidget ** radio, GSList * group,
    GCallback cb, gint value)
{
  GtkWidget *spacer;
  GtkWidget *row;

  row = gtk_hbox_new (FALSE, 0);
  spacer = gtk_label_new ("    ");
  *radio = gtk_radio_button_new_with_label (group, label);
  g_signal_connect (G_OBJECT (*radio), "toggled", cb, GINT_TO_POINTER (value));
  g_signal_connect (G_OBJECT (*radio), "key-press-event",
      G_CALLBACK (g2_player_key_pess), dialog);
  gtk_box_pack_start (GTK_BOX (row), spacer, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (row), *radio, FALSE, FALSE, 0);

  return row;
}

static GtkWidget *
create_sex_radios ()
{
  GtkWidget *vbox;
  GtkWidget *row;
  GSList *group;

#ifndef HILDON
  GtkWidget *frame = gtk_frame_new (_("Choose sex:"));
#endif
  vbox = gtk_vbox_new (TRUE, 0);

  row = create_radio_row (_("Random"), &noSexRadio, NULL,
      G_CALLBACK (gender_changed), G2_SEX_NONE);
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (noSexRadio));
  gtk_box_pack_start (GTK_BOX (vbox), row, FALSE, FALSE, 0);
  row = create_radio_row (_("Male"), &maleRadio, group,
      G_CALLBACK (gender_changed), G2_MALE);
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (noSexRadio));
  gtk_box_pack_start (GTK_BOX (vbox), row, FALSE, FALSE, 0);
  row = create_radio_row (_("Female"), &femaleRadio, group,
      G_CALLBACK (gender_changed), G2_FEMALE);
  gtk_box_pack_start (GTK_BOX (vbox), row, FALSE, FALSE, 0);

#ifndef HILDON
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  return frame;
#else
  return vbox;
#endif
}

static GtkWidget *
create_align_radios ()
{
  GtkWidget *vbox;
  GtkWidget *row;
  GSList *group;

#ifndef HILDON
  GtkWidget *frame = gtk_frame_new (_("Choose alignment:"));
#endif
  vbox = gtk_vbox_new (TRUE, 0);

  row = create_radio_row (_("Random"), &noAlignRadio, NULL,
      G_CALLBACK (alignment_changed), G2_ALIGN_NONE);
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (noAlignRadio));
  gtk_box_pack_start (GTK_BOX (vbox), row, FALSE, FALSE, 0);
  row = create_radio_row (_("Lawful"), &lawfulRadio, group,
      G_CALLBACK (alignment_changed), G2_LAWFUL);
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (noAlignRadio));
  gtk_box_pack_start (GTK_BOX (vbox), row, FALSE, FALSE, 0);
  row = create_radio_row (_("Neutral"), &neutralRadio, group,
      G_CALLBACK (alignment_changed), G2_NEUTRAL);
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (noAlignRadio));
  gtk_box_pack_start (GTK_BOX (vbox), row, FALSE, FALSE, 0);
  row = create_radio_row (_("Chaotic"), &chaoticRadio, group,
      G_CALLBACK (alignment_changed), G2_CHAOTIC);
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (noAlignRadio));
  gtk_box_pack_start (GTK_BOX (vbox), row, FALSE, FALSE, 0);


#ifndef HILDON
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  return frame;
#else
  return vbox;
#endif

}

void
g2_do_player_selection (GtkWidget * parent)
{
  GtkWidget *hbox;
  GtkWidget *mainVbox;
  GtkWidget *vbox;
  GtkWidget *list;
  GtkWidget *radios;
  gint response;

  rigid_role_checks ();

  if (is_selection_complete () || flags.randomall) {
    return;
  }
  dialog = gtk_dialog_new_with_buttons (_("Character creation"),
      GTK_WINDOW (g2_get_main_window ()),
      GTK_DIALOG_MODAL |
      GTK_DIALOG_NO_SEPARATOR, _("Random"),
      GTK_RESPONSE_CLOSE, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
  gtk_widget_set_name (dialog, "player dialog");
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 7);
  gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT,
      FALSE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
  mainVbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (mainVbox), 5);


#ifndef HILDON
  /* text entry in HILDON uses a insane amount of space */
  hbox = create_name_entry ();
  gtk_box_pack_start (GTK_BOX (mainVbox), hbox, TRUE, TRUE, 0);
#endif

  hbox = gtk_hbox_new (TRUE, 12);
  list =
      create_list_view (_("Choose role:"), &roleView, &roleStore,
      G_CALLBACK (role_changed));
  gtk_box_pack_start (GTK_BOX (hbox), list, TRUE, TRUE, 0);

  list =
      create_list_view (_("Choose race:"), &raceView, &raceStore,
      G_CALLBACK (race_changed));
  gtk_box_pack_start (GTK_BOX (hbox), list, TRUE, TRUE, 0);

  vbox = gtk_vbox_new (FALSE, 12);
  radios = create_sex_radios ();
  gtk_box_pack_start (GTK_BOX (vbox), radios, FALSE, FALSE, 0);
  radios = create_align_radios ();
  gtk_box_pack_start (GTK_BOX (vbox), radios, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

  fgInsensitive = &roleView->style->text[GTK_STATE_INSENSITIVE];
  fgNormal = &roleView->style->text[GTK_STATE_NORMAL];
  init_choices ();

  gtk_box_pack_start (GTK_BOX (mainVbox), hbox, TRUE, TRUE, 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), mainVbox, TRUE,
      TRUE, 0);
  /* XXX: we do this to prevent the roleView getting focus and thus triggering a selection */
  gtk_widget_set_sensitive (roleView, FALSE);
  gtk_widget_show_all (dialog);
  gtk_widget_set_sensitive (roleView, TRUE);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  if (response != GTK_RESPONSE_ACCEPT) {
    flags.initrole = -1;
    flags.initrace = -1;
    flags.initgend = -1;
    flags.initalign = -1;
  }

  gtk_widget_destroy (dialog);
}
