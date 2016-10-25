// ImageProg.cpp : Defines the entry point for the console application.
//The purpose of this project is to implement two image morphing method:
//The grid approach or feature vector
//OPENGL + GLUT

#include "stdafx.h"
#include <cstdlib>
#include<glut.h>
#include<gl/GL.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include <iomanip>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

int Algorithm = -1; //grid default, //1 is feature

//Global constants
const GLint WINDOW_WIDTH = 1020;//The -1 is to not be directly against the window's edge.
const GLint WINDOW_HEIGHT = 600;//The -1 is to not be directly against the window's edge.
const GLint WINDOW_DEPTH = 300;
const GLfloat PI = 3.14159265;
const GLfloat rDamp = 1.0;//Not used right now

const int offset = 10;
float Time = 0;

//State variables
GLint B_WIDTH = WINDOW_WIDTH/5;
GLint B_HEIGHT = WINDOW_HEIGHT/6;


class RGB{ // holds a color triple, each with 256 possible intensities
    public: unsigned char r,g,b;
};

struct pos{
	int setx,sety; //direction point can move
	int i,j;
	pos(){setx=0;sety=0;};
}; 
pos currPoint;

// The RGBpixmap class stores the number of rows and columns in
// the pixmap, as well as the address of the first pixel in memory:

class RGBpixmap{
  public:
    int nRows, nCols;       // dimensions of the pixmap
    RGB* pixel;             // array of pixels
    int readBMPFile(char* fname);       // read BMP file into this pixmap
    void setTexture(GLuint textureName);
};

//image 1 and image2
RGBpixmap image1,image2,image3;
RGBpixmap InterImage1,InterImage2;
RGBpixmap FinalImage1,FinalImage2;

//
// RGBpixmap.cpp - routines to read a BMP file
//#include "RGBpixmap.h"
//This part is provided by Dr.Cheng
typedef unsigned short ushort;
typedef unsigned long ulong;
fstream inf; // global in this file for convenience
//<<<<<<<<<<<<<<<<<<<<< getShort >>>>>>>>>>>>>>>>>>>>
ushort getShort() //helper function
{ //BMP format uses little-endian integer types
  // get a 2-byte integer stored in little-endian form
    char ic;
    ushort ip;
    inf.get(ic); ip = ic;  //first byte is little one 
    inf.get(ic);  ip |= ((ushort)ic << 8); // or in high order byte
    return ip;
}

//<<<<<<<<<<<<<<<<<<<< getLong >>>>>>>>>>>>>>>>>>>
ulong getLong() //helper function
{  //BMP format uses little-endian integer types
   // get a 4-byte integer stored in little-endian form
    ulong ip = 0;
    char ic = 0;
    unsigned char uc = ic;
    inf.get(ic); uc = ic; ip = uc;
    inf.get(ic); uc = ic; ip |=((ulong)uc << 8);
    inf.get(ic); uc = ic; ip |=((ulong)uc << 16);
    inf.get(ic); uc = ic; ip |=((ulong)uc << 24);
    return ip;
}

//set texture
void RGBpixmap :: setTexture(GLuint textureName)
{
    glBindTexture(GL_TEXTURE_2D,textureName);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,nCols,nRows,0, GL_RGB,
             GL_UNSIGNED_BYTE, pixel);
}

//<<<<<<<<<<<<<<<<<< RGBPixmap:: readBmpFile>>>>>>>>>>>>>
//int RGBpixmap:: readBMPFile(string fname)
int RGBpixmap:: readBMPFile(char* fname)
{  // Read into memory an RGB image from an uncompressed BMP file.
   // return 0 on failure, 1 on success
    //fstream inf; /********************/Commented out by Fuhua Cheng

    //inf.open(fname.c_str(), ios::in|ios::binary); //read binary char's
    inf.open(fname, ios::in|ios::binary); //read binary char's
    if(!inf){ cout << " can't open file: " << fname << endl; return 0;}
    int k, row, col, numPadBytes, nBytesInRow;
    // read the file header information
    char ch1, ch2;
    inf.get(ch1); inf.get(ch2); //type: always 'BM'
    ulong fileSize = getLong();
    ushort reserved1 = getShort();    // always 0
    ushort reserved2 = getShort();    // always 0 
    ulong offBits = getLong();    // offset to image - unreliable
    ulong headerSize = getLong(); // always 40
    ulong numCols = getLong();    // number of columns in image
    ulong numRows = getLong();    // number of rows in image
    ushort planes = getShort();      // always 1 
    ushort bitsPerPixel= getShort();    //8 or 24; allow 24 here
    ulong compression = getLong();      // must be 0 for uncompressed 
    ulong imageSize = getLong();       // total bytes in image 
    ulong xPels = getLong();    // always 0
    ulong yPels = getLong();    // always 0 
    ulong numLUTentries = getLong();    // 256 for 8 bit, otherwise 0 
    ulong impColors = getLong();       // always 0
    if(bitsPerPixel != 24) 
    { // error - must be a 24 bit uncompressed image
        cout << "not a 24 bit/pixelimage, or is compressed!\n"<<endl;
        inf.close(); return 0;
    }
    //add bytes at end of each row so total # is a multiple of 4
    // round up 3*numCols to next mult. of 4
    nBytesInRow = ((3 * numCols + 3)/4) * 4;
    numPadBytes = nBytesInRow - 3 * numCols; // need this many
    nRows = numRows; // set class's data members
    nCols = numCols;
    pixel = new RGB[nRows * nCols]; //make space for array
    if(!pixel) return 0; // out of memory!
    long count = 0;
    char dum;
    for(row = 0; row < nRows; row++) // read pixel values
    {
        for(col = 0; col < nCols; col++)
        {
            char r,g,b;
            inf.get(b); inf.get(g); inf.get(r); //read bytes
            pixel[count].r = r; //place them in colors
            pixel[count].g = g;
            pixel[count++].b = b;
        }
        for(k = 0; k < numPadBytes ; k++) //skip pad bytes at row's end
            inf >> dum;
    }
    inf.close(); return 1; // success
}
//End of RGBpixmap.h
//=============================================================================
//Everything else from this point, unless otherwise commented, is my work.
//(Exception: Algorithm for solving a system of equations)

//A simple struct defined to hold a 2-D point.
struct Point2D
{
	GLfloat x;
	GLfloat y;
	Point2D()  { x=0; y=0; }
	Point2D(GLfloat xc, GLfloat yc)
	{
		x = xc;
		y = yc;
	}
	void round()//Rounds both corrdinates
	{
		x = (GLint)(x+0.5);
		y = (GLint)(y+0.5);
	}
};


//this is a feature line use when scanning horizontal and vertical using grid approach
struct feature_line{
	Point2D P1,P2;
	int count;
	feature_line(){count = 0;};
}; feature_line FeatureLine,FeatureLine2;

//grids points
int gridCols = 6;
int gridRows = 6;

vector< vector<Point2D> > gridPointsSource;
vector< vector<Point2D> > gridPointsDes;

//A struct to hold a rectangular shape, as defined by the bottom left
//  and the upper right corner points.
struct Rect2D
{
	Point2D bottomleft;
	Point2D topright;
	Rect2D()  { bottomleft = Point2D();  topright = Point2D(); }
	Rect2D(Point2D bl, Point2D tr)
	{
		bottomleft = bl;
		topright = tr;
	}
	void operator=( const Rect2D& r )
	{	bottomleft = r.bottomleft;
		topright = r.topright;
	}
};


//A struct to represent a 1-D interval.
// Used by the algorithm that constructs the Arc Length Table
//Also used by the arcLengthTable::getParam function
struct interval
{
	GLfloat a;
	GLfloat b;
	interval()
	{	a=0.0;	b=0.0;}
	interval(GLfloat left, GLfloat right)
	{	a=left;	b=right;}
};

//A struct to implement an Arc Length Table
//The two vectors represent the parameter and arc length
// columns, respectively.
struct arcLengthTable
{
	vector<GLfloat> parameter;
	vector<GLfloat> arcLength;
	void store(GLfloat p, GLfloat s)
	{
		parameter.push_back(p);
		arcLength.push_back(s);
		index++;
	}
	GLfloat getAL(GLfloat param)//THIS IS NOT GUARANTEED TO WORK RIGHT
	{//This isn't really used by this program; it's an artifact
	 //Given an arbitrary in-range parameter value,
	 // return an associated (or interpolated) arc length.
		for(GLint i=0; i<(GLint)parameter.size(); i++)
		{//First, do a lookup
			if(parameter.at(i)==param)
				return arcLength.at(i);
		}
		//If not found, then use linear interpolation to get a value
		GLfloat lastT = parameter.at(0);
		GLfloat thisT;
		GLfloat len_less = -1;
		GLfloat len_more = -1;
		for(GLint i=1; i<(GLint)parameter.size(); i++)
		{
			thisT = parameter.at(i);
			if(lastT<=param && param<=thisT)
			{
				len_less = getAL(lastT);
				len_more = getAL(thisT);
			}
			else
				lastT = thisT;
		}
		if(len_less==-1 && len_more==-1)
			return -1;//An unsuccessful search for some reason
		return len_less + ((param-lastT)/(thisT-lastT))*(len_more-len_less);
	}
	GLfloat getParam(GLfloat arcLen)
	{//Lookups an associated parameter, given an arc length
		/*for(GLint i=0; i<(GLint)arcLength.size(); i++)//Linear search
		{//First, do a lookup
			if(arcLength.at(i)==arcLen)
				return parameter.at(i);
		}*/
		//If not found, then use linear interpolation to get a value

		GLfloat lastLen;
		GLfloat thisLen;
		GLfloat t_less = -1;
		GLfloat t_more = -1;
		interval span = interval(0, arcLength.size());
		GLfloat m = arcLength.size()/2.0;
		bool found = false;
		while(!found)//Binary search
		{
			GLint top_m = (GLint)ceil(m);
			if(top_m-1 < 0  ||  top_m>=(GLint)arcLength.size())
				break;
			lastLen = arcLength.at(top_m-1);
			thisLen = arcLength.at(top_m);
			if(lastLen<=arcLen && arcLen<=thisLen)
			{
				t_less = parameter.at(top_m-1);
				t_more = parameter.at(top_m);
				found = true;
			}
			else if(arcLen < lastLen)
			{
				span = interval(span.a, m);
				m = (m+span.a)/2.0;
			}
			else if(arcLen > thisLen)
			{
				span = interval(m, span.b);
				m = m + (span.b-span.a)/2.0;
			}
			if(!found && top_m==ceil(m))//Too much sub-division
				break;
		}
		if(!found)
			return -1;//Failure

		//Linear interpolation
		return t_less + ((arcLen-lastLen)/(thisLen-lastLen))*(t_more-t_less);
	}
	GLint index;
	void print()//For debugging
	{
		cout << "Index\tt\tLength" << endl;
		for(GLint i=0; i<(GLint)parameter.size(); i++)
		{
			cout << i << "\t" << parameter.at(i) << "\t\t" << arcLength.at(i) << endl;
		}
		cout << endl;
	}
};


