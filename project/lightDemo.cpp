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
#include <vector>
#include <ctime>

#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// Use Very Simple Libs
#include "AVTmathLib.h"
#include "vsShaderLib.h"
#include "basic_geometry.h"
#include "TGA.h"


// Definition of a Graphical Object
#include "AppObject.h"
#include "Car.h"
#include "Table.h"
#include "Cheerio.h"
#include "Butter.h"
#include "Orange.h"
#include "Light.h"
#include "lightDemo.h"
#include "RearMirror.h"
#include "Flag.h"
#include "LensFlare.h"

// assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/color4.h>

#define CAPTION "AVT Per Fragment Phong Lightning Demo"
#define MAX_PARTICLES 1000
#define frand()			((float)rand()/RAND_MAX)
#define M_PI			3.14159265
int WindowHandle = 0;
int WinX = 640, WinY = 480;

unsigned int FrameCount = 0;



VSShaderLib shader;

#define MAX_MESHES 500000
struct MyMesh mesh[MAX_MESHES];
int objId=0; //id of the object mesh - to be used as index of mesh: mesh[objID] means the current mesh
int maxObjId = 0;
int fireworksId;

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
GLint model_uniformId;
GLint view_uniformId;
GLint tex_loc, tex_loc1;
GLint texMode_uniformId;

GLuint TextureArray[10];
	
// Camera Position
float camX, camY, camZ;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 100.0f;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];
float lightPos[4] = {4.0f, 1000.0f, 2.0f, 1.0f};

// Object Buffer, used for rendering and updating
std::vector<AppObject*> appObjects = std::vector<AppObject*>();

//Initializing our car
Car* car = new Car();
RearMirror* rearMirror = new RearMirror();

// Lens Flare object
LensFlare* lensFlare = new LensFlare();
int xFlare = 0, yFlare = 0, xMouse, yMouse;

std::vector<CollisionObject*> collidingObjects = std::vector<CollisionObject*>();


// Counter for time between frames
long lastFrameTime = 0;


// Variable that controls if the game is paused
bool pause = false;


typedef struct {
	float	life;		// vida
	float	fade;		// fade
	float	r, g, b;    // color
	GLfloat x, y, z;    // posi‹o
	GLfloat vx, vy, vz; // velocidade 
	GLfloat ax, ay, az; // acelera‹o
} Particle;

Particle particles[MAX_PARTICLES];
int dead_num_particles = 0;
// Camera Switching
enum CameraId
{
	ORTHO,
	PERSPECTIVE,
	CAR,
	FRONT
};

CameraId cameraId = ORTHO;

struct OrthoProperties {
	float posX;
	float posY;
	float posZ;

	float lookX;
	float lookY;
	float lookZ;

	float upX;
	float upY;
	float upZ;

	float width;
	float height;

	float nearPlane;
	float farPlane;
};

struct PerspectiveProperties {
	float posX;
	float posY;
	float posZ;

	float lookX;
	float lookY;
	float lookZ;

	float upX;
	float upY;
	float upZ;

	float fov;
	float ratio;
	
	float nearPlane;
	float farPlane;
};

struct PerspectiveProperties perspectiveProperties;
struct PerspectiveProperties carCamProperties;
struct PerspectiveProperties frontCamProperties;
struct PerspectiveProperties backCamProperties;
struct OrthoProperties orthoProperties;


// Physics properties to move the car
#define CAR_MAX_SPEED 100.0f
#define CAR_ACCEL 80.0f
#define CAR_ANGULAR_SPEED 180.0f

struct LightProperties {
	bool isEnabled; // true to apply this light in this invocation
	bool isLocal;	// true for a point light or a spotlight,
					// false for a positional light
	bool isSpot;	// true if the light is a spotlight
	vec3 color;		// color of light
	vec3 position;	// location of light, if is Local is true,
					// otherwise the direction toward the light
	vec3 halfVector;	// direction of highlights for directional light
	vec3 coneDirection; // spotlight attributes
	float spotCosCutoff;
	float spotExponent;
	float constantAttenuation;	// local light attenuation coefficients
	float linearAttenuation;
	float quadraticAttenuation;
								// other properties you may desire
};

#define N_LIGHTS 9
LightProperties lightProperties[N_LIGHTS];

float orangeSpeed = 10;

#define MAX_ORANGES 5
Orange* orangeList[MAX_ORANGES];

bool enableFog = true;

void timer(int value)
{
	std::string hudString;
	if (pause) hudString = " | Pause |";
	else if (car->lives > 0) hudString = " | " + std::to_string(car->lives) + " Lives | " + std::to_string(car->laps) + " Laps | ";
	else hudString = " | Game Over |";

	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")" << hudString;
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
    FrameCount = 0;
    glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1000/60, refresh, 0);
}

// Set the camera projection based on the active camera
void setCameraProjection() {
	loadIdentity(PROJECTION);

	switch (cameraId) {
	case ORTHO:
		ortho(-orthoProperties.width / 2, orthoProperties.width / 2,
			-orthoProperties.height / 2, orthoProperties.height / 2,
			orthoProperties.nearPlane, orthoProperties.farPlane);
		break;

	case PERSPECTIVE:
		perspective(perspectiveProperties.fov, perspectiveProperties.ratio,
			perspectiveProperties.nearPlane, perspectiveProperties.farPlane);
		break;

	case CAR:
		perspective(carCamProperties.fov, carCamProperties.ratio,
			carCamProperties.nearPlane, carCamProperties.farPlane);
		break;

	case FRONT:
		perspective(frontCamProperties.fov, frontCamProperties.ratio,
			frontCamProperties.nearPlane, frontCamProperties.farPlane);
		break;
	}
}

