/*
  Remote Graphic Controller API
  Created by Marc Frincu 
  Email: marc@ieat.ro
  May 9th 2007
*/

char toShow[100];

#define GLUT_DISABLE_ATEXIT_HACK

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h> 
#include <GL/glu.h>
#include <GL/glut.h>
//#include "rgcapi.h"

extern float rgcX, rgcY, rgcZ, rgcLx, rgcLy, rgcLz;
extern int rgcMouseDownX, rgcMouseDownY;

/*
Demo variables
*/    
GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};  /* Red diffuse light. */
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */
GLfloat n[6][3] = {  /* Normals for the 6 faces of a cube. */
  {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };
GLint faces[6][4] = {  /* Vertex indices for the 6 faces of a cube. */
  {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
  {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };
GLfloat v[8][3];  /* Will be filled in with X,Y,Z vertexes. */

int angle = 0;                 


/*
float x=0.0f,y=0.0f,z=25.0f;
float lx=0.01f,ly=0.01f,lz=0.01f;
float deltaAngle = 0.0f;
int deltaMove = 0;

void orientMe(float);
void moveMeFlat(int);
*/
#define BUFSIZE 512
GLuint selectBuf[BUFSIZE];

void processHits (GLint hits, GLuint buffer[])
{
   unsigned int i, j;
   GLuint ii, jj, names, *ptr;

   printf ("hits = %d\n", hits);
   ptr = (GLuint *) buffer;
   for (i = 0; i < hits; i++) { /*  for each hit  */
      names = *ptr;
      printf (" number of names for this hit = %d\n", names);
         ptr++;
      printf("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
      printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
      printf ("   names are ");
      for (j = 0; j < names; j++) { /*  for each name */
         printf ("%d ", *ptr);
         if (j == 0)  /*  set row and column  */
            ii = *ptr;
         else if (j == 1)
            jj = *ptr;
         ptr++;
      }
      printf ("\n");
   }
}

void drawText(int x, int y, const char * message)
{
	/* raster pos sets the current raster position
	 * mapped via the modelview and projection matrices
	 */
	glRasterPos2i((GLfloat)x, (GLfloat)y);

	/*
	 * write using bitmap and stroke chars
	 */
	while (*message) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *message++);
	}
}

void display2DText(int x, int y, const char * mesg, int width, int height) {
	
	/* save lighting, depth buffer and colour attributes - use Client for speed */
	glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity(); 	/* clear projection matrix */
	gluOrtho2D(0,(GLdouble)width,0,(GLdouble)height);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();	/* clear modelview matrix */

	glColor3f(0.3, 0.0, 1.0); /* text colour */
	drawText(x,y,mesg);
	glPopMatrix();		/* restore modelview matrix */

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();		/* restore projection matrix */
	
	glPopAttrib();		/* restore attributes */

	glMatrixMode(GL_MODELVIEW);	/* for safty! */
}

void drawBox(void)
{
  int i;

  for (i = 0; i < 6; i++) {
    glBegin(GL_QUADS);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][0]][0]);
    glVertex3fv(&v[faces[i][1]][0]);
    glVertex3fv(&v[faces[i][2]][0]);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}
void draw(GLenum mode)
{
 if (mode == GL_SELECT)
 {
          glLoadName(1);
                    glLoadName(2);
          }
     glPushMatrix();
        rgcManageOperations();
        if (mode == GL_SELECT)
        glPushName(1);
        drawBox();
        if (mode == GL_SELECT)
        glPopName();
        glPushMatrix();
                glTranslatef(10,-10,10);
                if (mode == GL_SELECT)
                glPushName(2);
                glutSolidSphere(10,10,10);
                if (mode == GL_SELECT)
                glPopName();
        glPopMatrix();
     if (rgcMouseDownX > 0 && rgcMouseDownY > 0 && rgcMouseDownX < 301 && rgcMouseDownY < 301)
     {
          sprintf(toShow,"Mouse click at (%d,%d)",rgcMouseDownX, rgcMouseDownY);
          display2DText(5,10, toShow, 300, 300);  
     }
     glPopMatrix();
 }

void display(void)

