//
// AVT demo light 
// based on demos from GLSL Core Tutorial in Lighthouse3D.com   
//
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//

#include <math.h>
#include <iostream>
#include <sstream>

#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"

#define CAPTION "AVT Per Fragment Phong Lightning Demo"
int WindowHandle = 0;
int WinX = 640, WinY = 480;


//isto fui eu que pus
int lightFlag = 1; //ambiente
int lightFlag2 = 1; //tudo o resto
int lightFlag3 = 1; //holofotes
int cameraFlag = 1;
float cylinderX = 15;
float cylinderY = 1.5;
float cylinderZ = 2;
int angulo = 0;
float incremento = 0;
float incremento2 = 0;
int orangeRot = 0;
float orangeX = 0;
float orangeZ = rand()%33;
float orangeSpeed = 0;

unsigned int FrameCount = 0;

VSShaderLib shader;

struct MyMesh mesh[906];
int objId=0; //id of the object mesh - to be used as index of mesh: mesh[objID] means the current mesh


//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId;
GLint lPos_uniformId2;
GLint lPos_uniformId3;
GLint lPos_uniformId4;
GLint lPos_uniformId5;
GLint lPos_uniformId6;
GLint lStr_uniformId;
GLint lStr_uniformId2;
GLint lStr_uniformId3;
GLint lPos_uniformId7;
GLint lDir_uniformId;
GLint lAngle_uniformId;
GLint lPos_uniformId8;
	
// Camera Position
float camX, camY, camZ;
float followCamX, followCamZ;
float followRadius = 5;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];
float lightPos[4] = {40.0f, -1.0f, 2.0f, 1.0f};
float lightPos2[4] = { 0.0f, 6.0f, 100.0f, 1.0f };
float lightPos3[4] = { 20.0f, 1.0f, 20.0f, 1.0f };
float lightPos4[4] = { 1.0f, -3.0f, 1.0f, 1.0f };
float lightPos5[4] = { 10.0f, 6.0f, 10.0f, 1.0f };
float lightPos6[4] = { 5.0f, 6.0f, 5.0f, 1.0f };
float spotAngle = 10.0f;
float lightPos7[4] = {cylinderX,cylinderY,cylinderZ,1.0f};
float lightDir[4] = { 0.0f,0.0f,1.0f,1.0f };

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
    FrameCount = 0;
    glutTimerFunc(1000, timer, 0);
	
}



void move(int value)
{
	if (angulo > 360 || angulo < -360) {
		angulo = 0;
	}
	float converter = angulo * (3.14 / 180);
	cylinderZ += cos(converter) * incremento;
	cylinderX += sin(converter) * incremento;
	if (incremento < 0) {
		incremento += 0.0005f; //para corrigir
	}
	if (incremento > 0) {
		incremento -= 0.0005f;
	}
	if ((incremento > -0.0005f && incremento < 0) || (incremento < 0.0005f && incremento > 0)) { //correcao do bug do carro nunca parar por um erro qualquer de computacao
		incremento = 0;
	}

	glutTimerFunc(1, move, 0);
}

void movementOrange(int value) {
	orangeRot += 0.01;
	if (orangeRot >= 360) {
		orangeRot = 0;
	}
	orangeSpeed += 0.0001f;
	if (orangeSpeed > 0.04f) {
		orangeSpeed = 0.04f;
	}
	orangeX += orangeSpeed;
	if (orangeX > 32) {
		orangeX = 0;
		orangeZ= rand()%33;
		orangeSpeed = 0;
	}
	glutTimerFunc(1, movementOrange, 0);
}



