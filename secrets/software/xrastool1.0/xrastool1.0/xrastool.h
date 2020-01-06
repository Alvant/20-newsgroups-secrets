/* xrastool.h -- main header file for xrastool */

#include "COPYRIGHT" /* Please read this */

/* Other header files to include */

#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/cms.h>

/* Global definitions */

#define VERSION "xrastool version 1.0"

#define NUM_BACKDROPS 9 /* In bitmaps subdirectory */

#define LIVE_SLIDERS TRUE /* Set FALSE if dynamic color sliders too slow */

#define MAX_STR_LEN    256
#define MAX_NUM_COLORS 256
#define MAX_NUM_IMAGES 1000

/* Keyboard accelerators */

#define SET_KEY   '\015' /* Carriage return */
#define STEP_KEY  ' '    /* Space */
#define CYCLE_KEY 'c'
#define QUIT_KEY  'q'

/* Return values */

#define OK 0
#define ERROR 1

/* Bool type */

#ifndef Bool
#	define Bool int
#	define FALSE 0
#	define TRUE 1
#endif

/* Macros */

#define SQ(x) ((x) * (x))

#define PLURAL(n) ((n) != 1 ? "s" : "")

#define NO_IMAGES (NumImages == 0)

#define UNDEF_IMAGE (Current >= NumImages)

/* Definitions for use with main parameters */

#define NO_COMPRESS   0
#define HIST_COMPRESS 1
#define PAIR_COMPRESS 2

#define SRC 0
#define OR  1
#define AND 2
#define XOR 3
#define ERA 4
#define NEG 5

#define FORCE_SIZE -1

#define AUTO  0
#define FULL  1
#define FIXED 2

#define TIMER_US  0
#define TIMER_MS  1
#define TIMER_SEC 2

#define MIN_TIMER_VALUE 1
#define MAX_TIMER_VALUE 999

#define FWD 0
#define REV 1

#define LOOP      0
#define LOOP_BACK 1
#define ONE_WAY   2

#define NO_SCROLLING 1        /* Scrolling should be before resizing */
#define NO_RESIZING  (1 << 1)
#define NO_MOVING    (1 << 2)
#define NO_SCALING   (1 << 3)
#define NO_UPDATES   (1 << 4)
#define NO_BACKDROPS (1 << 5)
#define LOCK_COLORS  (1 << 6)
#define CENTERING    (1 << 7)
#define LIVE_CURSOR  (1 << 8)

#define FAST_MODE (NO_SCROLLING | NO_RESIZING | NO_MOVING | NO_SCALING |\
			NO_UPDATES | NO_BACKDROPS | LOCK_COLORS)

#define BW      0
#define GRAY    1
#define COLOR   2
#define CUTOFF  3
#define CONTOUR 4
#define RANDOM  5

#define MULT 0
#define DIV  1

/* Window geometry */

#define HEADER_MARGIN       20 /* How can we determine these? */
#define FOOTER_MARGIN       20
#define FRAME_BORDER        5
#define INNER_CANVAS_BORDER 1

#define MIN_IMAGE_WIDTH  64 /* Arbitrary...could be smaller */
#define MIN_IMAGE_HEIGHT 64

#define MAX_IMAGE_WIDTH (ScreenWidth - 2 * FRAME_BORDER -\
		2 * INNER_CANVAS_BORDER - ScrollMargin)

#define MAX_IMAGE_HEIGHT (ScreenHeight - HEADER_MARGIN  - 2 * FRAME_BORDER -\
		2 * INNER_CANVAS_BORDER - ScrollMargin -\
		((Options & LIVE_CURSOR) ? FOOTER_MARGIN : 0))

#define FRAME_GAP 10
#define DFLT_BASE_FRAME_XV_X 50
#define DFLT_BASE_FRAME_XV_Y 50
#define DFLT_SUB_FRAME_XV_X DFLT_BASE_FRAME_XV_X
#define DFLT_SUB_FRAME_XV_Y\
	(DFLT_BASE_FRAME_XV_Y + FRAME_BORDER + HEADER_MARGIN +\
	(int) xv_get(BaseFrame, XV_HEIGHT) + FOOTER_MARGIN + FRAME_BORDER +\
	FRAME_GAP)
#define DFLT_CANVAS_FRAME_XV_X\
	(DFLT_BASE_FRAME_XV_X + FRAME_BORDER + (int) xv_get(BaseFrame,\
	XV_WIDTH) + FRAME_BORDER + FRAME_GAP)
