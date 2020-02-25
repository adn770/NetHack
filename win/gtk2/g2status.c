/* the status window
 *
 * $Id: g2status.c,v 1.7 2004/11/07 12:05:27 miq Exp $
 *
 */

#include <stdlib.h>
#include <string.h>


#include "g2status.h"
#include "g2marsh.h"
#include "g2xpms.h"
#include "g2i18n.h"
#include "hack.h"

#define G2_ICON_SIZE 22         /* 22 */
#define G2_COND_ICON_SIZE  26   /* 26 */
#define G2_MIN_HP_LABEL_WIDTH 25        /* we need this to avoid resizing of status-window */
#define G2_HL_TIME 3

#define AL_LAW _("Lawful")
#define AL_NEU _("Neutral")
#define AL_CHA _("Chaotic")

#define G2_HALLU _("Hallucinating")
#define G2_FP _("Food Poisoned")
#define G2_ILL _("Ill")
#define G2_BLINDED _("Blind")
#define G2_STUNNED _("Stunned")
#define G2_CONFUSED _("Confused")

                                                                                                                                                                                                                      /*extern const gchar *hu_stat[]; *//* defined in eat.c */
/* copy of the array in eat.c for i18n! */
static const gchar *hu_stat[] = {
  N_("Satiated"),
  "",
  N_("Hungry"),
  N_("Weak"),
  N_("Fainting"),
  N_("Fainted"),
  N_("Starved")
};

static const gchar *enc_stat[] = {      /* defined in botl.c */
  "",
  N_("Burdened"),
  N_("Stressed"),
  N_("Strained"),
  N_("Overtaxed"),
  N_("Overloaded")
};

enum
{
  G2_NOT_ENC = 0,
  G2_SLT_ENC,
  G2_MOD_ENC,
  G2_HVY_ENC,
  G2_EXT_ENC,
  G2_OVR_ENC,
  G2_ENC_COUNT
};

typedef struct
{
  gchar *name;
  gint alignment;
  gint currHp;
  gint maxHp;
  gint currPower;
  gint maxPower;
  glong hlOld[HL_COUNT];
  guint conds;
} PlayerStatus;

static PlayerStatus player;
static GtkVBoxClass *parent_class;
static gint g2_status_signals[5];
static PangoAttrList *increaseAttrs;
static PangoAttrList *decreaseAttrs;
static PangoAttrList *normalAttrs;
static PangoAttrList *boldAttrList;
static PangoAttrList *smallAttrs;
static GdkColor hpColor = { 0, 40 * 255, 133 * 255, 49 * 255 };
static GdkColor pwColor = { 0, 40 * 255, 51 * 255, 133 * 255 };

static GdkPixbuf *lawfulPic;
static GdkPixbuf *neutralPic;
static GdkPixbuf *chaoticPic;
static GdkPixbuf *satiatedPic;
static GdkPixbuf *halluPic;
static GdkPixbuf *foodPoisonedPic;
static GdkPixbuf *illPic;
static GdkPixbuf *blindedPic;
static GdkPixbuf *stunnedPic;
static GdkPixbuf *confusedPic;
static GdkPixbuf *encPics[G2_ENC_COUNT];
static GdkPixbuf *hungryPic;
static GdkPixbuf *nothingPic;


static void g2_status_class_init (G2StatusClass * class);
static void g2_status_init (G2Status * status);


static void
change_label_highlight (G2Status * status, HighlightLabel label, gint newValue)
{
  if (newValue > player.hlOld[label]) {
    gtk_label_set_attributes (GTK_LABEL (status->hl[label]), increaseAttrs);
    status->hlFadeList[label] = G2_HL_TIME;
  } else if (newValue < player.hlOld[label]) {
    gtk_label_set_attributes (GTK_LABEL (status->hl[label]), decreaseAttrs);
    status->hlFadeList[label] = G2_HL_TIME;
  }
  player.hlOld[label] = newValue;
}

#ifndef HILDON
static GtkWidget *
get_image_from_xpm (const gchar ** xpm)
{
  GdkPixbuf *icon;
  GtkWidget *image;

  icon = gdk_pixbuf_new_from_xpm_data (xpm);
  image =
      gtk_image_new_from_pixbuf (gdk_pixbuf_scale_simple
      (icon, G2_ICON_SIZE, G2_ICON_SIZE, GDK_INTERP_BILINEAR));

  return image;
}
#endif

static void
set_initial_player_values ()
{
  player.currHp = 1;
  player.maxHp = 1;
  player.currPower = 1;
  player.maxPower = 1;
  player.hlOld[HL_DGN] = 1;
  player.hlOld[HL_AC] = 10;
  player.hlOld[HL_STR] = 10;
  player.hlOld[HL_DEX] = 10;
  player.hlOld[HL_CON] = 10;
  player.hlOld[HL_INT] = 10;
  player.hlOld[HL_WIS] = 10;
  player.hlOld[HL_CHA] = 10;
}