//This section is for various state and global variables

vector <Rect2D>buttons; //Specifies the buttons.
ostringstream textOutput;//Global output, sent to the screen by onDisplay()

int inputState;//A general program state
/*
*	0	-	The default start state
*	1	-	source select mode
*	2	-	des select mode
*	3	-	start
*	4	-	stop
*/

GLint menuState;//For the drop-down speed menu
/*
*	0	-	Nothing is shown
*	1	-	Choices for algorithm
*/

vector<Point2D> dataPoints;//The user-defined data points
vector<Point2D> controlPoints;//Interpolated control points
arcLengthTable ALTable;//The Arc Length Table

vector<vector<Point2D>> controlPointsGridSourceRows,controlPointsGridDesRows;
vector<vector<Point2D>> controlPointsGridSourceCols,controlPointsGridDesCols;

//#include <gl/glut.h>
#include <cmath>

//*************************************************************************************
//METHOD FOR APPROXIMATING A SOLUTION TO A SYSTEM OF EQUATIONS
//(with a tridiagonal matrix)
//NOTE: The following section was written by Tim Meyers for CS537.
//  All references herein for the "Textbook" indicate:
// 		"Numerical Analysis: Mathematics of Scientific Computing"
//			3rd ed., Kincaid & Cheney, 2002

/*************************************************************************
* FUNCTION_NAME   : printM
*
* INPUT           : float** A, the matrix to print
*					int n,m, the dimensions of the matrix
*
* OUTPUT          : The contents of matrix A (through cout)
*
* VALUE RETURNED  : None
*
* FUNCTIONS CALLED: None
*
* DESCRIPTION     : A debugging function that prints out a matrix
*
*************************************************************************/
void printM(float** A, int n, int m)
{
	cout << endl;
	for(int i=1; i<=m; i++)//Row
	{
		for(int j=1; j<=n; j++)//Column
			cout << fixed << A[i][j] << " ";
		cout << endl;
	}
	cout << endl;
}

/*************************************************************************
* FUNCTION_NAME   : printV
*
* INPUT           : Point2D* x, the vector to print
*					( or GLfloat* x, the vector to print)
*					int n, the size of the vector
*
* OUTPUT          : The contents of vector x (through cout)
*
* VALUE RETURNED  : None
*
* FUNCTIONS CALLED: None
*
* DESCRIPTION     : A debugging function that prints out a vector.
*
*************************************************************************/
void printV(Point2D* x, int n)
{
	cout << "[ " << endl;
	for(int i=1; i<=n; i++)
	{
		cout << fixed << "  " << x[i].x << '\t' << x[i].y << endl;
	}
	cout << "]" << endl;
}
void printV(GLfloat* x, int n)
{
	cout << "[ " << endl;
	for(int i=1; i<=n; i++)
	{
		cout << fixed << "  " << x[i] << endl;
	}
	cout << "]" << endl;
}

/*************************************************************************
* FUNCTION_NAME   : getTranspose
*
* INPUT           : float** A, the original matrix
*					float** A_T, the transpose to be stored
*					int n, size of both matrices
*
* OUTPUT          : None
*
* VALUE RETURNED  : None
*
* FUNCTIONS CALLED: None
*
* DESCRIPTION     : Stores the transpose of A in matrix A_T
*
*************************************************************************/
void getTranspose(GLfloat** A, GLfloat** A_T, GLint n)
{
	for(GLint i=1; i<=n; i++)
		for(GLint j=1; j<=n; j++)
			A_T[i][j] = A[j][i];
}


/*************************************************************************
* FUNCTION_NAME   : tridiagonal
*
* INPUT           : GLfloat** A, the matrix in the system of equations
*					Point2D* b, the RHS of the system of equations
*					int n, size of A and b
*
* OUTPUT          : None
*
* VALUE RETURNED  : Point2D*, the vector that represents x, and is the
*					solution to the system of equations.
*
* FUNCTIONS CALLED: getTranspose
*
*
* REMARKS		  :	This function uses three algorithms from the textbook:
*					Tridiagonal system:		page 180
*
*************************************************************************/
Point2D* tridiagonal(GLfloat** A, Point2D* b, GLint n)
{
	Point2D* x = new Point2D[n+1];//The solution to find
	for(GLint i=1; i<=n; i++)
		x[i] = Point2D(0.0,0.0);

	GLfloat* a = new GLfloat[n];
	for(GLint i=1; i<=n-1; i++)
		a[i] = A[i+1][i];
	GLfloat* c = new GLfloat[n];
	for(GLint i=1; i<=n-1; i++)
		c[i] = A[i][i+1];
	GLfloat* d = new GLfloat[n+1];
	for(GLint i=1; i<=n; i++)
		d[i] = A[i][i];

	for(GLint i=2; i<=n; i++)
	{
		d[i] = d[i] - (a[i-1]/d[i-1])*c[i-1];
		b[i] = Point2D(b[i].x - (a[i-1]/d[i-1])*b[i-1].x,
			b[i].y - (a[i-1]/d[i-1])*b[i-1].y);
	}

	x[n] = Point2D(b[n].x/d[n], b[n].y/d[n]);
	for(GLint i=n-1; i>=1; i--)
	{
		x[i] = Point2D((b[i].x-c[i]*x[i+1].x)/d[i],
			(b[i].y-c[i]*x[i+1].y)/d[i]);
	}

	delete [] a;
	delete [] c;
	delete [] d;

	return x;
}
//END OF TRIDIAGONAL APPROXIMATION METHOD CODE
//*************************************************************************************


//*************************************************************************************
//Name: initButtons
//Description: Initializes the menu buttons
//In-File Function calls: Point2D constructor, Rect2D constructor
//Uses:	<vector>, Rect2D, Point2D
//Preconditions: None
//Postconditions: The buttons are initiaized
//Parameters: None
//Returns: Nothing
void initButtons()
{	
	//Set the button vertices
	buttons.clear();
	buttons.push_back(Rect2D());//Dummy button for position 0 of the vector
	B_WIDTH = WINDOW_WIDTH/5;
	B_HEIGHT = WINDOW_HEIGHT/6 - 2*offset;
	
	for(GLint i=0; i<=4; i++)//5 main buttons 
	{
		buttons.push_back( Rect2D(
				Point2D(i*B_WIDTH,WINDOW_HEIGHT-B_HEIGHT),
				Point2D(i*B_WIDTH + B_WIDTH, WINDOW_HEIGHT)));
	}	
	for(GLint i=0; i<=1; i++)//2 menu choices
	{
		buttons.push_back( Rect2D(
				Point2D(B_WIDTH, WINDOW_HEIGHT-B_HEIGHT/2 -i*B_HEIGHT/2),
				Point2D(2*B_WIDTH, WINDOW_HEIGHT -i*B_HEIGHT/2) ));
	}

}

//*************************************************************************************
//Name: initialize
//Description: Initializes various variables and states
//In-File Function calls: Point2D constructor, initButtons
//Uses: Point2D, RGBpixmap
//Preconditions: None
//Postconditions: The program is initialized
//Parameters: None
//Returns: Nothing
void initialize() 
{		
	//Performs certain window initalizations
	glClearColor (1.0, 1.0, 1.0, 0.0);   // set background color to be white
	glColor3f (0.0f, 0.0f, 0.0f);       // set the drawing color to be black
	glMatrixMode (GL_PROJECTION);       // set "camera shape"
	glLoadIdentity ();                    // clearing the viewing matrix
	gluOrtho2D (0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);  // setting the world window to be 640 by 480
	initButtons();
	
	inputState = 0;
	menuState = 0;

}

//*************************************************************************************
//Name: displayText
//Description: A general use function to draw given text to the viewing area.
//In-File Function calls: None
//Uses:	N/A
//Preconditions: ostringstream os is not empty, x and y are in the view area
//Postconditions: The text is displayed to the viewing area at (x,y)
//Parameters: ostringstream os, the text to display
//Returns: Nothing
void displayText(ostringstream& os, GLfloat x, GLfloat y)
{	
	string output = os.str();
	//glRasterPos2f( WINDOW_WIDTH/50, WINDOW_HEIGHT/7);//Reset position
	glRasterPos2f(x, y);
	glColor3f( 0.0f, 0.0f, 0.0f ); //Black text
	for(GLint j=0; j<(GLint)output.length(); j++)
	{	glutBitmapCharacter( GLUT_BITMAP_HELVETICA_12, output[j] );}
}

