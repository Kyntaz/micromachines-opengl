#include <math.h>

#include "LensFlare.h"
#include "basic_geometry.h"
#include "TGA.h"
#include "lightDemo.h"
#include "loadlum.h"
#include <math.h>

#include <GL/freeglut.h>

#define HEIGHTFROMWIDTH(w, SCREENheight, SCREENwidth)  ((320*(w)*SCREENheight)/(240*SCREENwidth))

typedef struct elem_def {
	float           fDistance;        // Distance along ray from source (0.0-1.0)
	float           fSize;            // Size relative to flare envelope (0.0-1.0)
	float			rgba[4];
	char texture[50];
} elem_def;

elem_def elems[FLARE_MAXELEMENTSPERFLARE] = {
	{ -1.3f,  0.4f, { 0.4, 0.8, 1.0, 1}, "textures/Flare1.tga"},
	{ -1.0f,  1.f, { 0.4, 0.4, 0.8, 1}, "textures/Flare5.tga"},
	{ -0.5f,  1.75f, { 0.8, 1.0, 0.6, 1}, "textures/Flare5.tga"},
	{ -0.2f,  0.5f, { 0.8, 0.9, 0.6, 1}, "textures/Flare1.tga"},
	{ -0.0f,  0.4f, { 0.6, 0.5, 0.6, 1}, "textures/Flare1.tga"},
	{ 0.25f,  0.7f, { 0.4, 0.6, 1.0, 1}, "textures/Flare6.tga"},
	{ 0.4f,  0.2f, { 0.5, 1.0, 1.0, 1}, "textures/Flare6.tga"},
	{ 0.6f,  0.4f, { 0.6, 0.8, 0.8, 1}, "textures/Flare6.tga"},
	{ 1.0f,  0.3f, { 0.5, 0.8, 0.7, 1}, "textures/Flare6.tga"},
	
	// Shines
	{ -1.0f,  1.75f, { 0.8, 0.8, 1, 1}, "textures/Shine1.tga"},
	{ -1.0f,  1.5f, { 0.8, 1, 0.8, 1}, "textures/Shine2.tga"},
	{ -1.0f,  2.f, { 1.0, 0.8, 0.8, 1}, "textures/Shine3.tga"}
};



/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

extern VSShaderLib shader;
 
extern GLint pvm_uniformId;
extern GLint vm_uniformId;
extern GLint normal_uniformId;
extern GLint lPos_uniformId;
extern GLint model_uniformId;
extern GLint view_uniformId;
extern GLint tex_loc, tex_loc1;
extern GLint texMode_uniformId;

#define SHINE_TEXT_NUMBER 2
GLuint shineTex[SHINE_TEXT_NUMBER];


void printMatrix(MatrixTypes m) {

	float* t = get(m);

	for (int i = 0; i < 16; i++)
		printf("%f\n", t[i]);
}

LensFlare::LensFlare() {
	fScale = 3;
	fMaxSize = 1;
}

void LensFlare::initializeObject() {
	GLuint textureArray[FLARE_MAXELEMENTSPERFLARE];

	glGenTextures(FLARE_MAXELEMENTSPERFLARE, textureArray);


	Material material = {
		{  0, 1, 0, 1 },
		{  0, 1, 0, 1 },
		{  0, 1, 0, 1 },
		{0.0f, 0.0f, 0.0f, 1.0f},
		100.0f,
		1.0f,
		4
	};

	auto drawLensFlare = [this]() {
		::createQuad(50, 50);
	};

	for (int i = 0; i < FLARE_MAXELEMENTSPERFLARE; i++) {
		elem_def elem = elems[i];
		flare_element element;

		drawObject(material, drawLensFlare);

		TGA_Texture(textureArray, elem.texture, i);

		element = {
			(elem.fDistance + 1) / 2,
			elem.fSize,
			{elem.rgba[0], elem.rgba[1], elem.rgba[2], elem.rgba[3]},
			objId,
			textureArray[i],
			false
		};

		elements[i] = element;
	}
};

