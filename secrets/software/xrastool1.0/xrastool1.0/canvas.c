/* canvas.c -- routines for xrastool's canvas, including image display */

#include "xrastool.h"
#include <xview/icon.h>
#include <xview/notify.h>
#include <xview/scrollbar.h>
#include <xview/cursor.h>
#include <xview/xv_xrect.h>

/* Global variables */

Frame CanvasFrame;
int ScrollMargin;
Window CanvasXID;
Server_image Backdrop[NUM_BACKDROPS];
GC BackdropGC;
XImage *TmpXImage;
Xv_singlecolor TmpColors[MAX_NUM_COLORS];

/* Local variables */

static Canvas canvas;        /* Canvas handle */
static Xv_Cursor cursor;     /* Cursor handle */
static int image_x, image_y; /* Pos'n of top left of image in frame */

/* Bitmaps */

#include "bitmaps/canvas_icon.xbm"
#include "bitmaps/cursor.xbm"
#include "bitmaps/backdrops.xbm"

/* Local functions */

static Notify_value canvas_frame_quit();

static void canvas_event_proc(), repaint_canvas(), set_canvas_size_and_pos();

/************************************************************/

void MakeCanvas()
{
   /* Constructs canvas, icon, scrollbars, cursor and a few other things... */

   Server_image image;
   Icon icon;
   Xv_singlecolor fg, bg;
   Xv_window canvas_window;
   XGCValues values;

   /* Create icon */

   image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
      SERVER_IMAGE_X_BITS, canvas_icon_bits,
      XV_WIDTH, canvas_icon_width,
      XV_HEIGHT, canvas_icon_height,
      NULL);

   icon = (Icon) xv_create(XV_NULL, ICON,
      ICON_IMAGE, image,
      WIN_FOREGROUND_COLOR, 1,
      WIN_BACKGROUND_COLOR, 0,
      NULL);

   /* Create canvas frame */

   CanvasFrame = xv_create(XV_NULL, FRAME,
      XV_X, DFLT_CANVAS_FRAME_XV_X,
      XV_Y, DFLT_CANVAS_FRAME_XV_Y,
      FRAME_LABEL, "xrastool: Canvas",
      FRAME_SHOW_FOOTER, TRUE, /* Needed to propagate colors */
      FRAME_ICON, icon,
      NULL);

   /* Interpose destroy function */

   notify_interpose_destroy_func(CanvasFrame, canvas_frame_quit);

   /* Create canvas */

   canvas = xv_create(CanvasFrame, CANVAS,
      CANVAS_AUTO_EXPAND, FALSE,
      CANVAS_AUTO_SHRINK, FALSE,
      CANVAS_REPAINT_PROC, repaint_canvas,
      CANVAS_X_PAINT_WINDOW, TRUE,
      NULL);

   /* Get window ID */

   canvas_window = canvas_paint_window(canvas);

   /* Intercept key presses and mouse movements */

   (void) xv_set(canvas_window,
      WIN_CONSUME_EVENTS,
         WIN_NO_EVENTS, WIN_ASCII_EVENTS, LOC_MOVE, LOC_WINEXIT, LOC_DRAG,
         WIN_MOUSE_BUTTONS, NULL,
      WIN_EVENT_PROC, canvas_event_proc,
      NULL);

   /* Construct scollbars if desired initially */

   SetScrollbars(!(Options & NO_SCROLLING));

   /* Construct cursor */

   image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
      SERVER_IMAGE_X_BITS, cursor_bits,
      XV_WIDTH, cursor_width,
      XV_HEIGHT, cursor_height,
      NULL);

   bg.red = bg.green = bg.blue = 0;   /* Black */
   fg.red = fg.green = fg.blue = 255; /* White */

   cursor = (Xv_cursor) xv_create(canvas, CURSOR,
      CURSOR_IMAGE, image,
      CURSOR_FOREGROUND_COLOR, &fg,
      CURSOR_BACKGROUND_COLOR, &bg,
      CURSOR_XHOT, cursor_x_hot,
      CURSOR_YHOT, cursor_y_hot,
      NULL);

   (void) xv_set(canvas_window, WIN_CURSOR, cursor, NULL);

   /* Get canvas XID */

   CanvasXID = (Window) xv_get(canvas_window, XV_XID);

   /* Prepare GC for backdrops (stipple set in SetBackdrop()) */

   values.foreground = BlackPixel(MainDisplay, MainScreen);
   values.background = WhitePixel(MainDisplay, MainScreen);
   values.fill_style = FillOpaqueStippled;

   BackdropGC = XCreateGC(MainDisplay, CanvasXID,
      GCForeground | GCBackground | GCFillStyle, &values);
}

