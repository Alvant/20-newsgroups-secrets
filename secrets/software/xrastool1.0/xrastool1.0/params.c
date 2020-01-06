/* params.c -- routines for setting xrastool defaults and handling options */

#include "xrastool.h"

/* Default values for main parameters */

#define DFLT_ONE_CMAP           FALSE
#define DFLT_SHOW_SUB_PANEL     FALSE
#define DFLT_FIRST_IMAGE        0
#define DFLT_COMPRESS_OPTION    PAIR_COMPRESS
#define DFLT_LOGICAL            SRC
#define DFLT_FIXED_WIDTH        640
#define DFLT_FIXED_HEIGHT       480
#define DFLT_SIZING             AUTO
#define DFLT_TIMER_SCALE        TIMER_MS
#define DFLT_TIMER_VALUE        100
#define DFLT_DIRECTION          FWD
#define DFLT_BLINKING           FALSE
#define DFLT_CYCLING            FALSE
#define DFLT_CYCLE_OPTION       LOOP
#define DFLT_OPTIONS            0
#define DFLT_BACKDROP_INDEX     0
#define DFLT_SCALING            COLOR
#define DFLT_RED_SLIDER_VALUE   100
#define DFLT_RED_SLIDER_SCALE   0
#define DFLT_GREEN_SLIDER_VALUE 100
#define DFLT_GREEN_SLIDER_SCALE 0
#define DFLT_BLUE_SLIDER_VALUE  100
#define DFLT_BLUE_SLIDER_SCALE  0

#define INITIALIZE -1 /* Useful flag for SetOptions() */

/* Global variables */

int NumImages = 0;
struct itimerval Timer;
int NumLockedColors;
Xv_singlecolor *LockedColors;

/* Global parameters */

Bool ForceFirstResize = FALSE;
Bool StartCycling     = FALSE;
Bool OneCmap          = DFLT_ONE_CMAP;
Bool ShowSubPanel     = DFLT_SHOW_SUB_PANEL;
int  Current          = DFLT_FIRST_IMAGE;
int  CompressOption   = DFLT_COMPRESS_OPTION;
int  Logical          = DFLT_LOGICAL;
int  FixedWidth       = DFLT_FIXED_WIDTH;
int  FixedHeight      = DFLT_FIXED_HEIGHT;
int  Sizing           = DFLT_SIZING;
int  TimerScale       = DFLT_TIMER_SCALE;
int  TimerValue       = DFLT_TIMER_VALUE;
int  Direction        = DFLT_DIRECTION;
Bool Blinking         = DFLT_BLINKING;
Bool Cycling          = DFLT_CYCLING;
int  CycleOption      = DFLT_CYCLE_OPTION;
int  Options          = DFLT_OPTIONS;
int  BackdropIndex    = DFLT_BACKDROP_INDEX;
int  Scaling          = DFLT_SCALING;
int  RedSliderValue   = DFLT_RED_SLIDER_VALUE;
int  RedSliderScale   = DFLT_RED_SLIDER_SCALE;
int  GreenSliderValue = DFLT_GREEN_SLIDER_VALUE;
int  GreenSliderScale = DFLT_GREEN_SLIDER_SCALE;
int  BlueSliderValue  = DFLT_BLUE_SLIDER_VALUE;
int  BlueSliderScale  = DFLT_BLUE_SLIDER_SCALE;

/* Local functions */

static void show_usage();

/************************************************************/

/* Toggles for command line options */

#define ON  '+'
#define OFF '-'

