#include "Table.h"
#include "AVTmathLib.h"

Table::Table(float width, float height, float thickness) {
	this->width = width;
	this->height = height;
	this->thickness = thickness;
}

void Table::initializeObject() {
	Material tableMaterial = {
		{0.2f, 0.15f, 0.1f, 1.0f},
		{0.8f, 0.6f, 0.4f, 1.0f},
		{0.8f, 0.8f, 0.8f, 1.0f},
		{0.0f, 0.0f, 0.0f, 1.0f},
		1000.0f,
		1.0f,
		2
	};

	//------------------Table---------------------------
	auto drawTable = [this]() {
		::createCube();
		::translate(MODEL, -height / 2, -thickness, -width / 2);
		::scale(MODEL, height, thickness, width);
	};

	loadIdentity(MODEL);
	drawObject(tableMaterial, drawTable);
}
