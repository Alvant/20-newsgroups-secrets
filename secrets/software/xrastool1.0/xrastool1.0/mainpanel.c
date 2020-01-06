/* mainpanel.c -- xrastool main panel creation and event procedures */

#include "xrastool.h"
#include <xview/notice.h>

#define MAIN_PANEL_WIDTH 400 /* Looks nice... */

/* Global variables */

Panel MainPanel;

/* Local variables, mostly panel item handles */

static Panel_item image_number, image_name, width_item, height_item,
   sizing_item, set_item, timer_scale_item, timer_slider, dir_switch,
   starting_item;
static Server_image fwd_switch_image, rev_switch_image;
static Xv_notice notice;

/* Bitmaps */

#include "bitmaps/fwd.xbm"
#include "bitmaps/rev.xbm"
#include "bitmaps/cycle.xbm"
#include "bitmaps/loopback.xbm"
#include "bitmaps/oneway.xbm"

/* Local functions, mostly panel event procedures */

static void
   new_image(),
   load_image(),
   set_logical(),
   set_sizing(),
   set_size(),
   change_timer_scale(),
   change_timer_value(),
   set_fast_mode(),
   toggle_dir(),
   step_image(),
   blink_images(),
   cycle_images(),
   set_cycle_option(),
   refresh(),
   show_sub_frame(),
   info(),
   quit(),
   draw_switch(),
   set_timer();

static Panel_setting set_current(), load_image_from_text(), set_width(),
   set_height();

/************************************************************/

