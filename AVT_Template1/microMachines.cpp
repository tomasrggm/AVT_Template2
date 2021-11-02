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
#include <fstream>
#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>
#include <IL/il.h>

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"
#include "Texture_Loader.h"
#include "avtFreeType.h"
#include "meshFromAssimp.h"
using namespace std;

#define CAPTION "AVT MicroMachines Project - Delivery 1"

#define frand()			((float)rand()/RAND_MAX)
#define M_PI			3.14159265
#define MAX_PARTICULAS  1500

// Created an instance of the Importer class in the meshFromAssimp.cpp file
extern Assimp::Importer importer;
// the global Assimp scene object
extern const aiScene* scene;
char model_dir[50];  //initialized by the user input at the console
// scale factor for the Assimp model to fit in the window
extern float scaleFactor;

int WindowHandle = 0;
int WinX = 640, WinY = 480;


//isto fui eu que pus
int lightFlag = 1; //directional
int lightFlag2 = 1; //tudo o resto
int lightFlag3 = 1; //holofotes
int fogFlag = 1; //fog
int cameraFlag = 1;
float carX = 5.0f;
float carY = 1.5f;
float carZ = -48.0f;
float carXbackup = 5.0f;
float carZbackup = -48.0f;
int angulo = 0;
float accelerationIncrement = 0; //acelaracao do carro
int orangeRot = 0;
float orangeX[4] = { 0,0,0,0 };
float orangeZ[4] = { (rand() % 100) - 45, (rand() % 100) - 45, (rand() % 100) - 45, (rand() % 100) - 45 };
float orangeSpeed[4] = { 0, 0, 0, 0 };
float colisionBounce = 0.05f;

float torusX[633];
float torusZ[633];
int renderedFlag = 0;
int pauseFlag = 0;
int vidas = 5;
int pontos = 0;

unsigned int FrameCount = 0;

VSShaderLib shader, shaderText;

bool normalMapKey = TRUE; // by default if there is a normal map then bump effect is implemented. press key "b" to enable/disable normal mapping 

struct MyMesh mesh[786];
vector<struct MyMesh> myMeshes;
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
GLint fog_uniformId;

GLint normalMap_loc;
GLint specularMap_loc;
GLint diffMapCount_loc;

GLuint TextureArray[3];
	
// Camera Position
float camX, camY, camZ;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 5.0f;

//Particles fireworks
int fireworks = 0;

typedef struct {
	float	life;		// vida
	float	fade;		// fade
	float	r, g, b;    // color
	GLfloat x, y, z;    // posição
	GLfloat vx, vy, vz; // velocidade 
	GLfloat ax, ay, az; // aceleração
} Particle;

Particle particula[MAX_PARTICULAS];
int dead_num_particles = 0;

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
float lightPos7[4] = {carX,carY,carZ,1.0f};
float lightDir[4] = { 0.0f,0.0f,1.0f,1.0f };
const std::string font_name = "fonts/arial.ttf";



void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << glutGet(GLUT_WINDOW_WIDTH) << "x" << glutGet(GLUT_WINDOW_HEIGHT) << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
    FrameCount = 0;
    glutTimerFunc(1000, timer, 0);
	
}

void updateParticles()
{
	int i;
	float h;

	/* MÈtodo de Euler de integraÁ„o de eq. diferenciais ordin·rias
	h representa o step de tempo; dv/dt = a; dx/dt = v; e conhecem-se os valores iniciais de x e v */

	//h = 0.125f;
	h = 0.033;
	if (fireworks) {

		for (i = 0; i < MAX_PARTICULAS; i++)
		{
			particula[i].x += (h * particula[i].vx);
			particula[i].y += (h * particula[i].vy);
			particula[i].z += (h * particula[i].vz);
			particula[i].vx += (h * particula[i].ax);
			particula[i].vy += (h * particula[i].ay);
			particula[i].vz += (h * particula[i].az);
			particula[i].life -= particula[i].fade;
		}
	}
}

