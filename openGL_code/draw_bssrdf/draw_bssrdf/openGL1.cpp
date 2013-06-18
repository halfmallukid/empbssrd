/*
 * modified with an online code.
 * sample code with 3D scene. no shader.
 * use keyboard to control the user input. see function keyboard_callback
 */
#include <windows.h>
#include <string.h>
#include <GL\glut.h>
#include <GL\gl.h>
#include <iostream>
#include <fstream>
#include<math.h>
void dhsv2rgb(double h, double s, double v, 
              double *r, double *g, double *b) ;
//float* imgoutput(int width, int height,  Colortype *pixel[]);

typedef struct {
  float ti, g, r, ts, a,n; // useful information

}dataInput;
struct Colortype1{

  float rgb[3];
  };
float* imgoutput(int width, int height,  Colortype1 *pixel[]){

	 float* pixels = new float[width*height*3];
		for(int j=0;j<height;j++)
			for(int i=0;i<width;i++)
			{
				for(int x=0;x<3;x++){

					pixels[(j*width+i)*3+x]=pixel[width-1-i][j].rgb[x];
					//*colRange*2.0-colRange;
				}

			}
		return pixels;
}


GLfloat UpwardsScrollVelocity = -150.0;
float view=20.0;

char quote[7][80];
int numberOfQuotes=0,i;

//*********************************************
//*  glutIdleFunc(timeTick);                  *
//*********************************************

void timeTick(void)
{
    if (UpwardsScrollVelocity< -600)
        view-=0.000011;
    if(view < 0) {view=20; UpwardsScrollVelocity = -10.0;}
    //  exit(0);
    UpwardsScrollVelocity -= 0.015;
  glutPostRedisplay();

}
dataInput userInput;
float* wholepic(dataInput userInput, int width,int height);
void PPMoutput(dataInput userInput, int width,int height);
void readFitData();
//FitData  lerp_user_input(dataInput user_input);

int width=500;
 int height=500;

 void draw_legend(int l_height,int l_length,Colortype1 **legend_int)
{
	double r=0;
	double g = 0;
	double b = 0;
	double h = 0;
	double s = 0;
	double v = 0;
	for(int i=0;i<l_height;i++)
	{
		for(int j=0;j<l_length;j++)
		{
			double fj = j+1;
			double flength =l_length;
			h = fj/flength;
			s = 1;
			v = 1;
			dhsv2rgb(h,s,v,&r,&g,&b);
			legend_int[i][j].rgb[0] = r;
			legend_int[i][j].rgb[1] = g;
			legend_int[i][j].rgb[2] = b;
		}
	}

}
 
void DrawImg()
{

     glMatrixMode(GL_MODELVIEW);
    // clear the drawing buffer.
  //  glClear(GL_COLOR_BUFFER_BIT);
//     glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
   glLoadIdentity();
        glTranslatef(0.0,0.0,-10.5);

		int l_height = 100;
		int l_length = 100;

        float* pixels = wholepic(userInput,width,height);
		Colortype1* legend_int[100];
		for(int i=0;i<l_height;i++)
			legend_int[i]=new Colortype1[l_length];

		draw_legend(l_height,l_length,legend_int);
	
		float *l_pixels = imgoutput(l_length,l_height,legend_int);

        glPixelStorei(GL_UNPACK_ALIGNMENT,2);
		glDrawPixels(width, height, GL_RGB,GL_FLOAT, pixels);
		glPixelStorei(GL_UNPACK_ALIGNMENT,3);
		glTranslatef(0,0,0);
		glDrawPixels(l_length,l_height,GL_RGB,GL_FLOAT,l_pixels);

}



//*********************************************
//* printToConsoleWindow()                *
//*********************************************

void printToConsoleWindow()
{
    int l,lenghOfQuote, i;

    for(  l=0;l<numberOfQuotes;l++)
    {
        lenghOfQuote = (int)strlen(quote[l]);

        for (i = 0; i < lenghOfQuote; i++)
        {
         // cout<<quote[l][i];
        }
        //  cout<<endl;
    }

}

//*********************************************
//* RenderToDisplay()                       *
//*********************************************

