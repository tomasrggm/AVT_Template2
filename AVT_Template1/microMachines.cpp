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
#include "l3DBillboard.h"
#include "microMachines.h"
#include "flare.h"
using namespace std;

#define CAPTION "AVT MicroMachines Project - Delivery 2"

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
int useTeacherKeys = 0; //By default, use keys as defined by the teacher


//isto fui eu que pus
int lightFlag = 1; //directional
int lightFlag2 = 1; //tudo o resto
int lightFlag3 = 1; //holofotes
int fogFlag = 0; //fog
int cameraFlag = 1;
int drawRaceTrackShadowsReflection = 0;
float carX = 5.0f;
float carY = 0.55f;
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
int billboardType = 1; //By default, it starts on cheated cylindrical

float torusX[633];
float torusZ[633];
float torusXBackup[633];
float torusZBackup[633];
int renderedFlag = 0;
int pauseFlag = 0;
int vidas = 5;
int pontos = 0;

unsigned int FrameCount = 0;
float oldTimeSinceStart = 0;
float timeSinceStart = 0;
float deltaTime = 0;

VSShaderLib shader, shaderText;

bool normalMapKey = TRUE; // by default if there is a normal map then bump effect is implemented. press key "b" to enable/disable normal mapping 

struct MyMesh mesh[797];
vector<struct MyMesh> myMeshes;
int objId = 0; //id of the object mesh - to be used as index of mesh: mesh[objID] means the current mesh


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
GLint model_uniformId;
GLint view_uniformId;

GLint tex_loc, tex_loc1, tex_loc2, tex_loc3, tex_loc4, tex_loc5, tex_cube_loc, tex_normalMap_loc;
GLint texMode_uniformId, shadowMode_uniformId;
GLint fog_uniformId;

GLint normalMap_loc;
GLint specularMap_loc;
GLint diffMapCount_loc;

GLuint TextureArray[13];
GLuint FlareTextureArray[5];

//Flare effect
FLARE_DEF AVTflare;
float lightScreenPos[3];  //Position of the light in Window Coordinates

// Camera Position
float camX, camY, camZ;
float camXGlobal, camYGlobal, camZGlobal;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 5.0f;

//Particles fireworks
int fireworks = 0;

inline double clamp(const double x, const double min, const double max) {
	return (x < min ? min : (x > max ? max : x));
}

inline int clampi(const int x, const int min, const int max) {
	return (x < min ? min : (x > max ? max : x));
}

unsigned int getTextureId(char* name) {
	int i;

	for (i = 0; i < NTEXTURES; ++i)
	{
		if (strncmp(name, flareTextureNames[i], strlen(name)) == 0)
			return i;
	}
	return -1;
}

void    loadFlareFile(FLARE_DEF* flare, char* filename)
{
	int     n = 0;
	FILE* f;
	char    buf[256];
	int fields;

	memset(flare, 0, sizeof(FLARE_DEF));

	f = fopen(filename, "r");
	if (f)
	{
		fgets(buf, sizeof(buf), f);
		sscanf(buf, "%f %f", &flare->fScale, &flare->fMaxSize);

		while (!feof(f))
		{
			char            name[8] = { '\0', };
			double          dDist = 0.0, dSize = 0.0;
			float			color[4];
			int				id;

			fgets(buf, sizeof(buf), f);
			fields = sscanf(buf, "%4s %lf %lf ( %f %f %f %f )", name, &dDist, &dSize, &color[3], &color[0], &color[1], &color[2]);
			if (fields == 7)
			{
				for (int i = 0; i < 4; ++i) color[i] = clamp(color[i] / 255.0f, 0.0f, 1.0f);
				id = getTextureId(name);
				if (id < 0) printf("Texture name not recognized\n");
				else
					flare->element[n].textureId = id;
				flare->element[n].fDistance = (float)dDist;
				flare->element[n].fSize = (float)dSize;
				memcpy(flare->element[n].matDiffuse, color, 4 * sizeof(float));
				++n;
			}
		}

		flare->nPieces = n;
		fclose(f);
	}
	else printf("Flare file opening error\n");
}

typedef struct {
	float	life;		// lifespan
	float	fade;		// fade
	float	r, g, b;    // color
	GLfloat x, y, z;    // position
	GLfloat vx, vy, vz; // velocity 
	GLfloat ax, ay, az; // aceleration
} Particle;

Particle particula[MAX_PARTICULAS];
int dead_num_particles = 0;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];
float lightPos[4] = { -1.0f, 2.5f, -48.0f, 1.0f };
float lightPos2[4] = { 11.0f, 2.5f, -48.0f, 1.0f };
float lightPos3[4] = { -0.5f, 2.5f, 48.0f, 1.0f };
float lightPos4[4] = { -38.0f, 2.5f, 48.0f, 1.0f };
float lightPos5[4] = { -38.0f, 2.5f, -48.0f, 1.0f };
float lightPos6[4] = { -19.0f, 2.5f, 0.0f, 1.0f };
float spotAngle = 10.0f;
float lightPos7[4] = { carX,carY,carZ,1.0f };
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
	if (pauseFlag == 0) {
		pontos++;
	}
	glutTimerFunc(1000, timer, 0);

}

void updateParticles()
{
	int i;
	float h;

	/* Método de Euler de integração de eq. diferenciais ordinárias
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

	if (pauseFlag == 0) {
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
			particula[i].ay = -0.15f; /* simular a aceleração da gravidade */
			particula[i].az = 0.0f;

			/* tom amarelado que vai ser multiplicado pela textura que varia entre branco e preto */
			particula[i].r = 0.882f;
			particula[i].g = 0.552f;
			particula[i].b = 0.211f;

			particula[i].life = 1.0f;		/* vida inicial */
			particula[i].fade = 0.0025f;	    /* step de decréscimo da vida para cada iteração */
		}
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
				if (vidas == 0) {
					pauseFlag = 2;
				}
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

