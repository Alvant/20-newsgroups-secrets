/* cms.c -- routines for handling xrastool's colormap segment */

#include "xrastool.h"

/* Useful flags */

#define COLOR_OK    -1
#define COLOR_FIXED -2
#define COLOR_UNDEF -3

#define MAX_COLOR_DIST 195075 /* (3 * 255**2) */

/* Macro for calculating "distance" between two colors */

#define COLOR_DIST(i,j)\
   (SQ(colors[i].red - colors[j].red) +\
    SQ(colors[i].green - colors[j].green) +\
    SQ(colors[i].blue - colors[j].blue))

/* Global variables */

Cms MainCms;
int NumReservedColors;

/* Local functions */

static void hist_compress(), pair_compress();

/************************************************************/

void MakeCms()
{
   /* Creates dynamic global colormap segment */

   MainCms = xv_create(XV_NULL, CMS,
      CMS_TYPE, XV_DYNAMIC_CMS,
      CMS_SIZE, MAX_NUM_COLORS,
      NULL);

   /* If not private, obtain current colors from default colormap */

   if (CompressOption != NO_COMPRESS) {
      int i;
      Colormap colormap;
      XColor xcolors[MAX_NUM_COLORS];

      NumReservedColors = 30; /* How can we determine this? */

      /* Get default colormap */

      colormap = DefaultColormap(MainDisplay, MainScreen);

      /* Initialize color indices */

      for (i = 0; i < NumReservedColors; i++)
         xcolors[i].pixel = i;

      /* Get first NumReservedColors from default colormap */

      XQueryColors(MainDisplay, colormap, xcolors, NumReservedColors);

      (void) printf("Reserving %i color(s).\n", NumReservedColors);

      /* Place reserved colors in global colormap segment */

      (void) xv_set(MainCms,
         CMS_COLOR_COUNT, NumReservedColors,
         CMS_X_COLORS, xcolors,
         NULL);

      /* Assign colormap segment to base and sub frames */

      (void) xv_set(BaseFrame, WIN_CMS, MainCms, NULL);
      (void) xv_set(SubFrame, WIN_CMS, MainCms, NULL);
   }
   else
      NumReservedColors = 0;

   /* Assign colormap segment to canvas frame */

   (void) xv_set(CanvasFrame, WIN_CMS, MainCms, NULL);
}

void CompressColors(image, data)
IMAGE_T *image;
unsigned char *data;
{
   /* Compresses image colormap according to current option */

   static int first_call = TRUE;
   static unsigned char new_index[MAX_NUM_COLORS];

   int n;

   /* Apply compression choice if first call or not unique colormap */

   if (first_call || !OneCmap) {
      (void) printf("   [%i color(s) found", image->num_colors);
      first_call = FALSE;
      switch (CompressOption) {
         case NO_COMPRESS: /* Private colormap */
            NumReservedColors = 0;
            (void) printf(" -- using private colormap]\n");
            return;
         case HIST_COMPRESS: /* Remove least-used colors first */
            hist_compress(image, data, new_index);
            break;
         case PAIR_COMPRESS: /* Merge most-like colors */
            pair_compress(image, new_index);
            break;
         default:
            Error("Invalid colormap compression option.");
      }
   }

   /* Adjust image data to conform to new colormap */

   for (n = image->w * image->h - 1; n >= 0; n--)
      data[n] = NumReservedColors + new_index[data[n]];
}

void InstallCms(num_colors, colors)
int num_colors;
Xv_singlecolor *colors;
{
   /* Installs new colors in global colormap segment */

   (void) xv_set(MainCms,
      CMS_INDEX, NumReservedColors,
      CMS_COLOR_COUNT, num_colors,
      CMS_COLORS, colors,
      NULL);
}

void CopyColors(n, src, dst)
int n;
Xv_singlecolor *src, *dst;
{
   /* Copies n colors from src array to dst array */

   int i;

   for (i = 0; i < n; i++) {
      dst[i].red = src[i].red;
      dst[i].green = src[i].green;
      dst[i].blue = src[i].blue;
   }
}