void SetScrollbars(toggle_on)
Bool toggle_on;
{
   /* Creates (toggle_on) or destroys (!toggle_on) scrollbars */

   static Bool no_scrollbars = TRUE; /* To counter repeated requests */
   static Scrollbar vbar, hbar; /* Handles only need to be local */

   if (no_scrollbars && toggle_on) { /* Create scrollbars */
      vbar = xv_create(canvas, SCROLLBAR,
         SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
         NULL);

      /* Determine scrollbar thickness */

      ScrollMargin = (int) xv_get(vbar, XV_WIDTH);

      hbar = xv_create(canvas, SCROLLBAR,
         SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
         NULL);

      if (ScrollMargin != (int) xv_get(hbar, XV_HEIGHT))
         Warning("Unequal scrollbar thicknesses.");

      no_scrollbars = FALSE;
   }
   else if (!no_scrollbars && !toggle_on) { /* Destroy scrollbars */
      (void) xv_destroy_safe(hbar);
      (void) xv_destroy_safe(vbar);
      ScrollMargin = 0;
      no_scrollbars = TRUE;
   }
}

void ShowNewImage()
{
   /* Displays (or redisplays) current image */

   static int first_call = TRUE; /* For color management and XV_SHOW */
   static int ximage_exists = FALSE; /* For LIVE_CURSOR */

   IMAGE_T *image;

   /* Abort if image hasn't been loaded yet */

   if (UNDEF_IMAGE)
       return;

   /* Otherwise get image */

   image = Image[Current];

   /* If sizing is allowed or forced, set canvas size (also reposition) */

   if (!(Options & NO_RESIZING) || ForceFirstResize)
       set_canvas_size_and_pos();

   /* If colors are locked and first call, initialize locked colors */

   if (first_call && (Options & LOCK_COLORS)) {
       if (OneCmap) { /* Currently unique colormap must be from first image */
          NumLockedColors = Image[0]->num_colors;
          LockedColors = Image[0]->colors;
       }
       else {
          NumLockedColors = image->num_colors;
          LockedColors = image->colors;
       }
   }

   /*
    * If color scaling has been disabled, it may be necessary to install
    *    new colors here, unless colors are locked. Also TmpColors needs
    *    to be loaded with the current colors for LIVE_CURSOR to work
    *    (this is done automatically if color scaling is activated).
    */

   if (Options & NO_SCALING) {
      if (first_call || !(Options & LOCK_COLORS))
         InstallCms(image->num_colors, image->colors);
      if (Options & LIVE_CURSOR)
         if (Options & LOCK_COLORS) /* No need to do this every time... */
            CopyColors(NumLockedColors, LockedColors, TmpColors);
         else
            CopyColors(image->num_colors, image->colors, TmpColors);
   }
   else
       ApplyColorScaling();

   /* On first call, show the canvas frame (with footer if applicable) */

   if (first_call) {
      (void) xv_set(CanvasFrame,
         XV_SHOW, TRUE,
         FRAME_SHOW_FOOTER, (Options & LIVE_CURSOR),
         NULL);
      if (Options & LIVE_CURSOR)
         (void) xv_set(CanvasFrame, FRAME_LEFT_FOOTER, "(no cursor)", NULL);
      first_call = FALSE;
   }

   /* Repaint the canvas now */

   repaint_canvas();

   /* Update label items on the panels, etc. if allowed */

   if (!(Cycling && (Options & NO_UPDATES)))
      UpdateLabels();

   /*
    * Need an XImage of the current pixmap for LIVE_CURSOR to work.
    *    Note that any existing global XImage should be deleted,
    *    otherwise memory use will grow and an uncomfortable rate...
    */

   if (Options & LIVE_CURSOR) {
      if (ximage_exists)
         XDestroyImage(TmpXImage);
      else
         ximage_exists = TRUE;
      TmpXImage = XGetImage(MainDisplay, image->pixmap, 0, 0,
         image->w, image->h, AllPlanes, ZPixmap);
   }

   /* Enable cycling if there was a command line request */

   if (StartCycling) {
      StartCycling = FALSE;
      Timer.it_value.tv_sec = 1; /* Wait 1 sec before starting */
      KeyPressed(CYCLE_KEY); /* Simulate key press */
      Timer.it_value.tv_sec = 0; /* Remove delay */
   }
}

void MakeBackdropImages()
{
   /* Makes server images of the available backdrops */

   int i;

   for (i = 0; i < NUM_BACKDROPS; i++)
      Backdrop[i] = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
         SERVER_IMAGE_X_BITS, backdrop_bits[i],
         XV_WIDTH, BACKDROP_WIDTH,
         XV_HEIGHT, BACKDROP_HEIGHT,
         NULL);
}

