#include "mesh.h"
#include <GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <ctime>
#ifndef RES
#define RES 1
#endif


int th=-30;    //  Azimuth of view angle
int ph=+30;    //  Elevation of view angle
double dim=2;   // Dimension of orthogonal box
int nx = 4;    //torus divisions dim1
int ny = 4;    //torus divisions dim2
float last_turn = std::clock();
int view_mode = 0;
int NUM_MODES = 2;
Mesh M("./objects/small_bunny.obj");

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  // Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

void display()
{
    //  Clear the image
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST); 
   //  Reset previous transforms
   glLoadIdentity();
   //  Set view angle
   glRotated(ph,1,0,0);
   glRotated(th,0,1,0);

    //  Draw axes in white
   glColor3f(1,1,1);
   glBegin(GL_LINES);
   glVertex3d(0,0,0);
   glVertex3d(1,0,0);
   glVertex3d(0,0,0);
   glVertex3d(0,1,0);
   glVertex3d(0,0,0);
   glVertex3d(0,0,1);
   glEnd();
   //  Label axes
   glRasterPos3d(1,0,0);
   Print("X");
   glRasterPos3d(0,1,0);
   Print("Y");
   glRasterPos3d(0,0,1);
   Print("Z");

   M.displayPts(view_mode);
   M.displayTris(view_mode);

   glFlush();
   glutSwapBuffers();

};

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase azimuth by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease azimuth by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  Keep angles to +/-360 degrees
   th = (360 + (th%360)) % 360;
   ph = (360 + (ph%360)) % 360;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
};

void normal(unsigned char key, int x, int y)
{
   if(key=='m'){
      view_mode++;
      view_mode %= NUM_MODES;      
   }
   if(view_mode != 0){
      M.reset_pts();
   }
   glutPostRedisplay();
};

void idle()
{
   std::clock_t t;
   t = std::clock();
   if(view_mode==0){
      if(((t- M.last_update)/ (double) CLOCKS_PER_SEC) > 0.1){
         M.update_pts();
      }
      if(((t- M.last_reset)/ (double) CLOCKS_PER_SEC) > 5){
         M.reset_pts();
      }
   }
   if((t - last_turn)/ (double) CLOCKS_PER_SEC > 0.02){
      th = th + 1;
      th = (360 + (th%360)) % 360;
      last_turn = t;
   }
   glutPostRedisplay();
   
};


/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Set the viewport to the entire window
   glViewport(0,0, RES*width,RES*height);
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Orthogonal projection box adjusted for the
   //  aspect ratio of the window
   double asp = (height>0) ? (double)width/height : 1;
   glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create window
   glutInitWindowSize(700,700);
   glutCreateWindow("Mean Curvature Flow");
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Pass control to GLUT for events
   glutKeyboardFunc(normal);
   // Redisplay even when idle
   glutIdleFunc(idle);
   //  Pass control to GLUT for events
   glutMainLoop();
   //  Return to OS
   return 0;

}