static void hist_compress(image, data, new_index)
IMAGE_T *image;
unsigned char *data, *new_index;
{
   /*
    * Compresses image colormap by removing least-used colors first.
    *    This is accomplished by constructing a histogram of color usage
    *    from the image data and sorting through it. The map from old
    *    color values to the new ones is returned in new_index.
    */

   int i, j, num_colors, w, h, num_unused, num_dup, num_to_replace,
      status[MAX_NUM_COLORS];
   Xv_singlecolor *colors;
   unsigned char best_match, replace[MAX_NUM_COLORS];
   unsigned long hist[MAX_NUM_COLORS], min_hist, min_dist, dist;

   /* Get color and image info */

   num_colors = image->num_colors;
   colors = image->colors;
   w = image->w;
   h = image->h;

   (void) printf(" -- hist compress: ");

   /* Initialize */

   for (i = 0; i < num_colors; i++) {
      status[i] = COLOR_OK;
      hist[i] = 0;
   }

   /* Get usage histogram */

   for (i = 0; i < h; i++)
      for (j = 0; j < w; j++)
         ++hist[data[i * w + j]];

   /* Find any unused colors */

   num_unused = 0;

   for (i = 0; i < num_colors; i++)
      if (hist[i] == 0) {
         status[i] = COLOR_UNDEF;
         ++num_unused;
      }

   (void) printf("%i unused, ", num_unused);

   /* Find any duplicated colors */

   num_dup = 0;

   for (i = 0; i < num_colors - 1; i++) {
      if (status[i] != COLOR_OK)
         continue;
      for (j = i + 1; j < num_colors; j++)
         if (status[j] == COLOR_OK && 
            colors[i].red == colors[j].red &&
            colors[i].green == colors[j].green &&
            colors[i].blue == colors[j].blue) {
               status[j] = i;
               ++num_dup;
         }
   }

   (void) printf("%i duplicate(s) removed]\n", num_dup);

   /* Find replacements for reserved colors if required */

   if ((num_to_replace = NumReservedColors + num_colors - num_unused -
         num_dup - MAX_NUM_COLORS) < 0)
      num_to_replace = 0;

   if (num_to_replace > 0) {

      /* Sort colors by usage */

      for (i = 0; i < num_to_replace; i++) {
         min_hist = w * h + 1;
         for (j = 0; j < num_colors; j++)
            if (status[j] == COLOR_OK && hist[j] < min_hist) {
               replace[i] = j;
               min_hist = hist[j];
            }
         status[replace[i]] = COLOR_UNDEF;
      }

      /* Find close colors */

      for (i = 0; i < num_to_replace; i++) {
         best_match = COLOR_UNDEF;
         min_dist = MAX_COLOR_DIST + 1;
         for (j = 0; j < num_colors; j++)
            if (status[j] == COLOR_OK &&
                  (dist = COLOR_DIST(replace[i], j)) < min_dist) {
               best_match = j;
               min_dist = dist;
            }
         status[replace[i]] = best_match;
      }
   }

   /* Construct new_index, starting with colors that will be kept */

   for (i = j = 0; i < num_colors; i++)
      if (status[i] == COLOR_OK) {
         new_index[i] = j;
         colors[j].red = colors[i].red;
         colors[j].green = colors[i].green;
         colors[j].blue = colors[i].blue;
         ++j;
      }

   /* Now remap close colors */

   for (i = 0; i < num_colors; i++)
      if (status[i] != COLOR_OK && status[i] != COLOR_UNDEF)
         new_index[i] = new_index[status[i]];
      else if (status[i] == COLOR_UNDEF) /* For locked colors */
         new_index[i] = 0;

   /*
    * Zero any remaining colors up to MAX_NUM_COLORS so that if another
    *    image uses this colormap (e.g. if colors are locked), any
    *    undefined colors will use this first color (hopefully black)...
    */

   for (i = num_colors; i < MAX_NUM_COLORS; i++)
       new_index[i] = 0;

   /* Save new color count */

   image->num_colors = num_colors - num_unused - num_dup - num_to_replace;
}

