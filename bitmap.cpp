
/*	Important notes for usage:

	"bmpGetColorEXT ();"
   This get the address of the palette extension used by the bitmap reader.
   The address it creates is _only_ valid within the scope of bitmap.cpp.  This
   means that any calls to glColorTableEXT _must_ occur within this module.
   That is why the function bmpChangePalette is needed (see below).
   Should be called before any other bmpXXXXXX function in your "Init" function.
	-----------------------------------------------------------------------------

	"bmpLoadImage (file_name, &image, mode);"

   EG:
   bmpLoadImage ("wall_light2, &image, BMP_LUMINANCE | BMP_FREE);

   Takes a file name and the address of an image structure.  It will load the
   image and palette (if it has one) into the variable 'image'.
   The 'mode' argument can take multiple tokens.
   Valid 'modes' so far are:
   BMP_FREE     -- This auto frees the image after passing it to OpenGL.  Useful
                   if you don't need to reuse the palette in the image or if
                   its an RGB image.

   BMP_LUMINANCE - This forces an 8bit greyscale image to be uploaded as a
                   GL_LUMINANCE texture, useful for saving memory, lightmaps,
                   clouds, that type of thing.

   BMP_ALPHA    -- This forces an 8bit greyscale image to be uploaded as a
                   GL_ALPHA texture, we need this because BMP files don't have
                   an alpha channel.  Looks like I'll be writing a TGA library
                   soon :)
	-----------------------------------------------------------------------------

   "bmpLoadPalette (file_name, palette);"
   Takes a file name and an array of size 768 bytes.  It places the palette
   of the bitmap into the supplied array.  Useful for changing palettes without
   reloading whole images.
	-----------------------------------------------------------------------------

   "bmpChangePalette (palette);"
   Takes the name of a palette and uses glColorTableEXT in the bitmap.cpp module
   to switch the current active palette.
	-----------------------------------------------------------------------------

   "bmpFree (image)"
   Frees an image and its default palette.
	-----------------------------------------------------------------------------

*/


#include <math.h> 	/* for test_bit */
#include "bitmap.h"
#include <GL/glut.h>

#define BIN			2

/* Pointer to OpenGL extension function  */
PFNGLCOLORTABLEEXTPROC glColorTableEXT;

/* Fetch the address of the function 'glColorTableEXT' from the active opengl
 * renderer.  This will probably cause an access violation if the driver
 * doesn't support colour palettes.  Maybe I should check first  :)
 */
void bmpGetColorEXT ( void )
{
   glColorTableEXT = ( PFNGLCOLORTABLEEXTPROC )wglGetProcAddress ( "glColorTableEXT" );
}


/*	Check whether a bit in an integer is 0 or 1
 */
bmpFLAG test_bit ( int Bit, bmpFLAG FlagVar )
{
	if ( FlagVar & bmpFLAG ( pow ( BIN,Bit ) ) )
		return 1;
	else
		return 0;
}


/* Every forth byte in a .bmp palette is for padding purposes only.  This
 * function returns true if a value passed to it is divisable by four.
 */
bool is_pad_byte ( int test_me )
{
	div_t x;

   x = div ( test_me, 4 );
   if ( x.rem == 0 )
   	return true;
   else
   	return false;
}


/* "bmpFlip" ------------------------------------------------------------------
 * Flipping action.  This takes data from the bottom right end and places
 * it into the top left end.  Thus flipping the raw data vertically and
 * horizontally, as well as reversing the BGR byte order into RGB.
 */
void bmpFlip ( image_t *p, GLubyte *temp_data, int channels )
{
	long int temp_index, i;

	temp_index = p->info.width * p->info.height * channels - 1;

   for ( i = 0; i < p->info.width * p->info.height * channels; i++)
   {
     	p->data[i] = temp_data[temp_index];
      temp_index--;
   }
}

/* "bmpChangePalette" ---------------------------------------------------------
 * Internally calls glColorTableEXT so we don't have to share the function
 * pointer across modules
 */
void bmpChangePalette ( GLubyte *palette )
{
	glColorTableEXT ( GL_TEXTURE_2D, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, palette );
}