void move(int value) {
	if (pauseFlag == 0) {
		if (angulo > 360 || angulo < -360) {
			angulo = 0;
		}
		float converter = angulo * (3.14 / 180);
		carZ += cos(converter) * accelerationIncrement * deltaTime;
		carX += sin(converter) * accelerationIncrement * deltaTime;
		if (accelerationIncrement < 0) {
			accelerationIncrement += 1.0f * deltaTime;
		}
		if (accelerationIncrement > 0) {
			accelerationIncrement -= 3.0f * deltaTime;
		}
		if ((accelerationIncrement > -1.0f * deltaTime && accelerationIncrement < 0) || (accelerationIncrement < 1.0f * deltaTime && accelerationIncrement > 0)) { //correcao do bug do carro nunca parar por um erro qualquer de computacao
			accelerationIncrement = 0;
		}
	}
	glutTimerFunc(1, move, 0);
	oldTimeSinceStart = timeSinceStart;
}

void movementOrange(int value) {
	if (pauseFlag == 0) {
		orangeRot -= 2;
		if (orangeRot <= -360) {
			orangeRot = 0;
		}

		for (int i = 0; i < 4; i++) {
			orangeSpeed[i] += 0.005f * (i + 1) * deltaTime;

			orangeX[i] += orangeSpeed[i];
			if (orangeX[i] > 20) {
				orangeX[i] = -60;
				orangeZ[i] = (rand() % 100) - 48;
			}
		}
	}

	glutTimerFunc(1, movementOrange, 0);
}

void restart() {
	for (int i = 0; i < 4; i++) {
		orangeX[i] = 0;
		orangeZ[i] = (rand() % 100) - 45;
		orangeSpeed[i] = 0;

	}
	carX = 5;
	carZ = -48;
	accelerationIncrement = 0;
	vidas = 5;
	pontos = 0;
	angulo = 0;
	pauseFlag = 0;
	for (int i = 0; i < 633; i++) {
		torusX[i] = torusXBackup[i];
		torusZ[i] = torusZBackup[i];
	}
}

// ------------------------------------------------------------
// Reshape Callback Function

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
			ortho(-32.0, 32.0,
				-64.0 * (GLfloat)h / (GLfloat)w, 64.0 * (GLfloat)h / (GLfloat)w,
				-10, 10);
		else
			ortho(-32.0 * (GLfloat)w / (GLfloat)h, 32.0 * (GLfloat)w / (GLfloat)h,
				-64.0, 64.0,
				-10, 10);
	}

	else if (cameraFlag == 2) {
		perspective(100.0f, ratio, 0.1f, 1000.0f);
	}

	else if (cameraFlag == 3) {
		perspective(70.13f, ratio, 0.1f, 1000.0f);
	}

	else if (cameraFlag == 4) {
		perspective(60.0f, ratio, 0.1f, 1000.0f);
	}

	else if (cameraFlag == 5) {
		perspective(60.0f, ratio, 0.1f, 1000.0f);
	}
}


// ------------------------------------------------------------
// Render stufff

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
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.emissive");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.emissive);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[nd->mMeshes[n]].mat.shininess);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
		glUniform1i(loc, myMeshes[nd->mMeshes[n]].mat.texCount);

		unsigned int  diffMapCount = 0;  //read 2 diffuse textures

		//devido ao fragment shader suporta 2 texturas difusas simultaneas, 1 especular e 1 normal map

		glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
		glUniform1i(specularMap_loc, false);
		glUniform1ui(diffMapCount_loc, 0);

		if (myMeshes[nd->mMeshes[n]].mat.texCount != 0)
			for (unsigned int i = 0; i < myMeshes[nd->mMeshes[n]].mat.texCount; ++i) {
				if (myMeshes[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
					if (diffMapCount == 0) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff");
						glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i] + 5);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else if (diffMapCount == 1) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff1");
						glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i] + 5);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
				}
				else if (myMeshes[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitSpec");
					glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i] + 5);
					glUniform1i(specularMap_loc, true);
				}
				else if (myMeshes[nd->mMeshes[n]].texTypes[i] == NORMALS) { //Normal map
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitNormalMap");
					if (normalMapKey)
						glUniform1i(normalMap_loc, normalMapKey);
					glUniform1i(loc, myMeshes[nd->mMeshes[n]].texUnits[i] + 5);

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
		glBindVertexArray(myMeshes[nd->mMeshes[n]].vao);

		if (!shader.isProgramValid()) {
			printf("Program Not Valid!\n");
			exit(1);
		}
		// draw
		glDrawElements(myMeshes[nd->mMeshes[n]].type, myMeshes[nd->mMeshes[n]].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		aiRecursive_render(sc, nd->mChildren[n]);
	}
	popMatrix(MODEL);
}

