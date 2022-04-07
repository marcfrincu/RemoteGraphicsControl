/*
  Remote Graphic Controller API
  Created by Marc Frincu 
  Email: marc@ieat.ro
  May 9th 2007
*/

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <jpeglib.h>
#include <GL/gl.h> 
#include <GL/glu.h>

/*
API variables
*/

struct Operations
{
	float value;
	char name[100];

} operation[11];

char *taskId;
long lastTime;
int lastId;
int okShot;
int timeElapsed;
int timeLastTask;
float xrotate = 0, yrotate = 0, zrotate = 0, xtranslate = 0, ytranslate = 0, ztranslate = 0, zoom = 0;
float rgcX = 0.0f, rgcY = 0.0f, rgcZ = 25.0f;
float rgcLx = 0.0f, rgcLy = 0.0f, rgcLz = 0.0f;
float rgcDeltaAngle = 0.0f;
int rgcDeltaMove = 0;
int rgcMouseDownX = -1;
int rgcMouseDownY = -1;
int rgcKeyUnicode = -1;

void rgcSetTaskId(char* newTaskId)
{
     taskId = newTaskId;
}

long rgcGetTime()
{
     time_t timer;
     return (long)(time(&timer)); 
}

long rgcGetTimeLastTask()
{
     time_t timer;
     return (long)(time(&timer)); 
}

/*
This function reads the task file written by the PHP script after it received the task request from the client

INFO:
      
The file name is equal to the unique session id computed by the PHP script
The task file has the following structure (line items are separated by tabs \t):

xrotate  10
yrotate  0
zrotate -100
xtranslate 10
ytranslate 0
ztranslate 0
zoom -100

*/
void rgcReadTaskFile(char *file)
{
     FILE *f;
	 char line[255], string[255];
	 char seps[]   = " \t\n";
	 char *token;     
	 int i = 0, j = 0;
	 
     if ((f = fopen(file, "r")) == NULL)
	 {
        okShot = 0;
	    printf("Error opening task file!");
	 }
     else
     {
        fseek (f, 0, SEEK_END);
        int size = ftell (f);
        fseek (f, 0, SEEK_SET);
        if (0 == size)
        {
           okShot = 0;
        }
        else
        {
            timeLastTask = rgcGetTimeLastTask();
            okShot = 1;

        	 while (!feof(f))
        	 {
        	    fgets(line,255,f);
        	
        		strcpy(string, line);
        		token = strtok(string, seps);
        
        	//	for (j=0; j<strlen(token);j++)
        		//	operation[i].name[j] = token[j]; 
        	    sprintf(operation[i].name, "%s", token);
                token = strtok( NULL, seps );
        		operation[i++].value = atof(token); 
        		printf("%s %f\n" , operation[i-1].name, operation[i-1].value);
            }
            fclose(f);
            
            remove(file);
            f = fopen(file, "w+");
        }
    }
}
    
/*
This function is used to capture a JPG screenshot of the OpenGL scene
*/    
int rgcTakeScreenshotJPG(unsigned int width, unsigned int height, char *path, int quality)
{
    int ret = 0;
   
    struct jpeg_compress_struct cinfo; // the JPEG OBJECT
  
    struct jpeg_error_mgr jerr; // error handler struct
  
    unsigned char *row_pointer[1]; // pointer to JSAMPLE row[s]
  
    GLubyte *pixels = 0, *flip = 0;
  
    FILE *shot;
  
    int row_stride; // width of row in image buffer
  
    if((shot = fopen(path, "wb")) != NULL) // jpeg file
    { 
  
        // initializatoin
      
        cinfo.err = jpeg_std_error(&jerr); // error handler
      
        jpeg_create_compress(&cinfo); // compression object
      
        jpeg_stdio_dest(&cinfo, shot); // tie stdio object to JPEG object
      
        row_stride = width * 3;
      
        pixels = (GLubyte *)malloc(sizeof(GLubyte)*width*height*3);
      
        flip = (GLubyte *)malloc(sizeof(GLubyte)*width*height*3);
      
        if (pixels != NULL && flip != NULL) 
        {
      
            // save the screen shot into the buffer
            //glReadBuffer(GL_FRONT_LEFT);
          
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
          
            // give some specifications about the image to save to libjpeg
            cinfo.image_width = width;
            cinfo.image_height = height;
            cinfo.input_components = 3; // 3 for R, G, B
            cinfo.in_color_space = JCS_RGB; // type of image
 
            jpeg_set_defaults(&cinfo);
            jpeg_set_quality(&cinfo, quality, TRUE);
            jpeg_start_compress(&cinfo, TRUE);
          
            // OpenGL writes from bottom to top.
            // libjpeg goes from top to bottom.
            // flip lines.
        
            int y = 0;
            for (y=0;y<height;y++) 
            {
                int x = 0;
                for (x=0;x<width;x++) 
                {
                      flip[(y*width+x)*3] = pixels[((height-1-y)*width+x)*3];
                      flip[(y*width+x)*3+1] = pixels[((height-1-y)*width+x)*3+1];
                      flip[(y*width+x)*3+2] = pixels[((height-1-y)*width+x)*3+2];
                }
            }
          
            // write the lines
            while (cinfo.next_scanline < cinfo.image_height) 
            {
                  row_pointer[0] = &flip[cinfo.next_scanline * row_stride];
                  jpeg_write_scanlines(&cinfo, row_pointer, 1);
            }         
          
            ret = 1;
          
            // finish up and free resources
            jpeg_finish_compress(&cinfo);
            jpeg_destroy_compress(&cinfo);
        }
      
        fclose(shot);
    }
  
    if(pixels != 0)
        free(pixels);
  
    if(flip!=0)
        free(flip);
  
    return ret;
} 

