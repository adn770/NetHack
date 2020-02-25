/* the map window
 *
 * $Id: g2map.c,v 1.1.1.1 2004/06/23 02:01:44 miq Exp $
 *
 */

/* see gtk tutorial on www.gtk.org for creating a composite widget */

#include <stdio.h>

#include "g2map.h"
#include "g2main.h"
#include "g2marsh.h"
#include "tile2x11.h"
#include "hack.h"

/* XPM */
const char *pet_mark_xpm[] = {
  /* width height ncolors chars_per_pixel */
  "8 7 2 1",
  /* colors */
  ". c None",
  "  c #FF0000",
  /* pixels */
  "........",
  "..  .  .",
  ".       ",
  ".       ",
  "..     .",
  "...   ..",
  ".... ..."
};

static GdkColor cursColor = {
  200 * 255,
  200 * 255,
  200 * 255,
};


static gint tileWidth;
static gint tileHeight;
static gint tileCount;
static gint tilesPerRow;

//static gint tileSize;


static GdkPixbuf *tileSet = NULL;
static GdkPixbuf **tiles = NULL;
static GdkPixbuf *petMark = NULL;

/* A cache of the last scaled tile */
static GdkPixbuf *scaledTile = NULL;
static gint scaledTileGlyph = -1;
static gint scaledTileSize = -1;


static gint g2_map_signals[7];

extern short glyph2tile[];      /* from tile.c */


/*
static void g2_map_size_request(GtkWidget        *widget,
                                GtkRequisition   *requisition);
*/

static void g2_map_class_init (G2MapClass * class);
static void g2_map_init (G2Map * map);
static gboolean expose_event (GtkWidget * widget, GdkEventExpose * event,
    gpointer data);

static void
load_tiles (gchar * tile_file, gint width, gint height)
{
  gint i;
  gint srcX;
  gint srcY;
  GError *err = NULL;
  GdkColor white;

  white.red = 65535;
  white.green = 65535;
  white.blue = 65535;

  tileWidth = width;
  tileHeight = height;
  tileSet = gdk_pixbuf_new_from_file (tile_file, &err);

  if (!tileSet) {
    fprintf (stderr, "Error loading tile set:%s\n", err->message);
  }
  tilesPerRow = gdk_pixbuf_get_width (tileSet) / tileWidth;
  tileCount = tilesPerRow * (gdk_pixbuf_get_height (tileSet) / tileHeight);

  // tileSize = gdk_pixbuf_get_width(tileSet) / TILES_PER_ROW;

  tiles = g_new0 (GdkPixbuf *, tileCount);
  for (i = 0; i < tileCount; i++) {
    srcX = (i % tilesPerRow) * tileWidth;
    srcY = (i / tilesPerRow) * tileHeight;
    tiles[i] =
        gdk_pixbuf_new_subpixbuf (tileSet, srcX, srcY, tileWidth, tileHeight);
  }
}

static void
init_tiles ()
{
  if (iflags.wc_tile_file == NULL)
    load_tiles ("ascii.png", 16, 24);
  else {
    load_tiles (iflags.wc_tile_file, iflags.wc_tile_width,
        iflags.wc_tile_height);
  }
}

static void
finalize_tiles ()
{
  g_free (tiles);
  gdk_pixbuf_unref (tileSet);

  if (scaledTile)
    gdk_pixbuf_unref (scaledTile);
}

/** Returns the default size of the tiles.
 */
// gint g2_get_tile_size()
// { return tileSize; }

/** Returns the default size of the tiles.
 */
gint
g2_get_tile_width ()
{
  return tileWidth;
}

/** Returns the default size of the tiles.
 */
gint
g2_get_tile_height ()
{
  return tileHeight;
}


/** Returns the number of tiles.
 */
gint
g2_get_tile_count ()
{
  return tileCount;
}

/** Returns a pixbuf with the requested tile. */
GdkPixbuf *
g2_get_tile (gint glyph)
{
  g_assert (tiles != NULL);
  return tiles[glyph2tile[glyph]];
}

/** Returns a pixbuf with the requested tile scaled to the size
 *  Do not free this tile.
 *  Note: we don't do much optimization here. Please refrain from the urges to do something here until
 *  you notice that it's really needed.
 */