/* "bmpPassImage" -------------------------------------------------------------
 * This passes the image data to the OpenGL command glTexImage2D.  If the image
 * is 8bit palettised the palette contained with the image is uploaded as a
 * default. Also the 'format' of the image is passed.  Either GL_RGB for 24bit
 * RGB triples or GL_COLOR_INDEX for an image consisting of indices to a
 * palette.
 * GL_LUMINANCE or GL_ALPHA is passed to this function if the user has forced a
 * load of this type.
 */
void bmpPassImage ( image_t *p, GLenum colour_type )
{
   /*  Set OpenGL Texture Image */
   if ( colour_type == GL_COLOR_INDEX )
	{
	 	glColorTableEXT ( GL_TEXTURE_2D, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, p->palette );
     	glTexImage2D    ( GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, p->info.width, p->info.height, 0, colour_type, GL_UNSIGNED_BYTE, p->data );
   }
   else
		glTexImage2D	 ( GL_TEXTURE_2D, 0, colour_type, p->info.width, p->info.height, 0, colour_type, GL_UNSIGNED_BYTE, p->data );
}


/* "bmpSetTexParams" ----------------------------------------------------------
 * This is a wrapper to quickly set texture environment parameters
 */
void bmpSetTexParams ( GLenum filter_type, GLenum application )
{
   /*  Set Filtering type */
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_type );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_type );

   /*  Set Texture Evironment */
   glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, application );
}


/* "bmpAllocate" --------------------------------------------------------------
 * Uses the data contained within the 'image_t' structure to allocate the
 * required amount of memory for the texture and palette.
 * This memory MUST be freed elsewhere.
 */
void bmpAllocate ( image_t *p )
{
	long int	pixels;

   pixels = p->info.width * p->info.height * ( p->info.bit_count / 8 );
  	p->data = ( GLubyte * )malloc ( sizeof ( GLubyte ) * pixels );

   if ( p->info.bit_count == 8 )
   	p->palette = ( GLubyte * )malloc ( sizeof ( GLubyte ) * 768 );
}


/* "bmpFree" -----------------------------------------------------------------
 * Go figure.
 */
void bmpFree ( image_t *p )
{
	if ( p->data != NULL )
   	free ( p->data );

   if ( p->palette != NULL )
   	free ( p->palette );
}


/* "bmpInit" ------------------------------------------------------------------
 * Go figure.
 */
void bmpInit ( image_t *p )
{
	p->palette = NULL;
   p->data    = NULL;
}


/* "bmpGetInfo" ---------------------------------------------------------------
 * Reads the bitmap header to the image structure and checks if its valid.
 * In the future a check for (and functions to handle) compression will be
 * added.  (Also, add better error checking/handling)
 */
void bmpGetInfo ( FILE *file, image_t *p )
{
   fread ( &p->info, ( sizeof( bitmap_header_t ) ), 1, file );

   if ( p->info.type != 'BM' )
  		printf( "This is not a bitmap\n" );

   if ( p->info.planes != 1 )
  		printf( "This has more than one plane\n" );
}


/* "bmpGetPalette" ------------------------------------------------------------
 * Reads the next 1024 bytes, dumping every 4th byte.  This leaves us with 768
 * bytes of BGR palette colours.  We then flip the byte order of each triple
 * to RGB order.
 */
void bmpGetPalette ( FILE *file, GLubyte *palette )
{
   int 		i;
   int 		temp_index = 0;
   GLubyte 	temp;

   /* Get the .bmp palette.  This is an 'RGBQUAD' (4 bytes), we don't need
      the padding byte in the quad so we have to ignore it. */
   for ( i = 0; i < 1024; i++ )
   {
      if ( !is_pad_byte(i+1) )
      {
	      fread ( &palette[temp_index], 1, 1, file );
         temp_index++;
      }
      else
	      fread ( &temp, 1, 1, file );
   }

   /* Flip palette colour bytes from BGR to RGB (phew, I suppose its all
      worth it in the end!)
   */
   for ( i = 0; i < 768; i += 3 )
	{
		temp = palette[i];
		palette[i] = palette[i + 2];
		palette[i + 2] = temp;
	}
}