/*
This function is used to capture a BMP screenshot of the OpenGL scene
*/
void rgcTakeScreenshotBMP(int windowWidth, int windowHeight, char* filename)
{
     glReadBuffer(GL_BACK);
     byte*  bmpBuffer = (byte*)malloc(windowWidth*windowHeight*3);

     if(!bmpBuffer)
                   return;
                   
     glReadPixels((GLint)0, (GLint)0, (GLint)windowWidth-1, (GLint)windowHeight-1, GL_RGB,  GL_UNSIGNED_BYTE, bmpBuffer);
     
     FILE *filePtr = fopen(filename,  "wb");
     if (!filePtr)
        return;
        
     BITMAPFILEHEADER  bitmapFileHeader;
     BITMAPINFOHEADER  bitmapInfoHeader;
     
     bitmapFileHeader.bfType = 0x4D42;  //"BM"
     bitmapFileHeader.bfSize = windowWidth*windowHeight*3;
     bitmapFileHeader.bfReserved1 = 0;
     bitmapFileHeader.bfReserved2 = 0;
     bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +  sizeof(BITMAPINFOHEADER);
     bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
     bitmapInfoHeader.biWidth = windowWidth-1;
     bitmapInfoHeader.biHeight = windowHeight-1;
     bitmapInfoHeader.biPlanes = 1;
     bitmapInfoHeader.biBitCount = 24;
     bitmapInfoHeader.biCompression =  BI_RGB;
     bitmapInfoHeader.biSizeImage = 0;
     bitmapInfoHeader.biXPelsPerMeter = 0; // ?
     bitmapInfoHeader.biYPelsPerMeter = 0; //  ?
     bitmapInfoHeader.biClrUsed = 0;
     bitmapInfoHeader.biClrImportant = 0;
     
     fwrite(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1,  filePtr);
     fwrite(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1,  filePtr);
     fwrite(bmpBuffer, windowWidth*windowHeight*3, 1,  filePtr); 
     fclose(filePtr);
     
     free(bmpBuffer);
}

void rgcManageIdle(void)
{
     if (rgcGetTime() - lastTime > 1)
     {
        rgcReadTaskFile(taskId);
        lastTime = rgcGetTime();
     }
     
     if (rgcGetTimeLastTask() - timeLastTask > 120)
     {
          exit(2);
     }
}

void rgcOrientMe(float ang) {
	rgcLx = sin(ang);
	rgcLz = -cos(ang);
	glLoadIdentity();
	gluLookAt(rgcX, rgcY, rgcZ, 
		      rgcX + rgcLx, rgcY + rgcLy, rgcZ + rgcLz,
			  0.0f,1.0f,0.0f);
printf("\nRot %f %f %f %f %f %f", rgcX, rgcY, rgcZ, rgcX + rgcLx, rgcY + rgcLy, rgcZ + rgcLz);
}

