#include "xcb-canvas.h"

void print_modifiers (uint32_t mask)
{
  const char **mod, *mods[] = {
    "Shift", "Lock", "Ctrl", "Alt",
    "NumLock", "AltGr", "Win", "ScrollLock",
    "Button1", "Button2", "Button3", "Button4", "Button5"
  };
  printf ("Modifier mask: ");
  for (mod = mods ; mask; mask >>= 1, mod++)
    if (mask & 1)
      printf("%s+", *mod);
  putchar ('\n');
}

int init_xcb ()
{
  xcb_connection_t    *c;
  xcb_screen_t        *screen;
  xcb_window_t         win;
  uint32_t             mask = 0;
  uint32_t             values[2];

  /* Open the connection to the X server */
  c = xcb_connect (NULL, NULL);

  /* Get the first screen */
  screen = xcb_setup_roots_iterator (xcb_get_setup (c)).data;

  /* Ask for our window's Id */
  win = xcb_generate_id (c);

  /* Create the window */
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
              XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
              XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW   |
              XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE;
  xcb_create_window (c,                             /* Connection          */
                     0,                             /* depth               */
                     win,                           /* window Id           */
                     screen->root,                  /* parent window       */
                     10, 10,                        /* x, y                */
                     800, 600,                      /* width, height       */
                     10,                            /* border_width        */
                     XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                     screen->root_visual,           /* visual              */
                     mask, values);                 /* masks */

  /* Map the window on the screen */
  xcb_map_window (c, win);

  xcb_flush (c);

  handle_events (c);

  rendering_context.width = 800;
  rendering_context.height = 600;
  rendering_context.win = win;
  rendering_context.c = c;

  return 0;
}

void handle_events (xcb_connection_t *c)
{
  xcb_generic_event_t *e;
  while ((e = xcb_wait_for_event (c))) {
    switch (e->response_type & ~0x80) {
    case XCB_EXPOSE: {
      xcb_expose_event_t *ev = (xcb_expose_event_t *)e;

     /* printf ("Window %u exposed. Region to be redrawn at location (%d,%d), with dimension (%d,%d)\n", 
              ev->window, ev->x, ev->y, ev->width, ev->height); */
      break;
    }
    case XCB_BUTTON_PRESS: {
      xcb_button_press_event_t *ev = (xcb_button_press_event_t *)e;
      /* print_modifiers(ev->state); */

      switch (ev->detail) {
      case 4:
        /* printf ("Wheel Button up in window %u, at coordinates (%d,%d)\n",
                ev->event, ev->event_x, ev->event_y); */
        break;
      case 5:
        /* printf ("Wheel Button down in window %u, at coordinates (%d,%d)\n",
                ev->event, ev->event_x, ev->event_y); */
        break;
      default:
        /* printf ("Button %d pressed in window %u, at coordinates (%d,%d)\n",
                ev->detail, ev->event, ev->event_x, ev->event_y); */
      }
      break;
    }
    case XCB_BUTTON_RELEASE: {
      xcb_button_release_event_t *ev = (xcb_button_release_event_t *)e;
      /* print_modifiers(ev->state); */

      /* printf ("Button %d released in window %u, at coordinates (%d,%d)\n",
              ev->detail, ev->event, ev->event_x, ev->event_y); */
      break;
    }
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *)e;

      /* printf ("Mouse moved in window %u, at coordinates (%d,%d)\n",
              ev->event, ev->event_x, ev->event_y); */
      break;
    }
    case XCB_ENTER_NOTIFY: {
      xcb_enter_notify_event_t *ev = (xcb_enter_notify_event_t *)e;

      /* printf ("Mouse entered window %u, at coordinates (%d,%d)\n",
              ev->event, ev->event_x, ev->event_y); */
      break;
    }
    case XCB_LEAVE_NOTIFY: {
      xcb_leave_notify_event_t *ev = (xcb_leave_notify_event_t *)e;

      /* printf ("Mouse left window %u, at coordinates (%d,%d)\n",
              ev->event, ev->event_x, ev->event_y); */
      break;
    }
    case XCB_KEY_PRESS: {
      xcb_key_press_event_t *ev = (xcb_key_press_event_t *)e;
      /* print_modifiers(ev->state); */

      /* printf ("Key pressed in window %u\n",
              ev->event); */
      break;
    }
    case XCB_KEY_RELEASE: {
      xcb_key_release_event_t *ev = (xcb_key_release_event_t *)e;
      /* print_modifiers(ev->state); */

      /* printf ("Key released in window %u\n",
              ev->event); */
      break;
    }
    default:
      /* Unknown event type, ignore it */
      printf("Unknown event: %d\n", e->response_type);
      break;
    }
    /* Free the Generic Event */
    free (e);
  }
}

void set_window_size(RenderingContext* rendering_context, int new_width, int new_height)
{
    static uint32_t values[2];
    values[0] = new_width;
    values[1] = new_height;
    xcb_configure_window ( rendering_context->c, rendering_context->win,
            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    rendering_context->width = new_width;
    rendering_context->height = new_height;
}