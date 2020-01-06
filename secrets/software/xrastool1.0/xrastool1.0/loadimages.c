/* loadimages.c -- routines for loading images into xrastool */

#include "xrastool.h"
#include <malloc.h>
#include <rasterfile.h>

/* Useful definition */

#define BITS_PER_BYTE (MainVisual->bits_per_rgb)

/* Local functions */

static void dealloc_image();
static int load_sun_raster(), read_rle_data();

/************************************************************/

int LoadImages(num_images, filenames, index)
int num_images, index;
char *filenames[];
{
   /* Loads num_images images named filenames starting at Image[index] */

   int i, tmp;
   IMAGE_T *image;
   Bool save_colormap;
   unsigned char *data;

   /* Loop through requested filenames */

   tmp = index;

   for (i = 0; i < num_images; i++) {

      /* Error checking */

      if (NumImages == MAX_NUM_IMAGES) {
         Warning("Image array full -- aborting load.");
         break;
      }

      if (index > NumImages)
         Error("Invalid image index.");

      /* If image already exists, deallocate it */

      if (index < NumImages)
         dealloc_image(index);

      /* Attempt to allocate space for image structure */

      if ((image = (IMAGE_T *) malloc(sizeof(IMAGE_T))) == NULL) {
         Warning("Out of memory.");
         break;
      }

      /* Assign new structure and save filename */

      Image[index] = image;

      image->filename = malloc(strlen(filenames[i]) + 1);
      (void) strcpy(image->filename, filenames[i]);

      (void) printf("\"%s\": ", image->filename);

      /* Determine whether to retain this image's colormap */

      save_colormap = (!OneCmap || index == 0);

      /* Load rasterfile */

      if (load_sun_raster(image, &data, save_colormap) == ERROR) {
         Warning("Read error -- skipping...");
         continue;
      }

      /* Compress colors */

      CompressColors(image, data);

      /* Create client-resident ximage */

      TmpXImage = XCreateImage(MainDisplay, MainVisual, image->d,
         ZPixmap, 0, data, image->w, image->h, BITS_PER_BYTE, 0);

      /* Now move to server-resident pixmap and destroy ximage */

      image->pixmap = XCreatePixmap(MainDisplay, CanvasXID, image->w,
         image->h, image->d);

      XPutImage(MainDisplay, image->pixmap, MainGC, TmpXImage,
         0, 0, 0, 0, image->w, image->h);

      XDestroyImage(TmpXImage);

      /* Free data array */

      free((char *) data);

      /* If this was a new image, increment image counter */

      if (index == NumImages)
         ++NumImages;

      /* Point to next element in Image array */

      ++index;
   }

   /* Return number of files successfully loaded */

   return index - tmp;
}

static void dealloc_image(index)
int index;
{
   /* Frees memory associated with Image[index] */

   IMAGE_T *image;

   image = Image[index];

   free((char *) image->filename);
   free((char *) image->colors);
   XFreePixmap(MainDisplay, image->pixmap);
   free((char *) image);
}
      