void setBackProjection() {
	loadIdentity(PROJECTION);
	perspective(backCamProperties.fov, backCamProperties.ratio,
		backCamProperties.nearPlane, backCamProperties.farPlane);
}

// Billboard Helper:
void beginBillboard() {

	int i, j;

	// Note that a row in the C convention is a column 
	// in OpenGL convention (see the red book, pg.106 in version 1.2)
	// right vector is [1,0,0]  (1st column)
	// lookAt vector is [0,0,1] (3d column)
	// leave the up vector unchanged (2nd column)
	// notice the increment in i in the first cycle (i+=2)
	for (i = 0; i < 3; i += 2)
		for (j = 0; j < 3; j++) {
			if (i == j)
				mCompMatrix[VIEW_MODEL][i * 4 + j] = 1.0;
			else
				mCompMatrix[VIEW_MODEL][i * 4 + j] = 0.0;
		}
}

void beginSphericalBillboard() {

	int i, j;

	// Note that a row in the C convention is a column 
	// in OpenGL convention (see the red book, pg.106 in version 1.2)
	// right vector is [1,0,0]  (1st column)
	// lookAt vector is [0,0,1] (3d column)
	// leave the up vector unchanged (2nd column)
	// notice the increment in i in the first cycle (i+=2)
	for (i = 0; i < 3; i += 1)
		for (j = 0; j < 3; j++) {
			if (i == j)
				mCompMatrix[VIEW_MODEL][i * 4 + j] = 1.0;
			else
				mCompMatrix[VIEW_MODEL][i * 4 + j] = 0.0;
		}
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if(h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;

	// update cameras based on ratio
	perspectiveProperties.ratio = ratio;
	carCamProperties.ratio = ratio;
	orthoProperties.height = 220;
	orthoProperties.width = 220 * ratio;

	//perspective(53.13f, ratio, 0.1f, 1000.0f);
	setCameraProjection();
}

// ------------------------------------------------------------
// Update the app world
//


void updateOranges(float deltaTime) {
	for (int i = 0; i < MAX_ORANGES; i++) {
		Orange *orange = orangeList[i];
		struct orangePhysics* orangePhysics = &orange->orangePhysics;
		if (!orange->active) {
			orangePhysics->timeToRespawn -= deltaTime;
			if (orangePhysics->timeToRespawn <= 0) {
				orange->active = true;
				orange->moveTo((double)rand() / RAND_MAX * 150 - 75, 0, (double)rand() / RAND_MAX * 150 - 75);

				float dir = ((double) rand() / RAND_MAX) * 360.0f;
				orangePhysics->speedX = cos(dir);
				orangePhysics->speedZ = sin(dir);

				dir -= 90.0f;
				orangePhysics->rotationX = cos(dir);
				orangePhysics->rotationZ = sin(dir);
			}
		}
		else {
			orange->step(orangePhysics->speedX * orangeSpeed * deltaTime, 0, orangePhysics->speedZ * orangeSpeed * deltaTime);
			loadMatrix(MODEL, mesh[orange->meshes[0]].transform);
			pushMatrix(MODEL);
			rotate(MODEL, orangeSpeed * deltaTime / 0.087, orangePhysics->rotationX, 0 , orangePhysics->rotationZ);
			memcpy(mesh[orange->meshes[0]].transform, get(MODEL), 16 * sizeof(float));
			popMatrix(MODEL);

			if (orange->position.x > 100 || orange->position.x < -100 ||
				orange->position.z > 100 || orange->position.z < -100) {
				orange->active = false;
				orangePhysics->timeToRespawn = (double)rand() / RAND_MAX * 7 + 3;
			}
		}

		orangeSpeed += 0.2 * deltaTime;
	}
}
bool checkCollisions() {


	for (CollisionObject* obj : collidingObjects) {
		if (obj->active && car->isColliding(obj)) {
			obj->movement.speed = car->carPhysics.speed * 0.7;
			obj->movement.direction = car->carPhysics.angle;
			return true;
			std::cout << "Other object HIT\n";
		}
	}
	return false;
}

void checkLifeLoss() {
	for (Orange* orange : orangeList) {
		if (orange->active && car->isColliding(orange)) {
			car->lives--;
			car->resetPosition();
			orange->active = false;
			std::cout << "ORAGE HIT\n";
		}
	}
	if (car->position.x > 100 || car->position.x < -100 || car->position.z >100 || car->position.z < -100) {
		car->lives--;
		car->resetPosition();
	}
}

void collideObjects() {
	for (int i = 0; i < collidingObjects.size(); i++) {
		CollisionObject* obj1 = collidingObjects.at(i);

		for (int j = i + 1; j < collidingObjects.size(); j++) {
			CollisionObject* obj2 = collidingObjects.at(j);

			if (obj1 == obj2) continue;
			if (obj1->isColliding(obj2)) {
				CollisionObject* mover = (abs(obj1->movement.speed) >= abs(obj2->movement.speed)) ? obj1 : obj2;
				CollisionObject* stoper = (abs(obj1->movement.speed) >= abs(obj2->movement.speed)) ? obj2 : obj1;

				stoper->movement.direction = mover->movement.direction;
				stoper->movement.speed = mover->movement.speed * 0.9;
				mover->movement.speed = mover->movement.speed * 0.9;
			}
		}
	}
}

void updateCollisionObjects(float deltaTime) {
	for (CollisionObject* obj : collidingObjects) {
		if (obj->movement.speed > 0) {
			obj->movement.speed = fmaxf(obj->movement.speed - (obj->movement.drag * deltaTime), 0);
		}
		else if (obj->movement.speed < 0) {
			obj->movement.speed = fminf(obj->movement.speed + (obj->movement.drag * deltaTime), 0);
		}

		float dirX = cos(obj->movement.direction * 3.14 / 180);
		float dirZ = sin(obj->movement.direction * 3.14 / 180);
		obj->step(dirX * obj->movement.speed * deltaTime, 0, dirZ * obj->movement.speed * deltaTime);
		if (obj->position.x > 100 || obj->position.x < -100 || obj->position.z >100 || obj->position.z < -100) {
			obj->active = false;
		}
	}
}

void physicsUpdate() {
	// Get the delta time.
	float deltaTime = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);

	if (!pause && car->lives > 0) {
		deltaTime = (time - lastFrameTime) / 1000.0f;
	}

	lastFrameTime = time;

	
	// std::cout << time << ":" << lastFrameTime << std::endl;

	// Update the car's direction
	if (car->carPhysics.speed != 0) {
		int sign = (car->carPhysics.speed > 0) ? 1 : -1;

		car->carPhysics.angle += sign * car->carPhysics.angularSpeed * deltaTime * (cameraId == FRONT ? 0.5 : 1);
		car->rotate(0, -car->carPhysics.angle + 90, 0);
	}

	// Update the car speed
	if (car->carPhysics.accel != 0) {
		car->carPhysics.speed += car->carPhysics.accel * deltaTime;
	}
	else {
		if (car->carPhysics.speed > 0) {
			car->carPhysics.speed = fmaxf(car->carPhysics.speed - (CAR_ACCEL * deltaTime), 0);
		}
		else if (car->carPhysics.speed < 0) {
			car->carPhysics.speed = fminf(car->carPhysics.speed + (CAR_ACCEL * deltaTime), 0);
		}
	}

	// Clip by speed
	car->carPhysics.speed = fminf(car->carPhysics.speed, CAR_MAX_SPEED);
	car->carPhysics.speed = fmaxf(car->carPhysics.speed, -CAR_MAX_SPEED);


	// Update the car position
	float dirX = cos(car->carPhysics.angle * 3.14 / 180);
	float dirZ = sin(car->carPhysics.angle * 3.14 / 180);

	float perpX = cos((car->carPhysics.angle + 90) * 3.14 / 180);
	float perpZ = sin((car->carPhysics.angle + 90) * 3.14 / 180);

	car->step(dirX * car->carPhysics.speed * deltaTime, 0, dirZ * car->carPhysics.speed * deltaTime);
	if (checkCollisions()) {
		car->step(-dirX * car->carPhysics.speed * deltaTime, 0, -dirZ * car->carPhysics.speed * deltaTime);
		car->carPhysics.speed = -car->carPhysics.speed / 2;
	}

	// Update the car camera position
	carCamProperties.posX = camX + car->position.x;
	carCamProperties.posY = camY + car->position.y;
	carCamProperties.posZ = camZ + car->position.z;

	carCamProperties.lookX = car->position.x;
	carCamProperties.lookY = car->position.y;
	carCamProperties.lookZ = car->position.z;

	// Update the front camera position
	frontCamProperties.posX = car->position.x + dirX * 1;
	frontCamProperties.posY = 4;
	frontCamProperties.posZ = car->position.z + dirZ * 1;

	frontCamProperties.lookX = frontCamProperties.posX + dirX;
	frontCamProperties.lookY = frontCamProperties.posY;
	frontCamProperties.lookZ = frontCamProperties.posZ + dirZ;

	// Update the back camera position
	backCamProperties.posX = car->position.x - dirX * 1;
	backCamProperties.posY = 5;
	backCamProperties.posZ = car->position.z - dirZ * 1;

	backCamProperties.lookX = backCamProperties.posX - dirX;
	backCamProperties.lookY = backCamProperties.posY * 0.96;
	backCamProperties.lookZ = backCamProperties.posZ - dirZ;

	// Update the rear mirror position
	rearMirror->moveTo(car->position.x + dirX * 7, 4.7, car->position.z + dirZ * 7);
	rearMirror->rotation.y = car->rotation.y + 180;

	// Update the headlights
	int l = 7;
	lightProperties[l].position.x = car->position.x + dirX * 6.f + perpX;
	lightProperties[l].position.z = car->position.z + dirZ * 2 + perpZ * 1;
	lightProperties[l].coneDirection.x = cos((car->carPhysics.angle + 20) * 3.14 / 180);
	lightProperties[l].coneDirection.y = 0;
	lightProperties[l].coneDirection.z = sin((car->carPhysics.angle + 20) * 3.14 / 180);

	l = 8;
	lightProperties[l].position.x = car->position.x + dirX * 6.f - perpX;
	lightProperties[l].position.z = car->position.z + dirZ * 2 - perpZ * 1;
	lightProperties[l].coneDirection.x = cos((car->carPhysics.angle - 20) * 3.14 / 180);
	lightProperties[l].coneDirection.y = 0;
	lightProperties[l].coneDirection.z = sin((car->carPhysics.angle - 20) * 3.14 / 180);

	checkLifeLoss();
	updateOranges(deltaTime);
	updateCollisionObjects(deltaTime);
	collideObjects();
	
}