void render_flare(FLARE_DEF* flare, int lx, int ly, int* m_viewport) {  //lx, ly represent the projected position of light on viewport

	int     dx, dy;          // Screen coordinates of "destination"
	int     px, py;          // Screen coordinates of flare element
	int		cx, cy;
	float    maxflaredist, flaredist, flaremaxsize, flarescale, scaleDistance;
	int     width, height, alpha;    // Piece parameters;
	int     i;
	float	diffuse[4];

	GLint loc;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int screenMaxCoordX = m_viewport[0] + m_viewport[2] - 1;
	int screenMaxCoordY = m_viewport[1] + m_viewport[3] - 1;

	//viewport center
	cx = m_viewport[0] + (int)(0.5f * (float)m_viewport[2]) - 1;
	cy = m_viewport[1] + (int)(0.5f * (float)m_viewport[3]) - 1;

	// Compute how far off-center the flare source is.
	maxflaredist = sqrt(cx * cx + cy * cy);
	flaredist = sqrt((lx - cx) * (lx - cx) + (ly - cy) * (ly - cy));
	scaleDistance = (maxflaredist - flaredist) / maxflaredist;
	flaremaxsize = (int)(m_viewport[2] * flare->fMaxSize);
	flarescale = (int)(m_viewport[2] * flare->fScale);

	// Destination is opposite side of centre from source
	dx = clampi(cx + (cx - lx), m_viewport[0], screenMaxCoordX);
	dy = clampi(cy + (cy - ly), m_viewport[1], screenMaxCoordY);

	// Render each element. To be used Texture Unit 0

	glUniform1i(texMode_uniformId, 5); // draw modulated textured particles 
	glUniform1i(tex_loc4, 0);  //use TU 0

	for (i = 0; i < flare->nPieces; ++i)
	{
		// Position is interpolated along line between start and destination.
		px = (int)((1.0f - flare->element[i].fDistance) * lx + flare->element[i].fDistance * dx);
		py = (int)((1.0f - flare->element[i].fDistance) * ly + flare->element[i].fDistance * dy);
		px = clampi(px, m_viewport[0], screenMaxCoordX);
		py = clampi(py, m_viewport[1], screenMaxCoordY);

		// Piece size are 0 to 1; flare size is proportion of screen width; scale by flaredist/maxflaredist.
		width = (int)(scaleDistance * flarescale * flare->element[i].fSize);

		// Width gets clamped, to allows the off-axis flaresto keep a good size without letting the elements get big when centered.
		if (width > flaremaxsize)  width = flaremaxsize;

		height = (int)((float)m_viewport[3] / (float)m_viewport[2] * (float)width);
		memcpy(diffuse, flare->element[i].matDiffuse, 4 * sizeof(float));
		diffuse[3] *= scaleDistance;   //scale the alpha channel

		if (width > 1)
		{
			// send the material - diffuse color modulated with texture
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, diffuse);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, FlareTextureArray[flare->element[i].textureId]);
			pushMatrix(MODEL);
			translate(MODEL, (float)(px - width * 0.0f), (float)(py - height * 0.0f), 0.0f);
			scale(MODEL, (float)width, (float)height, 1);
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			glBindVertexArray(mesh[objId].vao);
			glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			popMatrix(MODEL);
		}
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void drawTable()
{
	GLint loc;

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

	translate(MODEL, -60.0f, 0.0, -70.0f);
	scale(MODEL, 80.0f, 0.0005f, 140.0f);


	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glUniform1i(texMode_uniformId, 1); // linha da textura
	glBindVertexArray(mesh[objId].vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);
	glUniform1i(texMode_uniformId, 0);
}

void drawAssimpModel()
{
	//SPIDER
	pushMatrix(MODEL);

	translate(MODEL, -2.0f, 0.80f, -45.0f);
	scale(MODEL, 0.02f, 0.02f, 0.02f);
	rotate(MODEL, 180, 0, 1.0f, 0);

	aiRecursive_render(scene, scene->mRootNode);

	popMatrix(MODEL);
}

void drawCar()
{
	GLint loc;

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
	drawCarWheels();
}

void drawCarWheels()
{
	GLint loc;

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
	translate(MODEL, 0.4f, -0.25f, -0.25f);
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
}

void drawOranges()
{
	GLint loc;

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

		translate(MODEL, orangeX[i], 1.0f, orangeZ[i]);
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

		translate(MODEL, orangeX[i], 1.0f, orangeZ[i]);
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
}

void drawButters()
{
	GLint loc;

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
		torusXBackup[629] = 0.5f;
		torusZBackup[629] = 0;
	}
	translate(MODEL, torusX[629], 0.1f, torusZ[629]);
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
		torusXBackup[630] = 4.5f;
		torusZBackup[630] = 20.0f;
	}
	translate(MODEL, torusX[630], 0.1f, torusZ[630]);
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
		torusXBackup[631] = 4.5f;
		torusZBackup[631] = -25.0f;
	}
	translate(MODEL, torusX[631], 0.1f, torusZ[631]);
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
		torusXBackup[632] = 0.5f;
		torusZBackup[632] = -40.0f;
	}
	translate(MODEL, torusX[632], 0.1f, torusZ[632]);
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
}

void drawRaceTrackCheerios()
{
	GLint loc;

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
			torusXBackup[i + 50] = 1.0f;
			torusZBackup[i + 50] = 1.5f + i;
		}
		translate(MODEL, torusX[i + 50], 0.15f, torusZ[i + 50]);
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
			torusXBackup[i + 58 + 98] = 9.0f;
			torusZBackup[i + 58 + 98] = 1.5f + i;
		}

		translate(MODEL, torusX[i + 58 + 98], 0.15f, torusZ[i + 58 + 98]);
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
			torusXBackup[i + 50 + 213] = 0.0f + i;
			torusZBackup[i + 50 + 213] = 57.5f;
		}
		translate(MODEL, torusX[i + 50 + 213], 0.15f, torusZ[i + 50 + 213]);
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
			torusXBackup[i + 50 + 273] = 0.0f + i;
			torusZBackup[i + 50 + 273] = -57.5f;
		}
		translate(MODEL, torusX[i + 50 + 273], 0.15f, torusZ[i + 50 + 273]);
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
			torusXBackup[i + 58 + 333] = -50.0f;
			torusZBackup[i + 58 + 333] = 1.5f + i;
		}
		translate(MODEL, torusX[i + 58 + 333], 0.15f, torusZ[i + 58 + 333]);
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
			torusXBackup[i + 40 + 448] = 0 + i;
			torusZBackup[i + 40 + 448] = -49.5f;
		}
		translate(MODEL, torusX[i + 40 + 448], 0.15f, torusZ[i + 40 + 448]);
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
			torusXBackup[i + 40 + 489] = 0 + i;
			torusZBackup[i + 40 + 489] = 49.5f;
		}
		translate(MODEL, torusX[i + 40 + 489], 0.15f, torusZ[i + 40 + 489]);
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
			torusXBackup[i + 50 + 530] = -40;
			torusZBackup[i + 50 + 530] = 1.5f + i;
		}
		translate(MODEL, torusX[i + 50 + 530], 0.15f, torusZ[i + 50 + 530]);
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
}

