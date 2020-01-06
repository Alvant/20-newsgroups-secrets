/* subpanel.c -- xrastool sub panel creation and event procedures */

#include "xrastool.h"

#define MAX_COLOR_INTENSITY (MAX_NUM_COLORS - 1)

#define NUM_CONTOURS 10

/* Global variables */

Frame SubFrame;
Panel SubPanel;

/* Local variables */

static Panel_item options_item, backdrops_item, scaling_item, red_slider,
   green_slider, blue_slider, red_slider_scale, green_slider_scale,
   blue_slider_scale;
static int num_colors;
static Xv_singlecolor *colors;

/* Contour colors, in order of increasing brightness */

static Xv_singlecolor contour_colors[NUM_CONTOURS] = {
   {  0,   0,   0}, /* black */
   {  0,   0, 255}, /* blue */
   {139,   0, 139}, /* magenta */
   { 93,  71, 139}, /* medium purple */
   {  0, 154, 205}, /* deep sky blue */
   { 50, 205,  50}, /* lime green */
   {238, 154,  73}, /* medium aquamarine */
   {205, 201, 165}, /* lemon */
   {255, 255,   0}, /* yellow */
   {255, 255, 255}  /* white */
};

/* Local functions */

static void
   set_option(),
   set_backdrop(),
   set_scaling(),
   change_color_slider(),
   change_color_slider_scale(),
   toggle_slider_scales_exposure(),
   set_color_sliders(),
   draw_bw_scale(),
   draw_gray_scale(),
   draw_color_scale(),
   draw_cutoff_scale(),
   draw_contour_scale(),
   draw_random_scale();

/************************************************************/

void MakeSubWindow()
{
   /* Creates sub panel */

   Attr_avlist attr_list;

   SubFrame = xv_create(BaseFrame, FRAME_CMD,
      FRAME_CMD_DEFAULT_PIN_STATE, FRAME_CMD_PIN_IN,
      XV_X, DFLT_SUB_FRAME_XV_X,
      XV_Y, DFLT_SUB_FRAME_XV_Y,
      FRAME_LABEL, "xrastool: Sub Frame",
      XV_SHOW, ShowSubPanel,
      NULL);

   SubPanel = (Panel) xv_get(SubFrame, FRAME_CMD_PANEL);

   options_item = (Panel_item) xv_create(SubPanel, PANEL_TOGGLE,
      PANEL_CHOICE_NCOLS, 3,
      PANEL_CHOICE_STRINGS, "No scrolling", "No resizing",
         "No moving", "No scaling", "No updates",
         "No backdrops", "Lock colors", "Centering",
         "Live cursor", NULL,
      PANEL_VALUE, Options,
      PANEL_NOTIFY_PROC, set_option,
      NULL);

   MakeBackdropImages();

   backdrops_item = xv_create(SubPanel, PANEL_CHOICE,
      PANEL_NEXT_ROW, -1,
      PANEL_LABEL_STRING, "Backdrops:",
      PANEL_CHOICE_IMAGES, Backdrop[0], Backdrop[1], Backdrop[2], Backdrop[3],
         Backdrop[4], Backdrop[5], Backdrop[6], Backdrop[7], Backdrop[8], NULL,
      PANEL_VALUE, BackdropIndex,
      PANEL_NOTIFY_PROC, set_backdrop,
      NULL);

   scaling_item = xv_create(SubPanel, PANEL_CHOICE,
      PANEL_NEXT_ROW, -1,
      PANEL_LABEL_STRING, "Scaling:",
      PANEL_CHOICE_NCOLS, 3,
      PANEL_CHOICE_STRINGS, "BW", "Gray", "Color", "Cutoff", "Contour",
         "Random", NULL,
      PANEL_VALUE, Scaling,
      PANEL_NOTIFY_PROC, set_scaling,
      NULL);

   attr_list = attr_create_list(
      PANEL_NEXT_ROW, -1,
      PANEL_TICKS, 5,
      PANEL_MIN_VALUE, 0,
      PANEL_MAX_VALUE, 100,
      PANEL_SLIDER_WIDTH, 149, /* To line up with main options buttons */
      PANEL_NOTIFY_PROC, change_color_slider,
      PANEL_NOTIFY_LEVEL, PANEL_ALL,
      NULL);

   /* NOTE: slider values are set during initialization (Cf. Initialize()) */

   red_slider = xv_create(SubPanel, PANEL_SLIDER,
      PANEL_LABEL_STRING, "R:",
      ATTR_LIST, attr_list,
      NULL);

   red_slider_scale = xv_create(SubPanel, PANEL_CHOICE,
      PANEL_CHOICE_STRINGS, "x", "/", NULL,
      PANEL_VALUE, RedSliderScale,
      PANEL_NOTIFY_PROC, change_color_slider_scale,
      NULL);

   green_slider = xv_create(SubPanel, PANEL_SLIDER,
      PANEL_LABEL_STRING, "G:",
      ATTR_LIST, attr_list,
      NULL);

   green_slider_scale = xv_create(SubPanel, PANEL_CHOICE,
      PANEL_CHOICE_STRINGS, "x", "/", NULL,
      PANEL_VALUE, GreenSliderScale,
      PANEL_NOTIFY_PROC, change_color_slider_scale,
      NULL);

   blue_slider = xv_create(SubPanel, PANEL_SLIDER,
      PANEL_LABEL_STRING, "B:",
      ATTR_LIST, attr_list,
      NULL);

   blue_slider_scale = xv_create(SubPanel, PANEL_CHOICE,
      PANEL_CHOICE_STRINGS, "x", "/", NULL,
      PANEL_VALUE, BlueSliderScale,
      PANEL_NOTIFY_PROC, change_color_slider_scale,
      NULL);

   window_fit(SubPanel);
   window_fit(SubFrame);
}