// Set the camera look at based on the active camera
void setCameraLookAt() {
	switch (cameraId) {
		case ORTHO:
			lookAt(orthoProperties.posX, orthoProperties.posY, orthoProperties.posZ, 
				   orthoProperties.lookX, orthoProperties.lookY, orthoProperties.lookZ,
				   orthoProperties.upX, orthoProperties.upY, orthoProperties.upZ);
			break;

		case PERSPECTIVE:
			lookAt(perspectiveProperties.posX, perspectiveProperties.posY, perspectiveProperties.posZ,
				perspectiveProperties.lookX, perspectiveProperties.lookY, perspectiveProperties.lookZ,
				perspectiveProperties.upX, perspectiveProperties.upY, perspectiveProperties.upZ);
			break;

		case CAR:
			lookAt(carCamProperties.posX, carCamProperties.posY, carCamProperties.posZ,
				carCamProperties.lookX, carCamProperties.lookY, carCamProperties.lookZ,
				carCamProperties.upX, carCamProperties.upY, carCamProperties.upZ);
			break;

		case FRONT:
			lookAt(frontCamProperties.posX, frontCamProperties.posY, frontCamProperties.posZ,
				frontCamProperties.lookX, frontCamProperties.lookY, frontCamProperties.lookZ,
				frontCamProperties.upX, frontCamProperties.upY, frontCamProperties.upZ);
			break;
	}
}