static void
create_pixbufs ()
{
  /* XXX refactor */
  lawfulPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (lawful_xpm),
      G2_COND_ICON_SIZE, G2_ICON_SIZE, GDK_INTERP_BILINEAR);
  neutralPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (neutral_xpm),
      G2_COND_ICON_SIZE, G2_ICON_SIZE, GDK_INTERP_BILINEAR);
  chaoticPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (chaotic_xpm),
      G2_COND_ICON_SIZE, G2_ICON_SIZE, GDK_INTERP_BILINEAR);
  nothingPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (nothing_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  satiatedPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (satiated_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  halluPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (hallu_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  foodPoisonedPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (sick_fp_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  illPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (sick_il_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  blindedPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (blind_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  stunnedPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (stunned_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  confusedPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (confused_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  encPics[G2_NOT_ENC] = nothingPic;
  encPics[G2_SLT_ENC] =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (slt_enc_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  encPics[G2_MOD_ENC] =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (mod_enc_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  encPics[G2_HVY_ENC] =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (hvy_enc_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  encPics[G2_EXT_ENC] =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (ext_enc_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  encPics[G2_OVR_ENC] =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (ovr_enc_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
  hungryPic =
      gdk_pixbuf_scale_simple (gdk_pixbuf_new_from_xpm_data (hungry_xpm),
      G2_COND_ICON_SIZE, G2_COND_ICON_SIZE, GDK_INTERP_BILINEAR);
}

static GtkWidget *
create_condition (G2Status * status, Condition cond, GdkPixbuf * pic,
    const gchar * tooltip)
{
  GtkWidget *eventBox;
  GtkWidget *image;

  image = gtk_image_new_from_pixbuf (pic);
  eventBox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (eventBox), image);
  status->condTips[cond] = gtk_tooltips_new ();
  gtk_tooltips_set_tip (status->condTips[cond], eventBox, tooltip, NULL);
  return eventBox;
}

static GtkWidget *
create_image_hl_label (G2Status * status, HighlightLabel label,
    const gchar ** xpm, const gchar * text, const gchar * tip)
{
  GtkWidget *imageLabel;

#ifndef HILDON                  /* for Hildon we don't want the icons (waste space) and no tooltips */
  GtkWidget *image;
  GtkWidget *eventBox;
#endif

  imageLabel = gtk_hbox_new (FALSE, 0);
#ifndef HILDON
  eventBox = gtk_event_box_new ();
  image = get_image_from_xpm (xpm);
#endif
  status->hl[label] = gtk_label_new (text);
  gtk_misc_set_alignment (GTK_MISC (status->hl[label]), 0.0f, 0.5f);
#ifndef HILDON
  gtk_box_pack_start (GTK_BOX (imageLabel), image, FALSE, FALSE, 0);
#endif
  gtk_box_pack_start (GTK_BOX (imageLabel), status->hl[label], TRUE, TRUE, 0);
#ifndef HILDON
  gtk_container_add (GTK_CONTAINER (eventBox), imageLabel);
  gtk_tooltips_set_tip (status->tooltips, eventBox, tip, NULL);

  return eventBox;
#else
  return imageLabel;
#endif
}

gboolean
draw_value_bar (GtkWidget * widget, GdkEventExpose * event, gpointer data)
{
  static GdkGC *gc = NULL;
  G2Status *status = G2_STATUS (data);
  gint left = 0;
  gint top = 4;
  gint totalWidth;
  gint totalHeight;
  gint barWidth;
  gint barHeight;
  gfloat percent = 0.0;

  if (!gc) {
    gc = gdk_gc_new (widget->window);
  }
  gdk_gc_set_rgb_fg_color (gc, &widget->style->bg[0]);
  gdk_draw_rectangle (widget->window, gc, TRUE, 0, 0,
      widget->allocation.width, widget->allocation.height);
  if (widget == status->hpBar) {
    percent = player.currHp / (gfloat) player.maxHp;
    gdk_gc_set_rgb_fg_color (gc, &hpColor);
  } else if (widget == status->pwBar) {
    gint max;

    /* XXX we can have the rare case of maxPower == 0 */
    if (player.maxPower == 0) {
      max = 1;
    } else {
      max = player.maxPower;
    }
    percent = player.currPower / (gfloat) max;
    gdk_gc_set_rgb_fg_color (gc, &pwColor);
  }
  /* when you die */
  if (percent < 0) {
    percent = 0;
  }
  totalWidth = widget->allocation.width - left - 5;
  totalHeight = widget->allocation.height - (2 * top);
  barHeight = totalHeight - 3;
  barWidth = (totalWidth - 3) * percent;
  /* clear the bar */
  gdk_draw_rectangle (widget->window, gc, FALSE, left, top, totalWidth,
      totalHeight);
  gdk_draw_rectangle (widget->window, gc, TRUE, left + 2, top + 2, barWidth,
      barHeight);
  return TRUE;
}

static void
update_player_name (G2Status * status)
{
  gchar *name;

  if (u.mtimedone) {
    name = g_strconcat (plname, " a ", mons[u.umonnum].mname, NULL);
  } else {
    name =
        g_strconcat (plname, " the ",
        rank_of (u.ulevel, pl_character[0], flags.female), NULL);
  }
  name[0] = g_ascii_toupper (name[0]);
  gtk_label_set_text (GTK_LABEL (status->charName), name);
  g_free (name);
}

static void
update_dungeon_name (G2Status * status)
{
  gchar *name;
  gchar level[3];

  g_snprintf (level, 3, "%d", depth (&u.uz));
  /* TODO: reformat and mark for proper i18n */
  name = g_strconcat ("Level ", level, NULL);
  gtk_label_set_text (GTK_LABEL (status->hl[HL_DGNLV]), name);
  g_free (name);

  name = g_strconcat (dungeons[u.uz.dnum].dname, NULL);
  gtk_label_set_text (GTK_LABEL (status->hl[HL_DGN]), name);
  g_free (name);
}

static void
update_alignment (G2Status * status)
{
  if (u.ualign.type == A_LAWFUL) {
    gtk_image_set_from_pixbuf (GTK_IMAGE (status->alignIcon), lawfulPic);
    gtk_label_set_text (GTK_LABEL (status->alignment), AL_LAW);
  } else if (u.ualign.type == A_NEUTRAL) {
    gtk_image_set_from_pixbuf (GTK_IMAGE (status->alignIcon), neutralPic);
    gtk_label_set_text (GTK_LABEL (status->alignment), AL_NEU);
  } else {
    gtk_image_set_from_pixbuf (GTK_IMAGE (status->alignIcon), chaoticPic);
    gtk_label_set_text (GTK_LABEL (status->alignment), AL_CHA);
  }
  player.alignment = u.ualign.type;
}

static void
update_gold (G2Status * status)
{
  glong newGold;
  gchar goldString[20];

  newGold = money_cnt (invent);

  g_snprintf (goldString, 20, _(" %ld"), newGold);
  gtk_label_set_text (GTK_LABEL (status->hl[HL_GOLD]), goldString);
  change_label_highlight (status, HL_GOLD, newGold);
}

static void
update_hp_and_pw (G2Status * status)
{
  GdkRectangle invalidRectangle;
  gchar buf[14];
  gint newHp;
  gint newHpMax;

  invalidRectangle.x = 0;
  invalidRectangle.y = 0;
  invalidRectangle.width = status->hpBar->allocation.width;
  invalidRectangle.height = status->hpBar->allocation.height;
  if (u.mtimedone > 0) {
    newHp = u.mh;
    newHpMax = u.mhmax;
  } else {
    newHp = u.uhp;
    newHpMax = u.uhpmax;
  }
  /* think of refactoring */
  g_snprintf (buf, 14, _("Hp: %d/%d"), newHp, newHpMax);
  gtk_label_set_text (GTK_LABEL (status->hl[HL_HP]), buf);
  if (player.currHp < newHp || player.maxHp < newHpMax) {
    gtk_label_set_attributes (GTK_LABEL (status->hl[HL_HP]), increaseAttrs);
    status->hlFadeList[HL_HP] = G2_HL_TIME;
  } else if (player.currHp > newHp || player.maxHp > newHpMax) {
    gtk_label_set_attributes (GTK_LABEL (status->hl[HL_HP]), decreaseAttrs);
    status->hlFadeList[HL_HP] = G2_HL_TIME;
  }
  player.currHp = newHp;
  player.maxHp = newHpMax;
  gdk_window_invalidate_rect (status->hpBar->window, &invalidRectangle, FALSE);

  g_snprintf (buf, 14, _("Pw: %d/%d"), u.uen, u.uenmax);
  gtk_label_set_text (GTK_LABEL (status->hl[HL_PW]), buf);
  if (player.currPower < u.uen || player.maxPower < u.uenmax) {
    gtk_label_set_attributes (GTK_LABEL (status->hl[HL_PW]), increaseAttrs);
    status->hlFadeList[HL_PW] = G2_HL_TIME;
  } else if (player.currPower > u.uen || player.maxPower > u.uenmax) {
    gtk_label_set_attributes (GTK_LABEL (status->hl[HL_PW]), decreaseAttrs);
    status->hlFadeList[HL_PW] = G2_HL_TIME;
  }
  player.currPower = u.uen;
  player.maxPower = u.uenmax;
  gdk_window_invalidate_rect (status->pwBar->window, &invalidRectangle, FALSE);
}

static void
update_attributes (G2Status * status)
{
  gchar attrString[11];

  if (ACURR (A_STR) > 118) {
    g_snprintf (attrString, 11, _("Str: %d"), ACURR (A_STR) - 100);
  } else if (ACURR (A_STR) == 118) {
    g_snprintf (attrString, 11, _("Str: 18/**"));
  } else if (ACURR (A_STR) > 18) {
    g_snprintf (attrString, 11, _("Str: 18/%02d"), ACURR (A_STR) - 18);
  } else {
    g_snprintf (attrString, 11, _("Str: %d"), ACURR (A_STR));
  }
  gtk_label_set_text (GTK_LABEL (status->hl[HL_STR]), attrString);
  change_label_highlight (status, HL_STR, ACURR (A_STR));

  g_snprintf (attrString, 11, _("Dex: %d"), ACURR (A_DEX));
  gtk_label_set_text (GTK_LABEL (status->hl[HL_DEX]), attrString);
  change_label_highlight (status, HL_DEX, ACURR (A_DEX));
  g_snprintf (attrString, 11, _("Con: %d"), ACURR (A_CON));
  gtk_label_set_text (GTK_LABEL (status->hl[HL_CON]), attrString);
  change_label_highlight (status, HL_CON, ACURR (A_CON));
  g_snprintf (attrString, 11, _("Int: %d"), ACURR (A_INT));
  gtk_label_set_text (GTK_LABEL (status->hl[HL_INT]), attrString);
  change_label_highlight (status, HL_INT, ACURR (A_INT));
  g_snprintf (attrString, 11, _("Wis: %d"), ACURR (A_WIS));
  gtk_label_set_text (GTK_LABEL (status->hl[HL_WIS]), attrString);
  change_label_highlight (status, HL_WIS, ACURR (A_WIS));
  g_snprintf (attrString, 11, _("Cha: %d"), ACURR (A_CHA));
  gtk_label_set_text (GTK_LABEL (status->hl[HL_CHA]), attrString);
  change_label_highlight (status, HL_CHA, ACURR (A_CHA));
}

static void
update_misc_labels (G2Status * status)
{
  gchar newString[20];

  g_snprintf (newString, 10, _("AC: %d"), u.uac);
  gtk_label_set_text (GTK_LABEL (status->hl[HL_AC]), newString);
  /* special case: lower AC is better than higher -> invert highlight */
  if (player.hlOld[HL_AC] > u.uac) {
    gtk_label_set_attributes (GTK_LABEL (status->hl[HL_AC]), increaseAttrs);
    status->hlFadeList[HL_AC] = G2_HL_TIME;
  } else if (player.hlOld[HL_AC] < u.uac) {
    gtk_label_set_attributes (GTK_LABEL (status->hl[HL_AC]), decreaseAttrs);
    status->hlFadeList[HL_AC] = G2_HL_TIME;
  }
  player.hlOld[HL_AC] = u.uac;

  g_snprintf (newString, 10, _("Lv: %d"), u.ulevel);
  gtk_label_set_text (GTK_LABEL (status->hl[HL_LVL]), newString);
  change_label_highlight (status, HL_LVL, u.ulevel);

#ifdef EXP_ON_BOTL
  if (flags.showexp) {
    g_snprintf (newString, 12, _("Exp: %-7ld"), u.uexp);
    gtk_label_set_text (GTK_LABEL (status->hl[HL_EXP]), newString);
    change_label_highlight (status, HL_EXP, u.uexp);
    gtk_widget_show (status->hl[HL_EXP]);
  } else {
    gtk_widget_hide (status->hl[HL_EXP]);
  }
#endif
  if (flags.time) {
    g_snprintf (newString, 20, _("Time: %ld"), moves);
    gtk_label_set_text (GTK_LABEL (status->hl[HL_TIME]), newString);
    gtk_widget_show (status->hl[HL_TIME]);
  } else {
    gtk_widget_hide (status->hl[HL_TIME]);
  }

#ifdef SCORE_ON_BOTL
  if (flags.showscore) {
    g_snprintf (newString, 20, _("Score: %ld"), botl_score ());
    gtk_label_set_text (GTK_LABEL (status->hl[HL_SCORE]), newString);
    change_label_highlight (status, HL_SCORE, botl_score ());
    gtk_widget_show (status->hl[HL_SCORE]);
  } else {
    gtk_widget_hide (status->hl[HL_SCORE]);
  }
#endif
  if (!flags.time && !flags.showscore) {
    gtk_widget_hide (status->optSep);
  } else {
    gtk_widget_show (status->optSep);
  }

  if (!flags.time) {
    gtk_widget_hide (status->optSep);
  } else {
    gtk_widget_show (status->optSep);
  }
}

static void
update_hunger_condition (G2Status * status)
{
  GtkWidget *image;

  image = gtk_bin_get_child (GTK_BIN (status->conds[COND_HUNGER]));
  if (u.uhs == SATIATED) {
    gtk_tooltips_enable (status->condTips[COND_HUNGER]);
    gtk_tooltips_set_tip (status->condTips[COND_HUNGER],
        status->conds[COND_HUNGER], _(hu_stat[u.uhs]), NULL);
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), satiatedPic);
  } else if (u.uhs == NOT_HUNGRY) {
    gtk_tooltips_disable (status->condTips[COND_HUNGER]);
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), nothingPic);
  } else {
    gtk_tooltips_enable (status->condTips[COND_HUNGER]);
    gtk_tooltips_set_tip (status->condTips[COND_HUNGER],
        status->conds[COND_HUNGER], _(hu_stat[u.uhs]), NULL);
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), hungryPic);
  }
}

static void
update_condition (G2Status * status, Condition cond, gint state,
    GdkPixbuf * pic, gchar * tooltip)
{
  GtkWidget *image;

  image = gtk_bin_get_child (GTK_BIN (status->conds[cond]));
  if (state) {
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), pic);
    gtk_tooltips_enable (status->condTips[cond]);
    gtk_tooltips_set_tip (status->condTips[cond], status->conds[cond],
        tooltip, NULL);
  } else {
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), nothingPic);
    gtk_tooltips_disable (status->condTips[cond]);
  }
}

static void
update_boolean_conditions (G2Status * status)
{
  update_condition (status, COND_CONFUSED, Confusion, confusedPic, G2_CONFUSED);
  update_condition (status, COND_STUNNED, Stunned, stunnedPic, G2_STUNNED);
  update_condition (status, COND_HALLU, Hallucination, halluPic, G2_HALLU);
  update_condition (status, COND_BLINDED, Blinded, blindedPic, G2_BLINDED);
  if (Sick) {
    if (u.usick_type & SICK_NONVOMITABLE) {
      update_condition (status, COND_SICK, TRUE, illPic, G2_ILL);
    } else {
      update_condition (status, COND_SICK, TRUE, foodPoisonedPic, G2_FP);
    }
  } else {
    update_condition (status, COND_SICK, FALSE, foodPoisonedPic, G2_FP);
  }
}

static void
update_encumbrance_condition (G2Status * status)
{
  GtkWidget *image;
  gint enc;

  image = gtk_bin_get_child (GTK_BIN (status->conds[COND_ENC]));
  enc = near_capacity ();
  gtk_image_set_from_pixbuf (GTK_IMAGE (image), encPics[enc]);
  if (!enc) {
    gtk_tooltips_disable (status->condTips[COND_ENC]);
  } else {
    gtk_tooltips_set_tip (status->condTips[COND_ENC],
        status->conds[COND_ENC], _(enc_stat[enc]), NULL);
    gtk_tooltips_enable (status->condTips[COND_ENC]);
  }
}

static void
update_status (G2Status * status)
{
  update_player_name (status);
  update_dungeon_name (status);
  if (player.alignment != u.ualign.type) {
    update_alignment (status);
  }
  update_gold (status);
  update_hp_and_pw (status);
  update_attributes (status);
  update_misc_labels (status);
  update_hunger_condition (status);
  update_boolean_conditions (status);
  update_encumbrance_condition (status);
}

static void
add_alignment_and_gold (G2Status * status)
{
  GtkWidget *row;
  GtkWidget *iconLabel;
  GtkWidget *icon;
  GtkWidget *eventBox;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  row = gtk_hbox_new (TRUE, 5);
  iconLabel = gtk_hbox_new (FALSE, 0);
  eventBox = gtk_event_box_new ();
  status->alignIcon = gtk_image_new_from_pixbuf (neutralPic);
  status->alignment = gtk_label_new (AL_NEU);
  gtk_misc_set_alignment (GTK_MISC (status->alignment), 0.5f, 0.5f);
  gtk_box_pack_start (GTK_BOX (iconLabel), status->alignIcon, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (iconLabel), status->alignment, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (eventBox), iconLabel);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Alignment"), NULL);
  gtk_box_pack_start (GTK_BOX (row), eventBox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (&status->vBox), row, TRUE, FALSE, 0);

  row = gtk_hbox_new (TRUE, 5);
  iconLabel = gtk_hbox_new (FALSE, 0);
  pixbuf = gdk_pixbuf_new_from_file ("gold24.png", &error);
  icon = gtk_image_new_from_pixbuf (pixbuf);
  eventBox = gtk_event_box_new ();
  status->hl[HL_GOLD] = gtk_label_new (": 0");
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_GOLD]), 0.5f, 0.5f);
  gtk_box_pack_start (GTK_BOX (iconLabel), icon, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (iconLabel), status->hl[HL_GOLD], TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (eventBox), iconLabel);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Gold pieces"), NULL);
  gtk_box_pack_start (GTK_BOX (row), eventBox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (&status->vBox), row, TRUE, FALSE, 0);
}

