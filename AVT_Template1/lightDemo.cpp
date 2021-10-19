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

#include <IL/il.h>

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"
#include "Texture_Loader.h"

#define CAPTION "AVT Per Fragment Phong Lightning Demo"
int WindowHandle = 0;
int WinX = 640, WinY = 480;


//isto fui eu que pus
int lightFlag = 1; //ambiente
int lightFlag2 = 1; //tudo o resto
int lightFlag3 = 1; //holofotes
int cameraFlag = 1;
float cylinderX = 5.0f;
float cylinderY = 1.5f;
float cylinderZ = -48.0f;
float cylinderXbackup = 5.0f;
float cylinderZbackup = -48.0f;
int angulo = 0;
float incremento = 0; //acelaracao do carro
int orangeRot = 0;
float orangeX = 0;
float orangeZ = (rand() % 100) - 45;
float orange2Z = (rand() % 100) - 45;
float orange3Z = (rand() % 100) - 45;
float orange4Z = (rand() % 100) - 45;
float orangeSpeed = 0;

float torusX[633];
float torusZ[633];
int renderedFlag = 0;

unsigned int FrameCount = 0;

VSShaderLib shader;

struct MyMesh mesh[789];
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

GLint tex_loc, tex_loc1, tex_loc2;
GLint texMode_uniformId;

GLuint TextureArray[3];
	
// Camera Position
float camX, camY, camZ;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 5.0f;

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

