#include <GL/glut.h>

GLfloat x=-0.2,y=0;
static GLfloat SPEED=0.00002;


void drawMove(void)
{	 
     glClearColor(1.0f, 1.0f, 1.0f, 0.5f);
     glClear(GL_COLOR_BUFFER_BIT);
	 glColor3f(0.0f,0.0f,0.0f);
     glBegin(GL_LINES);
		glVertex2f(0,0.2);
		glVertex2f(0,-0.2);
	 glEnd();
	 glColor3f(1.0f,0.0f,0.0f);
	 glPointSize(5.0f);
     glBegin(GL_POINTS);
		glVertex2f(x,y);
	 glEnd();
	 glFlush();
     glutSwapBuffers();
}


void move(void)
{
	 if(x+SPEED>=-0.01f && x+SPEED<=0.01f && y<=0.2f && y>=-0.2f)
		 y+=SPEED;
	 else
		 x+=SPEED;
     drawMove();
}

int main(int argc, char *argv[])
{
     glutInit(&argc, argv);
     glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
     glutInitWindowPosition(100, 100);
     glutInitWindowSize(800, 600);
     glutCreateWindow("");
     glutDisplayFunc(&drawMove);
	 glutIdleFunc(&move);
     glutMainLoop();
     return 0;
}