void UpdateOptionsItem()
{
   (void) xv_set(options_item, PANEL_VALUE, Options, NULL);
}

void ToggleScalingExposure(flag)
Bool flag;
{
   (void) xv_set(scaling_item, PANEL_INACTIVE, flag, NULL);
   (void) xv_set(red_slider, PANEL_INACTIVE, flag, NULL);
   (void) xv_set(green_slider, PANEL_INACTIVE, flag, NULL);
   (void) xv_set(blue_slider, PANEL_INACTIVE, flag, NULL);

   if (Scaling == COLOR)
      toggle_slider_scales_exposure(flag);
}

void ToggleBackdropsExposure(flag)
Bool flag;
{
   (void) xv_set(backdrops_item, PANEL_INACTIVE, flag, NULL);
}

void ApplyColorScaling()
{
   /* Applies color scaling to current or locked colors */

   if (Options & LOCK_COLORS) {
      num_colors = NumLockedColors;
      colors = LockedColors;
   }
   else {
      num_colors = Image[Current]->num_colors;
      colors = Image[Current]->colors;
   }

   switch (Scaling) {
      case BW:
         draw_bw_scale();
         break;
      case GRAY:
         draw_gray_scale();
         break;
      case COLOR:
         draw_color_scale();
         break;
      case CUTOFF:
         draw_cutoff_scale();
         break;
      case CONTOUR:
         draw_contour_scale();
         break;
      case RANDOM:
         draw_random_scale();
   }

   InstallCms(num_colors, TmpColors);
}

void SetColorSliders()
{
   toggle_slider_scales_exposure(Scaling != COLOR);

   set_color_sliders((Panel_item) NULL, (Event *) NULL);
}

static void set_option(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Sets option, overriding unlock colors request if unique colormap */

   if (((Options ^ value) & LOCK_COLORS) && OneCmap)
      UpdateOptionsItem();
   else
      SetOptions(Options ^ value);
}

static void set_backdrop(item, value, event)
Panel_item item;
int value;
Event *event;
{
   BackdropIndex = value;

   SetBackdrop();
}

static void set_scaling(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Sets colors scaling option */

   Scaling = value;

   SetColorSliderDefaults();

   SetScaling();
}

static void change_color_slider(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Changes color sliders depending on current scaling option */

   int n, d;

   switch (Scaling) {
      case BW:
         RedSliderValue = GreenSliderValue = BlueSliderValue = value;
         set_color_sliders(item, event);
         break;
      case GRAY:
         n = 100 - value;
         if (item == red_slider) {
            d = GreenSliderValue + BlueSliderValue;
            RedSliderValue = value;
            if (d > 0) {
               GreenSliderValue = GreenSliderValue * n / d;
               BlueSliderValue = BlueSliderValue * n / d;
            }
         }
         else if (item == green_slider) {
            d = RedSliderValue + BlueSliderValue;
            GreenSliderValue = value;
            if (d > 0) {
               RedSliderValue = RedSliderValue * n / d;
               BlueSliderValue = BlueSliderValue * n / d;
            }
         }
         else {
            d = RedSliderValue + GreenSliderValue;
            BlueSliderValue = value;
            if (d > 0) {
               RedSliderValue = RedSliderValue * n / d;
               GreenSliderValue = GreenSliderValue * n / d;
            }
         }
         set_color_sliders(item, event);
         break;
      case COLOR:
         if (item == red_slider)
            RedSliderValue = value;
         else if (item == green_slider)
            GreenSliderValue = value;
         else
            BlueSliderValue = value;
         break;
      case CUTOFF:
      case CONTOUR:
      case RANDOM:
         RedSliderValue = GreenSliderValue = BlueSliderValue = value;
         set_color_sliders(item, event);
         break;
      default:
         Error("Invalid color scaling option.");
   }

   if (!UNDEF_IMAGE)
       ApplyColorScaling(); /* Colors are probably already loaded...*/
}

