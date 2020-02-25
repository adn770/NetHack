/* the equipment window
 *
 * $Id: g2equip.c,v 1.4 2004/07/20 00:08:56 miq Exp $
 *
 */

#include "config.h"

#include "g2equip.h"
#include "g2marsh.h"
#include "g2map.h"
#include "config.h"
#include "g2i18n.h"
#include "hack.h"

static GtkVBoxClass *parent_class;
static gint g2_equipment_signals[5];

static void g2_equipment_class_init (G2EquipmentClass * class);
static void g2_equipment_init (G2Equipment * equip);

enum
{
  EQUIP_QUIVER,
  EQUIP_HELMET,
  EQUIP_ALT_WEAPON,
  EQUIP_FACE,
  EQUIP_AMULET,
  EQUIP_ARMOR,
  EQUIP_CAPE,
  EQUIP_R_RING,
  EQUIP_L_RING,
  EQUIP_GLOVES,
  EQUIP_WEAPON,
  EQUIP_SHIELD,
  EQUIP_TWO_WEAPON,
#ifdef TOURIST
  EQUIP_SHIRT,
#endif
  EQUIP_SKIN,
  EQUIP_FEET,
  EQUIP_COUNT
};

typedef struct
{
  gint x;
  gint y;
} Pos;

struct obj *currWorn[EQUIP_COUNT];
struct obj *lastWorn[EQUIP_COUNT];

static Pos positions[] = {
  {2, 33},                      /* Quiver */
  {50, 2},                      /* Helmet */
  {98, 9},                      /*alternative weapon */
  {76, 9},                      /* Face */
  {50, 33},                     /* Amulet */
  {50, 52},                     /* Body armor */
  {98, 33},                     /* Cape? */
  {2, 75},                      /* right ring */
  {98, 75},                     /* left ring */
  {2, 95},                      /* gloves */
  {2, 55},                      /* weapon */
  {98, 55},                     /* shield? */
  {98, 55},                     /* second weapon, same position as shield! */
#ifdef TOURIST
  {50, 71},                     /* Underwear?, should be ifdef's TOURIST */
#endif
  {50, 89},                     /* dragon skin? */
  {50, 141},                    /* feet */
};


static const GTypeInfo g2_equipment_info = {
  sizeof (G2EquipmentClass),
  NULL,                         /* base_init */
  NULL,                         /* base_finalize */
  (GClassInitFunc) g2_equipment_class_init,
  NULL,                         /* class_finalize */
  NULL,                         /* class_data */
  sizeof (G2Equipment),
  0,                            /* n_preallocs */
  (GInstanceInitFunc) g2_equipment_init
};

GType
g2_equipment_get_type ()
{
  static GType g2_equipment_type = 0;

  if (g2_equipment_type == 0) {
    g2_equipment_type = g_type_register_static (GTK_TYPE_VBOX,
        "G2Equipment", &g2_equipment_info, 0);
  }
  return g2_equipment_type;
}