static void
add_hp_and_pw (G2Status * status)
{
  GtkWidget *table;
  GtkWidget *eventBox;

  gtk_widget_set_name (GTK_WIDGET (status), "status window");

  table = gtk_table_new (2, 2, FALSE);

  eventBox = gtk_event_box_new ();
  status->hl[HL_HP] = gtk_label_new (_("Hp: "));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_HP]), 0.0f, 0.5f);
  gtk_misc_set_padding (GTK_MISC (status->hl[HL_HP]), 5, 0);
  gtk_widget_set_size_request (status->hl[HL_HP], G2_MIN_HP_LABEL_WIDTH, -1);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_HP]);
  gtk_tooltips_set_tip (status->tooltips, eventBox,
      _("Current and maximum hit points"), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), eventBox, 0, 1, 0, 1);

  eventBox = gtk_event_box_new ();
  status->hpBar = gtk_drawing_area_new ();
  gtk_widget_set_size_request (status->hpBar, -1, 10);
  g_signal_connect (G_OBJECT (status->hpBar), "expose_event",
      G_CALLBACK (draw_value_bar), status);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hpBar);
  gtk_tooltips_set_tip (status->tooltips, eventBox,
      _("Graphical hit point display"), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), eventBox, 1, 2, 0, 1);

  eventBox = gtk_event_box_new ();
  status->hl[HL_PW] = gtk_label_new (_("Pw: "));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_PW]), 0.0f, 0.5f);
  gtk_misc_set_padding (GTK_MISC (status->hl[HL_PW]), 5, 0);
  gtk_widget_set_size_request (status->hl[HL_PW], G2_MIN_HP_LABEL_WIDTH, -1);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_PW]);
  gtk_tooltips_set_tip (status->tooltips, eventBox,
      _("Current and maximum magic energy"), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), eventBox, 0, 1, 1, 2);

  eventBox = gtk_event_box_new ();
  status->pwBar = gtk_drawing_area_new ();
  gtk_widget_set_size_request (status->pwBar, -1, 10);
  g_signal_connect (G_OBJECT (status->pwBar), "expose_event",
      G_CALLBACK (draw_value_bar), status);
  gtk_container_add (GTK_CONTAINER (eventBox), status->pwBar);
  gtk_tooltips_set_tip (status->tooltips, eventBox,
      _("Graphical magic energy display"), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), eventBox, 1, 2, 1, 2);

  gtk_box_pack_start (GTK_BOX (&status->vBox), table, TRUE, TRUE, 5);   //F,T
}