//*************************************************************************************
//Name: displayDataPoints
//Description: Displays the coordinates of the user-defined data points, to the out area
//In-File Function calls: displayText
//Uses:	Point2D. <vector>
//Preconditions: The data does not exceed the capacity (size) of the output area
//Postconditions: The output is printed into the output area
//Parameters: None
//Returns: Nothing
void displayDataPoints()
{
	ostringstream marker;
	marker << "X";
	Point2D cursor = Point2D(WINDOW_WIDTH/50, WINDOW_HEIGHT/7-12);
	for(GLint i=0; i<(GLint)dataPoints.size(); i++, cursor.y-=12)
	{
		if(i%7==0)
			cursor = Point2D(WINDOW_WIDTH/50*i, WINDOW_HEIGHT/7-12);
		Point2D D = dataPoints.at(i);
		displayText(marker, (GLint)D.x, (GLint)D.y);
	}
}

//*************************************************************************************
//Name: drawButton
//Description: Draws a button for the menu
//In-File Function calls: None
//Uses:	<vector>, <iostream>
//Preconditions: buttonNum must be between 1 and 12, inclusively
//Postconditions: The button is drawn to the screen (if appropriate)
//Parameters: GLint buttonNum, the number of the button
//Returns: Nothing
void drawButton(GLint buttonNum)
{
	if(buttonNum == 1)
		glColor3f(1,0,0);//Red
	else if(buttonNum == 2)
		glColor3f(1,1,0);//Yellow
	else if(buttonNum == 3)
		glColor3f(0,1,0);//Green
	else if(buttonNum == 4)
		glColor3f(0,.5,1);//Light blue
	else if(buttonNum == 5)
		glColor3f(0.4,0.5,1);
	else
		glColor3f(0.5,0.5,0.5);//Light grey

	//Drawing the button
	glRectf(buttons.at(buttonNum).bottomleft.x, buttons.at(buttonNum).bottomleft.y,
			buttons.at(buttonNum).topright.x, buttons.at(buttonNum).topright.y);

	glColor3d( 0,0,0 ); //Black borders for the buttons
	glBegin(GL_LINE_LOOP);
	glVertex2f(buttons.at(buttonNum).bottomleft.x, buttons.at(buttonNum).bottomleft.y);
	glVertex2f(buttons.at(buttonNum).bottomleft.x, buttons.at(buttonNum).topright.y);
	glVertex2f(buttons.at(buttonNum).topright.x, buttons.at(buttonNum).topright.y);
	glVertex2f(buttons.at(buttonNum).topright.x, buttons.at(buttonNum).bottomleft.y);
	glEnd();

	//Outputting the button label to the screen
	glColor3d( 0,0,0 ); //Black text

	ostringstream output;
	output.str("");
	switch(buttonNum)//Button labels
	{
		case 1:
			output << "Choose Methods>";
			displayText(output, (buttonNum - 1)*B_WIDTH + B_WIDTH/10,WINDOW_HEIGHT - B_HEIGHT/2);
			break;
		case 2:
			output << "Source Image";
			displayText(output, (buttonNum - 1)*B_WIDTH + B_WIDTH/10,WINDOW_HEIGHT - B_HEIGHT/2);
			break;
		case 3:
			output << "Desination Image";
			displayText(output, (buttonNum - 1)*B_WIDTH + B_WIDTH/10,WINDOW_HEIGHT - B_HEIGHT/2);
			break;
		case 4:
			output << "Start";
			displayText(output, (buttonNum - 1)*B_WIDTH + B_WIDTH/10,WINDOW_HEIGHT - B_HEIGHT/2);
			break;
		case 5:
			output << "Stop ";
			displayText(output, (buttonNum - 1)*B_WIDTH + B_WIDTH/10,WINDOW_HEIGHT - B_HEIGHT/2);
			break;
		case 6:
			output << "Grid";
			displayText(output, buttons.at(buttonNum).bottomleft.x + B_WIDTH/10,
				buttons.at(buttonNum).bottomleft.y 
				+ (buttons.at(buttonNum).topright.y - buttons.at(buttonNum).bottomleft.y)/2.0);
			break;
		case 7:
			output << "Feature";
			displayText(output, buttons.at(buttonNum).bottomleft.x + B_WIDTH/10,
				buttons.at(buttonNum).bottomleft.y 
				+ (buttons.at(buttonNum).topright.y - buttons.at(buttonNum).bottomleft.y)/2.0);
			break;

		default://Invalid button number??? Farbotz!
			break;
	}

}

//*************************************************************************************
//Name: drawButtonBar
//Description: Draws the menu at the top of the window, with 4 main buttons
//In-File Function calls: initButtons, drawButton
//Uses:	N/A
//Preconditions: None
//Postconditions: The button bar is drawn
//Parameters: None
//Returns: Nothing
void drawButtonBar()
{
	glColor3f(0,0,1);//Set to grey
	//glRectf(0.0f, 5*WINDOW_HEIGHT/6, WINDOW_WIDTH, WINDOW_HEIGHT);//Drawing the bar background
	initButtons();
	for(GLint i=1; i<=5; i++)//The main buttons
		drawButton(i);
	if(menuState >= 1)
	{
		for(GLint i=6; i<=7; i++)//The 2 Choices in the drop-down menu
			drawButton(i);
	}
	glColor3f (0.0f, 0.0f, 0.0f); // Reset the drawing color to be black
}

//*************************************************************************************
//Name: C
//Description: A mathematical function to calculate a cubic uniform Bezier curve at t
//Function calls: Point2D constructor
//Preconditions: p0, p1, p2, p3, and t are all valid; t is in [0,1]
//Postconditions: None
//Parameters: Point2D p0, p1, p2, p3, control points that define the curve
//				GLfloat t, the value to plug into the equation
//Returns: Point2D, the result of the equation.
Point2D C(Point2D p0, Point2D p1, Point2D p2, Point2D p3, GLfloat t)
{
	GLfloat fx = (pow(1-t,3)/6.0)*p0.x + ((4-6*t*t+3*t*t*t)/6.0)*p1.x 
		+ ((1+3*t+3*t*t-3*t*t*t)/6.0)*p2.x + (pow(t,3)/6.0)*p3.x;
	GLfloat fy = (pow(1-t,3)/6.0)*p0.y + ((4-6*t*t+3*t*t*t)/6.0)*p1.y 
		+ ((1+3*t+3*t*t-3*t*t*t)/6.0)*p2.y + (pow(t,3)/6.0)*p3.y;
	return Point2D(fx, fy);
}

//custom datapoints
//this function get a vector or 2d points and is then used to draw 
//cubic b-spline curve, and allow the user to update the points and apply image morphing technique
vector<Point2D> * getCurve(vector<Point2D> DataPoints,vector<Point2D> &resultControlPoints){
	
	resultControlPoints.clear();
	
	GLfloat** A;
	GLint n = (GLint)DataPoints.size()-2;
	A = new GLfloat*[n+1];
	for(GLint row=1; row<=n; row++)
	{
		A[row] = new GLfloat[n+1];
		for(GLint col=1; col<=n; col++)
		{
			if(row == col)
				A[row][col]=4;
			else if((row + 1) == col)
				A[row][col]=1;
			else if((col + 1) == row)
				A[row][col]=1;
			else
				A[row][col]= 0;
		}
	}

	Point2D* b = new Point2D[n+1];

	for(GLint i=1; i<=n; i++)
	{
		
		if(i==1){
			b[i].x = 6*DataPoints.at(1).x - DataPoints.at(0).x;
			b[i].y = 6*DataPoints.at(1).y - DataPoints.at(0).y;

		}
		else if(i == n){
			b[i].x = 6*DataPoints.at(DataPoints.size() -2).x - DataPoints.at(DataPoints.size() -1).x;
			b[i].y = 6*DataPoints.at(DataPoints.size() -2).y - DataPoints.at(DataPoints.size() -1).y;
		}
		else{
			b[i].x = 6*DataPoints.at(i).x;
			b[i].y = 6*DataPoints.at(i).y;
		}
	}

	Point2D* p = tridiagonal(A, b, n);
	//for(GLint i=1; i<=n; i++)
	//	p[i].round();//Only used for neater output
	
	//2p1-p2
	Point2D P0;
	P0.x = 2*DataPoints.at(0).x - p[1].x;
	P0.y = 2*DataPoints.at(0).y - p[1].y;

	resultControlPoints.push_back(P0);
	resultControlPoints.push_back(DataPoints.at(0));//p_1 = D_0
	for(GLint i=1; i<= n; i++) //contain p2 to pn
		resultControlPoints.push_back(p[i]);
	resultControlPoints.push_back(DataPoints.back());//p_n+1 = D_n

	Point2D lastControlPoint;
	//2pn+1 - pn
	lastControlPoint.x = 2*DataPoints.at(DataPoints.size() -1).x - p[n].x;
	lastControlPoint.y = 2*DataPoints.at(DataPoints.size() -1).y - p[n].y;
	resultControlPoints.push_back(lastControlPoint);

	delete [] p;
	delete [] b;
	for(GLint i=1; i<=n; i++)
		delete [] A[i];
	delete [] A;
	//cout << "Curve has been generated"<<endl;
	return &resultControlPoints;
}