GdkPixbuf *
g2_get_tile_scaled (gint glyph, gint size)
{
  /* nothing to scale here */
  if (size == tileWidth) {
    return g2_get_tile (glyph);
  }

  /* we cache only one (the last requested) tile */
  if (scaledTileGlyph == glyph && scaledTileSize == size) {
    return scaledTile;
  }
  if (scaledTile)
    gdk_pixbuf_unref (scaledTile);
  scaledTile = gdk_pixbuf_scale_simple (g2_get_tile (glyph), size, tileHeight * size / tileWidth, GDK_INTERP_NEAREST);  //GDK_INTERP_NEAREST
  scaledTileGlyph = glyph;
  scaledTileSize = size;

  return scaledTile;
}


static gboolean
expose_event (GtkWidget * canvas, GdkEventExpose * event, gpointer data)
{
  int startX, startY, endX, endY;
  int x, y;
  G2Map *map = G2_MAP (data);

  g_assert (canvas != NULL);
  g_assert (map != NULL);
  g_assert (map->canvas);

  startX = (int) ((event->area.x) / (gdouble) map->tileWidth);
  startY = (int) ((event->area.y) / (gdouble) map->tileHeight);
  endX = (int) ((event->area.x + event->area.width) / (gdouble) map->tileWidth);
  endY =
      (int) ((event->area.y + event->area.height) / (gdouble) map->tileHeight);

  if (!map->gc) {
    map->gc = gdk_gc_new (map->canvas->window);
    gdk_gc_set_rgb_fg_color (map->gc, &cursColor);
  }

  /* -- iterate over all dirty tiles and draw them */
  for (x = startX; x <= endX; x++) {
    for (y = startY; y <= endY; y++) {

      /* -- draw a border with the "selected" style */
      if (x < 0 || x >= COLNO || y < 0 || y >= ROWNO) {

        gtk_paint_flat_box (GTK_WIDGET (canvas)->style, canvas->window,
            GTK_STATE_SELECTED, GTK_SHADOW_NONE,
            &(event->area), canvas, "base",
            x * map->tileWidth, y * map->tileHeight,
            map->tileWidth, map->tileHeight);

        /* draw not yet discovered tiles with the "normal" style */
      } else if (map->theMap[x][y] < 0) {

        gtk_paint_flat_box (GTK_WIDGET (canvas)->style, canvas->window,
            GTK_STATE_NORMAL, GTK_SHADOW_NONE,
            &(event->area), canvas, "base",
            x * map->tileWidth, y * map->tileHeight,
            map->tileWidth, map->tileHeight);


        /* draw the tile for discovered. */
      } else {
        gdk_pixbuf_render_to_drawable (g2_get_tile_scaled
            (map->theMap[x][y],
                map->tileWidth), canvas->window,
            canvas->style->white_gc, 0, 0,
            x * map->tileWidth,
            y * map->tileHeight,
            map->tileWidth, map->tileHeight,
            GDK_RGB_DITHER_NONE, x * map->tileWidth, y * map->tileHeight);

        if (glyph_is_pet (map->theMap[x][y]) && iflags.hilite_pet) {
          gint width = gdk_pixbuf_get_width (petMark);
          gint height = gdk_pixbuf_get_height (petMark);

          gdk_draw_pixbuf (canvas->window,
              map->canvas->style->white_gc, petMark, 0,
              0, x * map->tileWidth, y * map->tileHeight,
              width, height, GDK_RGB_DITHER_NONE, 0, 0);
        }

      }

      /* draw the cursor */
      if (x == map->cursorX && y == map->cursorY)
        gdk_draw_rectangle (canvas->window, canvas->style->white_gc,
            FALSE, x * map->tileWidth,
            y * map->tileHeight, map->tileWidth - 1, map->tileHeight - 1);

    }
  }

  return TRUE;
}

/* clear the map and re-paint the window */
static void
g2_map_clear (GtkWidget * win, gpointer gp)
{
  int x, y;
  G2Map *map = G2_MAP (win);

  g_assert (map != NULL);

  for (x = 0; x < COLNO; x++)
    for (y = 0; y < ROWNO; y++)
      map->theMap[x][y] = -1;

  gtk_widget_queue_draw (map->canvas);
}