static void
add_ac_level_and_exp (G2Status * status)
{
  GtkWidget *row;
  GtkWidget *eventBox;

  row = gtk_hbox_new (FALSE, 0);
  eventBox = gtk_event_box_new ();
  status->hl[HL_AC] = gtk_label_new (_("AC: "));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_AC]), 0.0f, 0.5f);
  gtk_misc_set_padding (GTK_MISC (status->hl[HL_AC]), 5, 0);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_AC]);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Armor Class"), NULL);
  gtk_box_pack_start (GTK_BOX (row), eventBox, TRUE, TRUE, 0);

  eventBox = gtk_event_box_new ();
  status->hl[HL_LVL] = gtk_label_new (_("Level: "));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_LVL]), 0.0f, 0.5f);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_LVL]);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Experience Level"),
      NULL);
  gtk_box_pack_start (GTK_BOX (row), eventBox, TRUE, TRUE, 0);
#ifdef EXP_ON_BOTL
//        if (flags.showexp) { /* if we don't create the hl EXP label here, we can't hide it afterwards. */
  eventBox = gtk_event_box_new ();
  status->hl[HL_EXP] = gtk_label_new (_("Exp: "));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_EXP]), 0.0f, 0.5f);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_EXP]);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Experience Points"),
      NULL);
  gtk_box_pack_start (GTK_BOX (row), eventBox, TRUE, TRUE, 0);