void setBackLookAt() {
	lookAt(backCamProperties.posX, backCamProperties.posY, backCamProperties.posZ,
		backCamProperties.lookX, backCamProperties.lookY, backCamProperties.lookZ,
		backCamProperties.upX, backCamProperties.upY, backCamProperties.upZ);
}


// ------------------------------------------------------------
//
// Render stufff
//


void sendLights()
{
	GLint loc;

	for (int i = 0; i < N_LIGHTS; i++) {

		/*struct LightProperties {
		bool isEnabled; // true to apply this light in this invocation
		bool isLocal;	// true for a point light or a spotlight,
		// false for a positional light
		bool isSpot;	// true if the light is a spotlight
		vec3 color;		// color of light
		vec3 position;	// location of light, if is Local is true,
		// otherwise the direction toward the light
		//vec3 halfVector;	// direction of highlights for directional light
		vec3 coneDirection; // spotlight attributes
		float spotCosCutoff;
		float spotExponent;
		float constantAttenuation;	// local light attenuation coefficients
		float linearAttenuation;
		float quadraticAttenuation;
		// other properties you may desire
		}*/


		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].isEnabled")).c_str());
		glUniform1i(loc, lightProperties[i].isEnabled);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].isLocal")).c_str());
		glUniform1i(loc, lightProperties[i].isLocal);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].isSpot")).c_str());
		glUniform1i(loc, lightProperties[i].isSpot);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].color")).c_str());
		glUniform3f(loc, lightProperties[i].color.x, lightProperties[i].color.y, lightProperties[i].color.z);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].position")).c_str());
		glUniform3f(loc, lightProperties[i].position.x, lightProperties[i].position.y, lightProperties[i].position.z);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].coneDirection")).c_str());
		glUniform3f(loc, lightProperties[i].coneDirection.x, lightProperties[i].coneDirection.y, lightProperties[i].coneDirection.z);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].spotCosCutoff")).c_str());
		glUniform1f(loc, lightProperties[i].spotCosCutoff);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].spotExponent")).c_str());
		glUniform1f(loc, lightProperties[i].spotExponent);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].constantAttenuation")).c_str());
		glUniform1f(loc, lightProperties[i].constantAttenuation);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].linearAttenuation")).c_str());
		glUniform1f(loc, lightProperties[i].linearAttenuation);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].quadraticrAttenuation")).c_str());
		glUniform1f(loc, lightProperties[i].quadraticAttenuation);
		loc = glGetUniformLocation(shader.getProgramIndex(), (std::string("Lights[") + std::to_string(i) + std::string("].halfVector")).c_str());
		glUniform3f(loc, lightProperties[i].halfVector.x, lightProperties[i].halfVector.y, lightProperties[i].halfVector.z);
	}
}

void renderObject(AppObject* o, bool drawOpaque) {
	float modelview[16];  //To be used in "Cheating" Matrix reset Billboard technique

	// Don't render inactive objects.
	if (!o->active) return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[o->texture_id1]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[o->texture_id2]);

	for (int meshId : o->meshes) {
		// Get the id of the object to be rendered.
		objId = meshId;

		if ((drawOpaque && mesh[objId].mat.transparency < 1.0f) ||
			(!drawOpaque && mesh[objId].mat.transparency >= 1.0f)) continue;

		// send the material
		GLint loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.transparency");
		glUniform1f(loc, mesh[objId].mat.transparency);
		pushMatrix(MODEL);

		// Apply global Transformations
		scale(MODEL, o->scale.x, o->scale.y, o->scale.z);
		translate(MODEL, o->position.x, o->position.y, o->position.z);

		// Rotate the model with Euler Angles
		rotate(MODEL, o->rotation.x, 1, 0, 0);
		rotate(MODEL, o->rotation.y, 0, 1, 0);
		rotate(MODEL, o->rotation.z, 0, 0, 1);

		// Apply local transformations
		multMatrix(MODEL, mesh[objId].transform);

		// send matrices to OGL
		if (o->billboard) {
			computeDerivedMatrix(VIEW_MODEL);
			memcpy(modelview, mCompMatrix[VIEW_MODEL], sizeof(float) * 16);  //save VIEW_MODEL in modelview matrix

			//reset VIEW_MODEL
			beginBillboard();

			computeDerivedMatrix_PVM(); // calculate PROJ_VIEW_MODEL
		}
		else computeDerivedMatrix(PROJ_VIEW_MODEL);

		glUniformMatrix4fv(model_uniformId, 1, GL_FALSE, get(MODEL));
		glUniformMatrix4fv(view_uniformId, 1, GL_FALSE, get(VIEW));
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// multi texturing
		glUniform1i(texMode_uniformId, mesh[objId].mat.texCount);



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
}

