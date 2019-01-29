#pragma once
#include "AppObject.h"

#define FLARE_MAXELEMENTSPERFLARE         12


typedef struct flare_element {
	float           fDistance;        // Distance along ray from source (0.0-1.0)
	float           fSize;            // Size relative to flare envelope (0.0-1.0)
	float			rgba[4];
	int				meshId;
	GLuint			texture;
	bool isShine;
} flare_element;

class LensFlare : public AppObject {
public:
	LensFlare();
	void initializeObject() override;
	void render(int lx, int ly);

private:
	float fScale;     // Scale factor for adjusting overall size of flare elements.
	float fMaxSize;   // Max size of largest element, as proportion of screen width (0.0-1.0)
	int nPieces = FLARE_MAXELEMENTSPERFLARE;    // Number of elements in use
	int shine_tic = 0;
	flare_element elements[FLARE_MAXELEMENTSPERFLARE];

	void render_quads(int lx, int ly, int cx, int cy, int screenWidth, int screenHeight);
	void drawQuad(flare_element* element, int px, int py, float width, float height, float alpha);
	void setup_texture(char *filename, GLuint texobj);
};