void PaintBackground()
{
   /*
    * Installs current backdrop on any exposed space behind current
    *    image. The background is painted in up to four rectangular
    *    regions, depending on the relation between the position of
    *    the top-left corner of the image (image_x,image_y), the
    *    image size, and the canvas frame size.
    */

   int cw, ch, iw, ih, x, y, w, h; /* Working variables */

   /* Get width and height of canvas frame */

   cw = (int) xv_get(CanvasFrame, XV_WIDTH) - ScrollMargin;
   ch = (int) xv_get(CanvasFrame, XV_HEIGHT) - ScrollMargin;

   /* If image hasn't been loaded yet, paint entire canvas and return */

   if (UNDEF_IMAGE) {
      XFillRectangle(MainDisplay, CanvasXID, BackdropGC, 0, 0, cw, ch);
      return;
   }

   /* Otherwise get image dimensions */

   iw = Image[Current]->w;
   ih = Image[Current]->h;

   /*
    * Return if image is larger than canvas, since image can only be
    *    centred or at top left of the frame.
    */

   if (iw >= cw && ih >= ch)
      return;

   /* Paint left- and/or right-most rectangles if frame is wider than image */

   if (iw < cw) {
      if (image_x > 0) {
         x = 0;
         y = 0;
         w = image_x;
         h = ch;
         XFillRectangle(MainDisplay, CanvasXID, BackdropGC, x, y, w, h);
      }
      if (image_x + iw < cw) {
         x = image_x + iw;
         y = 0;
         w = cw - x;
         h = ch;
         XFillRectangle(MainDisplay, CanvasXID, BackdropGC, x, y, w, h);
      }
   }

   /* Paint top- and/or bottom-most rectangles if frame is taller than image */

   if (ih < ch) {
      if (image_y > 0) {
         x = image_x;
         y = 0;
         w = iw;
         h = image_y;
         XFillRectangle(MainDisplay, CanvasXID, BackdropGC, x, y, w, h);
      }
      if (image_y + ih < ch) {
         x = image_x;
         y = image_y + ih;
         w = iw;
         h = ch - y;
         XFillRectangle(MainDisplay, CanvasXID, BackdropGC, x, y, w, h);
      }
   }
}

static Notify_value canvas_frame_quit(client, status)
Notify_client client;
Destroy_status status;
{
   /* Intercepts and ignores destroy requests unless NukeCanvas is set */

   if (status == DESTROY_CHECKING && !NukeCanvas) {
      notify_veto_destroy(client);
      return NOTIFY_DONE;
   }

   return notify_next_destroy_func(client, status);
}

static void canvas_event_proc(window, event, arg)
Xv_window window;
Event *event;
Notify_arg arg;
{
   /* Intercepts keys presses and mouse movements */

   if (event_is_ascii(event) && event_is_up(event))
      KeyPressed(event_action(event)); /* In mainpanel.c */
   else if (Options & LIVE_CURSOR) { /* Display color info */
      if (event_action(event) == LOC_MOVE || event_action(event) == LOC_DRAG) {
         static char msg[MAX_STR_LEN];
         IMAGE_T *image;
         int x, y, c;

         if (UNDEF_IMAGE)
             return;

         image = Image[Current];

         /* Get cursor position relative to image origin */

         x = event_x(event) - image_x;
         y = event_y(event) - image_y;

         /* Color is undefined if outside image */

         if (x < 0 || y < 0 || x >= image->w || y >= image->h)
            (void) sprintf(msg, "(%i,%i) = (undef)", x, y);
         else {
            Xv_singlecolor *orig_colors;

            /* Get colour, adjusting for reserved colors */

            c = XGetPixel(TmpXImage, x, y) - NumReservedColors;

            /* Get original colors */

            if (OneCmap)
               orig_colors = LockedColors;
            else
               orig_colors = image->colors;

            /* Show mapping from original colors to current colors */

            (void) sprintf(msg, "(%i,%i) = %i (%i,%i,%i) --> (%i,%i,%i)",
               x, y, c, orig_colors[c].red, orig_colors[c].green,
               orig_colors[c].blue, TmpColors[c].red, TmpColors[c].green,
               TmpColors[c].blue);
         }

         /* Display info in footer */

         (void) xv_set(CanvasFrame, FRAME_LEFT_FOOTER, msg, NULL);
      }
      else if (event_action(event) == LOC_WINEXIT && !UNDEF_IMAGE)
         (void) xv_set(CanvasFrame, FRAME_LEFT_FOOTER, "(no cursor)", NULL);
   }
}