//        }
#endif
  gtk_box_pack_start (GTK_BOX (&status->vBox), row, TRUE, FALSE, 0);
}

static void
add_attributes (G2Status * status)
{
  GtkWidget *row;
  GtkWidget *imageLabel;

  row = gtk_hbox_new (TRUE, 0);
  imageLabel =
      create_image_hl_label (status, HL_STR, str_xpm, _("Str: 9999"),
      _("Strength"));
  gtk_box_pack_start (GTK_BOX (row), imageLabel, TRUE, TRUE, 5);
  imageLabel =
      create_image_hl_label (status, HL_INT, int_xpm, _("Int: 9999"),
      _("Intelligence"));
  gtk_box_pack_start (GTK_BOX (row), imageLabel, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (&status->vBox), row, TRUE, FALSE, 0);

  row = gtk_hbox_new (TRUE, 0);
  imageLabel =
      create_image_hl_label (status, HL_DEX, dex_xpm, _("Dex: 9999"),
      _("Dexterity"));
  gtk_box_pack_start (GTK_BOX (row), imageLabel, TRUE, TRUE, 5);
  imageLabel =
      create_image_hl_label (status, HL_WIS, wis_xpm, _("Wis: 9999"),
      _("Wisdom"));
  gtk_box_pack_start (GTK_BOX (row), imageLabel, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (&status->vBox), row, TRUE, FALSE, 0);

  row = gtk_hbox_new (TRUE, 0);
  imageLabel =
      create_image_hl_label (status, HL_CON, con_xpm, _("Con: 9999"),
      _("Constitution"));
  gtk_box_pack_start (GTK_BOX (row), imageLabel, TRUE, TRUE, 5);
  imageLabel =
      create_image_hl_label (status, HL_CHA, cha_xpm, _("Cha: 9999"),
      _("Charisma"));
  gtk_box_pack_start (GTK_BOX (row), imageLabel, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (&status->vBox), row, TRUE, FALSE, 0);
}

