/* minipad widget
 *
 * g2minipad.c
 *
 * The mini game pad control.
 */

#include <math.h>
#include <stdio.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include "g2minipad.h"
#include "g2main.h"
#include "hack.h"

#define MINIPAD_DEFAULT_WIDTH 	100
#define MINIPAD_DEFAULT_HEIGHT 	 60
#define BUTTON_TIMER_DELAY	200

typedef struct
{
  const gchar key;
  const gchar num_key;
} DirectionButton;

static DirectionButton direction_buttons[] = {
  {'.', '.'},
  {'k', '8'},
  {'u', '9'},
  {'l', '6'},
  {'n', '3'},
  {'j', '2'},
  {'b', '1'},
  {'h', '4'},
  {'y', '7'},
  {'<', '<'},
  {'>', '>'},
};

/* Forward declarations */

static void gtk_minipad_class_init (GtkMinipadClass * klass);
static void gtk_minipad_init (GtkMinipad * minipad);
static void gtk_minipad_destroy (GtkObject * object);
static void gtk_minipad_realize (GtkWidget * widget);
static void gtk_minipad_size_request (GtkWidget * widget,
    GtkRequisition * requisition);
static void gtk_minipad_size_allocate (GtkWidget * widget,
    GtkAllocation * allocation);
static gboolean gtk_minipad_expose (GtkWidget * widget, GdkEventExpose * event);
static gboolean gtk_minipad_button_press (GtkWidget * widget,
    GdkEventButton * event);
static gboolean gtk_minipad_button_release (GtkWidget * widget,
    GdkEventButton * event);
static gboolean gtk_minipad_motion_notify (GtkWidget * widget,
    GdkEventMotion * event);

static void gtk_minipad_draw_blue_hotzone (GtkWidget * widget, gint hp_index);

/* Local data */

static GtkWidgetClass *parent_class = NULL;

static GdkPixbuf *pad_image_unclicked = NULL;
static GdkPixbuf *pad_image_clicked = NULL;
static gint width_pad_image;
static gint height_pad_image;

GType
gtk_minipad_get_type ()
{
  static GType minipad_type = 0;

  if (!minipad_type) {
    static const GTypeInfo minipad_info = {
      sizeof (GtkMinipadClass),
      NULL,
      NULL,
      (GClassInitFunc) gtk_minipad_class_init,
      NULL,
      NULL,
      sizeof (GtkMinipad),
      0,
      (GInstanceInitFunc) gtk_minipad_init,
    };

    minipad_type =
        g_type_register_static (GTK_TYPE_WIDGET, "GtkMinipad", &minipad_info,
        0);
  }

  return minipad_type;
}

static void
gtk_minipad_class_init (GtkMinipadClass * class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;

  parent_class = gtk_type_class (gtk_widget_get_type ());

  object_class->destroy = gtk_minipad_destroy;

  widget_class->realize = gtk_minipad_realize;
  widget_class->expose_event = gtk_minipad_expose;
  widget_class->size_request = gtk_minipad_size_request;
  widget_class->size_allocate = gtk_minipad_size_allocate;
  widget_class->button_press_event = gtk_minipad_button_press;
  widget_class->button_release_event = gtk_minipad_button_release;
  widget_class->motion_notify_event = gtk_minipad_motion_notify;
}

static void
load_pad_image ()
{
  GError *err = NULL;

  if (pad_image_unclicked == NULL) {
    pad_image_unclicked = gdk_pixbuf_new_from_file ("pad_unclicked.png", &err);
  }
  if (!pad_image_unclicked) {
    fprintf (stderr, "Error loading pad_image_unclicked:%s\n", err->message);
  }

  if (pad_image_clicked == NULL) {
    pad_image_clicked = gdk_pixbuf_new_from_file ("pad_clicked.png", &err);
  }
  if (!pad_image_clicked) {
    fprintf (stderr, "Error loading pad_image_clicked:%s\n", err->message);
  }

  width_pad_image = gdk_pixbuf_get_width (pad_image_unclicked);
  height_pad_image = gdk_pixbuf_get_height (pad_image_unclicked);
}

static void
gtk_minipad_init (GtkMinipad * minipad)
{
  minipad->button = 0;
  minipad->timer = 0;
  minipad->current_dir = -1;
  load_pad_image ();
}

GtkWidget *
gtk_minipad_new (void)
{
  GtkMinipad *minipad;

  minipad = g_object_new (gtk_minipad_get_type (), NULL);

  return GTK_WIDGET (minipad);
}

static void
gtk_minipad_destroy (GtkObject * object)
{
  GtkMinipad *minipad;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_MINIPAD (object));

  minipad = GTK_MINIPAD (object);