static void repaint_canvas(/* Arguments ignored */)
{
   /* Repaints canvas with current background and image */

   if (UNDEF_IMAGE)
      return;

   /* Assume image origin coincides with frame origin */

   image_x = image_y = 0;

   /* Paint background if frame size could exceed image size */

   if (Sizing == FIXED) {
      if ((Options & CENTERING)) { /* Determine origin for centering case */
         image_x = (FixedWidth - Image[Current]->w) / 2;
         image_y = (FixedHeight - Image[Current]->h) / 2;
         image_x = MAX(0, image_x);
         image_y = MAX(0, image_y);
      }
      if (!(Cycling && (Options & NO_BACKDROPS)))
         PaintBackground();
   }

   /* Copy image pixmap to canvas drawable */

   (void) XCopyArea(MainDisplay, Image[Current]->pixmap, CanvasXID, MainGC,
      0, 0, Image[Current]->w, Image[Current]->h, image_x, image_y);
}

static void set_canvas_size_and_pos()
{
   /* Sets frame size and moves frame if part of it lies off-screen */

   int w = 0, h = 0; /* (initialized here to keep gcc happy) */

   /* Set fixed width and/or height to image size if forced */

   if (FixedWidth == FORCE_SIZE)
      FixedWidth = MIN(Image[Current]->w, MAX_IMAGE_WIDTH);
   if (FixedHeight == FORCE_SIZE)
      FixedHeight = MIN(Image[Current]->h, MAX_IMAGE_HEIGHT);

   /* Determine new nominal width and height */

   switch (Sizing) {
      case AUTO:
         if (UNDEF_IMAGE) {
            w = FixedWidth;
            h = FixedHeight;
         }
         else {
            w = MIN(FixedWidth, Image[Current]->w);
            h = MIN(FixedHeight, Image[Current]->h);
         }
         break;
      case FULL:
         if (UNDEF_IMAGE)
            return;
         w = MIN(Image[Current]->w, MAX_IMAGE_WIDTH);
         h = MIN(Image[Current]->h, MAX_IMAGE_HEIGHT);
         break;
      case FIXED:
         w = FixedWidth;
         h = FixedHeight;
         break;
      default:
         Error("Undefined sizing option.");
   }

   /* Set the canvas to at least include the whole image */

   if (!UNDEF_IMAGE)
      (void) xv_set(canvas,
         CANVAS_WIDTH, MAX(w, Image[Current]->w),
         CANVAS_HEIGHT, MAX(h, Image[Current]->h),
         NULL);

   /* Add scrollbar thickness and borders */

   w += ScrollMargin + 2 * INNER_CANVAS_BORDER;
   h += ScrollMargin + 2 * INNER_CANVAS_BORDER;

   /* Set new canvas window size */

   (void) xv_set(canvas,
      XV_WIDTH, w,
      XV_HEIGHT, h,
      NULL);

   /* Fit the frame to the canvas */

   window_fit(CanvasFrame);

   /* Reposition frame if part is off-screen, if allowed */

   if (!(Options & NO_MOVING) || ForceFirstResize) {
      static int first_call = TRUE;
      int x, y, x_offset, y_offset;

      /* Get current frame position and true width and height */

      x = (int) xv_get(CanvasFrame, XV_X);
      y = (int) xv_get(CanvasFrame, XV_Y);
      w += 2 * FRAME_BORDER;
      h += HEADER_MARGIN + 2 * FRAME_BORDER;

      /* Add the footer width if LIVE_CURSOR */

      if (Options & LIVE_CURSOR)
         h += FOOTER_MARGIN;

      /* 
       * After first positioning (MakeCanvas()), frame origins seem to
       *    be automatically offset by their border widths, so we need
       *    to add these quantities here if NOT first image. WHY???
       *
       */

      if (first_call) {
         x_offset = y_offset = 0;
         first_call = FALSE;
      }
      else {
         x_offset = FRAME_BORDER;
         y_offset = HEADER_MARGIN + FRAME_BORDER;
      }

      /* Set new position */

      if (x < 0)
         (void) xv_set(CanvasFrame, XV_X, x_offset, NULL);
      else if (x + w > ScreenWidth)
         (void) xv_set(CanvasFrame, XV_X, x_offset + ScreenWidth - w, NULL);

      if (y < 0)
         (void) xv_set(CanvasFrame, XV_Y, y_offset, NULL);
      else if (y + h > ScreenHeight)
         (void) xv_set(CanvasFrame, XV_Y, y_offset + ScreenHeight - h, NULL);

      ForceFirstResize = FALSE;
   }
}

/* canvas.c */