void ReadCmdLine(argc, argv, files_start)
int argc, *files_start;
char *argv[];
{
   /* Reads any command line options and returns files_start */

   /* NOTE: trailing arguments override leading arguments */

   int  i;
   char toggle;

   /* Loop through arguments */

   for (i = 1; i < argc; i++)
      if ((toggle = argv[i][0]) == ON || toggle == OFF) {
         ++argv[i];
         if (!strcmp(argv[i], "fast")) { /* Fast mode */
            OneCmap = TRUE;
            ForceFirstResize = TRUE;
            FixedWidth = FORCE_SIZE;
            FixedHeight = FORCE_SIZE;
            Sizing = FIXED;
            TimerScale = TIMER_US;
            TimerValue = 1;
            Options = FAST_MODE;
         }
         else if (!strcmp(argv[i], "onecmap")) { /* Unique colormap */
            OneCmap = (toggle == OFF);
            if (OneCmap)
               Options |= LOCK_COLORS;
         }
         else if (!strcmp(argv[i], "subpanel"))
            ShowSubPanel = (toggle == OFF);
         else if (!strcmp(argv[i], "start")) {
            if (++i < argc) {
               Current = atoi(argv[i]);
               Current = MIN(Current, MAX_NUM_IMAGES);
               Current = MAX(Current, 1);
               --Current; /* Internally, indices start at 0 */
            }
         }
         else if (!strcmp(argv[i], "private"))
            CompressOption = NO_COMPRESS;
         else if (!strcmp(argv[i], "hist"))
            CompressOption = HIST_COMPRESS;
         else if (!strcmp(argv[i], "pair"))
            CompressOption = PAIR_COMPRESS;
         else if (!strcmp(argv[i], "logical")) { /* Logical functions */
            if (++i < argc) {
               if (!strcmp(argv[i], "SRC"))
                  Logical = SRC;
               else if (!strcmp(argv[i], "OR"))
                  Logical = OR;
               else if (!strcmp(argv[i], "AND"))
                  Logical = AND;
               else if (!strcmp(argv[i], "XOR"))
                  Logical = XOR;
               else if (!strcmp(argv[i], "ERA"))
                  Logical = ERA;
               else if (!strcmp(argv[i], "NEG"))
                  Logical = NEG;
            }
         }
         else if (!strcmp(argv[i], "w")) {
            if (++i < argc) {
               FixedWidth = atoi(argv[i]);
               FixedWidth = MIN(FixedWidth, MAX_IMAGE_WIDTH);
               FixedWidth = MAX(FixedWidth, MIN_IMAGE_WIDTH);
            }
         }
         else if (!strcmp(argv[i], "h")) {
            if (++i < argc) {
               FixedHeight = atoi(argv[i]);
               FixedHeight = MIN(FixedHeight, MAX_IMAGE_HEIGHT);
               FixedHeight = MAX(FixedHeight, MIN_IMAGE_HEIGHT);
            }
         }
         else if (!strcmp(argv[i], "auto"))
            Sizing = AUTO;
         else if (!strcmp(argv[i], "full"))
            Sizing = FULL;
         else if (!strcmp(argv[i], "fixed"))
            Sizing = FIXED;
         else if (!strcmp(argv[i], "timerscale")) { /* Timer scale */
            if (++i < argc) {
               if (!strcmp(argv[i], "us"))
                  TimerScale = TIMER_US;
               else if (!strcmp(argv[i], "ms"))
                  TimerScale = TIMER_MS;
               else if (!strcmp(argv[i], "sec"))
                  TimerScale = TIMER_SEC;
            }
         }
         else if (!strcmp(argv[i], "timervalue")) {
            if (++i < argc) {
               TimerValue = atoi(argv[i]);
               TimerValue = MIN(TimerValue, MAX_TIMER_VALUE);
               TimerValue = MAX(TimerValue, MIN_TIMER_VALUE);
            }
         }
         else if (!strcmp(argv[i], "fwd"))
            Direction = FWD;
         else if (!strcmp(argv[i], "rev"))
            Direction = REV;
         else if (!strcmp(argv[i], "blink"))
            Blinking = (toggle == OFF);
         else if (!strcmp(argv[i], "cycle"))
            Cycling = (toggle == OFF);
         else if (!strcmp(argv[i], "loop"))
            CycleOption = LOOP;
         else if (!strcmp(argv[i], "loopback"))
            CycleOption = LOOP_BACK;
         else if (!strcmp(argv[i], "oneway"))
            CycleOption = ONE_WAY;
         else if (!strcmp(argv[i], "scrolling"))
            Options = (toggle == ON ? Options & ~NO_SCROLLING :
               Options | NO_SCROLLING);
         else if (!strcmp(argv[i], "resizing")) {
            Options = (toggle == ON ? Options & ~NO_RESIZING :
               Options | NO_RESIZING);
            if (toggle == ON) { /* Ensure first image is correctly framed */
               ForceFirstResize = TRUE;
               FixedWidth = FORCE_SIZE;
               FixedHeight = FORCE_SIZE;
            }
            else
               ForceFirstResize = FALSE;
         }
         else if (!strcmp(argv[i], "moving"))
            Options = (toggle == ON ? Options & ~NO_MOVING :
               Options | NO_MOVING);
         else if (!strcmp(argv[i], "scaling"))
            Options = (toggle == ON ? Options & ~NO_SCALING :
               Options | NO_SCALING);
         else if (!strcmp(argv[i], "updates"))
            Options = (toggle == ON ? Options & ~NO_UPDATES :
               Options | NO_UPDATES);
         else if (!strcmp(argv[i], "backdrops"))
            Options = (toggle == ON ? Options & ~NO_BACKDROPS :
               Options | NO_BACKDROPS);
         else if (!strcmp(argv[i], "lockcolors"))
            Options = (toggle == ON && !OneCmap ? Options & ~LOCK_COLORS :
               Options | LOCK_COLORS);
         else if (!strcmp(argv[i], "centering"))
            Options = (toggle == ON ? Options & ~CENTERING :
               Options | CENTERING);
         else if (!strcmp(argv[i], "livecursor"))
            Options = (toggle == ON ? Options & ~LIVE_CURSOR :
               Options | LIVE_CURSOR);
         else if (!strcmp(argv[i], "backdrop")) {
            if (++i < argc) {
               BackdropIndex = atoi(argv[i]);
               BackdropIndex = MIN(BackdropIndex, NUM_BACKDROPS);
               BackdropIndex = MAX(BackdropIndex, 1);
               --BackdropIndex;
            }
         }
         else if (!strcmp(argv[i], "bw"))
            { Scaling = BW; SetColorSliderDefaults(); }
         else if (!strcmp(argv[i], "gray"))
            { Scaling = GRAY; SetColorSliderDefaults(); }
         else if (!strcmp(argv[i], "color"))
            { Scaling = COLOR; SetColorSliderDefaults(); }
         else if (!strcmp(argv[i], "cutoff"))
            { Scaling = CUTOFF; SetColorSliderDefaults(); }
         else if (!strcmp(argv[i], "contour"))
            { Scaling = CONTOUR; SetColorSliderDefaults(); }
         else if (!strcmp(argv[i], "random"))
            { Scaling = RANDOM; SetColorSliderDefaults(); }
         else if (!strcmp(argv[i], "red")) { /* Red slider */
            if (++i < argc) {
               RedSliderValue = atoi(argv[i]);
               if (RedSliderValue < 0) { /* Use neg val to select DIV */
                  RedSliderValue *= -1;
                  RedSliderScale = DIV;
               }
               else
                  RedSliderScale = MULT;
            }
         }
         else if (!strcmp(argv[i], "green")) {
            if (++i < argc) {
               GreenSliderValue = atoi(argv[i]);
               if (GreenSliderValue < 0) {
                  GreenSliderValue *= -1;
                  GreenSliderScale = DIV;
               }
               else
                  GreenSliderScale = MULT;
           }
         }
         else if (!strcmp(argv[i], "blue")) {
            if (++i < argc) {
               BlueSliderValue = atoi(argv[i]);
               if (BlueSliderValue < 0) {
                  BlueSliderValue *= -1;
                  BlueSliderScale = DIV;
               }
               else
                  BlueSliderScale = MULT;
            }
         }
         else {
            (void) fprintf(stderr, "\007Unknown option: %s.\n", argv[i]);
            show_usage();
         }
      }
      else
         break;

   /* Point to first non-option...hopefully it's a filename! */

   *files_start = i;

   /* Check to make sure no further options trail the first filename */

   for (i = *files_start + 1; i < argc; i++)
       if (argv[i][0] == ON || argv[i][0] == OFF) {
          (void) fprintf(stderr, "\007Bad syntax (%s).\n", argv[i]);
          show_usage();
       }
}