void iniParticles(void)
{
	GLfloat v, theta, phi;
	int i;

	for (i = 0; i < MAX_PARTICULAS; i++)
	{
		v = 0.8 * frand() + 0.2;
		phi = frand() * M_PI;
		theta = 2.0 * frand() * M_PI;

		particula[i].x = 0.0f;
		particula[i].y = 10.0f;
		particula[i].z = 0.0f;
		particula[i].vx = v * cos(theta) * sin(phi);
		particula[i].vy = v * cos(phi);
		particula[i].vz = v * sin(theta) * sin(phi);
		particula[i].ax = 0.1f; /* simular um pouco de vento */
		particula[i].ay = -0.15f; /* simular a aceleraÁ„o da gravidade */
		particula[i].az = 0.0f;

		/* tom amarelado que vai ser multiplicado pela textura que varia entre branco e preto */
		particula[i].r = 0.882f;
		particula[i].g = 0.552f;
		particula[i].b = 0.211f;

		particula[i].life = 1.0f;		/* vida inicial */
		particula[i].fade = 0.0025f;	    /* step de decrÈscimo da vida para cada iteraÁ„o */
	}
}

void colision(int value) {
	if (pauseFlag == 0) {
		for (int i = 0; i < 4; i++) {
			if (carX > orangeX[i] - 1 && carX < orangeX[i] + 1 && carZ > orangeZ[i] - 1 && carZ < orangeZ[i] + 1) {
				carX = 5;
				carZ = -48;
				accelerationIncrement = 0;
				vidas--;
			}
		}

		for (int i = 0; i < 633; i++) {
			if (carX < torusX[i] + 0.7f && carX > torusX[i] - 0.7f && carZ > torusZ[i] - 0.7f && carZ < torusZ[i] + 0.7f && i < 628) { //Cheerios
				if (carX <= torusX[i]) {
					torusX[i] += colisionBounce;
					if (carZ <= torusZ[i]) {
						torusZ[i] += colisionBounce;
					}
					else {
						torusZ[i] -= colisionBounce;
					}
				}
				else {
					torusX[i] -= colisionBounce;
					if (carZ <= torusZ[i]) {
						torusZ[i] += colisionBounce;
					}
					else {
						torusZ[i] -= colisionBounce;
					}
				}
				accelerationIncrement = 0;
				carX = carXbackup;
				carZ = carZbackup;
			}
			else if (carX < torusX[i] + 4.7f && carX > torusX[i] - 0.7f && carZ > torusZ[i] - 0.7f && carZ < torusZ[i] + 2.7f && i >= 628) { //Manteigas
				if (carX <= torusX[i] + 2.0f) {
					torusX[i] += colisionBounce;
					if (carZ <= torusZ[i] + 1.0f) {
						torusZ[i] += colisionBounce;
					}
					else {
						torusZ[i] -= colisionBounce;
					}
				}
				else {
					torusX[i] -= colisionBounce;
					if (carZ <= torusZ[i] + 1.0f) {
						torusZ[i] += colisionBounce;
					}
					else {
						torusZ[i] -= colisionBounce;
					}
				}
				accelerationIncrement = 0;
				carX = carXbackup;
				carZ = carZbackup;
			}
			else {
				carXbackup = carX;
				carZbackup = carZ;
			}
		}
	}



	glutTimerFunc(1, colision, 0);
}




void move(int value)
{
	if (pauseFlag == 0) {
		if (angulo > 360 || angulo < -360) {
			angulo = 0;
		}
		float converter = angulo * (3.14 / 180);
		carZ += cos(converter) * accelerationIncrement;
		carX += sin(converter) * accelerationIncrement;
		if (accelerationIncrement < 0) {
			accelerationIncrement += 0.0005f; //para corrigir
		}
		if (accelerationIncrement > 0) {
			accelerationIncrement -= 0.0005f;
		}
		if ((accelerationIncrement > -0.0005f && accelerationIncrement < 0) || (accelerationIncrement < 0.0005f && accelerationIncrement > 0)) { //correcao do bug do carro nunca parar por um erro qualquer de computacao
			accelerationIncrement = 0;
		}
	}
	glutTimerFunc(1, move, 0);
}