static void
add_optionals (G2Status * status)
{
  GtkWidget *row;
  GtkWidget *eventBox;

  row = gtk_hbox_new (TRUE, 0);
  eventBox = gtk_event_box_new ();
  status->hl[HL_TIME] = gtk_label_new (_("Time: "));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_TIME]), 0.0f, 0.5f);
  gtk_misc_set_padding (GTK_MISC (status->hl[HL_TIME]), 5, 0);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_TIME]);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Elapsed game moves"),
      NULL);
  gtk_box_pack_start (GTK_BOX (row), eventBox, TRUE, TRUE, 0);
#ifdef SCORE_ON_BOTL
  eventBox = gtk_event_box_new ();
  status->hl[HL_SCORE] = gtk_label_new (_("Score: "));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_SCORE]), 0.0f, 0.5f);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_SCORE]);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Approximate score"),
      NULL);
  gtk_box_pack_start (GTK_BOX (row), eventBox, TRUE, TRUE, 0);
#endif
  gtk_box_pack_start (GTK_BOX (&status->vBox), row, FALSE, FALSE, 0);
  status->optSep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (&status->vBox), status->optSep, FALSE, FALSE, 0);
}

static void
add_condition_row (G2Status * status)
{
  GtkWidget *row;

  row = gtk_hbox_new (FALSE, 1);
  status->conds[COND_HUNGER] =
      create_condition (status, COND_HUNGER, hungryPic, hu_stat[HUNGRY]);
  gtk_box_pack_start (GTK_BOX (row), status->conds[COND_HUNGER], FALSE, FALSE,
      1);
  status->conds[COND_SICK] =
      create_condition (status, COND_SICK, illPic, G2_ILL);
  gtk_box_pack_start (GTK_BOX (row), status->conds[COND_SICK], FALSE, FALSE, 1);
  status->conds[COND_BLINDED] =
      create_condition (status, COND_BLINDED, blindedPic, G2_BLINDED);
  gtk_box_pack_start (GTK_BOX (row), status->conds[COND_BLINDED], FALSE,
      FALSE, 1);
  status->conds[COND_STUNNED] =
      create_condition (status, COND_STUNNED, stunnedPic, G2_STUNNED);
  gtk_box_pack_start (GTK_BOX (row), status->conds[COND_STUNNED], FALSE,
      FALSE, 1);
  gtk_box_pack_start (GTK_BOX (&status->vBox), row, TRUE, FALSE, 0);

  row = gtk_hbox_new (FALSE, 1);
  status->conds[COND_HALLU] =
      create_condition (status, COND_HALLU, halluPic, G2_HALLU);
  gtk_box_pack_start (GTK_BOX (row), status->conds[COND_HALLU], FALSE, FALSE,
      1);
  status->conds[COND_CONFUSED] =
      create_condition (status, COND_CONFUSED, confusedPic, G2_CONFUSED);
  gtk_box_pack_start (GTK_BOX (row), status->conds[COND_CONFUSED], FALSE,
      FALSE, 1);
  status->conds[COND_ENC] =
      create_condition (status, COND_ENC, encPics[G2_OVR_ENC],
      enc_stat[G2_OVR_ENC]);
  gtk_box_pack_start (GTK_BOX (row), status->conds[COND_ENC], FALSE, FALSE, 1);

  gtk_box_pack_start (GTK_BOX (&status->vBox), row, TRUE, FALSE, 0);
}