void MakeMainPanel()
{
   /* Creates main panel and copyright notice */

   Rect *rect;
   Panel_item dum_item;
   Server_image cycle_image, loop_back_image, one_way_image;

   MainPanel = xv_create(BaseFrame, PANEL,
      XV_WIDTH, MAIN_PANEL_WIDTH,
      NULL);

   /* Create LOTS of items...note that some positions have been "hard-wired" */

   image_number = xv_create(MainPanel, PANEL_NUMERIC_TEXT,
      PANEL_LABEL_STRING, "Image #",
      PANEL_VALUE_DISPLAY_LENGTH, 4,
      PANEL_MIN_VALUE, 0,
      PANEL_MAX_VALUE, MAX_NUM_IMAGES + 1,
      PANEL_VALUE, Current + 1,
      PANEL_NOTIFY_PROC, set_current,
      NULL);

   (void) xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "New",
      PANEL_NOTIFY_PROC, new_image,
      NULL);

   (void) xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "Load",
      PANEL_NOTIFY_PROC, load_image,
      NULL);

   (void) xv_create(MainPanel, PANEL_CHOICE,
      PANEL_CHOICE_STRINGS, "+", "|", "&", "x", "e", "-", NULL,
      PANEL_VALUE, Logical,
      PANEL_NOTIFY_PROC, set_logical,
      NULL);

   image_name = xv_create(MainPanel, PANEL_TEXT,
      PANEL_NEXT_ROW, -1,
      PANEL_LABEL_STRING, "Image:",
      PANEL_VALUE_DISPLAY_LENGTH, 42,
      PANEL_VALUE_STORED_LENGTH, MAX_STR_LEN,
      PANEL_NOTIFY_PROC, load_image_from_text,
      NULL);

   width_item = xv_create(MainPanel, PANEL_NUMERIC_TEXT,
      PANEL_NEXT_ROW, -1,
      PANEL_LABEL_STRING, "W:",
      PANEL_VALUE_DISPLAY_LENGTH, 4,
      PANEL_MIN_VALUE, MIN_IMAGE_WIDTH,
      PANEL_MAX_VALUE, MAX_IMAGE_WIDTH,
      PANEL_VALUE, FixedWidth,
      PANEL_NOTIFY_PROC, set_width,
      NULL);

   height_item = xv_create(MainPanel, PANEL_NUMERIC_TEXT,
      PANEL_LABEL_STRING, "H:",
      PANEL_VALUE_DISPLAY_LENGTH, 4,
      PANEL_MIN_VALUE, MIN_IMAGE_HEIGHT,
      PANEL_MAX_VALUE, MAX_IMAGE_HEIGHT,
      PANEL_VALUE, FixedHeight,
      PANEL_NOTIFY_PROC, set_height,
      NULL);

   rect = (Rect *) xv_get(height_item, XV_RECT);

   sizing_item = xv_create(MainPanel, PANEL_CHOICE,
      PANEL_CHOICE_STRINGS, "Auto", "Full", "Fixed", NULL,
      XV_Y, rect->r_top - 3,
      PANEL_VALUE, Sizing,
      PANEL_NOTIFY_PROC, set_sizing,
      NULL);

   set_item = xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, " Set ",
      PANEL_NOTIFY_PROC, set_size,
      NULL);

   timer_scale_item = (Panel_item) xv_create(MainPanel, PANEL_CHOICE,
      PANEL_NEXT_ROW, -1,
      PANEL_CHOICE_STRINGS, "us", "ms", "sec", NULL,
      PANEL_VALUE, TimerScale,
      PANEL_NOTIFY_PROC, change_timer_scale,
      NULL);

   timer_slider = (Panel_item) xv_create(MainPanel, PANEL_SLIDER,
      PANEL_TICKS, 5,
      PANEL_SLIDER_WIDTH, 129,
      PANEL_MIN_VALUE, MIN_TIMER_VALUE,
      PANEL_MAX_VALUE, MAX_TIMER_VALUE,
      PANEL_VALUE, TimerValue,
      PANEL_NOTIFY_PROC, change_timer_value,
      NULL);

   (void) xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "Fast",
      PANEL_NOTIFY_PROC, set_fast_mode,
      NULL);

   fwd_switch_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
      SERVER_IMAGE_X_BITS, fwd_bits,
      XV_WIDTH, fwd_width,
      XV_HEIGHT, fwd_height,
      NULL);

   rev_switch_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
      SERVER_IMAGE_X_BITS, rev_bits,
      XV_WIDTH, rev_width,
      XV_HEIGHT, rev_height,
      NULL);

   dir_switch = xv_create(MainPanel, PANEL_MESSAGE,
      PANEL_NEXT_ROW, -1,
      PANEL_NOTIFY_PROC, toggle_dir,
      NULL);

   draw_switch();
   
   rect = (Rect *) xv_get(dir_switch, XV_RECT);

   (void) xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "Step",
      XV_X, rect_right(rect) + 19,
      PANEL_NOTIFY_PROC, step_image,
      NULL);

   (void) xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "Blink",
      PANEL_NOTIFY_PROC, blink_images,
      NULL);

   dum_item = xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "Cycle",
      PANEL_NOTIFY_PROC, cycle_images,
      NULL);

   rect = (Rect *) xv_get(dum_item, XV_RECT);

   cycle_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
      SERVER_IMAGE_X_BITS, cycle_bits,
      XV_WIDTH, cycle_width,
      XV_HEIGHT, cycle_height,
      NULL);

   loop_back_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
      SERVER_IMAGE_X_BITS, loop_back_bits,
      XV_WIDTH, loop_back_width,
      XV_HEIGHT, loop_back_height,
      NULL);

   one_way_image = (Server_image) xv_create(XV_NULL, SERVER_IMAGE,
      SERVER_IMAGE_X_BITS, one_way_bits,
      XV_WIDTH, one_way_width,
      XV_HEIGHT, one_way_height,
      NULL);

   (void) xv_create(MainPanel, PANEL_CHOICE,
      PANEL_CHOICE_IMAGES, cycle_image, loop_back_image,
         one_way_image, NULL,
      XV_X, rect_right(rect) + 19,
      XV_Y, rect->r_top - 3,
      PANEL_VALUE, CycleOption,
      PANEL_NOTIFY_PROC, set_cycle_option,
      NULL);

   (void) xv_create(MainPanel, PANEL_BUTTON,
      PANEL_NEXT_ROW, -1,
      PANEL_LABEL_STRING, "Refresh",
      PANEL_NOTIFY_PROC, refresh,
      NULL);

   starting_item = xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "More...",
      PANEL_NOTIFY_PROC, show_sub_frame,
      NULL);

   (void) xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_WIDTH, 179,
      PANEL_LABEL_STRING, "                  Info...",
      PANEL_NOTIFY_PROC, info,
      NULL);

   (void) xv_create(MainPanel, PANEL_BUTTON,
      PANEL_LABEL_STRING, "Quit",
      PANEL_NOTIFY_PROC, quit,
      NULL);

   window_fit_height(MainPanel);
   window_fit(BaseFrame);

   /* Info and copyright notice */

   notice = xv_create(MainPanel, NOTICE,
      NOTICE_MESSAGE_STRINGS,
         "xrastool version 1.0",
         "",
         "Copyright (C) 1993 Derek C. Richardson",
         "",
         "This software may be freely distributed",
         "under the terms of the GNU General Public License",
         "",
         "Comments welcome!",
         "",
         "snail-mail:",
         "Institute of Astronomy",
         "Cambridge, U.K. CB3 0HA",
         "",
         "e-mail: dcr@mail.ast.cam.ac.uk",
         NULL,
      NOTICE_BUTTON, "Done", 0,
      NOTICE_NO_BEEPING, TRUE,
      NULL);
}