static void
g2_map_print_glyph (GtkWidget * win, int x, int y, int glyph, gpointer gp)
{
  G2Map *map = G2_MAP (win);

  g_assert (map != NULL);
  g_assert (map->canvas != NULL);

  map->theMap[x][y] = glyph;
  gtk_widget_queue_draw_area (map->canvas,
      x * map->tileWidth, y * map->tileHeight, map->tileWidth, map->tileHeight);

}

static void
g2_map_cliparound (GtkWidget * win, int x, int y, gpointer gp)
{
  g_assert (win != NULL);

  G2Map *map = G2_MAP (win);
  GtkAdjustment *vAdj =
      gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (map));
  GtkAdjustment *hAdj =
      gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (map));

  g_assert (vAdj != NULL);
  g_assert (hAdj != NULL);

  if (vAdj->upper != map->mapHeight)
    vAdj->upper = map->mapHeight;
  if (hAdj->upper != map->mapWidth)
    hAdj->upper = map->mapWidth;

  gint height = win->allocation.height;
  gint width = win->allocation.width;

  gint requestedCenterX = x * map->tileWidth + map->tileWidth / 2;
  gint requestedCenterY = y * map->tileHeight + map->tileHeight / 2;

  if (requestedCenterX <= width / 2) {
    gtk_adjustment_set_value (GTK_ADJUSTMENT (hAdj), 0.0f);
  } else if (requestedCenterX >= map->mapWidth - width / 2) {
    gtk_adjustment_set_value (GTK_ADJUSTMENT (hAdj),
        hAdj->upper - hAdj->page_size);
  } else {
    gtk_adjustment_set_value (GTK_ADJUSTMENT (hAdj),
        requestedCenterX - width / 2);
  }
  if (requestedCenterY <= height / 2) {
    gtk_adjustment_set_value (GTK_ADJUSTMENT (vAdj), 0.0f);
  } else if (requestedCenterY >= map->mapHeight - height / 2) {
    gtk_adjustment_set_value (GTK_ADJUSTMENT (vAdj),
        vAdj->upper - vAdj->page_size);
  } else {
    gtk_adjustment_set_value (GTK_ADJUSTMENT (vAdj),
        requestedCenterY - height / 2);
  }
}

static void
g2_map_curs (GtkWidget * win, int x, int y, gpointer gp)
{
  G2Map *map = G2_MAP (win);

  /* refresh last cursor position from backbuffer */
  gtk_widget_queue_draw_area (map->canvas,
      map->cursorX * map->tileWidth,
      map->cursorY * map->tileHeight, map->tileWidth, map->tileHeight);
  map->cursorX = x;
  map->cursorY = y;
  /* schedule new position for refresh */
  gtk_widget_queue_draw_area (map->canvas,
      map->cursorX * map->tileWidth,
      map->cursorY * map->tileHeight, map->tileWidth, map->tileHeight);
}

static void
g2_map_display (GtkWidget * win, gboolean block, gpointer gp)
{
  g_assert (win != NULL);

  gtk_widget_show_all (win);
  gtk_widget_grab_focus (win);
}


/** Set's a new scale for the map and returns the new scale.
 *  @param scale The scale with which the old scale is multiplied. 1.0 if it should remain unchanged.
 *    The scale must not be zero.
 *  @returns The new scale factor.
 */
static gdouble
g2_map_rescale (GtkWidget * win, gdouble scale, gpointer gp)
{
  G2Map *map = G2_MAP (win);

  g_assert (map != NULL);

  if (g2_get_tile_width () * (map->factor + scale) >= 8) {
    map->factor += scale;
  }

  map->tileWidth = g2_get_tile_width () * map->factor;
  map->tileHeight = g2_get_tile_height () * map->factor;
  map->mapWidth = COLNO * map->tileWidth;
  map->mapHeight = ROWNO * map->tileHeight;

  gtk_widget_set_size_request (map->canvas, map->mapWidth, map->mapHeight);

  /* setting the middle again seems not to work right now.
     Maybe the size_request was not handled yet? */
  g2_map_cliparound (win, map->cursorX, map->cursorY, NULL);    /* center around cursor */
  // gtk_widget_queue_draw(map->canvas);
  return map->factor;
}