void drawCandles()
{
	GLint loc;

	//Velas
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

	translate(MODEL, -0.5f, 0.65f, -48.0f);

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


	//CANDLES
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

	translate(MODEL, 10.5f, 0.65f, -48.0f);

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


	//CANDLES
	objId = 784;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, -0.5f, 0.65f, 48.0f);

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


	//CANDLES
	objId = 785;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, -38.0f, 0.65f, 48.0f);

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


	//CANDLES
	objId = 786;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, -38.0f, 0.65f, -48.0f);

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


	//CANDLES
	objId = 787;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, -19.0f, 0.65f, 0.0f);
	scale(MODEL, 2.0f, 1.0f, 2.0f);

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
}

void drawSpectatorBillboards()
{
	GLint loc;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Despite of blending, they are considered opaque so z-buffer works normally

	float pos[3];
	float cam[3] = { camXGlobal, camYGlobal, camZGlobal };
	objId = 788;
	for (int i = -6; i < 2; i++) {
		for (int j = -7; j < 7; j++) {
			if (j > -7 && j < 6 && i > -6 && i < 1) { //only render billboards outside of race track
				continue;
			}
			pushMatrix(MODEL);
			translate(MODEL, 5 + i * 10.0, 0.0f, 5 + j * 10.0);
			scale(MODEL, 0.5, 0.5, 1);

			pos[0] = 5 + i * 10.0; pos[1] = 0; pos[2] = 5 + j * 10.0;

			if (billboardType == 2)
				l3dBillboardSphericalBegin(cam, pos);
			else if (billboardType == 3)
				l3dBillboardCylindricalBegin(cam, pos);

			//diffuse and ambient color are not used in the tree quads
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, mesh[objId].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, mesh[objId].mat.shininess);

			pushMatrix(MODEL);
			translate(MODEL, 0.0, 3.0, 0.0f);

			// send matrices to OGL
			if (billboardType == 0 || billboardType == 1) {     //Cheating matrix reset billboard techniques
				computeDerivedMatrix(VIEW_MODEL);

				//reset VIEW_MODEL
				if (billboardType == 0) BillboardCheatSphericalBegin();
				else BillboardCheatCylindricalBegin();

				computeDerivedMatrix_PVM(); // calculate PROJ_VIEW_MODEL
			}
			else computeDerivedMatrix(PROJ_VIEW_MODEL);

			// send matrices to OGL
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			// Render mesh
			glUniform1i(texMode_uniformId, 2); // linha da textura
			glBindVertexArray(mesh[objId].vao);

			if (!shader.isProgramValid()) {
				printf("Program Not Valid!\n");
				exit(1);
			}
			glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			popMatrix(MODEL);
			popMatrix(MODEL);
			glUniform1i(texMode_uniformId, 0);
		}
	}
	glDisable(GL_BLEND);
}

void drawEnvironmentalReflectionCube()
{
	//Environmental reflection cube

	GLint loc;
	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform1i(texMode_uniformId, 6);

	objId = 794;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, -4.0f, 2.0f, -48.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(view_uniformId, 1, GL_FALSE, mMatrix[VIEW]);
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
	glUniform1i(texMode_uniformId, 0);
}

void drawBumpMapCube()
{
	//BUMP MAP CUBE
	GLint loc;

	objId = 795;
	glUniform1i(texMode_uniformId, 7);

	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	translate(MODEL, -6.0f, 2.0f, -48.0f);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(view_uniformId, 1, GL_FALSE, mMatrix[VIEW]);
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

	glUniform1i(texMode_uniformId, 0);
}

void drawSkyBox()
{
	//SKY BOX
	GLint loc;

	objId = 792;
	glUniform1i(texMode_uniformId, 4);

	//it won't write anything to the zbuffer; all subsequently drawn scenery to be in front of the sky box.
	glDepthMask(GL_FALSE);
	glFrontFace(GL_CW); // set clockwise vertex order to mean the front

	pushMatrix(MODEL);
	pushMatrix(VIEW);  //se quiser anular a translação

					   //  Fica mais realista se não anular a translação da câmara
					   // Cancel the translation movement of the camera - de acordo com o tutorial do Antons
	mMatrix[VIEW][12] = 0.0f;
	mMatrix[VIEW][13] = 0.0f;
	mMatrix[VIEW][14] = 0.0f;

	scale(MODEL, 100.0f, 100.0f, 100.0f);
	translate(MODEL, -0.5f, -0.5f, -0.5f);

	// send matrices to OGL
	glUniformMatrix4fv(model_uniformId, 1, GL_FALSE, mMatrix[MODEL]); //Transformação de modelação do cubo unitário para o "Big Cube"
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);

	glBindVertexArray(mesh[objId].vao);
	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popMatrix(MODEL);
	popMatrix(VIEW);

	glFrontFace(GL_CCW); // restore counter clockwise vertex order to mean the front
	glDepthMask(GL_TRUE);
	glUniform1i(texMode_uniformId, 0);
}

void drawCarGlass()
{
	GLint loc;
	//Enable blending and make Z-buffer read-only
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	objId = 789;
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

	//Re-enable depth-test writing and disable blending
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void drawCarRearView()
{
	GLint loc;

	objId = 790;
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
	translate(MODEL, -0.25f, 1.0f, 0.4f);
	scale(MODEL, 0.50f, 0.15f, 0.1f);

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
}

void drawCarRearViewMirror()
{
	GLint loc;

	objId = 796;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess);
	pushMatrix(MODEL);

	/*
	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, -0.25f, 1.0f, 0.4f);
	scale(MODEL, 0.50f, 0.15f, 0.1f);
	*/

	translate(MODEL, carX, carY, carZ);
	rotate(MODEL, angulo, 0, 1.0f, 0);
	translate(MODEL, -0.235f, 1.015f, 0.39f);
	scale(MODEL, 0.47f, 0.12f, 0.1f);
	//scale(MODEL, 0.80f, 0.12f, 0.1f);


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
}