void KeyPressed(c)
char c;
{
   /* Interface for keyboard acceleration called by canvas_event_proc() */

   switch (c) {
      case SET_KEY:
         set_size((Panel_item) NULL, (Event *) NULL);
         break;
      case STEP_KEY:
         step_image((Panel_item) NULL, (Event *) NULL);
         break;
      case CYCLE_KEY:
         cycle_images((Panel_item) NULL, (Event *) NULL);
         break;
      case QUIT_KEY:
         quit((Panel_item) NULL, (Event *) NULL);
         break;
      default:
         /* Ignored */;
   }
}

void UpdateLabels()
{
   /* Updates image number, filename, width & height items, and main footer */

   IMAGE_T *image;
   char msg[MAX_STR_LEN];
   int num_colors;

   image = Image[Current];

   (void) xv_set(image_number, PANEL_VALUE, Current + 1, NULL);

   (void) xv_set(image_name, PANEL_VALUE, image->filename, NULL);

   if (!(Options & NO_RESIZING)) {
      (void) xv_set(width_item, PANEL_VALUE, (int) xv_get(CanvasFrame,
         XV_WIDTH) - ScrollMargin - 2 * INNER_CANVAS_BORDER, NULL);
      (void) xv_set(height_item, PANEL_VALUE, (int) xv_get(CanvasFrame,
         XV_HEIGHT) - ScrollMargin - 2 * INNER_CANVAS_BORDER, NULL);
   }

   (void) sprintf(msg, "%i image%s loaded", NumImages, PLURAL(NumImages));

   (void) xv_set(BaseFrame, FRAME_LEFT_FOOTER, msg, NULL);

   num_colors = (OneCmap ? NumLockedColors : image->num_colors);

   (void) sprintf(msg, "%i color%s shown", num_colors, PLURAL(num_colors));

   (void) xv_set(BaseFrame, FRAME_RIGHT_FOOTER, msg, NULL);
}

void SetFixedSizeItem()
{
   /* Set sizing button to Fixed */

   Sizing = FIXED;

   (void) xv_set(sizing_item, PANEL_VALUE, Sizing, NULL);
}

void TimerOn()
{
   /* Activates automatic calls of step_image() for blinking/cycling */

   notify_set_itimer_func(BaseFrame, (Notify_func) step_image, ITIMER_REAL,
      &Timer, NULL);
}

void TimerOff()
{
   /* Deactivates timed calls */

   notify_set_itimer_func(BaseFrame, NOTIFY_FUNC_NULL, ITIMER_REAL, NULL,
      NULL);
}

void MoveCursorAndCaret()
{
   /* Moves cursor onto main panel and positions main panel input caret */

   PointToItem(BaseFrame, (Rect *) xv_get(starting_item, XV_RECT));

   if (NO_IMAGES)
      (void) xv_set(MainPanel, PANEL_CARET_ITEM, image_name, NULL);
   else
      (void) xv_set(MainPanel, PANEL_CARET_ITEM, image_number, NULL);
}

void ToggleSizingExposure(flag)
Bool flag;
{
   (void) xv_set(width_item, PANEL_INACTIVE, flag, NULL);
   (void) xv_set(height_item, PANEL_INACTIVE, flag, NULL);
   (void) xv_set(sizing_item, PANEL_INACTIVE, flag, NULL);
   (void) xv_set(set_item, PANEL_INACTIVE, flag, NULL);
}