//  gdk_pixbuf_unref(pad_image_unclicked);
//  gdk_pixbuf_unref(pad_image_clicked);
  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_minipad_realize (GtkWidget * widget)
{
  GtkMinipad *minipad;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_MINIPAD (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  minipad = GTK_MINIPAD (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) |
      GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
      GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
      GDK_POINTER_MOTION_HINT_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window =
      gdk_window_new (widget->parent->window, &attributes, attributes_mask);

  widget->style = gtk_style_attach (widget->style, widget->window);

  gdk_window_set_user_data (widget->window, widget);

  gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
}

static void
gtk_minipad_size_request (GtkWidget * widget, GtkRequisition * requisition)
{
  requisition->width = width_pad_image; // MINIPAD_DEFAULT_WIDTH;
  requisition->height = height_pad_image;       // MINIPAD_DEFAULT_HEIGHT;
}

static void
gtk_minipad_size_allocate (GtkWidget * widget, GtkAllocation * allocation)
{
  GtkMinipad *minipad;
  gint radius;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_MINIPAD (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  minipad = GTK_MINIPAD (widget);

  if (GTK_WIDGET_REALIZED (widget)) {
    gdk_window_move_resize (widget->window,
        allocation->x, allocation->y, allocation->width, allocation->height);
  }

  radius = MIN (allocation->width, allocation->height) * 0.3;

  /* Self direction */
  minipad->hotzone[0].x = 98;
  minipad->hotzone[0].y = 41;
  minipad->hotzone[0].width = 32;
  minipad->hotzone[0].height = 32;
  /* North direction */
  minipad->hotzone[1].x = 101;
  minipad->hotzone[1].y = 14;
  minipad->hotzone[1].width = 26;
  minipad->hotzone[1].height = 24;
  /* North-east direction */
  minipad->hotzone[2].x = 127;
  minipad->hotzone[2].y = 20;
  minipad->hotzone[2].width = 25;
  minipad->hotzone[2].height = 25;
  /* East direction */
  minipad->hotzone[3].x = 136;
  minipad->hotzone[3].y = 45;
  minipad->hotzone[3].width = 25;
  minipad->hotzone[3].height = 25;
  /* South-east direction */
  minipad->hotzone[4].x = 126;
  minipad->hotzone[4].y = 69;
  minipad->hotzone[4].width = 24;
  minipad->hotzone[4].height = 24;
  /* South direction */
  minipad->hotzone[5].x = 102;
  minipad->hotzone[5].y = 80;
  minipad->hotzone[5].width = 25;
  minipad->hotzone[5].height = 24;
  /* South-west direction */
  minipad->hotzone[6].x = 77;
  minipad->hotzone[6].y = 70;
  minipad->hotzone[6].width = 25;
  minipad->hotzone[6].height = 24;
  /* West direction */
  minipad->hotzone[7].x = 69;
  minipad->hotzone[7].y = 45;
  minipad->hotzone[7].width = 25;
  minipad->hotzone[7].height = 25;
  /* North-west direction */
  minipad->hotzone[8].x = 77;
  minipad->hotzone[8].y = 20;
  minipad->hotzone[8].width = 25;
  minipad->hotzone[8].height = 24;
  /* Up direction */
  minipad->hotzone[9].x = 18;
  minipad->hotzone[9].y = 20;
  minipad->hotzone[9].width = 22;
  minipad->hotzone[9].height = 32;
  /* Down direction */
  minipad->hotzone[10].x = 18;
  minipad->hotzone[10].y = 60;
  minipad->hotzone[10].width = 22;
  minipad->hotzone[10].height = 32;
}

static gboolean
gtk_minipad_expose (GtkWidget * widget, GdkEventExpose * event)
{
  GtkMinipad *minipad;
  GdkGC *gc;
  GdkColor black, white;

  black.red = 0;
  black.green = 0;
  black.blue = 0;               // black
  white.red = 65535;
  white.green = 65535;
  white.blue = 65535;           // white

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_MINIPAD (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->count > 0)
    return FALSE;

  minipad = GTK_MINIPAD (widget);

  gc = gdk_gc_new (widget->window);

  gdk_draw_pixbuf (widget->window, gc, pad_image_unclicked, 0, 0,
      0, 0, width_pad_image, height_pad_image, GDK_RGB_DITHER_NONE, 0, 0);

  g_object_unref (gc);
  return FALSE;
}

static void
gtk_minipad_draw_blue_hotzone (GtkWidget * widget, gint hz_index)
{
  GtkMinipad *minipad;
  GdkGC *gc;
  GdkColor blue;

  blue.red = 0;
  blue.green = 0;
  blue.blue = 65535;            // blue

  minipad = GTK_MINIPAD (widget);

  gc = gdk_gc_new (widget->window);

  gdk_draw_pixbuf (widget->window, gc, pad_image_clicked,
      minipad->hotzone[hz_index].x, minipad->hotzone[hz_index].y,
      minipad->hotzone[hz_index].x, minipad->hotzone[hz_index].y,
      minipad->hotzone[hz_index].width,
      minipad->hotzone[hz_index].height, GDK_RGB_DITHER_NONE, 0, 0);
#if 0
  gdk_gc_set_rgb_fg_color (gc, &blue);
  gdk_gc_set_rgb_bg_color (gc, &blue);

  gdk_draw_rectangle (widget->window, gc, TRUE,
      minipad->hotzone[hz_index].x,
      minipad->hotzone[hz_index].y,
      minipad->hotzone[hz_index].width, minipad->hotzone[hz_index].height);
#endif

  g_object_unref (gc);
}

static gboolean
hotzone_touched (GdkPoint pointer, GdkRectangle hotzone)
{
  gint dist_x = pointer.x - hotzone.x;
  gint dist_y = pointer.y - hotzone.y;

  if (dist_x < 0)
    return FALSE;
  if (dist_y < 0)
    return FALSE;
  if (dist_x > hotzone.width)
    return FALSE;
  if (dist_y > hotzone.height)
    return FALSE;

  return TRUE;
}

static void
sendNHkey (gint db)
{
  gint key;

  if (iflags.num_pad)
    key = direction_buttons[db].num_key;
  else
    key = direction_buttons[db].key;

  keyBuffer = g_slist_append (keyBuffer, GINT_TO_POINTER (key));
}

static gint
gtk_minipad_button_timer (GtkMinipad * minipad)
{
  gboolean retval = FALSE;

  GDK_THREADS_ENTER ();

  if (minipad->timer) {
    //if (minipad->current_dir != -1) {
    if (gtk_grab_get_current () == GTK_WIDGET (minipad)) {
      sendNHkey (minipad->current_dir);
      minipad->timer = gtk_timeout_add
          (BUTTON_TIMER_DELAY,
          (GtkFunction) gtk_minipad_button_timer, (gpointer) minipad);
    } else {
      // grab was released in between or stolen (e.g. by modal dialog)
      // if stolen, we probably missed the button release: do it now
      if (minipad->current_dir != -1) {
        gtk_grab_remove (GTK_WIDGET (minipad));
        minipad->button = 0;
        minipad->current_dir = -1;
        gtk_widget_queue_draw (GTK_WIDGET (minipad));
      }
    }
  }
  GDK_THREADS_LEAVE ();

  return retval;
}

static gboolean
gtk_minipad_button_press (GtkWidget * widget, GdkEventButton * event)
{
  GtkMinipad *minipad;
  GdkPoint pointer;
  gint i;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_MINIPAD (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  minipad = GTK_MINIPAD (widget);

  pointer.x = event->x;
  pointer.y = event->y;

  for (i = 0; i < 11; i++) {
    if (hotzone_touched (pointer, minipad->hotzone[i])) {
      if (!minipad->button) {
        gtk_grab_add (widget);
        minipad->button = event->button;
        gtk_minipad_draw_blue_hotzone (widget, i);
      }
      minipad->current_dir = i;
      sendNHkey (minipad->current_dir);
      minipad->timer = gtk_timeout_add
          (BUTTON_TIMER_DELAY,
          (GtkFunction) gtk_minipad_button_timer, (gpointer) minipad);
      break;
    }
  }

  return FALSE;
}

static gboolean
gtk_minipad_button_release (GtkWidget * widget, GdkEventButton * event)
{
  GtkMinipad *minipad;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_MINIPAD (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  minipad = GTK_MINIPAD (widget);

  if (minipad->button == event->button) {
    gtk_grab_remove (widget);
    minipad->button = 0;
    minipad->current_dir = -1;
    gtk_widget_queue_draw (GTK_WIDGET (minipad));
  }

  return FALSE;
}

static gboolean
gtk_minipad_motion_notify (GtkWidget * widget, GdkEventMotion * event)
{
  GtkMinipad *minipad;
  GdkModifierType mods;
  gint x, y, mask;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_MINIPAD (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  minipad = GTK_MINIPAD (widget);

  if (minipad->button != 0) {
    x = event->x;
    y = event->y;

    if (event->is_hint || (event->window != widget->window))
      gdk_window_get_pointer (widget->window, &x, &y, &mods);

    switch (minipad->button) {
      case 1:
        mask = GDK_BUTTON1_MASK;
        break;
      case 2:
        mask = GDK_BUTTON2_MASK;
        break;
      case 3:
        mask = GDK_BUTTON3_MASK;
        break;
      default:
        mask = 0;
        break;
    }
  }

  return FALSE;
}