void drawFireworks()
{
	GLint loc;
	float particle_color[4];
	float pos[3];
	float cam[3] = { camXGlobal, camYGlobal, camZGlobal };

	//Enable blending and make Z-buffer read-only
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	objId = 791;  //quad for particle
	if (fireworks && pauseFlag == 0) {

		updateParticles();

		glUniform1i(texMode_uniformId, 3); // draw modulated textured particles 

		for (int i = 0; i < MAX_PARTICULAS; i++)
		{
			if (particula[i].life > 0.0f) /* só desenha as que ainda estão vivas */
			{

				/* A vida da partícula representa o canal alpha da cor. Como o blend está activo a cor final é a soma da cor rgb do fragmento multiplicada pelo
				alpha com a cor do pixel destino */

				particle_color[0] = particula[i].r;
				particle_color[1] = particula[i].g;
				particle_color[2] = particula[i].b;
				particle_color[3] = particula[i].life;

				pos[0] = particula[i].x; pos[1] = particula[i].y; pos[2] = particula[i].z;

				if (billboardType == 2)
					l3dBillboardSphericalBegin(cam, pos);
				else if (billboardType == 3)
					l3dBillboardCylindricalBegin(cam, pos);

				// send the material - diffuse color modulated with texture
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, particle_color);

				pushMatrix(MODEL);
				translate(MODEL, particula[i].x, particula[i].y, particula[i].z);

				// send matrices to OGL
				if (billboardType == 0 || billboardType == 1) {     //Cheating matrix reset billboard techniques
					computeDerivedMatrix(VIEW_MODEL);

					//reset VIEW_MODEL
					if (billboardType == 0) BillboardCheatSphericalBegin();
					else BillboardCheatCylindricalBegin();

					computeDerivedMatrix_PVM(); // calculate PROJ_VIEW_MODEL
				}
				else computeDerivedMatrix(PROJ_VIEW_MODEL);

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

	//Re-enable depth-test writing and disable blending
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glUniform1i(texMode_uniformId, 0);
}

void drawObjects()
{
	//SPIDER
	drawAssimpModel();

	//CAR
	drawCar();

	//ORANGES
	drawOranges();

	//BUTTERS
	drawButters();

	//RACE-TRACK-CHEERIOS
	drawRaceTrackCheerios();

	//CANDLES
	drawCandles();

	//Blended objects
	//BILLBOARDS
	drawSpectatorBillboards();

	//ENVIRONMENTAL REFLECTION CUBE
	drawEnvironmentalReflectionCube();

	//BUMP MAP CUBE
	drawBumpMapCube();

	//CAR-GLASS
	drawCarGlass();
}

void drawObjectsButRaceTrack()
{
	//SPIDER
	drawAssimpModel();

	//CAR
	drawCar();

	//ORANGES
	drawOranges();

	//BUTTERS
	drawButters();

	//CANDLES
	drawCandles();

	//Blended objects
	//BILLBOARDS
	drawSpectatorBillboards();

	//ENVIRONMENTAL REFLECTION CUBE
	drawEnvironmentalReflectionCube();

	//BUMP MAP CUBE
	drawBumpMapCube();

	//CAR-GLASS
	drawCarGlass();
}

void drawLensFlare()
{
	//LENS FLARE
	objId = 793;
	int flarePos[2];
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	pushMatrix(MODEL);
	loadIdentity(MODEL);
	computeDerivedMatrix(PROJ_VIEW_MODEL);  //pvm to be applied to lightPost. pvm is used in project function

	if (!project(lightPos6, lightScreenPos, m_viewport))
		printf("Error in getting projected light in screen\n");  //Calculate the window Coordinates of the light position: the projected position of light on viewport
	flarePos[0] = clampi((int)lightScreenPos[0], m_viewport[0], m_viewport[0] + m_viewport[2] - 1);
	flarePos[1] = clampi((int)lightScreenPos[1], m_viewport[1], m_viewport[1] + m_viewport[3] - 1);
	popMatrix(MODEL);

	//viewer looking down at  negative z direction
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	render_flare(&AVTflare, flarePos[0], flarePos[1], m_viewport);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
}

void drawHUDText()
{
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

	float h = glutGet(GLUT_WINDOW_HEIGHT);
	float w = glutGet(GLUT_WINDOW_WIDTH);
	if (pauseFlag == 1) {
		RenderText(shaderText, "Pause", w / 4.0f, h / 2.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	}
	if (pauseFlag == 2) {
		RenderText(shaderText, "Game Over", w / 4.0f, 2.0f * h / 3.0f, 1.0f, 0.5f, 0.8f, 0.2f);
		RenderText(shaderText, "Press R to restart", w / 4.0f, h / 2.0f, 0.6f, 0.5f, 0.8f, 0.2f);
	}
	std::string v = std::to_string(vidas);
	std::string p = std::to_string(pontos);
	RenderText(shaderText, "Lifes: " + v + " Points: " + p, 25.0f, 25.0f, 0.4f, 0.5f, 0.8f, 0.2f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void setupCameraLookAts()
{
	if (cameraFlag == 1) { //Fixed ortho camera
		changeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

		camXGlobal = -19.5f;
		camYGlobal = 10.0f;
		camZGlobal = 0.0f;
		lookAt(camXGlobal, camYGlobal, camZGlobal, -19.5f, 0, 0 + 0.00005f, 0, 1, 0);
	}
	else if (cameraFlag == 2) { //Fixed perspective camera
		changeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

		camXGlobal = 20.0f;
		camYGlobal = 40.0f;
		camZGlobal = 0.0f;
		lookAt(camXGlobal, camYGlobal, camZGlobal, -19.5f, 0, 0 + 0.01, 0, 1, 0);
	}
	else if (cameraFlag == 3) { //Movement camera
		changeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

		camXGlobal = camX + carX;
		camYGlobal = camY + carY;
		camZGlobal = camZ + carZ;
		lookAt(camXGlobal, camYGlobal, camZGlobal, carX, carY, carZ, 0, 1, 0);

		//Translate camera to car's coordinates, rotate, and return back to original position
		translate(VIEW, carX, carY, carZ);
		rotate(VIEW, -angulo, 0.0, 1.0, 0.0);
		translate(VIEW, -carX, -carY, -carZ);
	}

	else if (cameraFlag == 4) { //Driver's camera
		changeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

		camXGlobal = carX;
		camYGlobal = carY + 1.0f;
		camZGlobal = carZ - 1.0f;
		lookAt(camXGlobal, camYGlobal, camZGlobal, carX, carY + 1.0, carZ, 0, 1, 0);

		//Translate camera to car's coordinates, rotate, and return back to original position
		translate(VIEW, carX, carY, carZ);
		rotate(VIEW, -angulo, 0.0, 1.0, 0.0);
		translate(VIEW, -carX, -carY, -carZ);
	}

	else if (cameraFlag == 5) { //Rear-view Camera
		changeSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

		camXGlobal = carX;
		camYGlobal = carY + 1.0f;
		camZGlobal = carZ + 0.5f;
		lookAt(camXGlobal, camYGlobal, camZGlobal, carX, carY + 1.0, carZ, 0, 1, 0);

		//Translate camera to car's coordinates, rotate, and return back to original position
		translate(VIEW, carX, carY, carZ);
		rotate(VIEW, -angulo, 0.0, 1.0, 0.0);
		translate(VIEW, -carX, -carY, -carZ);
	}
}

void renderScene(void) {

	timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = timeSinceStart - oldTimeSinceStart;
	deltaTime = deltaTime / 1000;
	oldTimeSinceStart = timeSinceStart;

	FrameCount++;

	float mat[16];
	GLfloat plano_chao[4] = { 0,1,0,0 };

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);	// cull face
	glCullFace(GL_BACK);		 // cull back face
	glFrontFace(GL_CCW); // set counter-clockwise vertex order to mean the front
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	setupCameraLookAts();

	// use our shader
	glUseProgram(shader.getProgramIndex());

	// set textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, TextureArray[5]);

	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureArray[11]);

	glUniform1i(tex_loc, 0);
	glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);
	glUniform1i(tex_loc3, 3);
	glUniform1i(tex_normalMap_loc, 4);
	glUniform1i(tex_loc5, 5);
	glUniform1i(tex_cube_loc, 11);
	glUniform1i(tex_loc4, 0);

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
	multMatrixPoint(VIEW, lightPos, res);   //lightPos definido em World Coord so is converted to eye space
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

	float res8[4] = { carX + 0.5f,carY,carZ + 0.5f,1.0f };
	float res11[4];
	multMatrixPoint(VIEW, res8, res11);
	glUniform4fv(lPos_uniformId7, 1, res11);

	float res9[4] = { carX - 0.5f,carY,carZ + 0.5f,1.0f };
	float res10[4];
	multMatrixPoint(VIEW, res9, res10);
	glUniform4fv(lPos_uniformId8, 1, res10);


	//Draw sky box before any objects
	drawSkyBox();

	//Drawing objects
	if (camYGlobal >= 0.0f) { //Make sure reflections and shadows are not on the bottom side of the floor
		glEnable(GL_STENCIL_TEST);        // Escrever 1 no stencil buffer onde se for desenhar a reflexão e a sombra
		glStencilFunc(GL_NEVER, 0x1, 0x1); //We define how we will set the stencil function
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); //We define how we will modify the stencil

		// Fill stencil buffer with the table; never rendered into color buffer
		drawTable();

		glUniform1i(shadowMode_uniformId, 0);  //iluminação phong

		// Desenhar apenas onde o stencil buffer esta a 1
		glStencilFunc(GL_EQUAL, 0x1, 0x1); //Where stencil is 1
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// Render the reflected geometry
		lightPos6[1] *= (-1.0f);  //mirror the position of light
		multMatrixPoint(VIEW, lightPos6, res);

		glUniform4fv(lPos_uniformId6, 1, res);
		pushMatrix(MODEL);
		scale(MODEL, 1.0f, -1.0f, 1.0f);
		glCullFace(GL_FRONT);
		//Draw all objects but the table
		//Optimization: choose not to reflect nor draw the shadows of the race track cheerios
		if (drawRaceTrackShadowsReflection == 1) {
			drawObjects();
			drawCarRearView();
		}
		else {
			drawObjectsButRaceTrack();
			drawCarRearView();
		}
		//Draw fireworks since they should reflect
		drawFireworks();
		glCullFace(GL_BACK);
		popMatrix(MODEL);

		lightPos6[1] *= (-1.0f);  //reset the light position
		multMatrixPoint(VIEW, lightPos6, res);
		glUniform4fv(lPos_uniformId6, 1, res);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Blend specular Ground with reflected geometry
		drawTable();

		// Render the Shadows
		glUniform1i(shadowMode_uniformId, 1);  //Render with constant color
		computeShadowMatrix(mat, plano_chao, lightPos6);

		glDisable(GL_DEPTH_TEST); //To force the shadow geometry to be rendered even if behind the floor

		//Dark the color stored in color buffer
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);

		pushMatrix(MODEL);
		multMatrix(MODEL, mat);
		//Draw all objects' shadows but table, fireworks and transparent objects
		if (drawRaceTrackShadowsReflection == 1) {
			drawCar();
			drawOranges();
			drawButters();
			drawCandles();
			drawRaceTrackCheerios();
			drawSpectatorBillboards();
			drawEnvironmentalReflectionCube();
			drawBumpMapCube();
			drawCarRearView();
		}
		else {
			drawCar();
			drawOranges();
			drawButters();
			drawCandles();
			drawSpectatorBillboards();
			drawEnvironmentalReflectionCube();
			drawBumpMapCube();
			drawCarRearView();
		}
		popMatrix(MODEL);

		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glUniform1i(shadowMode_uniformId, 0);

		//Rear-View
		if (cameraFlag == 4) { //When we are in the driver's camera
			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);

			//Limit stencil to rear-view mirror
			glEnable(GL_STENCIL_TEST);
			//We fill the stencil with 1 where we will draw the objects we see in rear-view
			//We make it so we always fail the tests
			//We make it so, whenever we fail, we replace what was previously in the stencil
			glStencilFunc(GL_NEVER, 0x1, 0x1);
			glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

			//We limit the stencil to the rear-view mirror
			drawCarRearViewMirror();

			//Change to rear-view camera
			loadIdentity(VIEW);
			loadIdentity(MODEL);
			cameraFlag = 5;
			setupCameraLookAts();

			//Draw objects behind car in stencil
			//Where there is 1, we want to keep what we will draw next
			glStencilFunc(GL_EQUAL, 0x1, 0x1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			drawObjectsButRaceTrack();
			//drawCarGlass(); //Same idea
			drawFireworks();

			//Reset camera back to original position
			loadIdentity(VIEW);
			loadIdentity(MODEL);
			cameraFlag = 4;
			setupCameraLookAts();
			
			glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
			//Draw the objects above the table
			drawObjects();
			drawCarRearView();
			drawFireworks();
			glDisable(GL_STENCIL_TEST);
		}
		else {
			//Draw the objects above the table
			drawObjects();
			drawCarRearView();
			drawFireworks();
		}
	}

	else {
		drawTable();
	}

	renderedFlag = 1;
	glBindTexture(GL_TEXTURE_2D, 0);

	if (cameraFlag == 3 && camYGlobal >= 0.0f && lightFlag2 == 1 && fogFlag == 0) {
		//LENS FLARE
		drawLensFlare();
	}

	//HUD
	drawHUDText();

	glutSwapBuffers();
}