static const GTypeInfo g2_status_info = {
  sizeof (G2StatusClass),
  NULL,                         /* base_init */
  NULL,                         /* base_finalize */
  (GClassInitFunc) g2_status_class_init,
  NULL,                         /* class_finalize */
  NULL,                         /* class_data */
  sizeof (G2Status),
  0,                            /* n_preallocs */
  (GInstanceInitFunc) g2_status_init
};

GType
g2_status_get_type ()
{
  static GType g2_status_type = 0;

  if (g2_status_type == 0) {
    g2_status_type = g_type_register_static (GTK_TYPE_VBOX,
        "G2Status", &g2_status_info, 0);
  }
  return g2_status_type;
}

static void
g2_status_class_init (G2StatusClass * class)
{
  parent_class = gtk_type_class (gtk_vbox_get_type ());

  g2_status_signals[0] =
      g_signal_new ("curs",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2StatusClass, g2_status_curs),
      NULL, NULL,
      gtk_marshal_VOID__INT_INT, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);
  g2_status_signals[1] =
      g_signal_new ("putstr",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2StatusClass, g2_status_putstr),
      NULL, NULL,
      g2_marshal_VOID__INT_STRING, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_STRING);
  g2_status_signals[2] =
      g_signal_new ("player_acted",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2StatusClass, g2_status_player_acted),
      NULL, NULL, gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);
  g2_status_signals[3] =
      g_signal_new ("display",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2StatusClass, g2_status_display),
      NULL, NULL, gtk_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
g2_status_curs (GtkWidget * win, int x, int y, gpointer gp)
{
  /* XXX we do not need to do anything since we get our data directly out of the player structs */
}

static void
g2_status_putstr (GtkWidget * win, int attr, const char *text, gpointer gp)
{
  update_status (G2_STATUS (win));
}

static void
g2_status_display (GtkWidget * win, gboolean block, gpointer gp)
{
  gtk_widget_show_all (win);
}

static void
g2_status_player_acted (GtkWidget * win, gpointer gp)
{
  gint i;
  G2Status *status = G2_STATUS (win);

  for (i = 0; i < HL_COUNT; i++) {
    if (status->hlFadeList[i] > 1) {
      status->hlFadeList[i]--;
    } else if (status->hlFadeList[i] == 1) {
      gtk_label_set_attributes (GTK_LABEL (status->hl[i]), normalAttrs);
      status->hlFadeList[i]--;
    }
  }
}


