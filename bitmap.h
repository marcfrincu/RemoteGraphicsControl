
#include <windows.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {      
	WORD 		      type;
   long int       size;
   DWORD		      reserved;
   DWORD		      offset;
   DWORD      		sizeBMI;
   LONG       		width;
   LONG       		height;
   WORD       		planes;
   WORD       	  	bit_count;
   DWORD      		compression;
   DWORD     		SizeImage;
   LONG       		XPelsPerMeter;
   LONG       		YPelsPerMeter;
   DWORD      		ClrUsed;
   DWORD      		ClrImportant;
  } bitmap_header_t;

typedef struct {
	bitmap_header_t 	info;
   GLubyte 				*data;      /* Image data */
   GLubyte 				*palette;   /* Default palette - if required */
} image_t;


/*  Definitions for bitflags  */
typedef unsigned short int bmpFLAG;

#define  BMP_DEFAULT		0    /* In case we don't want any parameters */
#define 	BMP_FREE			1    /* Bit flag 0 */
#define	BMP_NO_FREE		2    /* Bit flag 1 */
#define	BMP_ALPHA		4    /* Bit flag 2 */
#define	BMP_LUMINANCE	8    /* Bit flag 3 */


void bmpFree (image_t *p);

void bmpGetColorEXT (void);

void bmpSetTexParams (GLenum filter_type, GLenum application);

int bmpLoadImage (char *file_name, image_t *p, bmpFLAG mode);

int bmpLoadPalette (char *file_name, GLubyte *palette);

void bmpChangePalette (GLubyte *palette);
