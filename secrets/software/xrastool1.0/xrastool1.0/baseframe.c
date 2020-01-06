/* baseframe.c -- routines associated with xrastool's base frame */

#include "xrastool.h"
#include <xview/icon.h>
#include <xview/notify.h>

/* Global variables */

Frame BaseFrame;

int NukeCanvas = FALSE; /* To allow clean exits from program */

/* Base frame icon */

#include "bitmaps/base_icon.xbm"

/* Local functions */

static Notify_value base_frame_quit();

/************************************************************/

void MakeBaseFrame()
{
   /* Modifies BaseFrame (created in main()) and creates icon */

   Server_image icon_image;
   Icon icon;

   icon_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
      SERVER_IMAGE_X_BITS, base_icon_bits,
      XV_WIDTH, base_icon_width,
      XV_HEIGHT, base_icon_height,
      NULL);

   icon = (Icon) xv_create(XV_NULL, ICON,
      ICON_IMAGE, icon_image,
      WIN_FOREGROUND_COLOR, 1, /* Use whatever colors are available */
      WIN_BACKGROUND_COLOR, 0,
      NULL);

   (void) xv_set(BaseFrame,
      XV_X, DFLT_BASE_FRAME_XV_X,
      XV_Y, DFLT_BASE_FRAME_XV_Y,
      FRAME_LABEL, "xrastool: Main Panel",
      FRAME_SHOW_FOOTER, TRUE,
      FRAME_LEFT_FOOTER, "No images loaded",
      FRAME_RIGHT_FOOTER, VERSION,
      FRAME_ICON, icon,
      NULL);

   /* Interpose a destroy function */

   notify_interpose_destroy_func(BaseFrame, base_frame_quit);
}

static Notify_value base_frame_quit(client, status)
Notify_client client;
Destroy_status status;
{
   /* Intercepts BaseFrame quit request so that the canvas can be killed too */

   NukeCanvas = TRUE;
   (void) xv_destroy_safe(CanvasFrame);

   return notify_next_destroy_func(client, status);
}

/* baseframe.c */