void refresh(int value)
{
	//PUT YOUR CODE HERE
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	if (cameraFlag == 1) {
		ortho(-16.0f, 16.0f, -12.0f, 12.0f, 0.1f, 1000.0f);
		//using WinX = 640, WinY = 480 aspect ratio
	}

	else if (cameraFlag == 2 || cameraFlag == 4) {
		perspective(90.0f, ratio, 0.1f, 1000.0f);
	}

	else if (cameraFlag == 3) {
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
}



// ------------------------------------------------------------
//
// Render stufff
//

void renderScene(void) {

	GLint loc;

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	if (cameraFlag == 1) {
		lookAt(cylinderX, 10.0f, cylinderZ, cylinderX, cylinderY, cylinderZ + 0.01, 0, 1, 0);
		changeSize(WinX, WinY);
	}
	else if (cameraFlag == 2) {
		lookAt(cylinderX, 15.0f, cylinderZ, cylinderX, cylinderY, cylinderZ + 0.01, 0, 1, 0);
		changeSize(WinX, WinY);
	}
	else if (cameraFlag == 3) {
		// Change camera position so as to change according to car's rotation
		followCamX = cylinderX + followRadius * sin(-angulo * 3.14f / 180.0f);
		followCamZ = cylinderZ - followRadius * cos(-angulo * 3.14f / 180.0f);

		lookAt(followCamX, cylinderY + 2, followCamZ, cylinderX, cylinderY, cylinderZ, 0, 1, 0);
		changeSize(WinX, WinY);
	}
	else if (cameraFlag == 4) {
		lookAt(camX + cylinderX, camY + 3, camZ + cylinderZ, cylinderX, cylinderY, cylinderZ, 0, 1, 0);
		changeSize(WinX, WinY);
	}
	
	// use our shader
	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates

		//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

		
		if (lightFlag == 1) {
			float res2[4] = { 1.0,1.0,1.0,1.0 };
			glUniform4fv(lStr_uniformId, 1, res2);
		}
		else {
			float res2[4] = { 0.0,0.0,0.0,0.0 };
			glUniform4fv(lStr_uniformId, 1, res2);
		}
		if (lightFlag2 == 1) {
			float res2[4] = { 1.0,1.0,1.0,1.0 };
			glUniform4fv(lStr_uniformId2, 1, res2);
		}
		else {
			float res2[4] = { 0.0,0.0,0.0,0.0 };
			glUniform4fv(lStr_uniformId2, 1, res2);
		}
		if (lightFlag3 == 1) {
			float res2[4] = { 1.0,1.0,1.0,1.0 };
			glUniform4fv(lStr_uniformId3, 1, res2);
		}
		else {
			float res2[4] = { 0.0,0.0,0.0,0.0 };
			glUniform4fv(lStr_uniformId3, 1, res2);
		}

		float res[4];
		multMatrixPoint(VIEW, lightPos,res);   //lightPos definido em World Coord so is converted to eye space
		glUniform4fv(lPos_uniformId, 1, res);

		float res3[4];
		multMatrixPoint(VIEW, lightPos2, res3);   //lightPos definido em World Coord so is converted to eye space
		glUniform4fv(lPos_uniformId2, 1, res3);

		float res4[4];
		multMatrixPoint(VIEW, lightPos3, res4);   //lightPos definido em World Coord so is converted to eye space
		glUniform4fv(lPos_uniformId3, 1, res4);

		float res5[4];
		multMatrixPoint(VIEW, lightPos4, res5);   //lightPos definido em World Coord so is converted to eye space
		glUniform4fv(lPos_uniformId4, 1, res5);

		float res6[4];
		multMatrixPoint(VIEW, lightPos5, res6);   //lightPos definido em World Coord so is converted to eye space
		glUniform4fv(lPos_uniformId5, 1, res6);

		float res7[4];
		multMatrixPoint(VIEW, lightPos6, res7);   //lightPos definido em World Coord so is converted to eye space
		glUniform4fv(lPos_uniformId6, 1, res7);

		

		lPos_uniformId7 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[6]");
		lPos_uniformId8 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[7]");
		
		float res8[4] = {cylinderX+1.0f,cylinderY,cylinderZ,1.0f};
		float res11[4];
		multMatrixPoint(VIEW, res8, res11);
		glUniform4fv(lPos_uniformId7, 1, res11);

		float res9[4] = { cylinderX-1.0f,cylinderY,cylinderZ,1.0f };
		float res10[4];
		multMatrixPoint(VIEW, res9, res10);
		glUniform4fv(lPos_uniformId8, 1, res10);



		



	objId=0;
	for (int i = 0 ; i < 30; ++i) {
		for (int j = 0; j < 30; ++j) {
			// send the material
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, mesh[objId].mat.ambient);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, mesh[objId].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc,mesh[objId].mat.shininess);
			pushMatrix(MODEL);
			translate(MODEL, i*1.0f, 0.0f, j*1.0f);

			// send matrices to OGL
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			// Render mesh
			glBindVertexArray(mesh[objId].vao);
			
			if (!shader.isProgramValid()) {
				printf("Program Not Valid!\n");
				exit(1);	
			}
			glDrawElements(mesh[objId].type,mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			popMatrix(MODEL);
			objId++;
		}
	}

	objId = 900;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);
	
	translate(MODEL, cylinderX, cylinderY, cylinderZ);
	rotate(MODEL, -90, 1.0f, 0, 0);
	rotate(MODEL, angulo, 0, 0, 1.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(mesh[objId].vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

	objId = 901;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	
	pushMatrix(MODEL);

	translate(MODEL, cylinderX, cylinderY, cylinderZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, -0.4f, -0.25f, 0.25f);
	rotate(MODEL, -90, 1.0f, 0, 0);
	rotate(MODEL, -90, 0, 0, 1.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(mesh[objId].vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

	objId = 902;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, cylinderX, cylinderY, cylinderZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, -0.4f, -0.25f, -0.25f);
	rotate(MODEL, -90, 1.0f, 0, 0);
	rotate(MODEL, -90, 0, 0, 1.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(mesh[objId].vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

	objId = 903;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	//rotate(MODEL, angulo, 0, cylinderY, cylinderZ);
	translate(MODEL, cylinderX, cylinderY, cylinderZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, 0.4f,- 0.25f,- 0.25f);
	rotate(MODEL, -90, 1.0f, 0, 0);
	rotate(MODEL, -90, 0, 0, 1.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(mesh[objId].vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

	objId = 904;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, cylinderX, cylinderY, cylinderZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, 0.4f, -0.25f, 0.25f);
	rotate(MODEL, -90, 1.0f, 0, 0);
	rotate(MODEL, -90, 0, 0, 1.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(mesh[objId].vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

	objId = 905;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);


	rotate(MODEL,orangeRot,0,1.0f,0);
	translate(MODEL, orangeX, 2.0f, orangeZ);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(mesh[objId].vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch (key) {

		case 27:
			glutLeaveMainLoop();
			break;

		case 'l': printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r); break;
		case 'm': glEnable(GL_MULTISAMPLE); break;
		//case 'n': glDisable(GL_MULTISAMPLE); break;
		case '1': cameraFlag = 1; break;
		case '2': cameraFlag = 2; break;
		case '3': cameraFlag = 3; break;
		case '4': cameraFlag = 4; break;

		case 'w': if (incremento <= 0.25f) {  incremento += 0.01f;  } break;
		case 's': if (incremento >= -0.25f) { incremento -= 0.01f;  } break;
		case 'a': angulo += 5; break;
		case 'd': angulo -= 5; break;
		case 'n': if (lightFlag == 1) { lightFlag = 0; }else { lightFlag = 1; } break;
		case 'c': if (lightFlag2 == 1) { lightFlag2 = 0; } else { lightFlag2 = 1; } break;
		case 'h': if (lightFlag3 == 1) { lightFlag3 = 0; } else { lightFlag3 = 1; } break;
	}
}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN)  {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX =  - xx + startX;
	deltaY =    yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camY = rAux *   						       sin(betaAux * 3.14f / 180.0f);

//  uncomment this if not using an idle or refresh func
//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);

//  uncomment this if not using an idle or refresh func
//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0,"colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	//glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos[0]");
	lPos_uniformId2 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[1]");
	lPos_uniformId3 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[2]");
	lPos_uniformId4 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[3]");
	lPos_uniformId5 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[4]");
	lPos_uniformId6 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[5]");
	lStr_uniformId = glGetUniformLocation(shader.getProgramIndex(), "luzAmbiente");
	lStr_uniformId2 = glGetUniformLocation(shader.getProgramIndex(), "luzDifusa");
	lStr_uniformId3 = glGetUniformLocation(shader.getProgramIndex(), "luzHolofote");

	
	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
	
	return(shader.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	// set the camera position based on its spherical coordinates
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);


	
	float amb[]= {0.2f, 0.15f, 0.1f, 1.0f};
	float diff[] = {0.8f, 0.6f, 0.4f, 1.0f};
	float spec[] = {0.8f, 0.8f, 0.8f, 1.0f};
	float emissive[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float shininess= 100.0f;
	int texcount = 0;

	float amb1[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float diff1[] = { 0.8f, 0.1f, 0.1f, 1.0f };
	float spec1[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	shininess = 500.0;

	float amb2[] = { 1.0f, 0.0f, 0.2f, 1.0f };
	float diff2[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float spec2[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	objId = 0;
	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 30; j++) {
			if ((i % 2 == 0 && j % 2 == 0) || (j % 2 != 0 && i % 2 != 0)) {
				memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
				memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
				memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
				memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
				mesh[objId].mat.shininess = shininess;
				mesh[objId].mat.texCount = texcount;
				createCube();
			}
			else {
				memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
				memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
				memcpy(mesh[objId].mat.specular, spec1, 4 * sizeof(float));
				memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
				mesh[objId].mat.shininess = shininess;
				mesh[objId].mat.texCount = texcount;
				createCube();
			}
			objId++;
		}
	}

	objId = 900;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCylinder(1.0f,0.3f,64);

	objId = 901;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.02f, 0.3f, 64, 64);

	objId = 902;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.02f, 0.3f, 64, 64);

	objId = 903;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.02f, 0.3f, 64, 64);

	objId = 904;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.02f, 0.3f, 64, 64);

	objId = 905;
	memcpy(mesh[objId].mat.ambient, amb2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(1.0f, 20);




	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}


// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char **argv) {

//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_MULTISAMPLE);

	glutInitContextVersion (3, 3);
	glutInitContextProfile (GLUT_CORE_PROFILE );
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100,100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	glutTimerFunc(0, move, 0);
	glutTimerFunc(0, movementOrange, 0);
	glutIdleFunc(renderScene);  // Use it for maximum performance
	//glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc ( mouseWheel ) ;

//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf ("Vendor: %s\n", glGetString (GL_VENDOR));
	printf ("Renderer: %s\n", glGetString (GL_RENDERER));
	printf ("Version: %s\n", glGetString (GL_VERSION));
	printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);

}