//*************************************************************************************
//Name: constructCurve
//Description: Interpolates a closed uniform cubic B-spline curve from the data points
//		Since A is tridiagonal, a special algorithm can be solved to approximate the
//		control points.
//Function calls: tridiagonal() 
//Preconditions: At least 3 data points were inputted
//Postconditions: the control points are created.
//Parameters: None
//Returns: Nothing
//col or row = 0 means do for all
void constructCurve(vector<vector<Point2D>> v, vector<vector<Point2D>> &vectorRows, vector<vector<Point2D>> &vectorCols,int numRow,int numCol)
{
	if(numRow== 0)
		vectorRows.clear(); //each element is a vector of Point2D, thus this contain n functions
	else if(numRow != -1)
		vectorRows.at(numRow).clear();
	if(numCol == 0)
		vectorCols.clear();
	else if(numRow != -1)
		vectorCols.at(numCol).clear();

	vector<Point2D> curve;

	if(numRow == 0){
		for(int row = 0; row < v.size(); row++){
			curve.clear();
			getCurve(v.at(row),curve);
			vectorRows.push_back(curve);
		}
	}else if(numRow != -1){
		getCurve(v.at(numRow),vectorRows.at(numRow));
	}

	//for cols
	if(numCol == 0){
		for(int col = 0; col < v.at(0).size(); col++){
			curve.clear();
			vector<Point2D> colPoints;
			for(int row = 0; row < v.size(); row++){
				colPoints.push_back(v.at(row).at(col));//get each col points
			}
			getCurve(colPoints,curve);
			vectorCols.push_back(curve);
		}
	}else if(numRow != -1){
		vector<Point2D> colPoints;
		for(int row = 0; row < v.size(); row++){
				colPoints.push_back(v.at(row).at(numCol));//get each col points
		}
		getCurve(colPoints,vectorCols.at(numCol));
	}

}

//*************************************************************************************
//Name: dist
//Description: A simple distance formula for two 2D cartesian points
//Function calls: None
//Preconditions: None
//Postconditions: None
//Parameters: Point2D p1,p2, the points to use
//Returns: The absolute distance between p1 and p2
//Note:	This is the CORRECT version
GLfloat dist(Point2D p1, Point2D p2)
{
	return sqrt(abs((p2.x-p1.x)*(p2.x-p1.x)+(p2.y-p1.y)*(p2.y-p1.y)));
}

//*************************************************************************************
//Name: buildALTable
//Description: Creates the Arc Length Table, from the control points and C(t)
//				Employs adaptive subdivision to accomplish this
//Function calls: C()
//Preconditions: The control points exist, and are valid
//Postconditions: The Arc Length Table (ALTable) is set up with values
//Parameters: None
//Returns: Nothing
void buildALTable()
{
	ALTable.index = 0;
	GLfloat last_s = 0;
	ALTable.store(0, 0);
	for(GLint curve=0; curve<(GLint)dataPoints.size()-1; curve++)
	{
		Point2D p0 = controlPoints.at(curve);
		Point2D p1 = controlPoints.at(curve+1);
		Point2D p2 = controlPoints.at(curve+2);
		Point2D p3 = controlPoints.at(curve+3);
		vector<interval> stk;
		stk.push_back(interval(0,1));
		for(GLint Q = 1; Q>0; Q--)//Forced pre-subdivision
		{
			vector<interval> temp;
			for(GLint k=0; k<(GLint)stk.size(); k++)
			{
				interval span = stk.at(k);
				GLfloat m = (span.a+span.b)/2.0;
				temp.push_back(interval(m, span.b));
				temp.push_back(interval(span.a, m));
			}
			stk = temp;
		}
		GLfloat i=0;
		GLfloat s=0;
		while(!stk.empty())
		{
			interval span = stk.back();
			stk.pop_back();
			GLfloat m = (span.a+span.b)/2.0;
			GLfloat L1 = dist(C(p0,p1,p2,p3,span.a),C(p0,p1,p2,p3,m));
			GLfloat L2 = dist(C(p0,p1,p2,p3,m),C(p0,p1,p2,p3,span.b));
			GLfloat L3 = dist(C(p0,p1,p2,p3,span.a),C(p0,p1,p2,p3,span.b));
			if(abs(L1+L2-L3)>.01)
			{//Subdivide further
				stk.push_back(interval(m, span.b));
				stk.push_back(interval(span.a, m));
			}
			else
			{//Store this data
				s+=L1;
				ALTable.store(curve+m, last_s+s);
				s+=L2;
				ALTable.store(curve+span.b, last_s+s);
			}
		}
		last_s = ALTable.arcLength.back();
	}
}

//*************************************************************************************
//Name: drawCurve
//Description: Draws the B-spline curve to the screen (main window area).
//Function calls: C()
//Preconditions: The control points exist, and are valid
//Postconditions: The Arc Length Table (ALTable) is set up with values
//Parameters: None
//Returns: Nothing
void drawCurve(vector<vector<Point2D>>v)
{

	for(int row = 0; row < v.size(); row++){

	GLint n = (GLint) gridPointsSource.size();
	vector<Point2D> tempCP = v.at(row);
	Point2D start;
	glColor3f(0,1,0);
	glPointSize(3);
	glBegin(GL_LINE_STRIP);
	for(GLint i=0; i<n-1; i++)
	{
		GLfloat step = 1.0/40.0;
		for(GLfloat t=0; t<=1; t+=step)
		{
			Point2D B = C(tempCP.at(i), tempCP.at(i+1),
						tempCP.at(i+2), tempCP.at(i+3), t);							
			glVertex2f(B.x, B.y);
		}
	}
	glEnd();
	/*
	glColor3f(1,0,0);
	for(int j = 0; j < tempCP.size(); j++){
		glPointSize(3);
		glBegin(GL_POINTS);
		glVertex2f(tempCP.at(j).x,tempCP.at(j).y);
		glEnd();
	}
	*/
	}//for loop
}


//*************************************************************************************
//Name: lighting
//Description: Turns on the lighting for the scene. Note that this lighting
//	model is similar in some ways to Dr. Cheng's shaped1.cpp.
//OpenGL Function calls: glEnable, glMaterialfv, glLightfv
//Prog2.cpp Function calls: None
//Preconditions: None
//Postconditions: The lighting is enabled.
//Parameters: RGBA color, what color to use (for different colored surfaces)
//Returns: Nothing
void lighting()
{
	float lightPos[] = {2, 1, -2, 0};//Mid-morning?
	float BLACK[]={1,1,1,0};
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos ) ; 
    glLightfv( GL_LIGHT0, GL_DIFFUSE, BLACK ) ;
    glLightfv( GL_LIGHT0, GL_AMBIENT, BLACK ) ;
	glEnable(GL_LIGHT0);
}
//*************************************************************************************

void drawInter(){
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glBindTexture(GL_TEXTURE_2D,7000);
	glBegin(GL_QUADS);          
	glTexCoord2f(0,1); glVertex2f(WINDOW_WIDTH/2 + offset,WINDOW_HEIGHT-B_HEIGHT-2*offset); //top left
	glTexCoord2f(1,1); glVertex2f(WINDOW_WIDTH + offset,WINDOW_HEIGHT-B_HEIGHT-2*offset); //top right
	glTexCoord2f(1,0); glVertex2f(WINDOW_WIDTH + offset,0); //bottom right
	glTexCoord2f(0,0); glVertex2f(WINDOW_WIDTH/2 + offset,0);	

	glEnd();
}

void drawDes(long id){
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glBindTexture(GL_TEXTURE_2D,id);
	glBegin(GL_QUADS);          
	glTexCoord2f(0,1); glVertex2f(WINDOW_WIDTH/2 + offset,WINDOW_HEIGHT-B_HEIGHT-2*offset); //top left
	glTexCoord2f(1,1); glVertex2f(WINDOW_WIDTH + offset,WINDOW_HEIGHT-B_HEIGHT-2*offset); //top right
	glTexCoord2f(1,0); glVertex2f(WINDOW_WIDTH + offset,0); //bottom right
	glTexCoord2f(0,0); glVertex2f(WINDOW_WIDTH/2 + offset,0);	

	glEnd();
}

void drawImage3(){
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glBindTexture(GL_TEXTURE_2D,6000);
	glBegin(GL_QUADS);          
	glTexCoord2f(0,1); glVertex2f(0,WINDOW_HEIGHT-B_HEIGHT-2*offset); //top left
	glTexCoord2f(1,1); glVertex2f(WINDOW_WIDTH/2 - offset,WINDOW_HEIGHT-B_HEIGHT-2*offset); //top right
	glTexCoord2f(1,0); glVertex2f(WINDOW_WIDTH/2 -offset,0); //bottom right
	glTexCoord2f(0,0); glVertex2f(0,0);	

	glEnd();
}

//draw source image
void drawSource(long id){
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glBindTexture(GL_TEXTURE_2D,id);
	glBegin(GL_QUADS);          
	glTexCoord2f(0,1); glVertex2f(0,WINDOW_HEIGHT-B_HEIGHT-2*offset); //top left
	glTexCoord2f(1,1); glVertex2f(WINDOW_WIDTH/2 - offset,WINDOW_HEIGHT-B_HEIGHT-2*offset); //top right
	glTexCoord2f(1,0); glVertex2f(WINDOW_WIDTH/2 -offset,0); //bottom right
	glTexCoord2f(0,0); glVertex2f(0,0);	

	glEnd();
}

void drawInterGrid(vector<vector<Point2D>> v){
	
	for(int i = 0; i < v.size(); i++){
		for(int j = 0; j < v.at(i).size(); j++){
			glPointSize(10);
			glBegin(GL_POINTS);
			glVertex2f(v.at(i).at(j).x,v.at(i).at(j).y);
			glEnd();
		}
	}

}

void drawGridsPoints(vector<vector<Point2D>> v){
	
	glColor3f(0,1,0);
	for(int i = 0; i < v.size(); i++){
		for(int j = 0; j < v.at(i).size(); j++){
			glPointSize(10);
			glBegin(GL_POINTS);
			glVertex2f(v.at(i).at(j).x,v.at(i).at(j).y);
			glEnd();
		}
	}

}
//vector<vector<Point2D>> interGrid;
//vector<vector<Point2D>> auxGrid;