void renderObjects(bool drawOpaque) {
	for (AppObject* o : appObjects) {
		renderObject(o, drawOpaque);
	}
}
void renderFireworks() {
	float modelview[16];  //To be used in "Cheating" Matrix reset Billboard technique
	if (car->fireworks) {
		float particle_color[4];
		// draw fireworks particles
		objId = fireworksId;  //quad for particle

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureArray[3]); //particle.bmp associated to TU0 

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);  //Depth Buffer Read Only


		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			if (particles[i].life > 0.0f) /* só desenha as que ainda estão vivas */
			{

				/* A vida da partícula representa o canal alpha da cor. Como o blend está activo a cor final é a soma da cor rgb do fragmento multiplicada pelo
				alpha com a cor do pixel destino */

				particle_color[0] = particles[i].r;
				particle_color[1] = particles[i].g;
				particle_color[2] = particles[i].b;
				particle_color[3] = particles[i].life;

				// send the material - diffuse color modulated with texture
				GLint loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, particle_color);

				pushMatrix(MODEL);
				translate(MODEL, particles[i].x, particles[i].y, particles[i].z);

				// send matrices to OGL
				computeDerivedMatrix(VIEW_MODEL);
				memcpy(modelview, mCompMatrix[VIEW_MODEL], sizeof(float) * 16);  //save VIEW_MODEL in modelview matrix

				//reset VIEW_MODEL
				beginSphericalBillboard();

				computeDerivedMatrix_PVM(); // calculate PROJ_VIEW_MODEL
				glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
				glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
				computeNormalMatrix3x3();
				glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

				glUniform1i(texMode_uniformId, 3);

				glBindVertexArray(mesh[objId].vao);
				glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
				popMatrix(MODEL);
			}
			else dead_num_particles++;
		}

		glDepthMask(GL_TRUE); //make depth buffer again writeable

		if (dead_num_particles == MAX_PARTICLES) {
			car->fireworks = 0;
			dead_num_particles = 0;
			printf("All particles dead\n");
		}

	}
}

void startFireworks() {
	GLfloat v, theta, phi;
	int i;

	for (i = 0; i < MAX_PARTICLES; i++)
	{
		v = 0.8*frand() + 0.8;
		phi = frand()*M_PI;
		theta = 2.0*frand()*M_PI;

		particles[i].x = car->position.x;
		particles[i].y = 5;
		particles[i].z = car->position.z;
		particles[i].vx = v * cos(theta) * sin(phi);
		particles[i].vy = v * cos(phi);
		particles[i].vz = v * sin(theta) * sin(phi);
		particles[i].ax = 0.1f; /* simular um pouco de vento */
		particles[i].ay = -0.15f; /* simular a aceleração da gravidade */
		particles[i].az = 0.0f;

		
		particles[i].r = frand();
		particles[i].g = frand();
		particles[i].b = frand();

		particles[i].life = 1.0f;		/* vida inicial */
		particles[i].fade = 0.005f;	    /* step de decréscimo da vida para cada iteração */
	}
}

void iterateFireworks(int value)
{
	int i;
	float h;

	/* Método de Euler de integração de eq. diferenciais ordinárias
	h representa o step de tempo; dv/dt = a; dx/dt = v; e conhecem-se os valores iniciais de x e v */

	h = 0.125f;
	//	h = 0.033;
	if (car->fireworks) {

		for (i = 0; i < MAX_PARTICLES; i++)
		{
			particles[i].x += (h*particles[i].vx);
			particles[i].y += (h*particles[i].vy);
			particles[i].z += (h*particles[i].vz);
			particles[i].vx += (h*particles[i].ax);
			particles[i].vy += (h*particles[i].ay);
			particles[i].vz += (h*particles[i].az);
			particles[i].life -= particles[i].fade;
		}
		glutPostRedisplay();
		glutTimerFunc(10, iterateFireworks, 0);
	}
}

void renderScene(void) {

		physicsUpdate();
		car->checkLap({ 0, 0, 77.5 }, { 0, 0, -77.5 }, 25.0f);
		if (car->fireworks == 2) {
			car->fireworks = 1;
			startFireworks();
			glutTimerFunc(0, iterateFireworks, 0);
		}
		GLint loc;

		FrameCount++;
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		// load identity matrices
		loadIdentity(VIEW);
		loadIdentity(MODEL);
		// set the camera using a function similar to gluLookAt
		//lookAt(camX, camY, camZ, 0,0,0, 0,1,0);
		setCameraLookAt();
		// use our shader
		glUseProgram(shader.getProgramIndex());

		//send the light position in eye coordinates

		//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

		float res[4];
		multMatrixPoint(VIEW, lightPos, res);   //lightPos definido em World Coord so is converted to eye space
		glUniform4fv(lPos_uniformId, 1, res);

		glUniform1i(tex_loc, 0);
		glUniform1i(tex_loc1, 1);

		// Pass the Light information
		sendLights();

		loc = glGetUniformLocation(shader.getProgramIndex(), "enableFog");
		glUniform1i(loc, enableFog);


		// Render all AppObjects registered in the appObjects vector.
		// Takes into consideration the transformations defined by the object.
		renderObjects(true);
		renderObjects(false);
		renderFireworks();
		// Render the rear mirror
		if (cameraId == FRONT) {
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_ALWAYS, 1, 1);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			renderObject(rearMirror, true);
			loadIdentity(VIEW);
			setBackLookAt();
			setBackProjection();
			glClear(GL_DEPTH_BUFFER_BIT);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glStencilFunc(GL_EQUAL, 1, 1);
			renderObjects(true);
			renderObjects(false);
			renderFireworks();
			glDisable(GL_STENCIL_TEST);
		}

		if (cameraId == CAR) {
			lensFlare->render(xFlare, yFlare);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glutSwapBuffers();
	
}