void rgcMoveMeFlat(int i) {
	rgcX = rgcX + i*(rgcLx)*0.1;
	rgcZ = rgcZ + i*(rgcLz)*0.1;
	glLoadIdentity();
	gluLookAt(rgcX, rgcY, rgcZ, 
		      rgcX + rgcLx, rgcY + rgcLy, rgcZ + rgcLz,
			  0.0f,1.0f,0.0f);
printf("\nMove %f %f %f %f %f %f", rgcX, rgcY, rgcZ, rgcLx, rgcLy, rgcLz);			  
}

void rgcManageKeys(int unicode)
{
        /*key unicodes*/
     	switch (unicode) {
        /*left key*/
        case 37 : 
			rgcDeltaAngle += -0.1f; rgcOrientMe(rgcDeltaAngle);break;
        /*right key*/
		case 39 : 
			rgcDeltaAngle += 0.1f; rgcOrientMe(rgcDeltaAngle);break;
		/*up key*/
		case 38 : 
			rgcDeltaMove = 10; rgcMoveMeFlat(rgcDeltaMove);break;
		/*down key*/	
		case 40 : 
			rgcDeltaMove = -10; rgcMoveMeFlat(rgcDeltaMove);break;
		/*Other keys can be added here*/
        }
}

void rgcManageOperations(void)
{

     int i = 0;
     rgcMouseDownX = -1;
     rgcMouseDownY = -1;
     rgcKeyUnicode = -1;
     for (i=0; i<11; i++)
     {
         if (strcmp(operation[i].name, "xrotate") == 0)
         {
            if (okShot == 1)
            {
               xrotate += operation[i].value;
            }
         }
         if (strcmp(operation[i].name, "yrotate") == 0)
         {
            if (okShot == 1)
            {
               yrotate += operation[i].value;
            }
         }
         if (strcmp(operation[i].name, "zrotate") == 0)
         {
            if (okShot == 1)
            {
               zrotate += operation[i].value;
            }
         }
         if (strcmp(operation[i].name, "xtranslate") == 0)
         {
            if (okShot == 1)
            {
               xtranslate += operation[i].value;
            }
         }
         if (strcmp(operation[i].name, "ytranslate") == 0)
         {
            if (okShot == 1)
            {
               ytranslate += operation[i].value;
            }
         }
         if (strcmp(operation[i].name, "ztranslate") == 0)
         {
            if (okShot == 1)
            {
               ztranslate += operation[i].value;
            }
         }
         if (strcmp(operation[i].name, "zoom") == 0)
         {
            zoom = operation[i].value; 
         }
         if (strcmp(operation[i].name, "key") == 0)
         {
             if ((okShot == 1) && ((int)operation[i].value != -1))
             {
                rgcManageKeys((int)operation[i].value);
                rgcKeyUnicode = (int)operation[i].value;
                operation[i].value = -1;
             }
         }
         if (strcmp(operation[i].name, "mousex") == 0)
         {
            if (okShot == 1)
            {
               rgcMouseDownX = (int)operation[i].value;
            }
         }
         if (strcmp(operation[i].name, "mousey") == 0)
         {
            if (okShot == 1)
            {
               rgcMouseDownY = (int)operation[i].value;
            }
         }

         if (strcmp(operation[i].name, "quit") == 0)
         {
            printf("%d\n", operation[i].value);
            if (operation[i].value == 1)
               exit(2); 
         }
        // printf("%d\n" , i);
        // getch();
     }
/*     printf("%f\n", xrotate);
     printf("%f\n", yrotate);
     printf("%f\n", zrotate);
     printf("%f\n", xtranslate);
     printf("%f\n", ytranslate);
     printf("%f\n", ztranslate);
*/
     glRotatef(xrotate, 1.0, 0.0, 0.0);
     glRotatef(yrotate, 0.0, 1.0, 0.0);
     glRotatef(zrotate, 0.0, 0.0, 1.0);
     glTranslatef(xtranslate, ytranslate, ztranslate);
     glScalef(zoom, zoom, zoom);
}

void rgcGetAppStartTime()
{
     lastTime = rgcGetTime(); 
}

void rgcPrintScene(void)
{
     if (okShot == 1)
     {
        //takeScreenshotJPG(300, 300, "test.jpg", 100);
        char name[50];
        sprintf(name, "dir%s/test%d.bmp", taskId, lastId);
        rgcTakeScreenshotBMP(300, 300, name);
        okShot = 0;
        lastId++;
     }

}