void outputBitmap(RGBpixmap bmp, string fn){

	ofstream fout(fn.c_str());

	for(int i = 0; i < bmp.nRows; i++){
		for(int j = 0; j < bmp.nCols; j++){
			fout<<(int)bmp.pixel[i*bmp.nCols + j].r<<"-"<<(int)bmp.pixel[i*bmp.nCols + j].g<<"-"<<(int)bmp.pixel[i*bmp.nCols + j].b<<" ";
		}
		fout<<endl;
	}

	fout.close();
}

//the following 3 functions step1-step3 apply the grid appraoch
//scan horizontal, vertical, and begin morphing
//give it source
void step1(vector<vector<Point2D>> &interGrid,vector<vector<Point2D>> & auxGridSource, vector<vector<Point2D>> & auxGridDes ,vector<vector<Point2D>> gridPointsSource, vector<vector<Point2D>> gridPointsDes){

	
	interGrid.clear(); auxGridSource.clear(); auxGridDes.clear();
	int rows = gridPointsSource.size();
	int cols = gridPointsSource.at(0).size();
	int x_offset = gridPointsDes.at(0).at(0).x;
	for(int i = 0; i < rows; i++){ 
		vector<Point2D> v;
		vector<Point2D> auxV;
		vector<Point2D> auxVDes;
		v.clear();
		auxV.clear(); 
		auxVDes.clear();
		for(int j = 0; j < cols; j++){
			
			Point2D p;
			p.x = (1 - Time) * gridPointsSource.at(i).at(j).x + Time * (gridPointsDes.at(i).at(j).x - x_offset);
			p.y = (1 - Time) * gridPointsSource.at(i).at(j).y + Time * gridPointsDes.at(i).at(j).y;
			v.push_back(p);
			
			Point2D auxP;
			auxP.x = gridPointsSource.at(i).at(j).x; //A(source_x,Inter_y)
			auxP.y = p.y;
			auxV.push_back(auxP);
			
			Point2D auxP2;
			auxP2.x = gridPointsDes.at(i).at(j).x;
			auxP2.y = p.y;
			auxVDes.push_back(auxP2);
		}
		interGrid.push_back(v);
		auxGridSource.push_back(auxV);
		auxGridDes.push_back(auxVDes);
	}


}

//result of morphing x intercept
void step2(RGBpixmap & result, RGBpixmap image, vector<vector<Point2D>> controlPointsGridAuxCols, vector<vector<Point2D>> controlPoints_Des_Source_cols,int offset){

	//source-aux
	int pixelRows = gridPointsSource.back().front().y; //number of pixels					 
	int pixelCols = gridPointsSource.at(0).back().x;	
	
	float deltaT = 0.0005;		//delta t

	//time for each spline
	float * timeArrayAux = new float[gridPointsSource.size()];	//time for aux and source
	float * timeArray_Des_Source = new float[gridPointsSource.size()]; 

	for(int i = 0; i < gridPointsSource.size(); i++){
		timeArrayAux[i] = 0; //0 and last one are dummys
		timeArray_Des_Source[i] = 0;
	}

	int setAux = false;
	int setSource = false;

	//x-intercept morph results
	//RGBpixmap ImageAuxSource;
	result.nCols = image1.nCols;
	result.nRows = image1.nRows;
	result.pixel = new RGB[image1.nCols * image1.nRows];
	
	for(int i = 0; i < (image1.nCols * image1.nRows); i++){
		result.pixel[i].r = 0;
		result.pixel[i].g = 0;
		result.pixel[i].b = 0;
	}

	for(int i = 0; i < pixelRows; i++){

		vector<int> x_interceptAux,x_interceptSource;
		x_interceptAux.push_back(0);
		x_interceptSource.push_back(0);

		for(int j = 1; j < (gridPointsSource.front().size() -1); j++){ //number of grids points
			
			vector<Point2D> colControlPoints = controlPointsGridAuxCols.at(j); //aux control points
			vector<Point2D> colSource = controlPoints_Des_Source_cols.at(j); //control points of compar
			setAux = false; setSource = false;
			
			while(1){ //while looking for intecept t
				
				if(!setAux){
					int N = floor((double) timeArrayAux[j]);
					Point2D pointAux = C(colControlPoints.at(0 + N),colControlPoints.at(1 + N),colControlPoints.at(2 + N),colControlPoints.at(3 + N),timeArrayAux[j] - floor(timeArrayAux[j]));
					timeArrayAux[j] += deltaT;
					if(pointAux.y > i && pointAux.y <( i + 1)){ //found
						x_interceptAux.push_back(pointAux.x - offset);
						setAux = true;
					}
				}
				if(!setSource){
					int NS = floor((double) timeArray_Des_Source[j]);
					Point2D pointSource = C(colSource.at(0 + NS),colSource.at(1 + NS),colSource.at(2 + NS),colSource.at(3 + NS),timeArray_Des_Source [j] - floor(timeArray_Des_Source[j]));
					timeArray_Des_Source[j] += deltaT;
					if(pointSource.y > i && pointSource.y < (i + 1)){ //found
						x_interceptSource.push_back(pointSource.x - offset);
						setSource = true;
					}
				}

				if(setSource && setAux)
				      break;

			}//while

		}

		x_interceptAux.push_back(pixelCols);
		x_interceptSource.push_back(pixelCols);

		for(int k = 1; k < x_interceptAux.size(); k++){
			float AuxInterval = x_interceptAux.at(k) - x_interceptAux.at(k-1);
			float SourceInterval = x_interceptSource.at(k) -  x_interceptSource.at(k-1);
			//cout<<"going from "<< x_interceptAux.at(k-1) << " to "<< x_interceptAux.at(k)<<endl;
			for(int l = x_interceptAux.at(k-1); l < x_interceptAux.at(k) ; l++){
				double relativePos = (l-x_interceptAux.at(k-1))/AuxInterval;
				double SourceRelativePos = (relativePos * SourceInterval);
				int pixel = i*image.nCols + floor(SourceRelativePos) + x_interceptAux.at(k-1);
				result.pixel[i*image.nCols + l] = image.pixel[pixel];
			}
		} 
	
		//print array
	}

}


void step3(RGBpixmap & result,RGBpixmap image, vector<vector<Point2D>> controlPointsGridAuxRows, vector<vector<Point2D>> controlPointsGridInterRows,int offset){

	//source-aux
	int pixelRows = gridPointsSource.back().front().y; //number of pixels					 
	int pixelCols = gridPointsSource.at(0).back().x;	
	
	float deltaT = 0.0005;		//delta t

	//time for each spline
	float * timeArrayAux = new float[gridPointsSource.size()];	//time for aux and source
	float * timeArray_inter = new float[gridPointsSource.size()]; 

	for(int i = 0; i < gridPointsSource.size(); i++){
		timeArrayAux[i] = 0; //0 and last one are dummys
		timeArray_inter[i] = 0;
	}

	int setAux = false;
	int setInter = false;

	//x-intercept morph results
	//RGBpixmap ImageAuxSource;
	result.nCols = image1.nCols;
	result.nRows = image1.nRows;
	result.pixel = new RGB[image1.nCols * image1.nRows];
	
	for(int i = 0; i < (image1.nCols * image1.nRows); i++){
		result.pixel[i].r = 0;
		result.pixel[i].g = 0;
		result.pixel[i].b = 0;
	}

	for(int i = 0; i < (pixelCols -1); i++){

		vector<int> y_interceptAux, y_intercepInter;
		
		y_interceptAux.push_back(0);
		y_intercepInter.push_back(0);

		for(int j = 1; j < (gridPointsSource.size()-2); j++){ //number of rows

			vector<Point2D> auxControlPoint = controlPointsGridAuxRows.at(j); //aux control points
			vector<Point2D> InterControlPoint = controlPointsGridInterRows.at(j); //control points of compar
			setAux = false; setInter= false;

			while(1){ //while looking for intecept t
				
				if(!setAux){
					int N = floor((double) timeArrayAux[j]);
					Point2D pointAux = C(auxControlPoint.at(0 + N),auxControlPoint.at(1 + N),auxControlPoint.at(2 + N),auxControlPoint.at(3 + N),timeArrayAux[j] - floor(timeArrayAux[j]));
					timeArrayAux[j] += deltaT;
					if((pointAux.x -offset) > i && (pointAux.x -offset) <( i + 1)){ //found
						y_interceptAux.push_back(pointAux.y);
						setAux = true;
					}
				}
				if(!setInter){
					int NS = floor((double) timeArray_inter[j]);
					Point2D pointSource = C(InterControlPoint.at(0 + NS),InterControlPoint.at(1 + NS),InterControlPoint.at(2 + NS),InterControlPoint.at(3 + NS),timeArray_inter[j] - floor(timeArray_inter[j]));
					timeArray_inter[j] += deltaT;
					if(pointSource.x > i && pointSource.x < (i + 1)){ //found
						y_intercepInter.push_back(pointSource.y);
						setInter = true;
					}
				}

				if(setAux && setInter)
				      break;

			}//while

		}

		y_interceptAux.push_back(pixelRows);
		y_intercepInter.push_back(pixelRows);

		for(int k = 1; k < y_intercepInter.size(); k++){
			float InterInterval = y_intercepInter.at(k) - y_intercepInter.at(k-1);
			float AuxInterval = y_interceptAux.at(k) -  y_interceptAux.at(k-1);
			
			for(int l = y_intercepInter.at(k-1); l < y_intercepInter.at(k) ; l++){
				double relativePos = (l-y_intercepInter.at(k-1))/InterInterval;
				double AuxRelativePos = (relativePos * AuxInterval);
				int row = floor(AuxRelativePos) + y_interceptAux.at(k-1);
				int pixel = row * image.nCols + i;
				int resultPos = l * image.nCols + i;
				result.pixel[resultPos] = image.pixel[pixel];
			}
		} //for

	}//for loop

}