static void change_color_slider_scale(item, value, event)
Panel_item item;
int value;
Event *event;
{
   /* Changes color slider scale (currently only affects COLOR scaling) */

   if (item == red_slider_scale)
      RedSliderScale = value;
   else if (item == green_slider_scale)
      GreenSliderScale = value;
   else
      BlueSliderScale = value;

   if (Scaling == COLOR)
      draw_color_scale();
}

static void toggle_slider_scales_exposure(flag)
int flag;
{
   (void) xv_set(red_slider_scale, PANEL_INACTIVE, flag, NULL);
   (void) xv_set(green_slider_scale, PANEL_INACTIVE, flag, NULL);
   (void) xv_set(blue_slider_scale, PANEL_INACTIVE, flag, NULL);
}

static void set_color_sliders(item, event)
Panel_item item;
Event *event;
{
   /* Loads current values into color sliders */

   if (LIVE_SLIDERS || !event || event_is_ascii(event) ||
         (event_is_button(event) && !event_is_down(event))) {
      if (item != red_slider)
         (void) xv_set(red_slider,
            PANEL_VALUE, RedSliderValue,
            NULL);
      if (item != green_slider)
         (void) xv_set(green_slider,
            PANEL_VALUE, GreenSliderValue,
            NULL);
      if (item != blue_slider)
         (void) xv_set(blue_slider,
            PANEL_VALUE, BlueSliderValue,
            NULL);
   }
}

static void draw_bw_scale()
{
   int i;
   unsigned long value;

   value = RedSliderValue * MAX_COLOR_INTENSITY / 100;
   value = 3 * SQ(value);

   for (i = 0; i < num_colors; i++) {
      if (SQ(colors[i].red) + SQ(colors[i].green) + SQ(colors[i].blue) > value)
         TmpColors[i].red = TmpColors[i].green = TmpColors[i].blue =
            MAX_COLOR_INTENSITY;
      else
         TmpColors[i].red = TmpColors[i].green = TmpColors[i].blue = 0;
   }
}

static void draw_gray_scale()
{
   int i;
   unsigned long value;

   for (i = 0; i < num_colors; i++) {
      value =
         RedSliderValue * colors[i].red +
         GreenSliderValue * colors[i].green +
         BlueSliderValue * colors[i].blue;
      TmpColors[i].red = TmpColors[i].green = TmpColors[i].blue = value / 100;
   }
}

#define SCALE(index, slider, div)\
   (div ?\
      (slider ?\
         (index) * 100 / (slider) :\
         MAX_COLOR_INTENSITY) :\
      (index) * (slider) / 100)

static void draw_color_scale()
{
   int i;
   unsigned long red, green, blue;

   for (i = 0; i < num_colors; i++) {
      red = SCALE(colors[i].red, RedSliderValue, RedSliderScale);
      green = SCALE(colors[i].green, GreenSliderValue, GreenSliderScale);
      blue = SCALE(colors[i].blue, BlueSliderValue, BlueSliderScale);
      TmpColors[i].red = MIN(red, MAX_COLOR_INTENSITY);
      TmpColors[i].green = MIN(green, MAX_COLOR_INTENSITY);
      TmpColors[i].blue = MIN(blue, MAX_COLOR_INTENSITY);
   }
}

#undef SCALE

static void draw_cutoff_scale()
{
   int i, cutoff;

   cutoff = num_colors * RedSliderValue / 100;

   for (i = 0; i < num_colors; i++)
      if (i < cutoff) {
         TmpColors[i].red = colors[i].red;
         TmpColors[i].green = colors[i].green;
         TmpColors[i].blue = colors[i].blue;
      }
      else
         TmpColors[i].red = TmpColors[i].green = TmpColors[i].blue = 0;
}

#define MAX_CONTOUR_VAL 195076 /* 3 * 255**2 + 1 */

static void draw_contour_scale()
{
   int i, num_contours, contour;
   unsigned long value, bin_size;

   num_contours = MAX(RedSliderValue, 1);

   bin_size = MAX_CONTOUR_VAL / num_contours;

   for (i = 0; i < num_colors; i++) {
      value = SQ(colors[i].red) + SQ(colors[i].green) + SQ(colors[i].blue);
      contour = (value / bin_size) % NUM_CONTOURS;
      TmpColors[i].red = contour_colors[contour].red;
      TmpColors[i].green = contour_colors[contour].green;
      TmpColors[i].blue = contour_colors[contour].blue;
   }
}

#undef MAX_CONTOUR_VAL

static void draw_random_scale()
{
   int i;

   for (i = 0; i < num_colors; i++) {
      TmpColors[i].red = (unsigned char) random();
      TmpColors[i].green = (unsigned char) random();
      TmpColors[i].blue = (unsigned char) random();
   }
}

/* subpanel.c */