// ------------------------------------------------------------
// Events from the Keyboard

void processKeys(unsigned char key, int xx, int yy) {

	if (useTeacherKeys == 0) {
		switch (key) {

		case 27:
			glutLeaveMainLoop();
			break;

		case '-': useTeacherKeys = 1;
			//case 'l': printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r); break;
			//case 'm': glEnable(GL_MULTISAMPLE); break;
			//case 'n': glDisable(GL_MULTISAMPLE); break;

			//Camera keys
		case '1': cameraFlag = 1; break;
		case '2': cameraFlag = 2; break;
		case '3': cameraFlag = 3; break;
		case '4': cameraFlag = 4; break;
		case '5': cameraFlag = 5; break;

			//Car Movement keys
		case 'q': if (accelerationIncrement <= 10.0f && pauseFlag == 0) { accelerationIncrement += 1.0f; } break; //Forwards
		case 'a': if (accelerationIncrement >= -10.0f && pauseFlag == 0) { accelerationIncrement -= 1.0f; } break; //Backwards
		case 'o': if (pauseFlag == 0) { angulo += 4; } break; //Left
		case 'p': if (pauseFlag == 0) { angulo -= 4; } break; //Right
		case 's': if (pauseFlag == 1) { pauseFlag = 0; }
				else { pauseFlag = 1; } break;

			//Light keys
		case 'n': if (lightFlag == 1) { lightFlag = 0; }
				else { lightFlag = 1; } break; //Disable Directional light
		case 'c': if (lightFlag2 == 1) { lightFlag2 = 0; }
				else { lightFlag2 = 1; } break; //Disable Candle lights
		case 'h': if (lightFlag3 == 1) { lightFlag3 = 0; }
				else { lightFlag3 = 1; } break; //Disable Spotlight lights

		/*
		case 'b': billboardType++; if (billboardType == 5) billboardType = 0;
			switch (billboardType) {
			case 0: printf("Cheating Spherical (matrix reset)\n"); break;
			case 1: printf("Cheating Cylindrical (matrix reset)\n"); break;
			case 2: printf("True Spherical\n"); break;
			case 3: printf("True Cylindrical\n"); break;
			case 4: printf("No billboarding\n"); break;
			}
		*/

		case 'e': fireworks = 1; iniParticles(); break;

		case 'f': if (fogFlag == 1) { fogFlag = 0; }
				else { fogFlag = 1; } break;

		case 'r': restart(); break;

		case '<': if (drawRaceTrackShadowsReflection == 1) { drawRaceTrackShadowsReflection = 0; }
				else { drawRaceTrackShadowsReflection = 1; } break;
		}
	}
	else {
		switch (key) {

		case 27:
			glutLeaveMainLoop();
			break;

		case '-': useTeacherKeys = 0;
			//case 'l': printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r); break;
			//case 'm': glEnable(GL_MULTISAMPLE); break;
			//case 'n': glDisable(GL_MULTISAMPLE); break;

			//Camera keys
		case '1': cameraFlag = 1; break;
		case '2': cameraFlag = 2; break;
		case '3': cameraFlag = 3; break;
		case '4': cameraFlag = 4; break;
		case '5': cameraFlag = 5; break;

			//Car Movement keys
		case 'w': if (accelerationIncrement <= 10.0f && pauseFlag == 0) { accelerationIncrement += 1.0f; } break; //Forwards
		case 's': if (accelerationIncrement >= -10.0f && pauseFlag == 0) { accelerationIncrement -= 1.0f; } break; //Backwards
		case 'a': if (pauseFlag == 0) { angulo += 4; } break; //Left
		case 'd': if (pauseFlag == 0) { angulo -= 4; } break; //Right
		case 'p': if (pauseFlag == 1) { pauseFlag = 0; }
				else { pauseFlag = 1; } break;

			//Light keys
		case 'n': if (lightFlag == 1) { lightFlag = 0; }
				else { lightFlag = 1; } break; //Disable Directional light
		case 'c': if (lightFlag2 == 1) { lightFlag2 = 0; }
				else { lightFlag2 = 1; } break; //Disable Candle lights
		case 'h': if (lightFlag3 == 1) { lightFlag3 = 0; }
				else { lightFlag3 = 1; } break; //Disable Spotlight lights

		/*
		case 'b': billboardType++; if (billboardType == 5) billboardType = 0;
			switch (billboardType) {
			case 0: printf("Cheating Spherical (matrix reset)\n"); break;
			case 1: printf("Cheating Cylindrical (matrix reset)\n"); break;
			case 2: printf("True Spherical\n"); break;
			case 3: printf("True Cylindrical\n"); break;
			case 4: printf("No billboarding\n"); break;
			}
		*/

		case 'e': fireworks = 1; iniParticles(); break;

		case 'f': if (fogFlag == 1) { fogFlag = 0; }
				else { fogFlag = 1; } break;

		case 'r': restart(); break;

		case '<': if (drawRaceTrackShadowsReflection == 1) { drawRaceTrackShadowsReflection = 0; }
				else { drawRaceTrackShadowsReflection = 1; } break;
		}
	}
}