void blend(RGBpixmap &finalImage,RGBpixmap imageSource,RGBpixmap imageDes){

	finalImage.nCols = imageSource.nCols;
	finalImage.nRows = imageSource.nRows;
	finalImage.pixel = new RGB[imageSource.nCols * imageSource.nRows];

	for(int i = 0; i < (image1.nCols * image1.nRows); i++){
		finalImage.pixel[i].b = (1-Time)*imageSource.pixel[i].b + Time*imageDes.pixel[i].b;
		finalImage.pixel[i].r = (1-Time)*imageSource.pixel[i].r + Time*imageDes.pixel[i].r;
		finalImage.pixel[i].g = (1-Time)*imageSource.pixel[i].g + Time*imageDes.pixel[i].g;
	}

}

//begin morphing using grid methods
void start_morphed_grid(){
	
	int x_offset = gridPointsDes.at(0).at(0).x;

	//step 1
	vector<vector<Point2D>> interGrid;//points
	vector<vector<Point2D>> auxGridSource; //result between aux grids and source
	vector<vector<Point2D>> auxGridDes;  //aux and destiation
	vector<vector<Point2D>> controlPointsGridAuxDesRows,controlPointsGridAuxDesCols; //aux for destination
	vector<vector<Point2D>> controlPointsGridAuxSourceRows,controlPointsGridAuxSourceCols; //aux  for source
	vector<vector<Point2D>> controlPointsGridInterRows,controlPointsGridInterCols; //intermediate grids

	 step1(interGrid,auxGridSource,auxGridDes,gridPointsSource,gridPointsDes); //step 1

	//construct curve for auxGrids source
	 constructCurve(auxGridSource,controlPointsGridAuxSourceRows,controlPointsGridAuxSourceCols,0,0);
	 //construct for inter 
	 constructCurve(interGrid,controlPointsGridInterRows,controlPointsGridInterCols,0,0);
	 //auxGrid des
	 constructCurve(auxGridDes,controlPointsGridAuxDesRows,controlPointsGridAuxDesCols,0,0);

	 RGBpixmap ImageAuxSource, ImageAuxDes;
	 //source
	 step2(ImageAuxSource,image1, controlPointsGridAuxSourceCols, controlPointsGridSourceCols,0);
	//des
	 step2(ImageAuxDes,image2, controlPointsGridAuxDesCols, controlPointsGridDesCols,x_offset);

	 RGBpixmap ImageAuxDesInter, ImageAuxSourceInter;
	
	 step3(ImageAuxSourceInter,ImageAuxSource,controlPointsGridAuxSourceRows,controlPointsGridInterRows,0);
	 step3(ImageAuxDesInter,ImageAuxDes,controlPointsGridAuxDesRows,controlPointsGridInterRows,x_offset);

	 //last step morph
	 RGBpixmap finalImage;
	 blend(finalImage,ImageAuxSourceInter,ImageAuxDesInter);

	//outputBitmap(ImageAuxSource,"file1.txt");
	 ImageAuxSourceInter.setTexture(8000);
	//ImageAuxDesInter.setTexture(8000);
	 finalImage.setTexture(7000);

	//aux-intermediate

	glutPostRedisplay();
}

//weight
int A = 0.1;
int B = 10;
int P = 1;

float getWeight(feature_line f, float distance){

	float result;

	result = pow(pow(dist(f.P1,f.P2),P)/(A + distance),B);

	if(result < 0)
		return 0;

	return result;
}

struct vector_{
	float x, y;
};

//freature
struct featureCoor{
	vector_ S,T;
	feature_line f;
};
vector<Point2D> sourcePoints,desPoints;

void drawFeatureLine(feature_line f){

	if(f.count == 1){

		glPointSize(10);
		glBegin(GL_POINTS);
		glVertex2f(f.P1.x,f.P1.y);
		glEnd();

	}else if(f.count == 2){
		glBegin(GL_LINES);
		glVertex2f(f.P1.x,f.P1.y);
		glVertex2f(f.P2.x,f.P2.y);
		glEnd();
	}
}

void setCoor(featureCoor & result,feature_line f, int x_offset){

	f.P1.x -= x_offset;
	f.P2.x -= x_offset;

	result.T.x = f.P2.x - f.P1.x;
	result.T.y = f.P2.y - f.P1.y;

	result.S.x = result.T.y;
	result.S.y = -result.T.x;

	result.f = f;
}

float dotProduct(vector_ p1, Point2D p2){
	return ((p1.x * p2.x) + (p1.y * p2.y));
}

double dot(Point2D p1, Point2D p2){
	return ((p1.x * p2.x) + (p1.y * p2.y));
}

Point2D perp(Point2D  p){
	Point2D resultPoint;

	float x = p.x;
	float y = p.y;

	resultPoint.x = y;
	resultPoint.y = -x;

	return resultPoint;
}

//p3 from line p1 and p2
float getDistance(Point2D P1, Point2D P2, Point2D P3){

	float top = abs((P2.y - P1.y)*P3.x - (P2.x - P1.x)*P3.y + P2.x*P1.y - P2.y*P1.x);
	float denom = sqrt(pow(P2.y - P1.y,2) + pow(P2.x - P1.x,2));

	return top/denom;

} 

vector<int> vectorWeight(vector<featureCoor> v, Point2D P){
	vector<int> result;

	for(int i = 0; i < v.size(); i++){
		float d = getDistance(v.at(i).f.P1,v.at(i).f.P2,P);
		float w = getWeight(v.at(i).f,d);
		result.push_back(w);
	}

	return result;
}

int getClosetFcIter(vector<featureCoor> v, Point2D P){

	float distance = 10000;
	int iter = 0;

	for(int i = 0; i < v.size(); i++){
		float d = getDistance(v.at(i).f.P1,v.at(i).f.P2,P);
		if(d < distance){
			distance = d;
			iter = i;
		}
	}

	return iter;
}

float cross(Point2D a,Point2D b){
	return a.x*b.y - a.y*b.x;
}

double length(Point2D a) {
	return a.x*a.x + a.y*a.y;
}


void calculateInterImage(RGBpixmap & result,RGBpixmap &result2, vector<featureCoor> sourceFc ,vector<featureCoor> InterFc,RGBpixmap image){


	int x_offset = WINDOW_WIDTH/2 + offset;

	result.nCols = image.nCols;
	result.nRows = image.nRows;
	result.pixel = new RGB[image.nCols * image.nRows];
	
	result2.nCols = image.nCols;
	result2.nRows = image.nRows;
	result2.pixel = new RGB[image.nCols * image.nRows];
	

	Point2D P3,P4;
	Point2D Q,WeightedQ;
	

	double u,v;

	float totalWeight,xDis,yDis;

	for(int i = 0; i < image.nRows; i++){
		
		for(int j = 0; j < image.nCols; j++){
		
		    totalWeight = 0;
			xDis = 0; yDis = 0;
			Point2D P;
			P.x = j; P.y = i; 
			vector<float> individualWeight;
			vector<RGB> vectorPixel;
			//get closet featureCoor
			//int iterCloset = getClosetFcIter(InterFc,P);
			//featureCoor interCoor = InterFc.at(iterCloset);
			
			//vector<int> weights = vectorWeight(InterFc,P);
			//vector<Point2D> resultPos; 
			float w;
			for(int k = 0; k < InterFc.size(); k++){
				featureCoor interCoor = InterFc.at(k);

				P3.x = (P.x - interCoor.f.P1.x); //p - p1
				P3.y = (P.y - interCoor.f.P1.y);
				P4.x = (interCoor.f.P2.x - interCoor.f.P1.x); //p2 - p1
				P4.y = (interCoor.f.P2.y - interCoor.f.P1.y); 
					
				v = dot(P3,P4)/length(P4);
				u = dot(P3,perp(P4))/length(P4);
				//u = cross(P3,P4)/pow(dist(interCoor.f.P1,interCoor.f.P2),2);
				featureCoor sourceCoor = sourceFc.at(k);

				
				Q.x = sourceCoor.f.P1.x + u*sourceCoor.S.x + v*sourceCoor.T.x;
				Q.y = sourceCoor.f.P1.y + u*sourceCoor.S.y + v*sourceCoor.T.y;

				if((Q.x < 0) || (Q.y < 0) || (Q.y > (image.nRows)) || (Q.x > image.nCols))
					continue;
				
				//compute its local point
				int loc = (int) Q.y * image.nCols + Q.x;
				vectorPixel.push_back(image.pixel[loc]);

				float d = getDistance(sourceCoor.f.P1,sourceCoor.f.P2,Q);
				w = getWeight(sourceCoor.f,d);
				individualWeight.push_back(w);
			
				totalWeight += w;
				xDis += (Q.x - j)*w;
				yDis += (Q.y - i)*w;
			}

			if(totalWeight == 0)
				continue;
		
			unsigned char red,green,blue;
			red = 0; green = 0; blue = 0;
			for(int l = 0; l < vectorPixel.size(); l++){ //get average
				red += vectorPixel.at(l).r*individualWeight.at(l)/totalWeight;
				green += vectorPixel.at(l).g*individualWeight.at(l)/totalWeight;
				blue += vectorPixel.at(l).b*individualWeight.at(l)/totalWeight;
			}
			

			xDis /= totalWeight;
			yDis /= totalWeight;
			Q.y = (int) (i + yDis);
			Q.x = (int) (j + xDis);

		
			//result 1
			if(Q.x >= 0 && Q.x <= image1.nCols){
				if(Q.y >= 0 && Q.y <= image1.nRows){
					int resultLoc = i*image.nCols + j;
					int loc = (int) Q.y * image.nCols + Q.x;
					result.pixel[resultLoc] =  image.pixel[loc];
					
				}
				
			}//if loop

			//result 2
			int resultLoc = i*image.nCols + j;
			result2.pixel[resultLoc].r = red;
			result2.pixel[resultLoc].g = green;
			result2.pixel[resultLoc].b = blue;
		
		}

			
	}

}