static void
g2_equipment_class_init (G2EquipmentClass * class)
{
  parent_class = gtk_type_class (gtk_vbox_get_type ());

  g2_equipment_signals[0] =
      g_signal_new ("update",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_FIRST,
      G_STRUCT_OFFSET (G2EquipmentClass, g2_equipment_update),
      NULL, NULL, gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void
g2_equipment_refresh_outline (GtkWidget * win, GdkEventExpose * event,
    gpointer data)
{
  G2Equipment *equip = G2_EQUIPMENT (data);

  gdk_draw_drawable (win->window, win->style->white_gc,
      equip->backBuffer, event->area.x, event->area.y,
      event->area.x, event->area.y, event->area.width, event->area.height);
}

static void
g2_equipment_set_worn ()
{
  currWorn[EQUIP_QUIVER] = uquiver;     /* Quiver */
  currWorn[EQUIP_HELMET] = uarmh;       /* Helmet */
  currWorn[EQUIP_ALT_WEAPON] = u.twoweap ? NULL : uswapwep;
  currWorn[EQUIP_FACE] = ublindf;       /* Face */
  currWorn[EQUIP_AMULET] = uamul;       /* Amulet */
  currWorn[EQUIP_ARMOR] = uarm; /* Body armor */
  currWorn[EQUIP_CAPE] = uarmc; /* Cape? */
  currWorn[EQUIP_R_RING] = uright;      /* right ring */
  currWorn[EQUIP_L_RING] = uleft;       /* left ring */
  currWorn[EQUIP_GLOVES] = uarmg;       /* gloves */
  currWorn[EQUIP_WEAPON] = uwep;        /* weapon */
  currWorn[EQUIP_SHIELD] = uarms;       /* shield? */
  currWorn[EQUIP_TWO_WEAPON] = u.twoweap ? uswapwep : NULL;
#ifdef TOURIST
  currWorn[EQUIP_SHIRT] = uarmu;        /* Underwear */
#endif
  currWorn[EQUIP_SKIN] = uskin; /* dragon-skin?! */
  currWorn[EQUIP_FEET] = uarmf; /* feet */
}


static void
g2_equipment_update (GtkWidget * win, gpointer gp)
{
  G2Equipment *equip = G2_EQUIPMENT (win);

  gint i;
  gint tileWidth = g2_get_tile_width ();
  gint tileHeight = g2_get_tile_height ();

  g2_equipment_set_worn ();
  for (i = 0; i < EQUIP_COUNT; i++) {
    if (currWorn[i] != lastWorn[i]) {
      if (currWorn[i]) {
        GdkPixbuf *tile =
            g2_get_tile (obj_to_glyph (currWorn[i], rn2_on_display_rng));

        gdk_draw_pixbuf (equip->backBuffer,
            equip->equipWin->style->white_gc, tile, 0, 0,
            positions[i].x, positions[i].y, tileWidth,
            tileHeight, GDK_RGB_DITHER_NONE, 0, 0);
        gtk_widget_queue_draw_area (equip->equipWin, positions[i].x,
            positions[i].y, tileWidth, tileHeight);
      } else {
        gdk_draw_pixbuf (equip->backBuffer,
            equip->equipWin->style->white_gc,
            equip->equipOutline, positions[i].x,
            positions[i].y, positions[i].x, positions[i].y,
            tileWidth, tileHeight, GDK_RGB_DITHER_NONE, 0, 0);
        gtk_widget_queue_draw_area (equip->equipWin, positions[i].x,
            positions[i].y, tileWidth, tileHeight);
      }
      lastWorn[i] = currWorn[i];
    }
  }
}

static void
g2_equipment_init (G2Equipment * equip)
{
  GError *err = NULL;
  PangoAttrList *boldAttrList;
  PangoAttribute *attr;
  GdkVisual *visual;

  g2_equipment_set_worn ();
  boldAttrList = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = G_MAXINT;
  pango_attr_list_insert (boldAttrList, attr);

  gtk_widget_set_name (GTK_WIDGET (equip), "equipment window");

  equip->equipWin = gtk_drawing_area_new ();
  equip->equipOutline = gdk_pixbuf_new_from_file ("equip.png", &err);
  /* XXX: determine depth somehow differently */
  visual = gdk_visual_get_system ();
  equip->backBuffer = gdk_pixmap_new (NULL,
      gdk_pixbuf_get_width (equip->equipOutline),
      gdk_pixbuf_get_height (equip->equipOutline), visual->depth);
  gdk_draw_pixbuf (equip->backBuffer, equip->equipWin->style->white_gc,
      equip->equipOutline, 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);

  gtk_drawing_area_size (GTK_DRAWING_AREA (equip->equipWin),
      gdk_pixbuf_get_width (equip->equipOutline),
      gdk_pixbuf_get_height (equip->equipOutline));
  g_signal_connect (G_OBJECT (equip->equipWin), "expose_event",
      G_CALLBACK (g2_equipment_refresh_outline), (gpointer) equip);
  gtk_box_pack_start (GTK_BOX (equip), equip->equipWin, TRUE, FALSE, 0);

  gtk_widget_set_size_request (GTK_WIDGET (equip),
      gdk_pixbuf_get_width (equip->equipOutline),
      gdk_pixbuf_get_height (equip->equipOutline));
  gtk_widget_show_all (GTK_WIDGET (equip));
}


GtkWidget *
g2_equipment_new ()
{
  G2Equipment *g2Equip;

  g2Equip = G2_EQUIPMENT (g_object_new (TYPE_G2_EQUIPMENT, NULL));
  g_signal_connect (G_OBJECT (g2Equip), "update",
      G_CALLBACK (g2_equipment_update), NULL);

  return GTK_WIDGET (g2Equip);
}