static void
reload_tiles (GtkWidget * win, gchar * tile_file, gint width, gint height,
    gdouble factor)
{
  G2Map *map = G2_MAP (win);

  g_assert (map != NULL);

  finalize_tiles ();
  load_tiles (tile_file, width, height);
  map->factor = factor;
  map->tileWidth = g2_get_tile_width () * map->factor;
  map->tileHeight = g2_get_tile_height () * map->factor;
  map->mapWidth = COLNO * map->tileWidth;
  map->mapHeight = ROWNO * map->tileHeight;

  gtk_widget_set_size_request (map->canvas, map->mapWidth, map->mapHeight);
  g2_map_cliparound (win, map->cursorX, map->cursorY, NULL);    /* center around cursor */
}

static void
g2_map_reload_tiles (GtkWidget * win, gint tileset, gpointer gp)
{
  switch (tileset) {
    case TILESET_DEFAULT:
      reload_tiles (win, iflags.wc_tile_file, iflags.wc_tile_width,
          iflags.wc_tile_height, 0.5);
      break;
    case TILESET_STANDARD:
      reload_tiles (win, "tiles32.png", 32, 32, 0.5);
      break;
    case TILESET_ASCII:
      reload_tiles (win, "ascii.png", 16, 24, 0.5);
      break;
  }
}

static gboolean
g2_map_button_press_event (GtkWidget * widget, GdkEventButton * event,
    gpointer data)
{
  G2Map *map = G2_MAP (widget);

  int tilePosX;
  int tilePosY;

  GtkAdjustment *vAdj =
      gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (map));
  GtkAdjustment *hAdj =
      gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (map));

  gdouble xOffset = gtk_adjustment_get_value (hAdj);
  gdouble yOffset = gtk_adjustment_get_value (vAdj);

  tilePosX = (int) ((event->x + xOffset) / (gdouble) map->tileWidth);
  tilePosY = (int) ((event->y + yOffset) / (gdouble) map->tileHeight);

  /* The values can be out of range if the map window has been resized */
  /* to be larger than the max size.                                     */
  if (tilePosX >= COLNO)
    tilePosX = COLNO - 1;
  if (tilePosY >= ROWNO)
    tilePosY = ROWNO - 1;

  /* signal a new pos-event */
  clickX = tilePosX;
  clickY = tilePosY;
  /* Map all buttons but the first to the second click */
  clickMod = (event->button == 1) ? CLICK_1 : CLICK_2;
  keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (0));

  return TRUE;
}


/** return the prefered size for the map widget */
/*
static void
g2_map_size_request (GtkWidget      *widget,
                     GtkRequisition *requisition)
{
  G2Map *map = G2_MAP (widget);

  requisition->height = map->mapWidth;
  requisition->width  = map->mapHeight;
} */


static void
g2_map_init (G2Map * map)
{
  map->factor = 1.0;
  map->tileWidth = g2_get_tile_width () * map->factor;
  map->tileHeight = g2_get_tile_height () * map->factor;
  map->canvas = gtk_drawing_area_new ();

  g2_map_clear (GTK_WIDGET (map), NULL);

  map->gc = NULL;
  map->mapWidth = COLNO * map->tileWidth;
  map->mapHeight = ROWNO * map->tileHeight;
  map->cursorX = 0;
  map->cursorY = 0;

  gtk_widget_set_name (GTK_WIDGET (map), "map window");
  gtk_widget_set_double_buffered (map->canvas, FALSE);
  gtk_widget_set_size_request (map->canvas, map->mapWidth, map->mapHeight);

  /* minimum size */
  gtk_widget_set_size_request (GTK_WIDGET (map), map->tileWidth * 5,
      map->tileHeight * 6);

  gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (map),
      GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0)));
  gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (map),
      GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0)));
#ifdef HILDON
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (map),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
#else
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (map),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#endif
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (map),
      map->canvas);
  g_signal_connect (G_OBJECT (map->canvas), "expose_event",
      G_CALLBACK (expose_event), (gpointer) map);
}