//feature_line interF;

void setVectoreFeautreCoor(vector<Point2D> points, vector<featureCoor>& v, int x_offset){
	
	v.clear();
	for(int i = 0; i < points.size(); i += 2){
		featureCoor fc; feature_line f;
		f.P1 = points.at(i);
		f.P2 = points.at(i + 1);
		setCoor(fc,f,x_offset);
		v.push_back(fc);
	}

}

//only return even number of smallest vector and remove extra, result is des
void getInter(vector<Point2D> &v , vector<Point2D> &v2, vector<Point2D> &result,int x_offset){

	result.clear();

	int diff = v2.size() - v.size();
	if(diff > 0){
		for(int i = 0; i < diff; i++)
			v2.pop_back();
	}
	else if (diff < 0){
		int absDiff = abs(diff);
		for(int i = 0; i < absDiff; i++)
			v.pop_back();
	}

	if(v.size() != v2.size()){
		cout<<"wtf"<<endl;
	}

	if(v.size() % 2 != 0){
		v.pop_back();
		v2.pop_back();
	}

	for(int i = 0; i < v.size(); i++){
		Point2D p;
		p.x = (1-Time) * v.at(i).x + Time * (v2.at(i).x - x_offset);
		p.y = (1-Time) * v.at(i).y + Time * v2.at(i).y;
		result.push_back(p);
	}

	
}

void start_morphed_feature(){

	int x_offset = WINDOW_WIDTH/2 + offset;

	vector<Point2D> interPoints;
	getInter(sourcePoints,desPoints,interPoints,x_offset); //get intermediate points, remove extra point in source and des

	//get fc for each one
	vector<featureCoor> vectorFcSource,vectorFcDes,vectorFcInter;
	setVectoreFeautreCoor(sourcePoints,vectorFcSource,0);
	setVectoreFeautreCoor(desPoints,vectorFcDes,x_offset);
	setVectoreFeautreCoor(interPoints,vectorFcInter,0);
	/*
	int x_offset = WINDOW_WIDTH/2 + offset;
	interF.count = 2;
	interF.P1.x = (1-Time) * FeatureLine.P1.x + Time * (FeatureLine2.P1.x -x_offset);
	interF.P1.y = (1-Time) * FeatureLine.P1.y + Time * FeatureLine2.P1.y;
	interF.P2.x = (1-Time) * FeatureLine.P2.x + Time * (FeatureLine2.P2.x -x_offset);
	interF.P2.y = (1-Time) * FeatureLine.P2.y + Time * FeatureLine2.P2.y;

	featureCoor source, sourceInter, des,desInter;
	setCoor(source,FeatureLine);
	setCoor(sourceInter,interF);

	setCoor(des,FeatureLine2);
	setCoor(desInter,interF);
	*/
	RGBpixmap interImageSource,interImageDes;
	RGBpixmap interImageSource2,interImageDes2;
	calculateInterImage(interImageSource,interImageSource2,vectorFcSource,vectorFcInter,image1);
	calculateInterImage(interImageDes,interImageDes2,vectorFcDes,vectorFcInter,image2);

	RGBpixmap finalImage,finalImage2;
	//blend
	blend(finalImage,interImageSource,interImageDes);
	blend(finalImage2,interImageSource2,interImageDes2);

	finalImage.setTexture(9000); // book
	finalImage2.setTexture(10000); //average

	glutPostRedisplay();
}

void drawFeatureLines(vector<Point2D> v){

	for(int i = 0; i < v.size(); i++){
	
		if(i == v.size() -1){
			//draw point
			glColor3f(0,1,0);
			glPointSize(5);
			glBegin(GL_POINTS);
			glVertex2f(v.at(i).x,v.at(i).y);
			glEnd();
		}
		else{ //draw line
			glColor3f(0,1,0);
			glBegin(GL_LINES);
			glVertex2f(v.at(i).x,v.at(i).y);
			i++; //draw nex point
			glVertex2f(v.at(i).x,v.at(i).y);
			glEnd();
		}
	}
}

//*****************************************************************************
//Name: onDisplay
//Description: The display callback
//Function calls: drawButtonBar, displayText, displayDataPoints, drawCurve,
//		updateBFly, drawBFly
//Preconditions: None
//Postconditions: The contents of the window are shown (button menu, viewing
//	area, b-spline curve, text output area)
//Parameters: None
//Returns: Nothing
void onDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clearing the buffer

	//displayText(textOutput, WINDOW_WIDTH/50, WINDOW_HEIGHT/30);//General state info
	drawButtonBar();
	if(inputState != 3){
		drawSource(4000);
		drawDes(5000);
	}
	drawImage3();
	glDisable(GL_TEXTURE_2D);
	if(Algorithm == 0){
		drawGridsPoints(gridPointsSource);
		drawGridsPoints(gridPointsDes);
		drawCurve(controlPointsGridSourceRows);
		drawCurve(controlPointsGridSourceCols);
		drawCurve(controlPointsGridDesRows);
		drawCurve(controlPointsGridDesCols);
	}
	if(inputState == 3){
		//glColor3f(0.3,0,0.5);
		//drawInterGrid(interGrid);
		//glColor3f(1,1,0);
		//drawInterGrid(auxGrid);
		drawSource(8000);
		drawDes(7000);
		glDisable(GL_TEXTURE_2D);
	}
	if(inputState == 4){
		drawFeatureLines(sourcePoints);
		drawFeatureLines(desPoints);
	}

	if(inputState == 5){
		//if(interF.count != 0){
			//drawFeatureLine(interF);
			drawSource(9000);
			drawDes(10000);
			glDisable(GL_TEXTURE_2D);
		//}
	}

	glutSwapBuffers();//display the buffer
	glutPostRedisplay();
}

//*************************************************************************************
//Name: onKeyboard
//Description: The keyboard event callback
//Function calls: exit
//Preconditions: A key was hit
//Postconditions: The key event is handles
//Parameters: unsigned char key, the key hit
//				int x, y, the current position
//Returns: Nothing
void onKeyboard(unsigned char key, int x, int y)
{//Handles keyboard events
  switch (key)
  {
     case 27:  //Typed the Escape key, so exit.
      //  exit(0);
        break;
	 case 120://Lower case 'x', globally speeds up the animation
//		 save();
		 break;
	 case 122://Lower case 'z', globally slows down the animation
		
		 break;
     default:
        break;
  }
}

//*****************************************************************************
//Name: makeMenuSelection
//Description: Hanldes the event of releasing the mouse button when using the
//		drop-down menu
//Function calls: None
//Preconditions: menuState>=1, the mouse button was released
//Postconditions: Global variables may be changed
//Parameters: GLint x,y the mouse coordinates
//Returns: Nothing
void makeMenuSelection(GLint x, GLint y)
{
	if(menuState == 1)//First drop-down menu
	{
		for(int i = 0; i < 2; i++){
			if((x > buttons.at(6 + i).bottomleft.x && x < buttons.at(6 + i).topright.x) &&
				(y > buttons.at(6 + i).bottomleft.y && y < buttons.at(6 + i).topright.y)){
					Algorithm = i; // 0 is grid, 1 is feature
					if(i == 0)
						cout<<"selected Algorithm: "<<"grid"<<endl;
					else{
						inputState = 4;
						cout<<"selected Algorithm: "<<"feature"<<endl;
					}
					break;
			}//found
		}
	}

	menuState = 0;//Exit the menu no matter what
	//glutMotionFunc(NULL);//Diable the callback
}

//*************************************************************************************
//Name: onMouseMove
//Description: For showing the drop-down menu when selecting speeds
//Function calls: glutPostRedisplay()
//Preconditions: x and y are valid points, within the central window area
//Postconditions: The menu is displayed (in part, perhaps)
//Parameters: GLint x, y, the mouse position
//Returns: Nothing
void onMouseMove(GLint x, GLint y)
{
	y = WINDOW_HEIGHT-y;
	if(Algorithm >= 0 )//First drop-down menu
	{
		//update grids
		if(inputState == 1 || inputState == 2){
			if(currPoint.setx){
				if(inputState == 1)
					gridPointsSource.at(currPoint.i).at(currPoint.j).x = x;
				else if(inputState == 2)
					gridPointsDes.at(currPoint.i).at(currPoint.j).x = x;
			}
			if(currPoint.sety){
				if(inputState == 1)
					gridPointsSource.at(currPoint.i).at(currPoint.j).y = y;
				else if(inputState == 2)
					gridPointsDes.at(currPoint.i).at(currPoint.j).y = y;
			}

			if(inputState == 1)
					constructCurve( gridPointsSource,controlPointsGridSourceRows,controlPointsGridSourceCols,currPoint.i,currPoint.j);
			else if(inputState == 2)
					constructCurve( gridPointsDes,controlPointsGridDesRows,controlPointsGridDesCols,currPoint.i,currPoint.j);
		}//
	}
	glutPostRedisplay();
}