void restart() {
	for (int i = 0; i < MAX_ORANGES; i++) {
		orangeList[i]->active = false;
		orangeList[i]->orangePhysics = { 0.0f,0.0f, 0.0f,0.0f, 5.0f + 10.0f * i };
	}
	int num_elements = sizeof(collidingObjects) / sizeof(collidingObjects[0]);
	for (CollisionObject* c : collidingObjects) {
		c->resetPosition();
	}
	car->resetCar();
}

void setMirrorColor(float r, float g, float b, float a) {
	mesh[rearMirror->meshes[0]].mat.diffuse[0] = r;
	mesh[rearMirror->meshes[0]].mat.diffuse[1] = g;
	mesh[rearMirror->meshes[0]].mat.diffuse[2] = b;
	mesh[rearMirror->meshes[0]].mat.diffuse[3] = a;

	mesh[rearMirror->meshes[0]].mat.specular[0] = r;
	mesh[rearMirror->meshes[0]].mat.specular[1] = g;
	mesh[rearMirror->meshes[0]].mat.specular[2] = b;
	mesh[rearMirror->meshes[0]].mat.specular[3] = a;

	mesh[rearMirror->meshes[0]].mat.ambient[0] = r;
	mesh[rearMirror->meshes[0]].mat.ambient[1] = g;
	mesh[rearMirror->meshes[0]].mat.ambient[2] = b;
	mesh[rearMirror->meshes[0]].mat.ambient[3] = a;
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch(key) {

		case 27:
			glutLeaveMainLoop();
			break;

		case 'a':
			car->carPhysics.accel = -CAR_ACCEL;  //move backwards "a"
			break;
		case 'q':
			car->carPhysics.accel = CAR_ACCEL; //move forward "q" 
			break;
		case 'o':
			car->carPhysics.angularSpeed = -CAR_ANGULAR_SPEED; //move left "o"
			break;
		case 'p':
			car->carPhysics.angularSpeed = CAR_ANGULAR_SPEED; //move right "p"
			break;

		case 's':
			pause = !pause;
			break;

		case 'r':
			restart();
			break;

		case 'f':
			enableFog = !enableFog;
			if (enableFog) {
				glClearColor(0.8, 0.6, 0.87, 1);
				setMirrorColor(0.8, 0.6, 0.87, 1);
			}
			else {
				glClearColor(1.0, 1.0, 1.0, 1.0);
				setMirrorColor(1.0, 1.0, 1.0, 1.0);
			}
			break;

		case '1':
			cameraId = ORTHO;
			setCameraProjection();
			break;
		case '2':
			cameraId = PERSPECTIVE;
			setCameraProjection();
			break;
		case '3':
			cameraId = CAR;
			setCameraProjection();
			break;

		case '4':
			cameraId = FRONT;
			setCameraProjection();
			break;

		case 'n':
			lightProperties[6].isEnabled = !lightProperties[6].isEnabled;
			break;

		case 'c':
			for (int i = 0; i < 6; i++) {
				lightProperties[i].isEnabled = !lightProperties[i].isEnabled;
			}
			break;

		case 'h':
			for (int i = 7; i < 9; i++) {
				lightProperties[i].isEnabled = !lightProperties[i].isEnabled;
			}
			break;
	}
}

void processKeysUp(unsigned char key, int xx, int yy) {
	switch (key) {
		case 'a':
		case 'q':
			car->carPhysics.accel = 0; //stop
			break;
		case 'o':	
		case 'p':
			car->carPhysics.angularSpeed = 0; //stop rotating
			break;
	}
}

// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// Only enable dragging the camera if the moveable camera is active
	if (cameraId != CAR) return;

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

			if (beta > 85.0f)
				beta = 85.0f;
			else if (beta < 10.0f)
				beta = 10.0f;
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
void processMouseMotion(int xx, int yy) {
	if (cameraId != CAR) return;

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux = r;

	deltaX =  - xx + startX;
	deltaY =    yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < 10.0f)
			betaAux = 10.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 5.0f);
		if (rAux < 20.0f)
			rAux = 20.0f;
		if (rAux > 700.0f)
			rAux = 700.0f;
		r = rAux;
	}

	camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camY = rAux *   						       sin(betaAux * 3.14f / 180.0f);

	if (tracking == 1) {
		int SCREENwidth = glutGet(GLUT_WINDOW_WIDTH),
			SCREENheight = glutGet(GLUT_WINDOW_HEIGHT);
		// Scale mouse coordinates to compensate for window size.
		xFlare += (xx - xMouse) * SCREENwidth;
		yFlare += ( (SCREENheight - yy) - yMouse) * SCREENheight;

		// Clamping -- wouldn't be needed in fullscreen mode.
		if (xFlare >= SCREENwidth)
			xFlare = SCREENwidth - 1;
		if (xFlare < 0)
			xFlare = 0;
		if (yFlare >= SCREENheight)
			yFlare = SCREENheight - 1;
		if (yFlare < 0)
			yFlare = 0;

		xFlare = xx;
		yFlare = SCREENheight - yy;
	}

	// Remember last mouse position.
	xMouse = xx;
	yMouse = yy;
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 5.0f;
	if (r < 20.0f)
		r = 20.0f;
	if (r > 700.0f)
		r = 700.0f;

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
	glBindAttribLocation(shader.getProgramIndex(), VSShaderLib::VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), VSShaderLib::NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), VSShaderLib::TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode"); // different modes of texturing
	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	model_uniformId = glGetUniformLocation(shader.getProgramIndex(), "model");
	view_uniformId = glGetUniformLocation(shader.getProgramIndex(), "view");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");

	
	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
	
	return(shader.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void initializeCameras() {
	// set the camera position based on its spherical coordinates
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	// initialize cameras

	// Ortho
	orthoProperties.posX = 0;
	orthoProperties.posY = 100;
	orthoProperties.posZ = 0;

	orthoProperties.lookX = 0;
	orthoProperties.lookY = 0;
	orthoProperties.lookZ = 0;

	orthoProperties.upX = 1;
	orthoProperties.upY = 0;
	orthoProperties.upZ = 0;

	orthoProperties.nearPlane = 1;
	orthoProperties.farPlane = 1500;

	// Perspective
	perspectiveProperties.posX = 220;
	perspectiveProperties.posY = 100;
	perspectiveProperties.posZ = 220;

	perspectiveProperties.lookX = 0;
	perspectiveProperties.lookY = 0;
	perspectiveProperties.lookZ = 0;

	perspectiveProperties.upX = 0;
	perspectiveProperties.upY = 1;
	perspectiveProperties.upZ = 0;

	perspectiveProperties.fov = 30;
	perspectiveProperties.ratio = WinX / WinY;

	perspectiveProperties.nearPlane = 10;
	perspectiveProperties.farPlane = 1500;

	// Car Camera
	carCamProperties.upX = 0;
	carCamProperties.upY = 1;
	carCamProperties.upZ = 0;

	carCamProperties.fov = 20;
	carCamProperties.ratio = WinX / WinY;

	carCamProperties.nearPlane = 3;
	carCamProperties.farPlane = 1000;

	// Front Camera
	frontCamProperties.upX = 0;
	frontCamProperties.upY = 1;
	frontCamProperties.upZ = 0;

	frontCamProperties.fov = 20;
	frontCamProperties.ratio = WinX / WinY;

	frontCamProperties.nearPlane = 3;
	frontCamProperties.farPlane = 1000;

	// Back Camera
	backCamProperties.upX = 0;
	backCamProperties.upY = 1;
	backCamProperties.upZ = 0;

	backCamProperties.fov = 20;
	backCamProperties.ratio = WinX / WinY;

	backCamProperties.nearPlane = 1;
	backCamProperties.farPlane = 1000;
}

template<typename Functor>
void drawObject(int id, AppObject* object, Material material, Functor &f) {
	objId = id;
	memcpy(mesh[objId].mat.ambient, material.ambient, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, material.diffuse, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, material.specular, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, material.emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = material.shininess;
	mesh[objId].mat.texCount = material.texCount;

	pushMatrix(MODEL);
	f();
	memcpy(mesh[objId].transform, get(MODEL), 16 * sizeof(float));
	popMatrix(MODEL);

	object->addMesh(objId);
}

void initializeTable(float width, float height) {
	Table* tableObject = new Table(width, height, 10);
	tableObject->initializeObject();
	appObjects.push_back(tableObject);
}


void initializeCar() {
	car->initializeObject();
	rearMirror->initializeObject();
	// add car object
	appObjects.push_back(car);
}

void initializeCheerios() {
	for (int i = 0; i < 5; i++) {
		Cheerio* cheerioObject = new Cheerio(1);
		cheerioObject->initializeObject();
		cheerioObject->moveTo(10 * i - 30, 0, 50);
		appObjects.push_back(cheerioObject);
	}
}

void initializeButter() {
	Butter* butterObject = new Butter();
	butterObject->initializeObject();
	appObjects.push_back(butterObject);
	collidingObjects.push_back(butterObject);

}

void initializeOranges() {
	for (int i = 0; i < MAX_ORANGES; i++) {
		Orange* orangeObject = new Orange();
		orangeObject->initializeObject();
		orangeObject->moveTo(0, 0, 0);
		appObjects.push_back(orangeObject);

		orangeObject->active = false;
		orangeList[i] = orangeObject;
		orangeObject->orangePhysics = { 0.0f, 0.0f, 0.0f, 0.0f, 5.0f + 10.0f * i };
	}
}

void cheerio(float size, float x, float y) {
	Cheerio* cheerioObject = new Cheerio(size);
	cheerioObject->initializeObject();
	cheerioObject->moveTo(x, 0, y);
	appObjects.push_back(cheerioObject);
	collidingObjects.push_back(cheerioObject);
	cheerioObject->defineInitialPosition();
}

void initializeRoad(float table_width, float table_heigth, float out_distance, float road_width, float cheerio_distance, float cheerio_size) {
	float limit_w = table_width / 2 - out_distance;
	float limit_h = table_heigth / 2 - out_distance;

	int num_cheerios = ceil(2 * (table_heigth - 2 * out_distance + cheerio_distance) / (float) (cheerio_size + cheerio_distance)) - 1;

	for (int i = 0; i < num_cheerios; i++) {
		cheerio(cheerio_size, limit_h - i * cheerio_distance, limit_w);
		cheerio(cheerio_size, limit_h - i * cheerio_distance, - limit_w);
	}

	num_cheerios = ceil(2 * (table_width - 2 * out_distance + cheerio_distance) / (float)(cheerio_size + cheerio_distance)) - 2;

	for (int i = 0; i < num_cheerios; i++) {
		cheerio(cheerio_size, limit_h, limit_w - i * cheerio_distance);
		cheerio(cheerio_size, -limit_h, limit_w - i * cheerio_distance);
	}


	limit_w -= road_width;
	limit_h -= road_width;

	num_cheerios = ceil(2 * (table_heigth - 2 * (out_distance + road_width) + cheerio_distance) / (float)(cheerio_size + cheerio_distance)) - 2;

	for (int i = 1; i < num_cheerios; i++) {
		cheerio(cheerio_size, limit_h - i * cheerio_distance, limit_w);
		cheerio(cheerio_size, limit_h - i * cheerio_distance, -limit_w);
	}

	num_cheerios = ceil(2 * (table_width - 2 * (out_distance + road_width) + cheerio_distance) / (float)(cheerio_size + cheerio_distance)) - 2;

	for (int i = 1; i < num_cheerios; i++) {
		cheerio(cheerio_size, limit_h, limit_w - i * cheerio_distance);
		cheerio(cheerio_size, -limit_h, limit_w - i * cheerio_distance);
	}
}

void initializeFlags() {
	Flag* flagObject = new Flag(0, 95);
	flagObject->initializeObject();
	appObjects.push_back(flagObject);

	flagObject = new Flag(0, 40);
	flagObject->initializeObject();
	appObjects.push_back(flagObject);

	flagObject = new Flag(0, -40);
	flagObject->initializeObject();
	appObjects.push_back(flagObject);

	flagObject = new Flag(0, -95);
	flagObject->initializeObject();
	appObjects.push_back(flagObject);
}


void initializeLights() {
	// 6 point lights

	/*struct LightProperties {
		bool isEnabled; // true to apply this light in this invocation
		bool isLocal;	// true for a point light or a spotlight,
						// false for a positional light
		bool isSpot;	// true if the light is a spotlight
		vec3 color;		// color of light
		vec3 position;	// location of light, if is Local is true,
						// otherwise the direction toward the light
		//vec3 halfVector;	// direction of highlights for directional light
		vec3 coneDirection; // spotlight attributes
		float spotCosCutoff;
		float spotExponent;
		float constantAttenuation;	// local light attenuation coefficients
		float linearAttenuation;
		float quadraticAttenuation;
									// other properties you may desire
	}*/

	for (int i = 0; i < 6; i++) {
		vec3 position = { (50.0f * (i % 3) - 50.0f) , 15.0f , (i > 2) ? (75.0f) : (-75.0f) };

		lightProperties[i].isEnabled = true;
		lightProperties[i].isLocal = true;
		lightProperties[i].isSpot = false;
		lightProperties[i].color = { 0.95f * 0.5, 0.75f * 0.5, 0.25f * 0.5 };
		lightProperties[i].position = position;
		lightProperties[i].constantAttenuation = 0.1f;
		lightProperties[i].linearAttenuation = 0.01f;
		lightProperties[i].quadraticAttenuation = 0.01f;

		Light* light = new Light(position);
		light->initializeObject();
		appObjects.push_back(light);
	}

	// 1 directional light

	int l = 6;
	lightProperties[l].isEnabled = true;
	lightProperties[l].isLocal = false;
	lightProperties[l].isSpot = false;
	lightProperties[l].color = { .6f, .6f, .6f };
	lightProperties[l].position = { -20.0f, 50.0f, -20.0f };
	lightProperties[l].halfVector = { 0.0f, 1.0f, 0.0f };

	// 2 spotlights
	l = 7;
	lightProperties[l].isEnabled = true;
	lightProperties[l].isLocal = true;
	lightProperties[l].isSpot = true;
	lightProperties[l].color = { .6f, .6f, .6f };
	lightProperties[l].position = { -1.0f, 1.0f, -1.0f };
	lightProperties[l].constantAttenuation = 0.0f;
	lightProperties[l].linearAttenuation = 0.01f;
	lightProperties[l].quadraticAttenuation = 0.1f;
	lightProperties[l].coneDirection = { 1.0f, 0, 0 };
	lightProperties[l].spotCosCutoff = 0.9f;
	lightProperties[l].spotExponent = 10.0f;

	l = 8;
	lightProperties[l].isEnabled = true;
	lightProperties[l].isLocal = true;
	lightProperties[l].isSpot = true;
	lightProperties[l].color = { .6f, .6f, .6f };
	lightProperties[l].position = { -1.0f, 1.0f, -1.0f };
	lightProperties[l].constantAttenuation = 0.0f;
	lightProperties[l].linearAttenuation = 0.01f;
	lightProperties[l].quadraticAttenuation = 0.1f;
	lightProperties[l].coneDirection = { 1.0f, 0, 0 };
	lightProperties[l].spotCosCutoff = 0.9f;
	lightProperties[l].spotExponent = 10.0f;

}

void initializeParticles() {
	fireworksId = ++objId;
	mesh[fireworksId].mat.texCount = 1;
	createQuad(4, 4);
}

void initializeLensFlare() {
	lensFlare->initializeObject();
}

void init()
{
	float tableWidth = 200, tableHeight = 200;

	//Texture Object definition
	char chessTex[] = "textures/chess.tga";
	char lightwoodTex[] = "textures/lightwood.tga";
	char flagTex[] = "textures/flag.tga";
	char particleTex[] = "textures/particle.tga";

	glGenTextures(4, TextureArray);
	TGA_Texture(TextureArray, chessTex, 0);
	TGA_Texture(TextureArray, lightwoodTex, 1);
	TGA_Texture(TextureArray, flagTex, 2);
	TGA_Texture(TextureArray, particleTex, 3);

	initializeCameras();
	initializeTable(tableWidth, tableHeight);
	initializeRoad(tableWidth, tableHeight, 10, 45, 10, 9);
	initializeCar();
	initializeButter();
	initializeOranges();
	initializeLights();
	initializeFlags();
	initializeLensFlare();
	initializeParticles();
	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// ------- FOG CHANGES -------
	//glClearColor(0.05,0.03,0.00,1);//blackish color
	glClearColor(0.8, 0.6, 0.87, 1);//gray color, same as fog color
	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);
	// ------- FOG CHANGES -------

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	srand(std::time(nullptr));

	std::cerr << "Total Objects to Render: " << appObjects.size() << "\n";
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
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutKeyboardUpFunc(processKeysUp);
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