{
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glClearColor(1.0f,1.0f,1.0f,1.0f);
     
     draw(GL_RENDER);
     glutSwapBuffers();
     rgcPrintScene();
}

void init(void)
{
  /* Setup cube vertex data. */
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;

  /* Enable a single OpenGL light. */
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  /* Use depth buffering for hidden surface elimination. */
  glEnable(GL_DEPTH_TEST);

  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 40.0,
    /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 100.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0,0,25,  /* eye is at (0,0,5) */
    0,0,0,      /* center is at (0,0,0) */
    0.0, 1.0, 0.);      /* up is in positive Y direction */
    
// gluLookAt(rgcX, rgcY, rgcZ,  /* eye is at (0,0,5) */
 //   rgcX + rgcLx, rgcY + rgcLy, rgcZ + rgcLz,      /* center is at (0,0,0) */
 //   0.0, 1.0, 0.);      /* up is in positive Y direction */

  /* Adjust cube position to be asthetic angle. */
  glTranslatef(0.0, 0.0, -1.0);
  glRotatef(60, 1.0, 0.0, 0.0);
  glRotatef(-20, 0.0, 0.0, 1.0);
}

void Idle(void)
{
     rgcManageIdle();
     glutPostRedisplay();
}


void pick(int button, int state, int x, int y)
{
   GLuint selectBuf[BUFSIZE];
   GLint hits;
   GLint viewport[4];

   if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
      return;

   glGetIntegerv (GL_VIEWPORT, viewport);

   glSelectBuffer (BUFSIZE, selectBuf);
   (void) glRenderMode (GL_SELECT);

   glInitNames();
   glPushName(0);

   glMatrixMode (GL_PROJECTION);
   glPushMatrix ();
   glLoadIdentity ();
/*  create 5x5 pixel picking region near cursor location      */
   gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 
                  5.0, 5.0, viewport);
   gluOrtho2D (0.0, 3.0, 0.0, 3.0);
   draw (GL_SELECT);

   glMatrixMode (GL_PROJECTION);
   glPopMatrix ();
   glFlush ();

   hits = glRenderMode (GL_RENDER);
   processHits (hits, selectBuf);
   glutPostRedisplay();
} 

/*
void orientMe(float ang) {
	lx = sin(ang);
	lz = -cos(ang);
	glLoadIdentity();
	gluLookAt(x, y, z, 
		      x + lx,y + ly,z + lz,
			  0.0f,1.0f,0.0f);
			  printf("Rot %f %f %f %f %f %f", x, y, z, lx, ly, lz);			  			  
}

void moveMeFlat(int i) {
	x = x + i*(lx)*0.1;
	z = z + i*(lz)*0.1;
	glLoadIdentity();
	gluLookAt(x, y, z, 
		      x + lx,y + ly,z + lz,
			  0.0f,1.0f,0.0f);
printf("Move %f %f %f %f %f %f", x, y, z, lx, ly, lz);			  			  
}
*/

void Key(int key, int x, int y)
{
	switch (key) {
        case GLUT_KEY_LEFT : 
			//deltaAngle += -0.01f; orientMe(deltaAngle);break;
			rgcManageKeys(37);break;
		case GLUT_KEY_RIGHT : 
			//deltaAngle += 0.01f; orientMe(deltaAngle);break;
			rgcManageKeys(39);break;
		case GLUT_KEY_UP : 
			//deltaMove = 1; moveMeFlat(deltaMove);break;
			rgcManageKeys(38);break;
		case GLUT_KEY_DOWN : 
//			deltaMove = -1; moveMeFlat(deltaMove);break;
          rgcManageKeys(40);break;
      }
}


int main(int argc, char **argv)
{
    
  rgcSetTaskId(argv[1]);
  rgcReadTaskFile(argv[1]);  
  rgcGetAppStartTime();
    
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize  ( 300, 300 );
  glutCreateWindow("Remote Graphic Controller API test application");
  glutDisplayFunc(display);
    glutSpecialFunc(Key);
  glutIdleFunc(Idle);
  glutMouseFunc(pick);
  init();
  glutMainLoop();
  return 0;
}    