#undef OFF
#undef ON

void Initialize()
{
   /* Applies defaults and/or command line options as required */

   SetLogical();

   SetTimer();

   if (Blinking)
      Cycling = TRUE;

   if (Cycling) {
      StartCycling = TRUE;
      Cycling = FALSE; /* For correct toggling behaviour */
   }

   SetOptions(INITIALIZE);

   SetBackdrop();

   SetScaling();

   SetColorSliders(); /* In subpanel.c */
}

void SetLogical()
{
   /* Changes logical function of MainGC */

   XGCValues xgcv;

   /* Determine function */

   switch (Logical) {
      case SRC:
         xgcv.function = GXcopy;
         break;
      case OR:
         xgcv.function = GXor;
         break;
      case AND:
         xgcv.function = GXand;
         break;
      case XOR:
         xgcv.function = GXxor;
         break;
      case ERA:
         xgcv.function = GXandInverted;
         break;
      case NEG:
         xgcv.function = GXcopyInverted;
         break;
      default:
         Error("Invalid logical function.");
   }

   /* Apply change */

   XChangeGC(MainDisplay, MainGC, GCFunction, &xgcv);
}

void SetTimer()
{
   /* Sets elements of Timer structure */

   /* First stop cycling if applicable */

   if (Cycling && !NO_IMAGES)
      TimerOff();

   /* Scale by 1000 for milliseconds */

   if (TimerScale == TIMER_MS)
      TimerValue *= 1000;

   /* Set microsecond or second elements as appropriate */

   if (TimerScale == TIMER_US || TimerScale == TIMER_MS) {
      Timer.it_value.tv_usec = Timer.it_interval.tv_usec = TimerValue;
      Timer.it_value.tv_sec = Timer.it_interval.tv_sec = 0;
   }
   else {
      Timer.it_value.tv_usec = Timer.it_interval.tv_usec = 0;
      Timer.it_value.tv_sec = Timer.it_interval.tv_sec = TimerValue;
   }

   /* Switch cycling back on if applicable */

   if (Cycling && !NO_IMAGES)
      TimerOn();
}