GtkWidget *
g2_map_new ()
{
  G2Map *g2Map;

  g2Map = G2_MAP (g_object_new (G2_MAP_TYPE, NULL));

  g_signal_connect (G_OBJECT (g2Map), "print_glyph",
      G_CALLBACK (g2_map_print_glyph), NULL);
  g_signal_connect (G_OBJECT (g2Map), "clear", G_CALLBACK (g2_map_clear), NULL);
  g_signal_connect (G_OBJECT (g2Map), "cliparound",
      G_CALLBACK (g2_map_cliparound), NULL);
  g_signal_connect (G_OBJECT (g2Map), "curs", G_CALLBACK (g2_map_curs), NULL);
  g_signal_connect (G_OBJECT (g2Map), "display", G_CALLBACK (g2_map_display),
      NULL);
  g_signal_connect (G_OBJECT (g2Map), "button_press_event",
      G_CALLBACK (g2_map_button_press_event), NULL);
  g_signal_connect (G_OBJECT (g2Map), "rescale", G_CALLBACK (g2_map_rescale),
      NULL);
  g_signal_connect (G_OBJECT (g2Map), "reload_tiles",
      G_CALLBACK (g2_map_reload_tiles), NULL);
  gtk_widget_set_events (GTK_WIDGET (g2Map),
      gtk_widget_get_events (GTK_WIDGET (g2Map)) | GDK_BUTTON_PRESS_MASK |
      GDK_BUTTON_RELEASE_MASK);
  return GTK_WIDGET (g2Map);
}


static void
g2_map_class_init (G2MapClass * class)
{
  init_tiles ();
  petMark = gdk_pixbuf_new_from_xpm_data (pet_mark_xpm);

  /* activating this removes the scroll-bars! */
  /*  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
     widget_class->size_request = g2_map_size_request; */
  g2_map_signals[0] =
      g_signal_new ("print_glyph",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (G2MapClass, g2_map_print_glyph),
      NULL, NULL,
      g2_marshal_VOID__INT_INT_INT, G_TYPE_NONE, 3, G_TYPE_INT,
      G_TYPE_INT, G_TYPE_INT);
  g2_map_signals[1] =
      g_signal_new ("clear",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (G2MapClass, g2_map_clear),
      NULL, NULL, gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);
  g2_map_signals[2] =
      g_signal_new ("cliparound",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (G2MapClass, g2_map_cliparound),
      NULL, NULL, gtk_marshal_VOID__INT_INT, G_TYPE_NONE, 2,
      G_TYPE_INT, G_TYPE_INT);
  g2_map_signals[3] =
      g_signal_new ("curs",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (G2MapClass, g2_map_curs),
      NULL, NULL, gtk_marshal_VOID__INT_INT, G_TYPE_NONE, 2,
      G_TYPE_INT, G_TYPE_INT);
  g2_map_signals[4] =
      g_signal_new ("display",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (G2MapClass, g2_map_display),
      NULL, NULL, gtk_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
  g2_map_signals[5] =
      g_signal_new ("rescale",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (G2MapClass, g2_map_rescale),
      NULL, NULL, g2_marshal_DOUBLE__DOUBLE, G_TYPE_DOUBLE, 1, G_TYPE_DOUBLE);
  g2_map_signals[6] =
      g_signal_new ("reload_tiles",
      G_OBJECT_CLASS_TYPE (class),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (G2MapClass, g2_map_reload_tiles),
      NULL, NULL, gtk_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

/** Note: I don't know how to put this into the "static" g2Map type */
static void
g2_map_class_finalize (G2MapClass * class)
{
  finalize_tiles ();
}

GType
g2_map_get_type ()
{
  static GType g2_map_type = 0;

  if (!g2_map_type) {
    static const GTypeInfo g2_map_info = {
      sizeof (G2MapClass),
      NULL,                     /* base_init */
      NULL,                     /* base_finalize */
      (GClassInitFunc) g2_map_class_init,
      NULL,                     /* class_finalize */
      NULL,                     /* class_data */
      sizeof (G2Map),
      0,                        /* n_preallocs */
      (GInstanceInitFunc) g2_map_init
    };

    g2_map_type = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW,
        "G2Map", &g2_map_info, 0);
  }
  return g2_map_type;
}