#define DFLT_CANVAS_FRAME_XV_Y DFLT_BASE_FRAME_XV_Y

/* Definition of image structure */

typedef struct {
	char *filename;
	int num_colors;
	Xv_singlecolor *colors;
	int w, h, d;
	Pixmap pixmap;
} IMAGE_T;

/* Main parameters, in order of appearance (defaults set in params.c) */

extern int  Current;          /* Index of current image */
extern int  CompressOption;   /* NO_COMPRESS, HIST_COMPRESS, PAIR_COMPRESS */
extern int  Logical;          /* SRC, OR, AND, XOR, ERA, NEG */
extern int  FixedWidth;       /* Image width for AUTO or FIXED sizing */
extern int  FixedHeight;      /* Image height for AUTO or FIXED sizing */
extern int  Sizing;           /* AUTO, FULL, FIXED */
extern int  TimerScale;       /* TIMER_US, TIMER_MS, TIMER_SEC */
extern int  TimerValue;       /* Timer value between MIN_ and MAX_TIMER_VALUE */
extern int  Direction;        /* FWD, REV */
extern Bool Blinking;         /* TRUE, FALSE */
extern Bool Cycling;          /* TRUE, FALSE */
extern int  CycleOption;      /* LOOP, LOOP_BACK, ONE_WAY */
extern int  Options;          /* see MakeSubWindow() */
extern int  BackdropIndex;    /* Index of current backdrop */
extern int  Scaling;          /* BW, GRAY, COLOR, CUTOFF, CONTOUR, RANDOM */
extern int  RedSliderValue;   /* Between 0 and 100 */
extern int  RedSliderScale;   /* MULT, DIV */
extern int  GreenSliderValue; /* Between 0 and 100 */
extern int  GreenSliderScale; /* MULT, DIV */
extern int  BlueSliderValue;  /* Between 0 and 100 */
extern int  BlueSliderScale;  /* MULT, DIV */

/* Other global parameters */

extern int  NukeCanvas;       /* TRUE, FALSE */
extern int  ForceFirstResize; /* TRUE, FALSE */
extern int  StartCycling;     /* TRUE, FALSE */
extern Bool OneCmap;          /* TRUE, FALSE */
extern Bool ShowSubPanel;     /* TRUE, FALSE */

/* Global description of display/screen (xrastool.c) */

extern Display *MainDisplay;
extern int MainScreen, ScreenWidth, ScreenHeight, ScreenDepth;
extern Visual *MainVisual;
extern GC MainGC;

/* Other global variables, in file order */

extern Frame BaseFrame;
extern Frame CanvasFrame;
extern int ScrollMargin;
extern Window CanvasXID;
extern Pixmap Backdrop[NUM_BACKDROPS];
extern GC BackdropGC;
extern XImage *TmpXImage;
extern Xv_singlecolor TmpColors[MAX_NUM_COLORS];
extern Cms MainCms;
extern int NumReservedColors;
extern Panel MainPanel;
extern int NumImages;
extern struct itimerval Timer;
extern int NumLockedColors;
extern Xv_singlecolor *LockedColors;
extern Frame SubFrame;
extern Panel SubPanel;
extern IMAGE_T *Image[MAX_NUM_IMAGES];

/* Global functions, in file order */

extern void MakeBaseFrame();
extern void MakeCanvas();
extern void SetScrollbars();
extern void ShowNewImage();
extern void MakeBackdropImages();
extern void PaintBackground();
extern void MakeCms();
extern void CompressColors();
extern void InstallCms();
extern void CopyColors();
extern int  LoadImages();
extern void MakeMainPanel();
extern void KeyPressed();
extern void UpdateLabels();
extern void SetFixedSizeItem();
extern void TimerOn();
extern void TimerOff();
extern void MoveCursorAndCaret();
extern void ReadCmdLine();
extern void Initialize();
extern void SetLogical();
extern void SetTimer();
extern void SetOptions();
extern void SetBackdrop();
extern void SetColorSliderDefaults();
extern void SetScaling();
extern void MakeSubWindow();
extern void UpdateOptionsItem();
extern void ToggleScalingExposure();
extern void ToggleBackdropsExposure();
extern void ApplyColorScaling();
extern void SetColorSliders();
extern void PointToItem();
extern void Warning();
extern void Error();

/* Following is to keep gcc happy... */

#ifndef _STDIO_H
extern char tolower();
extern int fclose(), fprintf(), fread(), printf(), _filbuf();
extern long random();
extern void setbuf();
#endif

/* xrastool.h */