void RenderToDisplay()
{
    int l,lenghOfQuote, i;

    glTranslatef(0.0, -100, UpwardsScrollVelocity);
    glRotatef(-20, 1.0, 0.0, 0.0);
    glScalef(0.1, 0.1, 0.1);



    for(  l=0;l<numberOfQuotes;l++)
    {
        lenghOfQuote = (int)strlen(quote[l]);
        glPushMatrix();
        glTranslatef( 1000, (l*400), 0.0);
		glScalef(0.5,0.5,0);
		glLineWidth(0.1);
        for (i = 0; i < lenghOfQuote; i++)
        {
            glColor3f((UpwardsScrollVelocity/10)+300+(l*10),(UpwardsScrollVelocity/10)+300+(l*10),0.0);
			//glutStrokeWidth(GLUT_STROKE_ROMAN,10);
			glutStrokeCharacter(GLUT_STROKE_ROMAN, quote[l][i]);
			
        }
        glPopMatrix();
    }
    DrawImg();
}
//*********************************************
//* glutDisplayFunc(myDisplayFunction);       *
//*********************************************

void myDisplayFunction(void)
{
  glClear(GL_COLOR_BUFFER_BIT);
  glLoadIdentity();
  gluLookAt(0.0, 30.0, 100.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  RenderToDisplay();
  glutSwapBuffers();
}
//*********************************************
//* glutReshapeFunc(reshape);               *
//*********************************************

void reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, 1.0, 1.0, 3200);
  glMatrixMode(GL_MODELVIEW);
}

//*********************************************
//* int main()                                *
//*********************************************

void keyboard_callback( unsigned char key, int x, int y )
{
	if(key=='o') {
		PPMoutput(userInput, width,height);
		return;
	}
	switch ( key ) {

	   case 'y':
					userInput.ti-=0.15;
					sprintf(quote[0],"theta i=%.1f",userInput.ti*180/3.14);
			    	break;

	   case 't':
					userInput.ti+=0.15;
					sprintf(quote[0],"theta i=%.1f",userInput.ti*180/3.14);
					break;
	    case 'd':
				userInput.ts-=0.15;
				sprintf(quote[1],"theta s=%.1f",userInput.ts*180/3.14);
		    	break;

		case 's':
				userInput.ts+=0.15;
				sprintf(quote[1],"theta s=%.1f",userInput.ts*180/3.14);
				break;
		case 'r':
			userInput.r-=0.1;
			sprintf(quote[2],"r=%g",userInput.r);
	    	break;

		case 'e':
			userInput.r+=0.1;
			sprintf(quote[2],"r=%g",userInput.r);
			break;
		case 'b':
			if(userInput.a <= 0.01)
				userInput.a = userInput.a;
			else
			userInput.a-=0.1;

			sprintf(quote[3],"a=%g",userInput.a);
			break;

		case 'a':
			if(userInput.a+0.1 > 0.99)
				userInput.a = userInput.a;
			else
			userInput.a+=0.1;
			sprintf(quote[3],"a=%g",userInput.a);
			break;
		case 'h':
					if(userInput.g <= -0.9)
						userInput.g = userInput.g;
					else
					userInput.g-=0.1;

					sprintf(quote[4],"g=%g",userInput.g);
					break;

		case 'g':
					if(userInput.g >=0.99)
						userInput.g = userInput.g;
					else
					userInput.g+=0.1;

					sprintf(quote[4],"g=%g",userInput.g);
					break;

		case 'n':
					userInput.n-=0.1;
					sprintf(quote[5],"n=%g",userInput.n);
					break;

		case 'f':
					userInput.n+=0.1;
					sprintf(quote[5],"n=%g",userInput.n);
					break;
		case 033:  // octal ascii code for ESC

			exit( 0 );
			break;

    }
	glutPostRedisplay();
}

int main( int argc, char **argv )
{
	readFitData();

	userInput.ti=60*3.1415926/180;
	userInput.ts=60*3.1415926/180;
	userInput.r=0.8;
	userInput.a=0.9;
	userInput.g=-0.3;
	userInput.n=1.4;
	glutInit( &argc, argv );
	strcpy(quote[0],"theta i=60");
    strcpy(quote[1],"theta s=60");
    strcpy(quote[2],"r=0.8");
    strcpy(quote[3],"a=0.9");
    strcpy(quote[4],"g=-0.3");
    strcpy(quote[5],"n=1.4");
    numberOfQuotes=6;

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(700, 500);
    glutCreateWindow("BSSRDF");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glLineWidth(3);

    glutDisplayFunc(myDisplayFunction);
    glutReshapeFunc(reshape);
    glutKeyboardFunc( keyboard_callback );
   // glutIdleFunc(timeTick);

    glutMainLoop();

    return 0;
}