// ------------------------------------------------------------
// Mouse Events

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
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

	deltaX = -xx + startX;
	deltaY = yy - startY;

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
	camY = rAux * sin(betaAux * 3.14f / 180.0f);

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
// Shader Stuff

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
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");
	glBindAttribLocation(shader.getProgramIndex(), TANGENT_ATTRIB, "tangent");
	glBindAttribLocation(shader.getProgramIndex(), BITANGENT_ATTRIB, "bitangent");

	glLinkProgram(shader.getProgramIndex());

	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	shadowMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "shadowMode");
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
	tex_loc3 = glGetUniformLocation(shader.getProgramIndex(), "texmap3");
	tex_loc4 = glGetUniformLocation(shader.getProgramIndex(), "texmap4");
	tex_loc5 = glGetUniformLocation(shader.getProgramIndex(), "texmap5");
	tex_normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "texNormalMap");
	normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
	specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
	diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");
	tex_cube_loc = glGetUniformLocation(shader.getProgramIndex(), "cubeMap");
	model_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_Model");
	view_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_View");



	printf("InfoLog for MicroMachines Project\n%s\n\n", shader.getAllInfoLogs().c_str());

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
// Model loading and OpenGL setup

void init()
{
	// set the camera position based on its spherical coordinates
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	freeType_init(font_name);

	glGenTextures(5, TextureArray);
	Texture2D_Loader(TextureArray, "happy-human.tga", 0);
	Texture2D_Loader(TextureArray, "marble.jpg", 1);
	Texture2D_Loader(TextureArray, "tiled_floor.jpg", 2);
	Texture2D_Loader(TextureArray, "particle.tga", 3);
	Texture2D_Loader(TextureArray, "normal.tga", 4);
	Texture2D_Loader(TextureArray, "stone.tga", 5);
	const char* filenames[] = { "posx.jpg", "negx.jpg", "posy.jpg", "negy.jpg", "posz.jpg", "negz.jpg" };
	TextureCubeMap_Loader(TextureArray, filenames, 11);
	Import3DFromFile("spider/spider.obj");


	glGenTextures(5, FlareTextureArray);
	Texture2D_Loader(FlareTextureArray, "crcl.tga", 0);
	Texture2D_Loader(FlareTextureArray, "flar.tga", 1);
	Texture2D_Loader(FlareTextureArray, "hxgn.tga", 2);
	Texture2D_Loader(FlareTextureArray, "ring.tga", 3);
	Texture2D_Loader(FlareTextureArray, "sun.tga", 4);


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

	//Cream-Candle (light yellow)
	float amb6[] = { 1.0f, 1.0f, 0.6f, 1.0f };
	float diff6[] = { 1.0f, 1.0f, 0.6f, 1.0f };
	float spec6[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	//Grey
	float amb7[] = { 0.8f, 0.8f, 1.0f, 1.0f };
	float diff7[] = { 0.8f, 0.8f, 1.0f, 1.0f };
	float spec7[] = { 0.5f, 0.5f, 0.5f, 1.0f };

	//Billboard specular color
	float billboard_spec[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float billboard_shininess = 100.0f;


	//TABLE
	objId = 0;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	diff[3] = 0.90f;
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	diff[3] = 1.0f;
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//SPIDER
	//creation of Mymesh array with VAO Geometry and Material
	myMeshes = createMeshFromAssimp(scene);

	//CAR
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

	//WHEELS
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

	//ORANGES
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

	//LEAFS
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

	//BUTTERS
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


	//RACETRACK CHEERIOS
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

	//CANDLES
	objId = 782;
	for (int i = 0; i < 6; i++) {
		memcpy(mesh[objId].mat.ambient, amb6, 4 * sizeof(float));
		memcpy(mesh[objId].mat.diffuse, diff6, 4 * sizeof(float));
		memcpy(mesh[objId].mat.specular, spec6, 4 * sizeof(float));
		memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
		mesh[objId].mat.shininess = shininess;
		mesh[objId].mat.texCount = texcount;
		createCylinder(1.2f, 1.0f, 16);
		objId++;
	}

	//BILLBOARDS
	objId = 788;
	//memcpy(mesh[objId].mat.ambient, amb4, 4 * sizeof(float));
	//memcpy(mesh[objId].mat.diffuse, diff4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, billboard_spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = billboard_shininess;
	mesh[objId].mat.texCount = texcount;
	createQuad(6.0, 6.0);

	//CAR GLASS
	objId = 789;
	memcpy(mesh[objId].mat.ambient, amb4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//CAR REAR-VIEW
	objId = 790;
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//FIREWORK PARTICLES
	objId = 791;
	mesh[objId].mat.texCount = texcount;
	createQuad(2, 2);

	//SKY BOX
	objId = 792;
	memcpy(mesh[objId].mat.ambient, amb7, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff7, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec7, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//LENS FLARES
	objId = 793;
	createQuad(1, 1);
	loadFlareFile(&AVTflare, "flare.txt");

	//ENVIRONMENTAL REFLECTION CUBE
	objId = 794;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//BUMP CUBE
	objId = 795;
	memcpy(mesh[objId].mat.ambient, amb7, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff7, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec7, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	//CAR REAR-VIEW MIRROR
	objId = 796;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

}


// ------------------------------------------------------------
// Main function

int main(int argc, char** argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
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
	glutMouseWheelFunc(mouseWheel);

	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);

}