/* the map window as a GObject
 *
 * we derive from <vbox> to build our own map widget with our own signals which
 * g2bind will emit on calls from the nethack engine
 *
 * $Id: g2map.h,v 1.1.1.1 2004/06/23 02:01:44 miq Exp $
 *
 */

#ifndef G2_MAP_WINDOW_H
#define G2_MAP_WINDOW_H

#include <glib.h>
#include <gtk/gtkscrolledwindow.h>

G_BEGIN_DECLS
#ifndef ROWNO
#define ROWNO 21
#endif
#ifndef COLNO
#define COLNO 80
#endif
#define G2_MAP_TYPE          (g2_map_get_type())
#define G2_MAP(obj)          GTK_CHECK_CAST(obj, G2_MAP_TYPE, G2Map)
#define G2_MAP_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, G2_MAP_TYPE, G2MapClass)
#define IS_G2_MAP(obj)       GTK_CHECK_TYPE(obj, G2_MAP_TYPE)
#define IS_G2_MAP_CLASS(obj) GTK_CHECK_CLASS_TYPE(obj, G2_MAP_TYPE)
    typedef struct
{
  GtkScrolledWindow scrolled_window;    /* a scrolled_window the parent */
  GtkWidget *canvas;                    /* a drawing_area */

  int theMap[COLNO][ROWNO];

  gdouble factor;
  gint tileWidth;
  gint tileHeight;
  GdkGC *gc;
  gint mapWidth;
  gint mapHeight;
  gint cursorX;
  gint cursorY;
} G2Map;

typedef struct
{
  GtkScrolledWindowClass parent_class;

  void (*g2_map_print_glyph) (G2Map * g2Map);
  void (*g2_map_clear) (G2Map * g2Map);
  void (*g2_map_cliparound) (G2Map * g2Map);
  void (*g2_map_curs) (G2Map * g2Map);
  void (*g2_map_display) (G2Map * g2Map);
  void (*g2_map_rescale) (G2Map * g2Map);
  void (*g2_map_reload_tiles) (G2Map * g2Map);
  void (*g2map) (G2Map * g2Map);
} G2MapClass;

GType g2_map_get_type (void);

//gint g2_get_tile_size();
gint g2_get_tile_width ();
gint g2_get_tile_height ();
gint g2_get_tile_count ();
GtkWidget *g2_map_new (void);
GdkPixbuf *g2_get_tile (gint glyph);
GdkPixbuf *g2_get_tile_scaled (gint glyph, gint size);

G_END_DECLS
#endif /* G2_MAP_WINDOW_H */