static void pair_compress(image, new_index)
IMAGE_T *image;
unsigned char *new_index;
{
   /*
    * Compresses image colormap by joining colors that are most alike.
    *    Note that no reference is made to the image data itself, so this
    *    form of compression is better suited for locked colors. It's
    *    MUCH slower, however, so the -one_cmap option is recommended.
    */

   int i, j, k, num_colors, num_to_replace, status[MAX_NUM_COLORS];
   Xv_singlecolor *colors;

   /* Get color info */

   num_colors = image->num_colors;
   colors = image->colors;

   (void) printf(" -- pair compress]\n");

   /* Initialize */

   for (i = 0; i < num_colors; i++)
      status[i] = COLOR_OK;

   /* Find replacements for reserved colors if required */

   if ((num_to_replace = NumReservedColors + num_colors - MAX_NUM_COLORS) < 0)
      num_to_replace = 0;

   if (num_to_replace > 0) {
      int tmp, best_match;
      unsigned long min_dist, dist, closest_dist[MAX_NUM_COLORS];
      unsigned char closest[MAX_NUM_COLORS];

      /* Construct "distance" table */

      for (i = 0; i < num_colors; i++) {
         best_match = COLOR_UNDEF;
         min_dist = MAX_COLOR_DIST + 1;
         for (j = 0; j < num_colors; j++) {
            if (i != j && (dist = COLOR_DIST(i, j)) < min_dist) {
               min_dist = dist;
               best_match = j;
            }
         }
         closest[i] = best_match;
         closest_dist[i] = min_dist;
      }

      /* Merge colors pair-wise */

      for (i = 0; i < num_to_replace; i++) {
         tmp = COLOR_UNDEF;
         min_dist = MAX_COLOR_DIST + 1;
         for (j = 0; j < num_colors; j++)
            if (status[j] == COLOR_OK &&
                  (status[closest[j]] == COLOR_OK ||
                   status[closest[j]] == COLOR_FIXED) &&
                   closest_dist[j] < min_dist) {
               tmp = j;
               min_dist = closest_dist[j];
            }
         if (tmp == COLOR_UNDEF) /* This should be VERY hard to do... */
            Warning("Unable to find close color.");

         /*
          * Could apply average here, but would need to search for references
          *    to BOTH colors when recalculating distances below...
          */

         best_match = closest[tmp];

         if (status[best_match] == COLOR_OK) {
            status[tmp] = COLOR_FIXED;
            status[best_match] = tmp;
         }
         else { /* i.e. if (status[best_match] == COLOR_FIXED) */
            status[tmp] = best_match;
            best_match = tmp;
         }

         /* Recalculate distances for colors which matched best_match */

         for (j = 0; j < num_colors; j++)
            if (status[j] == COLOR_OK && closest[j] == best_match) {
               tmp = COLOR_UNDEF;
               min_dist = MAX_COLOR_DIST + 1;
               for (k = 0; k < num_colors; k++) {
                  if (j != k && status[k] == COLOR_OK &&
                     (dist = COLOR_DIST(j, k)) < min_dist) {
                        min_dist = dist;
                        tmp = k;
                     }
               }
               closest[j] = tmp;
               closest_dist[j] = min_dist;
            }
      }
   }

   /* Construct new_index and save new color count */

   for (i = j = 0; i < num_colors; i++)
      if (status[i] == COLOR_OK || status[i] == COLOR_FIXED) {
         new_index[i] = j;
         colors[j].red = colors[i].red;
         colors[j].green = colors[i].green;
         colors[j].blue = colors[i].blue;
         ++j;
      }

   for (i = 0; i < num_colors; i++)
      if (status[i] != COLOR_OK && status[i] != COLOR_FIXED)
         new_index[i] = new_index[status[i]];

   for (i = num_colors; i < MAX_NUM_COLORS; i++)
       new_index[i] = 0;

   image->num_colors = num_colors - num_to_replace;
}

/* cms.c */