static int load_sun_raster(image, data, save_colormap)
IMAGE_T *image;
unsigned char **data;
Bool save_colormap;
{
   /* Loads Sun rasterfile info into image and data */

   FILE *fp;
   struct rasterfile header;
   int i, w, h, d, num_colors, odd_byte, ras_length, num_pixels;

   /* Attempt to open rasterfile */

   if ((fp = fopen(image->filename, "r")) == NULL) {
      (void) fprintf(stderr, "Unable to open rasterfile.\n");
      return ERROR;
   }

   /* Read header */

   (void) fread((char *) &header, sizeof(struct rasterfile), 1, fp);

   if (header.ras_magic != RAS_MAGIC) {
      (void) fprintf(stderr, "Not rasterfile.\n");
      return ERROR;
   }

   /* Determine type */

   switch (header.ras_type) {
      case RT_OLD:
         (void) printf("Old ras ");
         break;
      case RT_STANDARD:
         (void) printf("Std ras ");
         break;
      case RT_BYTE_ENCODED:
         (void) printf("RLE ras ");
         break;
      default:
         (void) fprintf(stderr, "-- Unknown encoding.\n");
         return ERROR;
   }

   /* Read colormap */

   switch (header.ras_maptype) {
      case RMT_NONE:
         (void) fprintf(stderr, "-- B&W images not currently supported.\n");
         return ERROR;
      case RMT_EQUAL_RGB:
         (void) printf("RGB ");
         break;
      case RMT_RAW:
         (void) fprintf(stderr, "-- Raw colors not supported.\n");
         return ERROR;
      default:
         (void) fprintf(stderr, "-- Unknown colormap type.\n");
         return ERROR;
   }

   num_colors = (header.ras_maptype == RMT_NONE ? 2 : header.ras_maplength / 3);

   if (num_colors <= 0) {
      (void) fprintf(stderr, "-- No colors found!\n");
      return ERROR;
   }

   (void) printf("max %i colors ", num_colors);

   if (save_colormap) {
      image->num_colors = num_colors;
      if ((image->colors = (Xv_singlecolor *) malloc(num_colors *
            sizeof(Xv_singlecolor))) == NULL) {
         (void) fprintf(stderr, "-- Out of memory.\n");
         return ERROR;
      }
  }
   else {
      image->num_colors = 0;
      image->colors = NULL;
   }

   if (header.ras_maptype == RMT_EQUAL_RGB) {
      unsigned char *r, *g, *b;

      r = (unsigned char *) malloc(num_colors * sizeof(unsigned char));
      g = (unsigned char *) malloc(num_colors * sizeof(unsigned char));
      b = (unsigned char *) malloc(num_colors * sizeof(unsigned char));

      (void) fread((char *) r, sizeof(unsigned char), num_colors, fp);
      (void) fread((char *) g, sizeof(unsigned char), num_colors, fp);
      (void) fread((char *) b, sizeof(unsigned char), num_colors, fp);

      if (save_colormap)
         for (i = 0; i < num_colors; i++) {
            image->colors[i].red = r[i];
            image->colors[i].green = g[i];
            image->colors[i].blue = b[i];
         }
   }

   /* Read image data (note image lines rounded to a multiple of 16 bits...) */

   w = header.ras_width;
   h = header.ras_height;
   d = header.ras_depth;

   odd_byte = w % 2;

   ras_length = (w + odd_byte) * h * d / BITS_PER_BYTE;

   num_pixels = w * h * d / BITS_PER_BYTE;

   if (header.ras_length != ras_length && header.ras_type != RT_OLD &&
         header.ras_type != RT_BYTE_ENCODED) {
      (void) fprintf(stderr, "Inconsistent data length.\n");
      return ERROR;
   }

   if (ras_length <= 0) {
      (void) fprintf(stderr, "-- No data found!\n");
      return ERROR;
   }

   (void) printf("%i bytes ", header.ras_length);

   if ((*data = (unsigned char *) malloc(num_pixels * sizeof(unsigned char)))
         == NULL) {
      (void) fprintf(stderr, "-- Out of memory.\n");
      return ERROR;
   }

   if (header.ras_type == RT_BYTE_ENCODED) { /* RLE raster */
      if (read_rle_data(fp, ras_length, w, odd_byte, *data) == ERROR)
         return ERROR;
   }
   else /* Standard raster */
      for (i = 0; i < h; i++) {
         if (fread((char *) &((*data)[i * w]), sizeof(unsigned char),
               w, fp) != w) {
            (void) fprintf(stderr, "-- Error reading rasterfile data.\n");
            return ERROR;
         }
         if (odd_byte)
            (void) getc(fp);
      }

   image->w = w;
   image->h = h;
   image->d = d;

   (void) printf("%ix%i depth %i.\n", w, h, d);

   (void) fclose(fp);

   return OK;
}

#define RLE_ESC 128 /* Escape character for RLE encoding */

/* Useful macro */

#define PUT(c) {++i; if (!odd_byte || (odd_byte && i % w)) data[p++] = (c);}

static int read_rle_data(fp, l, w, odd_byte, data)
FILE *fp;
int l, w, odd_byte;
unsigned char *data;
{
   /* Reads RLE data on fp */

   /* NOTE: can't check for EOF since byte value 255 (-1) is valid data */

   int i, p, nc;
   unsigned char c;

   i = p = 0;

   /* It's a bit complicated... */

   while (i < l) {
      if ((c = getc(fp)) == RLE_ESC) {
         nc = getc(fp);
         if (nc == 0)
            PUT(RLE_ESC)
         else if (nc == 1) {
            if ((c = getc(fp)) != RLE_ESC) {
               (void) fprintf(stderr, "-- Corrupt RLE file.\n");
               return ERROR;
            }
            PUT(RLE_ESC)
            PUT(RLE_ESC)
         }
         else
            for (c = getc(fp); nc >= 0; nc--)
               PUT(c)
      }
      else
         PUT(c)
   }

   return OK;
}

#undef PUT
#undef RLE_ESC

/* loadimages.c */