/* "bmpGetImageData" ----------------------------------------------------------
 * Reads the required number of bytes into a temporary image array.  Calls
 * the flip function, which places the correctly ordered data into the 'image_t'
 * structure.  Finally, it passes the image to OpenGL with the proper colour
 * parameters and sets default environment settings.
 * If the image is 8bit we also fetch a palette.  Palettes, as we know, are
 * always 1024 bytes long in .bmp files.
 */
int bmpGetImageData ( FILE *file, image_t *p, GLenum colour_type )
{
	long int pixels;
   GLubyte  *temp_data;

   /* If this is an 8bit image, read past the palette in the file and collect
      the palette data. */
   if ( p->info.bit_count == 8 )
		bmpGetPalette ( file, p->palette );

   /* If we have a 'forced' image we don't need the palette even though
      its there...  */
   if (( colour_type == GL_LUMINANCE ) || ( colour_type == GL_ALPHA ))
   {
   	free ( p->palette );
      p->palette = NULL;
   }

   pixels = p->info.width * p->info.height * ( p->info.bit_count / 8 );
	temp_data = ( GLubyte * )malloc (sizeof ( GLubyte ) * pixels );
   if ( temp_data == NULL )
   	return 0;

   /* Read the proper number of bytes into the temporary array  */
   fread ( temp_data, p->info.width * p->info.height * ( p->info.bit_count / 8 ), 1, file );

   /* flip the temp_data into our target image array (held in p)   */
   bmpFlip ( p, temp_data, ( p->info.bit_count / 8) );
   free ( temp_data );

   bmpPassImage    ( p, colour_type );
   bmpSetTexParams ( GL_NEAREST, GL_MODULATE );
   return 1;
}


/* "bmpLoadImage" -------------------------------------------------------------
 * The function that opens files, orders memory allocation, decides what to do..
 */
int bmpLoadImage ( char *file_name, image_t *p, bmpFLAG mode )
{
	FILE 		*file;

 	if (( file = fopen ( file_name, "rb" )) == NULL )
   {
  		printf ( "File Not Found : %s\n", file_name );
      fclose ( file );
      return 0;
   }

   bmpInit     ( p );
   bmpGetInfo  ( file, p );
   bmpAllocate ( p );

   /* Load data for 24 bit RGB image */
   if ( p->info.bit_count == 24 )
      bmpGetImageData ( file, p, GL_RGB );

   /* If the user has set the BMP_LUMINANCE flag force load it as such...

   	This takes a standard greyscale palettised 8bit image (BMPs don't do this

      any other way) and loads it as a GL_LUMINANCE image and DOESN'T set any

      palettes.

    */

   if (( p->info.bit_count == 8 ) && ( test_bit ( 3, mode )))

   	bmpGetImageData ( file, p, GL_LUMINANCE );


   else if ( (p->info.bit_count == 8) && (test_bit (2, mode)) )

   	bmpGetImageData ( file, p, GL_ALPHA );


	/* If they haven't, it is just an 8bit image with a palette  */
   else if ( p->info.bit_count == 8 )
   	bmpGetImageData ( file, p, GL_COLOR_INDEX );

	/* Programmer should be responsible for freeing the image?  A bit flag can be
   	set to decide this... If the flag is true then we auto free the image and
      palette.
    */
   if ( test_bit (0, mode ))  /*  mode == BMP_FREE  */
   	bmpFree ( p );

   fclose ( file );
  	return 1;
}


/* "bmpLoadPalette" -----------------------------------------------------------
	Reads a standard bitmap image but only returns the palette (if one existed
   in the image).
   This will allow multiple images to share the same palette (assuming they were
   'drawn' correctly.
   It doesn't have much use yet.  It *will* become more important when I remove
   the 'palette' from the 'image_t' structure...
*/
int bmpLoadPalette (char *file_name, GLubyte *palette)
{
	FILE 		*file;
   bitmap_header_t info;

 	if (( file = fopen ( file_name, "rb" )) ==NULL )
  	{
  		printf ( "File Not Found : %s\n", file_name );
  		return 0;
  	}

   fread ( &info, ( sizeof ( bitmap_header_t )), 1, file );

   if ( info.type != 'BM' )
  		printf ( "This is not a bitmap\n" );

   if ( info.planes != 1 )
  		printf ( "This has more than one plane\n" );

   if ( info.bit_count == 8 )
		bmpGetPalette ( file, palette );


   fclose ( file );
  	return 1;
}