static void
g2_status_init (G2Status * status)
{
  GtkWidget *sep;
  GtkWidget *eventBox;
  PangoAttribute *attr;
  gint i;

  /* first clear the fields */
  for (i = 0; i < HL_COUNT; i++) {
    status->hl[i] = NULL;
    status->hlFadeList[i] = 0;
  }
  for (i = 0; i < COND_COUNT; i++) {
    status->conds[i] = NULL;
    status->condTips[i] = NULL;
  }
  status->tooltips = gtk_tooltips_new ();
  /* XXX think about using markup instead of attr-lists */
  /* TODO extract into function */
  /* create the attribute lists for label attributes (e.g. bold, color change etc.) */
  increaseAttrs = pango_attr_list_new ();
  attr = pango_attr_foreground_new (0, 30000, 0);
  attr->start_index = 0;
  attr->end_index = G_MAXINT;
  pango_attr_list_insert (increaseAttrs, attr);

  decreaseAttrs = pango_attr_list_new ();
  attr = pango_attr_foreground_new (50000, 0, 0);
  attr->start_index = 0;
  attr->end_index = G_MAXINT;
  pango_attr_list_insert (decreaseAttrs, attr);

  normalAttrs = pango_attr_list_new ();
  attr = pango_attr_foreground_new (0, 0, 0);
  attr->start_index = 0;
  attr->end_index = G_MAXINT;
  pango_attr_list_insert (normalAttrs, attr);

  boldAttrList = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = G_MAXINT;
  pango_attr_list_insert (boldAttrList, attr);

  smallAttrs = pango_attr_list_new ();
  attr = pango_attr_foreground_new (0, 0, 0);
  attr->start_index = 0;
  attr->end_index = G_MAXINT;
  pango_attr_list_insert (smallAttrs, attr);
  attr = pango_attr_size_new (8000);
  attr->start_index = 0;
  attr->end_index = G_MAXINT;
  pango_attr_list_insert (smallAttrs, attr);

  set_initial_player_values ();
  create_pixbufs ();

  /* first two rows */
  eventBox = gtk_event_box_new ();
  status->charName = gtk_label_new (_("Adventurer's Name"));
  gtk_misc_set_alignment (GTK_MISC (status->charName), 0.0f, 0.5f);
  gtk_misc_set_padding (GTK_MISC (status->charName), 5, 0);
  gtk_label_set_attributes (GTK_LABEL (status->charName), boldAttrList);
  gtk_label_set_line_wrap (GTK_LABEL (status->charName), TRUE);
  gtk_container_add (GTK_CONTAINER (eventBox), status->charName);
  gtk_tooltips_set_tip (status->tooltips, eventBox,
      _("Character rame and rank"), NULL);
  gtk_box_pack_start (GTK_BOX (&status->vBox), eventBox, FALSE, FALSE, 0);

  eventBox = gtk_event_box_new ();
  status->hl[HL_DGN] = gtk_label_new (_("Dungeon name"));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_DGN]), 0.0f, 0.5f);
  gtk_misc_set_padding (GTK_MISC (status->hl[HL_DGN]), 5, 0);
  gtk_label_set_line_wrap (GTK_LABEL (status->hl[HL_DGN]), TRUE);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_DGN]);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Dungeon name"), NULL);
  gtk_box_pack_start (GTK_BOX (&status->vBox), eventBox, FALSE, FALSE, 0);      //T,F,0

  eventBox = gtk_event_box_new ();
  status->hl[HL_DGNLV] = gtk_label_new (_("Depth"));
  gtk_misc_set_alignment (GTK_MISC (status->hl[HL_DGNLV]), 0.0f, 0.5f);
  gtk_misc_set_padding (GTK_MISC (status->hl[HL_DGNLV]), 5, 0);
  gtk_label_set_line_wrap (GTK_LABEL (status->hl[HL_DGNLV]), TRUE);
  gtk_container_add (GTK_CONTAINER (eventBox), status->hl[HL_DGNLV]);
  gtk_tooltips_set_tip (status->tooltips, eventBox, _("Depth"), NULL);
  gtk_box_pack_start (GTK_BOX (&status->vBox), eventBox, FALSE, FALSE, 0);      //T,F,0

  /* third row with aligment and gold */
  add_alignment_and_gold (status);

  sep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (&status->vBox), sep, FALSE, FALSE, 0);
  add_hp_and_pw (status);
  add_ac_level_and_exp (status);
  sep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (&status->vBox), sep, FALSE, FALSE, 0);
  add_attributes (status);
  sep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (&status->vBox), sep, FALSE, FALSE, 0);

  /* misc stuff section */
  add_optionals (status);
  add_condition_row (status);

  gtk_widget_show (GTK_WIDGET (status));
}


GtkWidget *
g2_status_new ()
{
  G2Status *g2Status = G2_STATUS (g_object_new (TYPE_G2_STATUS, NULL));

  g_signal_connect (G_OBJECT (g2Status), "curs", G_CALLBACK (g2_status_curs),
      NULL);
  g_signal_connect (G_OBJECT (g2Status), "putstr",
      G_CALLBACK (g2_status_putstr), NULL);
  g_signal_connect (G_OBJECT (g2Status), "display",
      G_CALLBACK (g2_status_display), NULL);
  g_signal_connect (G_OBJECT (g2Status), "player_acted",
      G_CALLBACK (g2_status_player_acted), NULL);

  return GTK_WIDGET (g2Status);
}