static Panel_setting set_current(item, event)
Panel_item item;
Event *event;
{
   /* Reads image number from panel and displays corresponding image */

   int value;

   value = (int) xv_get(image_number, PANEL_VALUE);

   /* Handle out of range values */

   if (value > NumImages) {
      value = Current + 1;
      (void) xv_set(image_number, PANEL_VALUE, value, NULL);
   }
   else if (value < 1) {
      value = 1;
      (void) xv_set(image_number, PANEL_VALUE, 1, NULL);
   }

   /* Internally images start at 0... */

   Current = value - 1;

   /* Show image */

   if (!UNDEF_IMAGE)
      ShowNewImage();

   /* Do not advance caret */

   return PANEL_NONE;
}

static void new_image(item, event)
Panel_item item;
Event *event;
{
   /* Prompts user for filename of new image */

   Current = NumImages;

   (void) xv_set(image_number, PANEL_VALUE, NumImages + 1, NULL);

   (void) xv_set(image_name, PANEL_VALUE, "", NULL);

   (void) xv_set(BaseFrame, FRAME_RIGHT_FOOTER, "Waiting for new image",
      NULL);

   if (Options & LIVE_CURSOR)
      (void) xv_set(CanvasFrame, FRAME_LEFT_FOOTER, "(no image)",
         NULL);

   (void) xv_set(MainPanel, PANEL_CARET_ITEM, image_name, NULL);

   /* Erase any existing display */

   PaintBackground();
}

static void load_image(item, event)
Panel_item item;
Event *event;
{
   /* Reads filename from panel, loads image, and displays it */

   char *filename[1];

   *filename = (char *) xv_get(image_name, PANEL_VALUE);

   if (strlen(*filename) == 0)
      return;

   if (LoadImages(1, filename, Current) != 1)
      (void) xv_set(image_name, PANEL_VALUE, "", NULL);
   else
      ShowNewImage();
}

static void set_logical(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Sets logical function */

   Logical = value;

   SetLogical();
}

static Panel_setting load_image_from_text(item, event)
Panel_item item;
Event *event;
{
   /* Calls load_image() */

   load_image((Panel_item) NULL, (Event *) NULL);

   /* Do not advance caret */

   return PANEL_NONE;
}

static Panel_setting set_width(item, event)
Panel_item item;
Event *event;
{
   /* Changes FixedWidth and forces fixed sizing */

   FixedWidth = (int) xv_get(width_item, PANEL_VALUE);

   if (Sizing != FIXED)
      SetFixedSizeItem();

   ShowNewImage();

   /* Advance caret to height item */

   return PANEL_NEXT;
}

static Panel_setting set_height(item, event)
Panel_item item;
Event *event;
{
   /* Changes FixedHeight and forces fixed sizing */

   FixedHeight = (int) xv_get(height_item, PANEL_VALUE);

   if (Sizing != FIXED)
      SetFixedSizeItem();

   ShowNewImage();

   /* Move caret to width item */

   return PANEL_PREVIOUS;
}

static void set_sizing(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Changes sizing value */

   Sizing = value;

   if (Sizing == FULL) {
      if (UNDEF_IMAGE)
         return;
      (void) xv_set(width_item, PANEL_VALUE, Image[Current]->w, NULL);
      (void) xv_set(height_item, PANEL_VALUE, Image[Current]->h, NULL);
   }
   else {
      FixedWidth = (int) xv_get(width_item, PANEL_VALUE);
      FixedHeight = (int) xv_get(height_item, PANEL_VALUE);
   }

   ShowNewImage();
}

static void set_size(item, event)
Panel_item item;
Event *event;
{
   /* Fixes size according to current canvas window dimensions */

   FixedWidth = (int) xv_get(CanvasFrame, XV_WIDTH) - ScrollMargin -
      2 * INNER_CANVAS_BORDER;

   FixedHeight = (int) xv_get(CanvasFrame, XV_HEIGHT) - ScrollMargin -
      2 * INNER_CANVAS_BORDER;

   (void) xv_set(width_item, PANEL_VALUE, FixedWidth, NULL);
   (void) xv_set(height_item, PANEL_VALUE, FixedHeight, NULL);

   SetFixedSizeItem();

   ShowNewImage();
}

static void change_timer_scale(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Changes timer scale */

   TimerScale = value;

   SetTimer();
}

