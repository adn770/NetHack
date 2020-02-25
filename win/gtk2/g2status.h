/* the status window
 *
 * $Id: g2status.h,v 1.1.1.1 2004/06/23 02:01:45 miq Exp $
 *
 */

#ifndef G2_STATUS_WINDOW_H
#define G2_STATUS_WINDOW_H

#include <gtk/gtk.h>
#include "config.h"

#define TYPE_G2_STATUS (g2_status_get_type())
#define G2_STATUS(obj)          GTK_CHECK_CAST(obj, g2_status_get_type(), G2Status)
#define G2_STATUS_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, g2_status_get_type(), G2StatusClass)
#define IS_G2_STATUS(obj)       GTK_CHECK_TYPE(obj, g2_status_get_type())

typedef enum
{
  HL_DGN,
  HL_DGNLV,
  HL_GOLD,
  HL_HP,
  HL_PW,
  HL_AC,
  HL_LVL,
#ifdef EXP_ON_BOTL
  HL_EXP,
#endif
  HL_STR,
  HL_DEX,
  HL_CON,
  HL_INT,
  HL_WIS,
  HL_CHA,
  HL_TIME,
#ifdef SCORE_ON_BOTL
  HL_SCORE,
#endif
  HL_COUNT
} HighlightLabel;

typedef enum
{
  COND_HUNGER,
  COND_SICK,
  COND_BLINDED,
  COND_STUNNED,
  COND_HALLU,
  COND_CONFUSED,
  COND_ENC,
  COND_COUNT
} Condition;

typedef struct
{
  GtkVBox vBox;

  GtkWidget *charName;
  GtkWidget *alignment;
  GtkWidget *alignIcon;
  GtkWidget *hpBar;
  GtkWidget *pwBar;
  GtkWidget *hl[HL_COUNT];
  GtkWidget *optSep;
  gint hlFadeList[HL_COUNT];
  GtkWidget *conds[COND_COUNT];
  GtkTooltips *condTips[COND_COUNT];
  GtkTooltips *tooltips;
} G2Status;

typedef struct
{
  GtkVBoxClass parent_class;

  void (*g2_status_curs) (G2Status * g2Status);
  void (*g2_status_putstr) (G2Status * g2Status);
  void (*g2_status_player_acted) (G2Status * g2Status);
  void (*g2_status_display) (G2Status * g2Status);
  void (*g2status) (G2Status * g2Status);
} G2StatusClass;

GType g2_status_get_type (void);
GtkWidget *g2_status_new (void);

#endif /* G2_STATUS_WINDOW_H */
