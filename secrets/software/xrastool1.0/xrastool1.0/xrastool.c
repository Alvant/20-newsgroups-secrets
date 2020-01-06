/* xrastool.c -- main xrastool initialization and miscellaneous functions */

/* no xview/xlib error handling currently */

#include "xrastool.h"

/* Global variables */

Display *MainDisplay;
int MainScreen, ScreenWidth, ScreenHeight, ScreenDepth;
Visual *MainVisual;
GC MainGC;
IMAGE_T *Image[MAX_NUM_IMAGES];

/************************************************************/

int main(argc, argv)
int argc;
char *argv[];
{
   /* Main program initialization and window loop start. */

   int files_start; /* Location of first filename in argv, if any */

   /* Disable all output buffering (not essential) */

   setbuf(stdout, (char *) NULL);

   /* Friendly message */

   (void) printf("xrastool ver 1.0, Copyright (C) 1993 Derek C. Richardson\n");
   (void) printf("----------------\n");
   (void) printf("xrastool comes with ABSOLUTELY NO WARRANTY. This is free\n");
   (void) printf("software, and you are welcome to redistribute it under\n");
   (void) printf("the conditions of the GNU General Public License.\n");
   (void) printf("See the LICENSE included with the source for details.\n\n");

   /* Initialize XView and strip XView options */

   xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, NULL);

   /* Create simple frame now to get display parameters */

   BaseFrame = (Frame) xv_create(XV_NULL, FRAME, NULL);

   /* Get the default display, screen, size, depth, visual, and GC */

   MainDisplay  = (Display *) xv_get(BaseFrame, XV_DISPLAY);
   MainScreen   = DefaultScreen(MainDisplay);
   ScreenWidth  = WidthOfScreen(ScreenOfDisplay(MainDisplay, MainScreen));
   ScreenHeight = HeightOfScreen(ScreenOfDisplay(MainDisplay, MainScreen));
   ScreenDepth  = DefaultDepth(MainDisplay, MainScreen);
   MainVisual   = DefaultVisual(MainDisplay, MainScreen);
   MainGC       = DefaultGC(MainDisplay, MainScreen);

   /* Error check */

   if (ScreenDepth != 8)
      Error("Need 8 bit display for viewing -- sorry!");

   /* Interpret any remaining command line options and get files_start */

   ReadCmdLine(argc, argv, &files_start);

   (void) printf("Default display %ix%i with %i bit plane%s.\n",
      ScreenWidth, ScreenHeight, ScreenDepth, PLURAL(ScreenDepth));

   /* Construct windows and icons */

   MakeBaseFrame();
   MakeMainPanel();
   MakeSubWindow();
   MakeCanvas();

   /*
    * Construct global colormap segment and assign it to windows
    *    (performed AFTER window creation so tvtwm will behave properly).
    */

   MakeCms();

   /* Apply defaults and any command line options */

   Initialize();

   /* Load any specified images and display the first one */

   if (files_start < argc) {
      (void) printf("Loading initial frames...\n");
      (void) LoadImages(argc - files_start, argv + files_start, 0);
      (void) printf("Done!\n");
      if (Current < 0 || Current >= NumImages) { /* if -start n out of range */
         Current = 0;
         if (!NO_IMAGES)
            (void) printf("Starting at first image.\n");
      }
      ShowNewImage();
   }

   /* Move the cursor onto main panel and position main panel caret */

   MoveCursorAndCaret();

   /* Enter main XView loop */

   xv_main_loop(BaseFrame);

   /* Termination */

   (void) printf("xrastool done.\n");

   /* Successful completion */

   return OK;
}

void PointToItem(frame, item_rect)
Frame frame;
Rect *item_rect;
{
   /* Moves cursor to center of item_rect relative to frame */

   Rect *frame_rect;
   int x, y;

   /* Get position and size of frame */

   frame_rect = (Rect *) xv_get(frame, XV_RECT);

   /* Get new cursor position, allowing for frame borders and margins */

   x = frame_rect->r_left + item_rect->r_left + item_rect->r_width / 2 +
      FRAME_BORDER;

   y = frame_rect->r_top + item_rect->r_top + item_rect->r_height / 2 +
      FRAME_BORDER + HEADER_MARGIN;

   /* Move cursor */

   (void) xv_set(xv_get(frame, XV_ROOT), WIN_MOUSE_XY, x, y, NULL);
}

void Warning(msg)
char *msg;
{
   /* Sends msg to stderr */

   (void) fprintf(stderr, "xrastool warning: %s\n", msg);
}

void Error(msg)
char *msg;
{
   /* Sends msg to stderr and aborts with error status 1 */

   (void) fprintf(stderr, "\007xrastool error: %s\n", msg);

   exit(1);
}

/* xrastool.c */
