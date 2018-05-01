/*******************************************************************
		   Multi-Part Model Construction and Manipulation
		   Rahul Bilimoria
		   500569144
********************************************************************/

#define _USE_MATH_DEFINES
#define DTR (M_PI/180)

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "CubeMesh.h"
#include "QuadMesh.h"

void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
VECTOR3D ScreenToWorld(int x, int y);

static int currentButton;
static unsigned char currentKey;
float height, width, xCam, yCam, zCam, testX, testY, testZ;
VECTOR3D pos = VECTOR3D(0, 0, 0);
int lastMouseX, lastMouseY;
boolean light;
boolean pressed;
boolean camera;

GLfloat light_position0[] = { -6.0,  12.0, 0.0,1.0 };
GLfloat light_position1[] = { 6.0,  12.0, 0.0,1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };


// Set up lighting/shading and material properties for submarine - upcoming lecture - just copy for now
GLfloat submarine_mat_ambient[] = { 0.25, 0.25, 0.25, 1.0 };
GLfloat submarine_mat_specular[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat submarine_mat_diffuse[] = { 0.774597, 0.774597, 0.774597, 1.0 };
GLfloat submarine_mat_shininess[] = { 76.8 };

QuadMesh *groundMesh = NULL;


struct BoundingBox {
	VECTOR3D min;
	VECTOR3D max;
} BBox;

Metaballs * blobList;
int blobCount;

// Default Mesh Size
int meshSize = 64;

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Scene Modeller");

	initOpenGL(500, 500);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	glutMainLoop();
	return 0;
}



// Setup openGL */
void initOpenGL(int w, int h) {
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and modeling transforms
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.2, 80.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.6, 0.6, 0.6, 0.0);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	// This one is important - renormalize normal vectors 
	glEnable(GL_NORMALIZE);

	//Nice perspective.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//
	// INITIALIZE VARIABLES
	//
	light = false;
	pressed = false;
	camera = false;
	height = 3;
	width = 0.5;
	blobCount = 0;
	xCam = 0.0;
	yCam = 15.0;
	zCam = 22.0;
	lastMouseX = 0;
	lastMouseY = 0;
	testX = 0;
	testY = 0;
	testZ = 0;
	blobList = new Metaballs[10];
	blobList[blobCount].pos = VECTOR3D(0, 0, 0);
	blobList[blobCount].height = 0;
	blobList[blobCount].width = 0;

	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-8.0f, 0.0f, 8.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 16.0);
	groundMesh->InitMesh(meshSize, origin, 16.0, 16.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.5f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	//groundMesh->SetMaterial(ambient,diffuse,specular,shininess);

	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	BBox.min.Set(-8.0f, 0.0, -8.0);
	BBox.max.Set(8.0f, 6.0, 8.0);
}



void display(void)
{
	glShadeModel(GL_SMOOTH); //updates lighting
	if (light) {
		glShadeModel(GL_FLAT);
	}
	if (pressed && !camera) { //updates mesh
		groundMesh->UpdateMesh(blobList, blobCount);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Set up the camera
	gluLookAt(xCam, yCam, zCam, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Submarine

	// Set submarine material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);

	// Apply transformations to move submarine

	// ...
	// Apply transformations to construct submarine

	// Draw ground
	groundMesh->DrawMesh(meshSize);

	glutSwapBuffers();
}

// Called at initialization and whenever user resizes the window */
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.2, 40.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y) {
	currentButton = button;
	POINT mouse;
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) { //creates a new blob
			if (!camera) {
				GetCursorPos(&mouse);
				ScreenToClient(GetForegroundWindow(), &mouse);
				pos = ScreenToWorld(mouse.x, mouse.y);
				blobList[blobCount].pos = VECTOR3D(pos.x, pos.y, pos.z);
				blobList[blobCount].height = height;
				blobList[blobCount].width = width;
				pressed = true;
			}
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) { //drops a blob onto the ground
			if (!camera) {
				if (pressed) {
					blobCount++;
					if (blobCount > 9)
						blobCount = 9;
					blobList[blobCount].pos = VECTOR3D(0, 0, 0);
					blobList[blobCount].height = 0;
					blobList[blobCount].width = 0;
					pressed = false;
				}
			}
		}
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void mouseMotionHandler(int xMouse, int yMouse)  {
	POINT mouse;
	GetCursorPos(&mouse);
	if (camera) {  //Moves based on how the mouse moves
		if (lastMouseX < xMouse) {
			xCam -= 0.5;
			if (xCam < -15)
				xCam = -15;
		}
		else if (lastMouseX > xMouse) {
			xCam += 0.5;
			if (xCam > 15)
				xCam = 15;
		}
		else if (lastMouseY < yMouse) {
			yCam -= 0.5;
			if (yCam < 3)
				yCam = 3;
		}
		else if (lastMouseY > yMouse) {
			yCam += 0.5;
			if (yCam > 27)
				yCam = 27;
		}
	}
	else {
		if (currentButton == GLUT_LEFT_BUTTON) {
			if (pressed) {
				ScreenToClient(GetForegroundWindow(), &mouse);
				pos = ScreenToWorld(mouse.x, mouse.y);
				blobList[blobCount].pos = VECTOR3D(pos.x, pos.y, pos.z);
				blobList[blobCount].height = height;
				blobList[blobCount].width = width;
			}
		}
	}
	lastMouseX = xMouse;
	lastMouseY = yMouse;
	glutPostRedisplay();
}


VECTOR3D ScreenToWorld(int x, int y) { // converts mouse coordinates to world coordinates
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return VECTOR3D(posX, posY, posZ);
}

/* Handles input from the keyboard, non-arrow keys */
void keyboard(unsigned char key, int x, int y) {
	switch (key) { // changes height and width of blob
	case 'w':
		height += 0.1;
		if (height > 5)
			height = 5;
		break;
	case 's':
		height -= 0.1;
		if (height < 1)
			height = 1;
		break;
	case 'a':
		width += 0.05;
		if (width > 1)
			width = 1;
		break;
	case 'd':
		width -= 0.05;
		if (width < 0.1)
			width = 0.1;
		break;
	}
	if (pressed && !camera) {
		system("cls"); // this line only works for windows (i think)
		std::cout << "Height: " << height << std::endl;
		std::cout << "Width: " << width << std::endl;
		blobList[blobCount].height = height;
		blobList[blobCount].width = width;
	}
	glutPostRedisplay();
}

void functionKeys(int key, int x, int y) {
	VECTOR3D min, max;

	if (key == GLUT_KEY_F1)
		camera = !camera;
	else if (key == GLUT_KEY_F2)
		light = !light;
	if (camera) {
		if (key == GLUT_KEY_UP) { //zooms in and out
			zCam -= 0.5;
			if (zCam <= 5)
				zCam = 5;
		}
		else if (key == GLUT_KEY_DOWN) {
			zCam += 0.5;
			if (zCam >= 30)
				zCam = 30;
		}
	}
	glutPostRedisplay();
}