void movementOrange(int value) {
	if (pauseFlag == 0) {
		orangeRot -= 2;
		if (orangeRot <= -360) {
			orangeRot = 0;
		}

		for (int i = 0; i < 4; i++) {
			orangeSpeed[i] += 0.00005f * (i + 1);

			orangeX[i] += orangeSpeed[i];
			if (orangeX[i] > 50) {
				orangeX[i] = -50;
				orangeZ[i] = (rand() % 100) - 48;
			}
		}
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
		if (w <= h)
			ortho(-16.0, 16.0, -16.0 * (GLfloat)h / (GLfloat)w, 16.0 * (GLfloat)h / (GLfloat)w, -10, 10);
		else
			ortho(-16.0 * (GLfloat)w / (GLfloat)h, 16.0 * (GLfloat)w / (GLfloat)h, -16.0, 16.0, -10, 10);

		//ortho(-16.0f, 16.0f, -16.0f, 16.0f, 0.1f, 1000.0f);
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

void aiRecursive_render(const aiScene* sc, const aiNode* nd)
{
	GLint loc;

	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	pushMatrix(MODEL);

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);
	multMatrix(MODEL, aux);


	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {


		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[nd->mMeshes[n]].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[nd->mMeshes[n]].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[nd->mMeshes[n]].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.emissive");
		glUniform4fv(loc, 1, mesh[nd->mMeshes[n]].mat.emissive);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[nd->mMeshes[n]].mat.shininess);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
		glUniform1i(loc, mesh[nd->mMeshes[n]].mat.texCount);

		unsigned int  diffMapCount = 0;  //read 2 diffuse textures

		//devido ao fragment shader suporta 2 texturas difusas simultaneas, 1 especular e 1 normal map

		glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
		glUniform1i(specularMap_loc, false);
		glUniform1ui(diffMapCount_loc, 0);

		if (mesh[nd->mMeshes[n]].mat.texCount != 0)
			for (unsigned int i = 0; i < mesh[nd->mMeshes[n]].mat.texCount; ++i) {
				if (mesh[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
					if (diffMapCount == 0) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff");
						glUniform1i(loc, mesh[nd->mMeshes[n]].texUnits[i]);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else if (diffMapCount == 1) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff1");
						glUniform1i(loc, mesh[nd->mMeshes[n]].texUnits[i]);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
				}
				else if (mesh[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitSpec");
					glUniform1i(loc, mesh[nd->mMeshes[n]].texUnits[i]);
					glUniform1i(specularMap_loc, true);
				}
				else if (mesh[nd->mMeshes[n]].texTypes[i] == NORMALS) { //Normal map
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitNormalMap");
					if (normalMapKey)
						glUniform1i(normalMap_loc, normalMapKey);
					glUniform1i(loc, mesh[nd->mMeshes[n]].texUnits[i]);

				}
				else printf("Texture Map not supported\n");
			}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// bind VAO
		glBindVertexArray(mesh[nd->mMeshes[n]].vao);

		if (!shader.isProgramValid()) {
			printf("Program Not Valid!\n");
			exit(1);
		}
		// draw
		glDrawElements(mesh[nd->mMeshes[n]].type, mesh[nd->mMeshes[n]].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		aiRecursive_render(sc, nd->mChildren[n]);
	}
	popMatrix(MODEL);
}


void renderScene(void) {
	
	GLint loc;

	FrameCount++;

	float particle_color[4];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	if (cameraFlag == 1) { //Fixed ortho camera
		lookAt(carX, 10.0f, carZ, carX, carY, carZ + 0.01, 0, 1, 0);
		changeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
	else if (cameraFlag == 2) { //Fixed perspective camera
		lookAt(carX, 15.0f, carZ, carX, carY, carZ + 0.01, 0, 1, 0);
		changeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
	else if (cameraFlag == 3) { //Movement camera
		lookAt(camX + carX, camY + carY, camZ + carZ, carX, carY, carZ, 0, 1, 0);

		//Translate camera to car's coordinates, rotate, and return back to original position
		translate(VIEW, carX, carY, carZ);
		rotate(VIEW, -angulo, 0.0, 1.0, 0.0);
		translate(VIEW, -carX, -carY, -carZ);

		changeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
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

		//fog flag
		if (fogFlag == 1) {
			glUniform1i(fog_uniformId, 1);
		}
		else {
			glUniform1i(fog_uniformId, 0);
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
		
		float res8[4] = {carX+0.5f,carY,carZ+0.5f,1.0f};
		float res11[4];
		multMatrixPoint(VIEW, res8, res11);
		glUniform4fv(lPos_uniformId7, 1, res11);

		float res9[4] = { carX-0.5f,carY,carZ+0.5f,1.0f };
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
	glUniform1i(texMode_uniformId, 1);


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
	
	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, -90, 1.0f, 0, 0);
	rotate(MODEL, angulo, 0, 0, 1.0f);

	//aiRecursive_render(scene, scene->mRootNode);

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

	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, -0.4f, -0.25f, 0.25f);
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

	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, -0.4f, -0.25f, -0.25f);
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

	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, 0.4f,- 0.25f,- 0.25f);
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

	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, 0.4f, -0.25f, 0.25f);
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
	for (int i = 0; i < 4; i++) {
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);

		translate(MODEL, orangeX[i], 2.0f, orangeZ[i]);
		rotate(MODEL, orangeRot, 0, 0, 0.1f);
		

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

	//FOLHAS
	objId = 10;
	for (int i = 0; i < 4; i++) {
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);

		translate(MODEL, orangeX[i], 2.0f, orangeZ[i]);
		rotate(MODEL, orangeRot, 0, 0, 1.0f);
		translate(MODEL, 0, 1.0f, 0);

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

	//MANTEIGAS
	objId = 14;
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

	objId = 15;
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

	objId = 16;
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

	objId = 17;
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
	objId = 18;

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
	objId = 134;

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
	objId = 250;

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
	objId = 366;

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
	objId = 482;

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
	objId = 599;
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
	
	objId = 643;
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
	objId = 684;

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
	
	//VIDRO DO CARRO
	objId = 782;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, -0.5f, 0.25f, 0.5f);
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

	//VIDRO NA PISTA
	objId = 783;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, 2.5f, 0.0f, -45.0f);
	scale(MODEL, 5.0f, 5.0f, 0.1f);

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

	if (fireworks) {

		updateParticles();

		// draw fireworks particles
		objId = 784;  //quad for particle

		glBindTexture(GL_TEXTURE_2D, TextureArray[0]); //particle.tga associated to TU0 

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);  //Depth Buffer Read Only

		glUniform1i(texMode_uniformId, 2); // draw modulated textured particles 

		for (int i = 0; i < MAX_PARTICULAS; i++)
		{
			if (particula[i].life > 0.0f) /* sÛ desenha as que ainda est„o vivas */
			{

				/* A vida da partÌcula representa o canal alpha da cor. Como o blend est· activo a cor final È a soma da cor rgb do fragmento multiplicada pelo
				alpha com a cor do pixel destino */

				particle_color[0] = particula[i].r;
				particle_color[1] = particula[i].g;
				particle_color[2] = particula[i].b;
				particle_color[3] = particula[i].life;

				// send the material - diffuse color modulated with texture
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, particle_color);

				pushMatrix(MODEL);
				translate(MODEL, particula[i].x, particula[i].y, particula[i].z);

				// send matrices to OGL
				computeDerivedMatrix(PROJ_VIEW_MODEL);
				glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
				glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
				computeNormalMatrix3x3();
				glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

				glBindVertexArray(mesh[objId].vao);
				glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
				popMatrix(MODEL);
			}
			else dead_num_particles++;
		}

		glDepthMask(GL_TRUE); //make depth buffer again writeable

		if (dead_num_particles == MAX_PARTICULAS) {
			fireworks = 0;
			dead_num_particles = 0;
			printf("All particles dead\n");
		}

	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	renderedFlag = 1;
	glBindTexture(GL_TEXTURE_2D, 0);

	//Texto
	glDisable(GL_DEPTH_TEST);
	//the glyph contains background colors and non-transparent for the actual character pixels. So we use the blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	if (pauseFlag == 1) {
		RenderText(shaderText, "Pausa", 2.0f * WinX / 5.0f, WinY / 2.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	}
	std::string v = std::to_string(vidas);
	std::string p = std::to_string(pontos);
	RenderText(shaderText, "vidas: " + v + " pontos: " + p, 25.0f, 25.0f, 0.4f, 0.5f, 0.8f, 0.2f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);



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

		//case 'l': printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r); break;
		//case 'm': glEnable(GL_MULTISAMPLE); break;
		//case 'n': glDisable(GL_MULTISAMPLE); break;

		//Camera keys
		case '1': cameraFlag = 1; break;
		case '2': cameraFlag = 2; break;
		case '3': cameraFlag = 3; break;

		//Car Movement keys
		case 'q': if (accelerationIncrement <= 0.15f && pauseFlag == 0) {  accelerationIncrement += 0.01f;  } break; //Forwards
		case 'a': if (accelerationIncrement >= -0.15f && pauseFlag == 0) { accelerationIncrement -= 0.01f;  } break; //Backwards
		case 'o': if (pauseFlag == 0) { angulo += 4; if (accelerationIncrement <= 0.02f) { accelerationIncrement += 0.01f; } } break; //Left
		case 'p': if (pauseFlag == 0) { angulo -= 4; if (accelerationIncrement <= 0.02f) { accelerationIncrement += 0.01f; } } break; //Right
		case 's': if (pauseFlag == 1) { pauseFlag = 0; } else{ pauseFlag = 1; } break;
		case 'f': if (fogFlag == 1) { fogFlag = 0; } else { fogFlag = 1; } break;
		case 'e': fireworks = 1; iniParticles(); break;

		//Light keys
		case 'n': if (lightFlag == 1) { lightFlag = 0; } else { lightFlag = 1; } break; //Disable Directional light
		case 'c': if (lightFlag2 == 1) { lightFlag2 = 0; } else { lightFlag2 = 1; } break; //Disable Candle lights
		case 'h': if (lightFlag3 == 1) { lightFlag3 = 0; } else { lightFlag3 = 1; } break; //Disable Spotlight lights

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

	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");
	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0,"colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");
	glBindAttribLocation(shader.getProgramIndex(), TANGENT_ATTRIB, "tangent");
	glBindAttribLocation(shader.getProgramIndex(), BITANGENT_ATTRIB, "bitangent");

	glLinkProgram(shader.getProgramIndex());

	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	fog_uniformId = glGetUniformLocation(shader.getProgramIndex(), "fog");
	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos[0]");
	lPos_uniformId2 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[1]");
	lPos_uniformId3 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[2]");
	lPos_uniformId4 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[3]");
	lPos_uniformId5 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[4]");
	lPos_uniformId6 = glGetUniformLocation(shader.getProgramIndex(), "l_pos[5]");
	lStr_uniformId = glGetUniformLocation(shader.getProgramIndex(), "luzDirectional");
	lStr_uniformId2 = glGetUniformLocation(shader.getProgramIndex(), "luzDifusa");
	lStr_uniformId3 = glGetUniformLocation(shader.getProgramIndex(), "luzHolofote");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
	specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
	diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");


	
	printf("InfoLog for MicroMachines Project\n%s\n\n", shader.getAllInfoLogs().c_str());
	
	return(shader.isProgramLinked() && shaderText.isProgramLinked());
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

	freeType_init(font_name);

	glGenTextures(3, TextureArray);
	Texture2D_Loader(TextureArray, "particle.tga", 0);
	Texture2D_Loader(TextureArray, "table-cloth3.jpg", 1);
	Texture2D_Loader(TextureArray, "cloth2.jpg", 2);
	Import3DFromFile("spider/spider.obj");

	
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

	//Green
	float amb5[] = { 0.0f, 0.3f, 0.0f, 1.0f };
	float diff5[] = { 0.0f, 0.3f, 0.0f, 1.0f };
	float spec5[] = { 0.9f, 0.9f, 0.9f, 1.0f };


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
	//creation of Mymesh array with VAO Geometry and Material
	//myMeshes = createMeshFromAssimp(scene);
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCylinder(1.0f, 0.3f, 64);

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
	for (int i = 0; i < 4; i++) {
		memcpy(mesh[objId].mat.ambient, amb2, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff2, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec2, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createSphere(1.0f, 20);
		objId++;
	}

	//FOLHAS
	objId = 10;
	for (int i = 0; i < 4; i++) {
		memcpy(mesh[objId].mat.ambient, amb5, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff5, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec5, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createCylinder(0.2f, 0.08f, 16);
		objId++;
	}

	//MANTEIGAS
	objId = 14;
	for (int i = 0; i < 4; i++) {
		memcpy(mesh[objId].mat.ambient, amb3, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createCube();
		objId++;
	}
	

	//Cheerios
	objId = 18;
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

	objId = 134;
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

	objId = 250;
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

	objId = 366;
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

	objId = 482;
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

	objId = 599;
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

	objId = 643;
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

	objId = 684;
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

	//VIDRO DO CARRO
	objId = 782;
	memcpy(mesh[objId].mat.ambient, amb4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//VIDRO NA PISTA
	diff1[3] = 0.5f;
	objId = 783;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();
	diff1[3] = 1.0f;

	//PARTICULAS
	objId = 784;
	mesh[objId].mat.texCount = texcount;
	createQuad(2, 2);


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