void SetOptions(change)
int change;
{
   /* Apply change to Options (subpanel options) */

   int new_options;

   /* Ignore change on first call; otherwise determine new (set) options */

   if (change == INITIALIZE)
      change = new_options = Options;
   else
      new_options = Options ^ change;

   /* Toggle canvas scrollbars if requested */

   if (change & NO_SCROLLING)
      SetScrollbars(!(new_options & NO_SCROLLING));

   /* Set fixed sizing if NO_RESIZING selected and hide sizing controls */

   if (change & NO_RESIZING) {
      if (new_options & NO_RESIZING)
         SetFixedSizeItem();
      ToggleSizingExposure(new_options & NO_RESIZING);
   }

   /* Hide or expose color scaling options and sliders if NO_SCALING changed */

   if (change & NO_SCALING)
      ToggleScalingExposure(new_options & NO_SCALING);

   /* Hide or expose backdrop choices if NO_BACKDROPS changed */

   if (change & NO_BACKDROPS)
      ToggleBackdropsExposure(new_options & NO_BACKDROPS);

   /* Assign locked colors if requested */

   if ((change & LOCK_COLORS) && (new_options & LOCK_COLORS) && !UNDEF_IMAGE) {
      NumLockedColors = Image[Current]->num_colors;
      LockedColors = Image[Current]->colors;
   }

   /* Toggle cursor if requested */

   if (change & LIVE_CURSOR) 
      if (new_options & LIVE_CURSOR) { /* Store colors and show canvas footer */
         (void) xv_get(MainCms, CMS_COLORS, TmpColors);
         (void) xv_set(CanvasFrame,
            FRAME_SHOW_FOOTER, TRUE,
            FRAME_LEFT_FOOTER, (UNDEF_IMAGE ? "(no image)" :
               "(no cursor)"),
            NULL);
      }
      else { /* Destroy XImage if applicable and hide canvas footer */
         if (!NO_IMAGES)
            (void) XDestroyImage(TmpXImage);
         (void) xv_set(CanvasFrame, FRAME_SHOW_FOOTER, FALSE,
            NULL);
      }

   /* Update options variable */

   Options = new_options;

   /* Redisplay image if applicable */

   ShowNewImage();
}