static void change_timer_value(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Changes timer value */

   TimerValue = value;

   SetTimer();
}

static void set_fast_mode()
{
   /* Activates fast mode */

   SetOptions(Options ^ FAST_MODE);

   UpdateOptionsItem();

   TimerScale = TIMER_US;
   TimerValue = 1;

   (void) xv_set(timer_scale_item, PANEL_VALUE, TimerScale, NULL);

   (void) xv_set(timer_slider, PANEL_VALUE, TimerValue, NULL);

   SetTimer();
}

static void toggle_dir()
{
   /* Changes current direction flag */

   Direction = (Direction == FWD ? REV : FWD);

   if (!(Blinking && (Options & NO_UPDATES)))
      draw_switch();
}

static void step_image(item, event)
Panel_item item;
Event *event;
{
   /* Displays next or prev item depending on direction and cycling option */

   if (NumImages == 0)
      return;

   Current += (Direction == FWD ? 1 : -1);

   if (Blinking)
      toggle_dir();

   if (Blinking || !Cycling) {
      if (Current < 0)
         Current = NumImages - 1;
      else if (Current >= NumImages)
         Current = 0;
   }
   else
      switch (CycleOption) {
         case LOOP:
            if (Current < 0)
               Current = NumImages - 1;
            else if (Current >= NumImages)
               Current = 0;
            break;
         case LOOP_BACK:
            if (Current < 0) {
               Current = 1;
               toggle_dir();
            }
            else if (Current >= NumImages) {
               Current = NumImages - 2;
               toggle_dir();
            }
            break;
         case ONE_WAY:
            if (Current < 0) {
               Current = 0;
               cycle_images((Panel_item) NULL,
                  (Event *) NULL);
               return;
            }
            else if (Current >= NumImages) {
               Current = NumImages - 1;
               cycle_images((Panel_item) NULL,
                  (Event *) NULL);
               return;
            }
            break;
      }

   ShowNewImage();
}

static void blink_images(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Toggles blinking */

   Blinking = (Bool) value;

   if (NumImages > 1) {
      if (!Blinking) {
         Blinking = TRUE;
         if (Cycling)
            return;
      }
      cycle_images((Panel_item) NULL, (Event *) NULL);
   }
}

static void cycle_images(item, event)
Panel_item item;
Event *event;
{
   /* Toggles cycling */

   if (NumImages > 1) {
      if (!Cycling && CycleOption == ONE_WAY) {
         if (Current == NumImages - 1 && Direction == FWD) {
            Current = 0;
            ShowNewImage();
         }
         else if (Current == 0 && Direction == REV) {
            Current = NumImages - 1;
            ShowNewImage();
         }
      }
      set_timer(Cycling = !Cycling);
      if (Blinking && !Cycling)
         Blinking = FALSE;
   }
}

static void set_cycle_option(item, value, event)
Panel_item item;
int value;
Event *event;
{
   CycleOption = value;
}

static void refresh(item, event)
Panel_item item;
Event *event;
{
   ShowNewImage();
}

static void show_sub_frame(item, event)
Panel_item item;
Event *event;
{
   (void) xv_set(SubFrame, XV_SHOW, TRUE, NULL);
}

static void info(item, event)
Panel_item item;
Event *event;
{
   (void) xv_set(notice, XV_SHOW, TRUE, NULL);
}

static void quit(item, event)
Panel_item item;
Event *event;
{
   (void) xv_destroy_safe(BaseFrame);
}

static void draw_switch()
{
   (void) xv_set(dir_switch, PANEL_LABEL_IMAGE,
      (Direction == FWD ? fwd_switch_image : rev_switch_image),
      NULL);
}

static void set_timer(sw)
int sw;
{
   /* Activates or deactivates cycling mode */

   static char frame_header[MAX_STR_LEN];

   if (sw) {
      (void) strcpy(frame_header,
         (char *) xv_get(CanvasFrame, FRAME_LABEL));
      (void) xv_set(CanvasFrame, FRAME_LABEL, "CYCLING", NULL);
      TimerOn();
   }
   else {
      TimerOff();
      (void) xv_set(CanvasFrame, FRAME_LABEL, frame_header, NULL);
      if (Blinking && (Options & (NO_UPDATES | NO_BACKDROPS)))
         draw_switch();
   }
}

/* mainpanel.c */