//fidn and set
bool findPos(vector<vector<Point2D>>v ,Point2D point, pos &p){
	
	float e = 10;

	for(int i = 0; i < v.size(); i++){
		for(int j = 0; j < v.at(i).size(); j++){
			if(dist(v.at(i).at(j),point) < e){
				p.i = i; p.j = j;
				if(i == 0 && j == 0 || i == (v.size()-1) && j == 0 ||
				   i == 0 && j == (v.at(0).size()-1) || i ==  (v.size()-1) && j == (v.at(0).size()-1)){
					   p.setx = false;
					   p.sety = false;
				}else if(i == 0 || i == (v.size()-1)){
					p.setx = true;
					p.sety = false;
				}else if(j == 0 || j == (v.at(0).size()-1)){
					p.setx = false;
					p.sety = true;
				}else{
					p.setx = true;
					p.sety = true;
				}
					
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
//Name: getButtonPushed
//Description: Finds which main button was pushed
//Function calls: glutIdleFunc(), glutMotionFunc()
//Preconditions: x and y are valid points, vector buttons was initialized
//Postconditions: The appropriate button's behavior is handled
//Parameters: GLint x, y, the mouse position
//Returns: Nothing
void buttonPushed(GLint x, GLint y)
{
	//y = WINDOW_HEIGHT-y;
	//textOutput.str("");
	
	if(( x>=buttons.at(1).bottomleft.x && x<=buttons.at(1).topright.x )
		&& ( y>=(buttons.at(1).bottomleft.y) && y<=(buttons.at(1).topright.y) ))
	{  //Input points button
		if(menuState==0)
			menuState = 1;
		else if(menuState == 1)
			menuState = 0;
	}
	else if(( x>=buttons.at(2).bottomleft.x && x<=buttons.at(2).topright.x )
		&& ( y>=(buttons.at(2).bottomleft.y) && y<=(buttons.at(2).topright.y) ))
	{
		if(inputState == 0)
			inputState = 1;
		else if(inputState == 1)
			inputState = 0;
	}
	else if(( x>=buttons.at(3).bottomleft.x && x<=buttons.at(3).topright.x )
		&& ( y>=(buttons.at(3).bottomleft.y) && y<=(buttons.at(3).topright.y) ))
	{
		if(inputState == 0)
			inputState = 2;
		else if(inputState == 2)
			inputState = 0;
	}
	else if(( x>=buttons.at(5).bottomleft.x && x<=buttons.at(5).topright.x )
		&& ( y>=(buttons.at(5).bottomleft.y) && y<=(buttons.at(5).topright.y) )){
	
	}
	else if(x>=buttons.at(4).bottomleft.x && x<=buttons.at(4).topright.x){
		if( y>=buttons.at(4).bottomleft.y && y<=buttons.at(4).topright.y ){//start
				
			cout<<"got here"<<endl;	
			if(Algorithm == 0){
				if(inputState == 0){
					Time += 0.1;
					inputState = 3;
				}
				else if(inputState == 3){
					inputState = 0;
					Time = 0;
				}

				start_morphed_grid();
			}

			if(Algorithm){
				inputState = 5;
			}
		}
	}
}

//*************************************************************************************
//Name: pointSelect
//Description: Handles the case when the user is selecting a point in the main window
//		area, using the mouse (for data point and radius inputs)
//Function calls: constructCurve(), buildALTable()
//Preconditions: x and y are valid points, within the central window area
//Postconditions: The mouse event is handled appropriately
//Parameters: GLint x, y, the mouse position
//Returns: Nothing
void pointSelect(GLint button, GLint x, GLint y)
{


	if(button == GLUT_LEFT_BUTTON)//Select another new point
	{
		//find point
		Point2D point(x,y);
		if(inputState == 1){
			cout<<y<<" "<<WINDOW_HEIGHT - B_HEIGHT<<endl;
			if(y < (WINDOW_HEIGHT - B_HEIGHT)){
				findPos(gridPointsSource,point,currPoint);
			}
		}
		else if(inputState == 2){
			if(y < (WINDOW_HEIGHT - B_HEIGHT)){
				findPos(gridPointsDes,point,currPoint);
			}
		}else if(inputState == 4){
			if(x < WINDOW_WIDTH/2 - offset){

				//source
				sourcePoints.push_back( Point2D(x,y));

			}else if(x > WINDOW_WIDTH/2 +offset){
				desPoints.push_back( Point2D(x,y));

			}//else if
		}
	}
	else if(button == GLUT_MIDDLE_BUTTON)//Stop selecting points
	{
		if(inputState == 1 || inputState == 2)
			inputState = 0;
	}
	else if(button == GLUT_RIGHT_BUTTON)
	{
		//if(inputState==1 && dataPoints.size()>0)//Remove the last selected point
			//dataPoints.pop_back();
	}

	glutPostRedisplay();
}

//*****************************************************************************
//Name: onMouse
//Description: The mouse event callback
//Function calls: buttonPushed, glutPostResdisplay
//Preconditions: x,y is a valid point
//Postconditions: The mouse event is handled
//Parameters: GLint x, y, the mouse position
//				button, the mouse buttons used
//				state, the properties of the mouse
//Returns: Nothing*-
void onMouse(int button, int state, int x, int y)
{
	y = WINDOW_HEIGHT-y;
//	cout<<x<<" "<<y<<endl;

	if( inputState == 1 || inputState == 2 && state==GLUT_DOWN ){ //Data pt selection
		if(currPoint.setx || currPoint.sety){
			currPoint.setx = false;
			currPoint.sety = false;
		}
	
			pointSelect(button, x, y);
	}
	else if(menuState >= 1 && state==GLUT_UP)//End the menu
		makeMenuSelection(x, y);
	else if( state==GLUT_DOWN ) 
		buttonPushed(x, y);
	else if(inputState == 4){
		pointSelect(button,x,y);
		buttonPushed(x, y);
	}

	 if(inputState == 3 && state == GLUT_UP){
			Time += 0.1;
			if(Time >= 1)
				Time -=1;
			start_morphed_grid();
	}

	 if(inputState == 5 && state == GLUT_UP){
			cout<<"time: "<<Time<<endl;
			Time += 0.1;
			start_morphed_feature();
			if((Time - 1) > 0)
				Time -= 1;
	 }

	cout<<"input state "<<inputState<<endl;
	cout<<"menu state "<<menuState<<endl;

	//glutPostRedisplay();
}

//*****************************************************************************
//Name: onResize
//Description: Effectively disables window resizing, to make things simpler
//Function calls: glutReshapeWindow
//Preconditions: newWidth and newHeight are valid window dimensions
//Postconditions: The window remains the same size
//Parameters: GLint newWidth, newHeight, the new dimensions to ignore.
//Returns: Nothing
void onResize(GLint newWidth, GLint newHeight)
{
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
}

//load and set texture to 4000,and 5000
void loadImages(){
	if(image1.readBMPFile("image1.bmp")){
		cout<<"successful loaded image1"<<endl;
		image1.setTexture(4000);
	}
	if(image2.readBMPFile("image2.bmp")){
		cout<<"successful loaded image2"<<endl;
		image2.setTexture(5000);
	}
}

void test(){
	int Rows =  image1.nRows;
	int Cols =  image1.nCols;
	cout<<"Rows "<<Rows<<endl;
	cout<<"Cols: "<<Cols<<endl;
	image3.pixel = new RGB[Rows * Cols];
	image3.nRows = Rows;
	image3.nCols = Cols;
	long count = 0;
	for(int i = 0; i < Rows; i++){
		for(int j = 0; j < Cols ; j++){
			int row = count/Cols;
			int col = count%Cols;
			if(row > 500 && col > 500){
				image3.pixel[count] = image1.pixel[count];
				image3.pixel[count].b += 100;
			}else
				image3.pixel[count] = image1.pixel[count];
			count++;
		}
	}

	image3.setTexture(6000);
}

void outputImageSize(){
	Point2D p1 = gridPointsSource.back().at(0);
	Point2D p2 = gridPointsSource.at(0).back();
	cout<<"width: "<<p2.x - p1.x<<endl;
	cout<<"height: "<<p1.y - p2.y<<endl;
}

//make grids point starting bottom left x y
void initGridPoints(int startX,int startY, int width, int height,vector<vector <Point2D> > &v){
	
	int block_width = width/gridCols;
	int block_height = height/gridRows;
	vector<Point2D> tempV;
	int offsetX = startX;
	for(int i = 0; i <= gridRows; i++){
		tempV.clear();
		for(int j = 0; j <= gridCols; j++){
			startX = j * block_width + offsetX;
			startY = i * block_height;
			 tempV.push_back(Point2D((float)startX,(float)startY));
		}
		v.push_back(tempV);
	}
}


//*************************************************************************************
//Name: main
//Description: The main function
//Function calls: glutInit, glutInitDisplayMode, glutInitWindowSize, glutInitWindowPosition,
//		glutCreateWindow, initialize, glutKeyboardFunc, glutMouseFunc, glutDisplayFunc,
//		glutIdleFunc, glutMainLoop
//Preconditions: None
//Postconditions: The program runs in an infinite loop, for displaying grapics.
//					An OpenGL graphics window is displayed
//Parameters: int argc, the number of command line arguments
//				char** argv, the command line arguments
//Returns: int, the exit code
int main(int argc, char** argv)
{
   //Initialization functions
   glutInit(&argc, argv);                         
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  
   glutInitWindowSize(1020, 600);     
   glutInitWindowPosition (WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
   glutCreateWindow ("Program 2");  
   initialize();
   loadImages();
   test();

   initGridPoints(0,0,WINDOW_WIDTH/2-offset,5*WINDOW_HEIGHT/6, gridPointsSource); //create grid points for source
   initGridPoints(WINDOW_WIDTH/2 + offset,0,WINDOW_WIDTH/2-offset,5*WINDOW_HEIGHT/6, gridPointsDes); //des
   outputImageSize();
   //construct curve
   constructCurve( gridPointsSource,controlPointsGridSourceRows,controlPointsGridSourceCols,0,0);
   constructCurve( gridPointsDes,controlPointsGridDesRows,controlPointsGridDesCols,0,0);

   //Call-back functions
   glutDisplayFunc(onDisplay);
   glutKeyboardFunc(onKeyboard);
   glutMouseFunc(onMouse);
   glutReshapeFunc(onResize); 
   glutMotionFunc(onMouseMove);
   
   //Infinite Loop
   glutMainLoop();           
   return 0;
}

//End of Prog2.cpp