void SetBackdrop()
{
   /* Changes the current backdrop */

   XGCValues values;

   /* Assign pixmap to stipple */

   values.stipple = (Pixmap) xv_get(Backdrop[BackdropIndex], XV_XID);

   /* Apply change */

   XChangeGC(MainDisplay, BackdropGC, GCStipple, &values);

   /* If undefined image, paint entire canvas; otherwise redisplay image */

   if (UNDEF_IMAGE && !NO_IMAGES)
      PaintBackground();
   else
      ShowNewImage();
}


void SetColorSliderDefaults()
{
   /* Sets default slider values */

   switch (Scaling) {
      case BW:
         RedSliderValue = GreenSliderValue = BlueSliderValue = 50;
         break;
      case GRAY:
         RedSliderValue = 30;   /* Standard values...must add to 100 */
         GreenSliderValue = 59;
         BlueSliderValue = 11;
         break;
      case COLOR:
         RedSliderValue = GreenSliderValue = BlueSliderValue = 100;
         break;
      case CUTOFF:
         RedSliderValue = GreenSliderValue = BlueSliderValue = 100;
         break;
      case CONTOUR:
         RedSliderValue = GreenSliderValue = BlueSliderValue = 25;
         break;
      case RANDOM:
         RedSliderValue = GreenSliderValue = BlueSliderValue = 50;
         break;
      default:
         Error("Invalid color scaling option.");
   }
}

void SetScaling()
{
   /* Initializes color sliders and applies scaling */

   SetColorSliders();

   /* Redisplay image with new scaling if applicable */

   if (!UNDEF_IMAGE && !(Options & NO_SCALING))
      ApplyColorScaling();
}

static void show_usage()
{
   /* Prints a usage message to stderr and aborts with error status 1 */

   (void) fprintf(stderr, "Usage: (standard window options omitted)\n");

   (void) fprintf(stderr,
"xrastool [-fast] [+|-onecmap] [+|-subpanel] [-start (index)]\n");
   (void) fprintf(stderr,
"         [-private|-hist|-pair] [-logical SRC|OR|AND|XOR|ERA|NEG]\n");
   (void) fprintf(stderr,
"         [-w (width)] [-h (height)] [-auto|-full|-fixed]\n");
   (void) fprintf(stderr,
"         [-timerscale us|ms|sec] [-timervalue (timer)]\n");
   (void) fprintf(stderr,
"         [-fwd|-rev] [-blink|-cycle] [-loop|-loopback|-oneway]\n");
   (void) fprintf(stderr,
"         [+|-scrolling] [+|-resizing] [+|-moving] [+|-scaling]\n");
   (void) fprintf(stderr,
"         [+|-updates] [+|-backdrops] [+|-lockcolors]\n");
   (void) fprintf(stderr,
"         [+|-centering] [+|-livecursor] [-backdrop (index)]\n");
   (void) fprintf(stderr,
"         [-bw|-gray|-color|-cutoff|-contour|-random]\n");
   (void) fprintf(stderr,
"         [-red (value)] [-green (value)] [-blue (value)]\n");
   (void) fprintf(stderr,
"         [filename...]\n");
   (void) fprintf(stderr,
"\"[ ]\" indicate optional items; \"|\" indicates exclusive choices;\n");
   (void) fprintf(stderr,
"\"( )\" indicate user-supplied values; colors may be negative to denote\n");
   (void) fprintf(stderr,
"inverse scale; use \"+\" modifiers where applicable to override -fast.\n");

   exit(1);
}

/* params.c */