void LensFlare::render(int lx, int ly) {
	int SCREENwidth = glutGet(GLUT_WINDOW_WIDTH);
	int SCREENheight = glutGet(GLUT_WINDOW_HEIGHT);
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW); //viewer looking down at negative z direction
	ortho(0, SCREENwidth, 0, SCREENheight, -1, 1);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	render_quads(lx, ly, SCREENwidth / 2, SCREENheight / 2, SCREENwidth, SCREENheight);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	popMatrix(MODEL);
	popMatrix(PROJECTION);
	popMatrix(VIEW);

}


void LensFlare::render_quads(int lx, int ly, int cx, int cy, int screenWidth, int screenHeight) {
	float     dx, dy;          // Screen coordinates of "destination"
	float     px, py;          // Screen coordinates of flare element
	float     maxflaredist, flaredist, distanceScale, flaremaxsize, flarescale;
	float     width, height, alpha;    // Piece parameters;
	int     i;
	flare_element* element;

	// Compute how far off-center the flare source is.
	maxflaredist = sqrt(cx*cx + cy * cy);
	flaredist = sqrt((lx - cx)*(lx - cx) + (ly - cy)*(ly - cy));
	
	distanceScale = (maxflaredist - flaredist) / maxflaredist;
	//flaremaxsize = (int)(screenWidth * fMaxSize);
	//flarescale = (int)(screenWidth * fScale);

	// Destination is opposite side of centre from source
	dx = cx + (cx - lx);
	dy = cy + (cy - ly);

	// Render each element.
	for (i = 0; i < FLARE_MAXELEMENTSPERFLARE; i++){
		element = &elements[i];

		// Position is interpolated along line between start and destination.
		px = (int)((1.0f - element->fDistance)*lx + element->fDistance*dx);
		py = (int)((1.0f - element->fDistance)*ly + element->fDistance*dy);

		// Piece size are 0 to 1; flare size is proportion of
		// screen width; scale by flaredist/maxflaredist.
		width = distanceScale * fScale * element->fSize;

		// Width gets clamped, to allows the off-axis flares
		// to keep a good size without letting the elements get
		// too big when centered.
		//if (width > flaremaxsize){
		//	width = flaremaxsize;
		//}

		// Flare elements are square (round) so height is just
		// width scaled by aspect ratio.
		height = width * (screenWidth / screenHeight);
		alpha = distanceScale;

		//if (width > 1){
		drawQuad(element, px - width / 2, py - height / 2, width, height, alpha);
		//}
	}
}

void LensFlare::drawQuad(flare_element* element, int px, int py, float width, float height, float alpha) {
	objId = element->meshId;

	glActiveTexture(GL_TEXTURE0);
	
	glBindTexture(GL_TEXTURE_2D, element->texture);


	
	GLuint shaderId = shader.getProgramIndex();

	// send the material
	// these colors should not be necessary
	/*glUniform4fv(loc, 1, mesh[objId].mat.ambient);
	loc = glGetUniformLocation(shaderId, "mat.specular");
	glUniform4fv(loc, 1, mesh[objId].mat.specular);
	loc = glGetUniformLocation(shaderId, "mat.shininess");
	glUniform1f(loc, mesh[objId].mat.shininess); */

	GLint loc = glGetUniformLocation(shaderId, "mat.ambient");

	loc = glGetUniformLocation(shaderId, "mat.diffuse");
	glUniform4fv(loc, 1, element->rgba);

	loc = glGetUniformLocation(shaderId, "mat.transparency");
	glUniform1f(loc, alpha * element->rgba[3]);

	glUniform1i(texMode_uniformId, mesh[objId].mat.texCount);

	auto matrix = MODEL;
	pushMatrix(matrix);
	::translate(matrix, px, py, 0);
	::scale(matrix, width, height, 0);

	computeDerivedMatrix(VIEW_MODEL);
	computeDerivedMatrix(PROJ_VIEW_MODEL);

	glUniformMatrix4fv(model_uniformId, 1, GL_FALSE, get(MODEL));
	glUniformMatrix4fv(view_uniformId, 1, GL_FALSE, get(VIEW));
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);


	// Render mesh
	glBindVertexArray(mesh[objId].vao);

	glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	

	popMatrix(matrix);

}