void colision(int value) {

	if (cylinderX > orangeX - 1 && cylinderX < orangeX + 1 && cylinderZ > orangeZ - 1 && cylinderZ < orangeZ + 1) {
		cylinderX = 5;
		cylinderZ = -48;
		incremento = 0;
	}
	else if (cylinderX > orangeX - 1 && cylinderX < orangeX + 1 && cylinderZ > orange2Z - 1 && cylinderZ < orange2Z + 1) {
		cylinderX = 5;
		cylinderZ = -48;
		incremento = 0;
	}
	else if (cylinderX > orangeX - 1 && cylinderX < orangeX + 1 && cylinderZ > orange3Z - 1 && cylinderZ < orange3Z + 1) {
		cylinderX = 5;
		cylinderZ = -48;
		incremento = 0;
	}
	else if (cylinderX > orangeX - 1 && cylinderX < orangeX + 1 && cylinderZ > orange4Z - 1 && cylinderZ < orange4Z + 1) {
		cylinderX = 5;
		cylinderZ = -48;
		incremento = 0;
	}
	for (int i = 0; i < 633; i++) {
		if (cylinderX < torusX[i] + 0.7f && cylinderX > torusX[i]-0.7f && cylinderZ > torusZ[i] - 0.7f && cylinderZ < torusZ[i] + 0.7f && i < 628) {
			if (cylinderX <= torusX[i]) {
				torusX[i] += 0.01;
				if (cylinderZ <= torusZ[i]) {
					torusZ[i] += 0.01;
				}
				else {
					torusZ[i] -= 0.01;
				}
			}
			else {
				torusX[i] -= 0.01;
				if (cylinderZ <= torusZ[i]) {
					torusZ[i] += 0.01;
				}
				else {
					torusZ[i] -= 0.01;
				}
			}
			incremento = 0;
			cylinderX = cylinderXbackup;
			cylinderZ = cylinderZbackup;
		} else if (cylinderX < torusX[i] + 4.7f && cylinderX > torusX[i] -0.7f && cylinderZ > torusZ[i] -0.7f && cylinderZ < torusZ[i] + 1.7f && i >= 628) {
			if (cylinderX <= torusX[i]+2.0f) {
				torusX[i] += 0.01;
				if (cylinderZ <= torusZ[i]+1.0f) {
					torusZ[i] += 0.01;
				}
				else {
					torusZ[i] -= 0.01;
				}
			}
			else {
				torusX[i] -= 0.01;
				if (cylinderZ <= torusZ[i]+1.0f) {
					torusZ[i] += 0.01;
				}
				else {
					torusZ[i] -= 0.01;
				}
			}
			incremento = 0;
			cylinderX = cylinderXbackup;
			cylinderZ = cylinderZbackup;
		}
		else {
			cylinderXbackup = cylinderX;
			cylinderZbackup = cylinderZ;
		}
	}

	glutTimerFunc(1, colision, 0);
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
	if (orangeX > 50) {
		orangeX = -50;
		orangeZ= (rand() % 100) - 48;
		orange2Z = (rand() % 100) - 48;
		orange3Z = (rand() % 100) - 48;
		orange4Z = (rand() % 100) - 48;
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

	else if (cameraFlag == 2) {
		perspective(90.0f, ratio, 0.1f, 1000.0f);
	}

	else if (cameraFlag == 3) {
		perspective(70.13f, ratio, 0.1f, 1000.0f);
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
	if (cameraFlag == 1) { //Fixed ortho camera
		lookAt(cylinderX, 10.0f, cylinderZ, cylinderX, cylinderY, cylinderZ + 0.01, 0, 1, 0);
		changeSize(WinX, WinY);
	}
	else if (cameraFlag == 2) { //Fixed perspective camera
		lookAt(cylinderX, 15.0f, cylinderZ, cylinderX, cylinderY, cylinderZ + 0.01, 0, 1, 0);
		changeSize(WinX, WinY);
	}
	else if (cameraFlag == 3) { //Movement camera
		lookAt(camX + cylinderX, camY + cylinderY, camZ + cylinderZ, cylinderX, cylinderY, cylinderZ, 0, 1, 0);

		//Translate camera to car's coordinates, rotate, and return back to original position
		translate(VIEW, cylinderX, cylinderY, cylinderZ);
		rotate(VIEW, -angulo, 0.0, 1.0, 0.0);
		translate(VIEW, -cylinderX, -cylinderY, -cylinderZ);

		changeSize(WinX, WinY);
	}
	
	// use our shader
	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates

		//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

		
		if (lightFlag == 1) {
			float res2[4] = { 1.0,1.0,1.0,0.0 };
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

		

		lPos_uniformId7 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[6]"); //holofote
		lPos_uniformId8 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[7]"); //holofote
		
		float res8[4] = {cylinderX+0.5f,cylinderY,cylinderZ+0.5f,1.0f};
		float res11[4];
		multMatrixPoint(VIEW, res8, res11);
		glUniform4fv(lPos_uniformId7, 1, res11);

		float res9[4] = { cylinderX-0.5f,cylinderY,cylinderZ+0.5f,1.0f };
		float res10[4];
		multMatrixPoint(VIEW, res9, res10);
		glUniform4fv(lPos_uniformId8, 1, res10);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

		glUniform1i(tex_loc, 0);
		glUniform1i(tex_loc1, 1);
		glUniform1i(tex_loc2, 2);

	//MESA
	objId = 0;
	// send the material
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);
	translate(MODEL, -100.0f, 0, -100.0f);
	scale(MODEL, 200.0f, 1.0f, 200.0f);


	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glUniform1i(texMode_uniformId, 0); // linha da textura
	glBindVertexArray(mesh[objId].vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

	//CARRO
	objId = 1;
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

	//RODAS
	objId = 2;
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

	objId = 3;
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

	objId = 4;
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

	objId = 5;
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

	//LARANJAS
	objId = 6;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);


	rotate(MODEL,orangeRot, 0, 1.0f, 0);
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

	objId = 7;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);


	rotate(MODEL, orangeRot, 0, 1.0f, 0);
	translate(MODEL, orangeX, 2.0f, orange2Z);

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

	objId = 8;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);


	rotate(MODEL, orangeRot, 0, 1.0f, 0);
	translate(MODEL, orangeX, 2.0f, orange3Z);

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

	objId = 9;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);


	rotate(MODEL, orangeRot, 0, 1.0f, 0);
	translate(MODEL, orangeX, 2.0f, orange4Z);

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

	//MANTEIGAS
	objId = 10;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	if (renderedFlag == 0) {
		torusX[629] = 0.5f;
		torusZ[629] = 0;
	}
	translate(MODEL, torusX[629], 1.0f, torusX[629]);
	scale(MODEL, 4.0f, 2.0f, 2.0f);

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

	objId = 11;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);
	if (renderedFlag == 0) {
		torusX[630] = 4.5f;
		torusZ[630] = 20.0f;
	}
	translate(MODEL, torusX[630], 1.0f, torusZ[630]);
	scale(MODEL, 4.0f, 2.0f, 2.0f);

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

	objId = 12;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);
	if (renderedFlag == 0) {
		torusX[631] = 4.5f;
		torusZ[631] = -25.0f;
	}
	translate(MODEL, torusX[631], 1.0f, torusZ[631]);
	scale(MODEL, 4.0f, 2.0f, 2.0f);

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

	objId = 13;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);
	if (renderedFlag == 0) {
		torusX[632] = 0.5f;
		torusZ[632] = -40.0f;
	}
	translate(MODEL, torusX[632], 1.0f, torusZ[632]);
	scale(MODEL, 4.0f, 2.0f, 2.0f);

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


	//CHEERIOS
	objId = 14;

	for (int i = -50; i < 48; ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (renderedFlag == 0) {
			torusX[i + 50] = 1.0f;
			torusZ[i + 50] = 1.5f + i;
		}
		translate(MODEL, torusX[i + 50], 1.15f, torusZ[i + 50]);
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
		objId++;
	}

	//CHEERIOS
	objId = 130;

	for (int i = -58; i < 57; ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (renderedFlag == 0) {
			torusX[i + 58 + 98] = 9.0f;
			torusZ[i + 58 + 98] = 1.5f + i;
		}
		
		translate(MODEL, torusX[i + 58 + 98], 1.15f, torusZ[i + 58 + 98]);
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
		objId++;
	}
	

	//CHEERIOS
	objId = 246;

	for (int i = -50; i < 10; ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (renderedFlag == 0) {
			torusX[i + 50 + 213] = 0.0f + i;
			torusZ[i + 50 + 213] = 57.5f;
		}
		translate(MODEL, torusX[i + 50 + 213], 1.15f, torusZ[i + 50 + 213]);
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
		objId++;
	}
	

	//CHEERIOS
	objId = 362;

	for (int i = -50; i < 10; ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (renderedFlag == 0) {
			torusX[i + 50 + 273] = 0.0f + i;
			torusZ[i + 50 + 273] = -57.5f;
		}
		translate(MODEL, torusX[i + 50 + 273], 1.15f, torusZ[i + 50 + 273]);
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
		objId++;
	}
	
	//CHEERIOS
	objId = 478;

	for (int i = -58; i < 57; ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (renderedFlag == 0) {
			torusX[i + 58 + 333] = -50.0f;
			torusZ[i + 58 + 333] = 1.5f + i;
		}
		translate(MODEL, torusX[i + 58 + 333], 1.15f, torusZ[i + 58 + 333]);
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
		objId++;
	}
	
	//CHEERIOS
	objId = 595;
	for (int i = -40; i < 1; ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (renderedFlag == 0) {
			torusX[i + 40 + 448] = 0 + i;
			torusZ[i + 40 + 448] = -49.5f;
		}
		translate(MODEL, torusX[i + 40 + 448], 1.15f, torusZ[i + 40 + 448]);
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
		objId++;
	}
	
	objId = 639;
	for (int i = -40; i < 1; ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (renderedFlag == 0) {
			torusX[i + 40 + 489] = 0 + i;
			torusZ[i + 40 + 489] = 49.5f;
		}
		translate(MODEL, torusX[i + 40 + 489], 1.15f, torusZ[i + 40 + 489]);
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
		objId++;
	}

	
	//CHEERIOS
	objId = 680;

	for (int i = -50; i < 48; ++i) {
		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (renderedFlag == 0) {
			torusX[i + 50 + 530] = -40;
			torusZ[i + 50 + 530] = 1.5f + i;
		}
		translate(MODEL, torusX[i + 50 + 530], 1.15f, torusZ[i + 50 + 530]);
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
		objId++;
	}

	//Transparency
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	
	//VIDRO
	objId = 778;
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
	translate(MODEL, -0.5f, 0.25f, 0.5f);
	//rotate(MODEL, -90, 1.0f, 0, 0);
	//rotate(MODEL, -90, 0, 0, 1.0f);
	scale(MODEL, 1.0f, 1.0f, 0.1f);

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

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	renderedFlag = 1;

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


		case 'w': if (incremento <= 0.15f) {  incremento += 0.01f;  } break;
		case 's': if (incremento >= -0.15f) { incremento -= 0.01f;  } break;
		case 'a': angulo += 4; if (incremento <= 0.02f) { incremento += 0.01f; } break;
		case 'd': angulo -= 4; if (incremento <= 0.02f) { incremento += 0.01f; } break;
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
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
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
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");

	
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

	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();


	glGenTextures(3, TextureArray);
	Texture2D_Loader(TextureArray, "stone.tga", 0);
	Texture2D_Loader(TextureArray, "checker.png", 1);
	Texture2D_Loader(TextureArray, "lightwood.tga", 2);

	
	//Cream
	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	//Red
	float amb1[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float diff1[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float spec1[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	shininess = 500.0;

	//Orange
	float amb2[] = { 0.7f, 0.3f, 0.0f, 1.0f };
	float diff2[] = { 0.7f, 0.3f, 0.0f, 1.0f };
	float spec2[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	//Yellow
	float amb3[] = { 0.8f, 0.8f, 0.0f, 1.0f };
	float diff3[] = { 0.8f, 0.8f, 0.0f, 1.0f };
	float spec3[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	//Transparent Blue
	float amb4[] = { 0.3f, 0.0f, 1.0f, 1.0f };
	float diff4[] = { 0.8f, 0.6f, 0.4f, 0.5f };
	float spec4[] = { 1.0f, 1.0f, 1.0f, 1.0f };


	//MESA
	objId = 0;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//CARRO
	objId = 1;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCylinder(1.0f,0.3f,64);

	//RODAS
	objId = 2;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.02f, 0.3f, 64, 64);

	objId = 3;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.02f, 0.3f, 64, 64);

	objId = 4;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.02f, 0.3f, 64, 64);

	objId = 5;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createTorus(0.02f, 0.3f, 64, 64);

	//LARANJAS
	objId = 6;
	memcpy(mesh[objId].mat.ambient, amb2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(1.0f, 20);

	objId = 7;
	memcpy(mesh[objId].mat.ambient, amb2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(1.0f, 20);

	objId = 8;
	memcpy(mesh[objId].mat.ambient, amb2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(1.0f, 20);

	objId = 9;
	memcpy(mesh[objId].mat.ambient, amb2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(1.0f, 20);

	//MANTEIGAS
	objId = 10;
	memcpy(mesh[objId].mat.ambient, amb3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 11;
	memcpy(mesh[objId].mat.ambient, amb3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 12;
	memcpy(mesh[objId].mat.ambient, amb3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 13;
	memcpy(mesh[objId].mat.ambient, amb3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//Cheerios
	objId = 14;
	for (int i = -50; i < 48; ++i) {
		memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createTorus(0.02f, 0.3f, 64, 64);
		objId++;
	}

	objId = 130;
	for (int i = -58; i < 57; ++i) {
		memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createTorus(0.02f, 0.3f, 64, 64);
		objId++;
	}

	objId = 246;
	for (int i = -50; i < 10; ++i) {
		memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createTorus(0.02f, 0.3f, 64, 64);
		objId++;
	}

	objId = 362;
	for (int i = -50; i < 10; ++i) {
		memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createTorus(0.02f, 0.3f, 64, 64);
		objId++;
	}

	objId = 478;
	for (int i = -58; i < 57; ++i) {
		memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createTorus(0.02f, 0.3f, 64, 64);
		objId++;
	}

	objId = 595;
	for (int i = -40; i < 1; ++i) {
		memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createTorus(0.02f, 0.3f, 64, 64);
		objId++;
	}

	objId = 639;
	for (int i = -40; i < 1; ++i) {
		memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createTorus(0.02f, 0.3f, 64, 64);
		objId++;
	}

	objId = 680;
	for (int i = -50; i < 48; ++i) {
		memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createTorus(0.02f, 0.3f, 64, 64);
		objId++;
	}

	//VIDRO
	objId = 778;
	memcpy(mesh[objId].mat.ambient, amb4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();


	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	glutTimerFunc(0, colision, 0);
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

