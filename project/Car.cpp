#include "Car.h"
#include "AVTmathLib.h"

Car::Car(){
	collisionBox.center = {0, 0, 2}; 
	collisionBox.radius = 4;
	fireworks = 0;
	laps = 0;
	lives = 5;
	doneHalfLap = false;
}


void Car::initializeObject() {

		auto importer = new Assimp::Importer();

		const aiScene* sc = importer->ReadFile("objs/Car.dae", NULL);

		if (sc == NULL) {
			printf("Error reading object file:  %s\n", importer->GetErrorString());
			exit(1);
		}
		genVAOsAndUniformBuffer(sc);



		resetCar();
}

void Car::resetPosition() {
	// apply relative coordinates
	moveTo(-60, 1.5f, 60);
	rotate(0, 90, 0);

	// Set the car's physics properties
	carPhysics.accel = 0;
	carPhysics.angularSpeed = 0;
	carPhysics.speed = 0;
	carPhysics.angle = 0;

	doneHalfLap = false;
}

void Car::checkLap(vec3 startPoint, vec3 halfPoint, float radius) {
	if (doneHalfLap && isInside(startPoint, radius)) {
		laps++;
		fireworks = 2;
		doneHalfLap = false;
	}
	else if (!doneHalfLap && isInside(halfPoint, radius)) {
		doneHalfLap = true;
	}
}

void Car::resetCar() {
	resetPosition();
	laps = 0;
	lives = 5;
}



void set_float4(float f[4], float a, float b, float c, float d) {
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

void color4_to_float4(const struct aiColor4t<ai_real> *c, float f[4]) {
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}


void Car::genVAOsAndUniformBuffer(const aiScene *sc) {
	// For each mesh
	for (unsigned int n = 0; n < sc->mNumMeshes; ++n){
		struct Material aMat;

		auto test = [&]() {
			MyMesh* aMesh = &mesh[objId];
			GLuint buffer;

			const aiMesh* mesh = sc->mMeshes[n];

			// create array with faces
			// have to convert from Assimp format to array
			unsigned int *faceArray;
			faceArray = (unsigned int *)malloc(sizeof(unsigned int) * mesh->mNumFaces * 3);
			unsigned int faceIndex = 0;

			for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
				const aiFace* face = &mesh->mFaces[t];

				memcpy(&faceArray[faceIndex], face->mIndices, 3 * sizeof(unsigned int));
				faceIndex += 3;
			}

			aMesh->numIndexes = sc->mMeshes[n]->mNumFaces * 3;
			aMesh->type = GL_TRIANGLES;

			// generate Vertex Array for mesh
			glGenVertexArrays(1, &(aMesh->vao));
			glBindVertexArray(aMesh->vao);

			// buffer for faces
			glGenBuffers(1, &buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, faceArray, GL_STATIC_DRAW);

			// buffer for vertex positions
			if (mesh->HasPositions()) {
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
				glEnableVertexAttribArray(VSShaderLib::VERTEX_COORD_ATTRIB);
				glVertexAttribPointer(VSShaderLib::VERTEX_COORD_ATTRIB, 3, GL_FLOAT, 0, 0, 0);
			}

			// buffer for vertex normals
			if (mesh->HasNormals()) {
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
				glEnableVertexAttribArray(VSShaderLib::NORMAL_ATTRIB);
				glVertexAttribPointer(VSShaderLib::NORMAL_ATTRIB, 3, GL_FLOAT, 0, 0, 0);
			}

			// buffer for vertex texture coordinates
			if (mesh->HasTextureCoords(0)) {
				float *texCoords = (float *)malloc(sizeof(float) * 2 * mesh->mNumVertices);
				for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {

					texCoords[k * 2] = mesh->mTextureCoords[0][k].x;
					texCoords[k * 2 + 1] = mesh->mTextureCoords[0][k].y;

				}
				glGenBuffers(1, &buffer);
				glBindBuffer(GL_ARRAY_BUFFER, buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
				glEnableVertexAttribArray(VSShaderLib::TEXTURE_COORD_ATTRIB);
				glVertexAttribPointer(VSShaderLib::TEXTURE_COORD_ATTRIB, 2, GL_FLOAT, 0, 0, 0);
			}

			// unbind buffers
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			// create material uniform buffer
			aiMaterial *mtl = sc->mMaterials[mesh->mMaterialIndex];

			/*aiString texPath;
			if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)) {
				//bind texture
				unsigned int texId = textureIdMap[texPath.data];
				aMesh.texIndex = texId;
				aMat.texCount = 1;
			}
			else {*/
			aMat.texCount = 0;
			/*}*/

			float c[4];
			set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
			aiColor4D diffuse;
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
				color4_to_float4(&diffuse, c);
			memcpy(aMat.diffuse, c, sizeof(c));

			set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
			aiColor4D ambient;
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
				color4_to_float4(&ambient, c);
			memcpy(aMat.ambient, c, sizeof(c));

			set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
			aiColor4D specular;
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
				color4_to_float4(&specular, c);
			memcpy(aMat.specular, c, sizeof(c));

			set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
			aiColor4D emission;
			if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
				color4_to_float4(&emission, c);
			memcpy(aMat.emissive, c, sizeof(c));

			float shininess = 0.0;
			unsigned int max;
			aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
			aMat.shininess = shininess;

			
			aMat.transparency = diffuse.a;
					

			// Set the material
			memcpy(aMesh->mat.ambient, aMat.ambient, 4 * sizeof(float));
			memcpy(aMesh->mat.diffuse, aMat.diffuse, 4 * sizeof(float));
			memcpy(aMesh->mat.specular, aMat.specular, 4 * sizeof(float));
			memcpy(aMesh->mat.emissive, aMat.emissive, 4 * sizeof(float));
			aMesh->mat.shininess = aMat.shininess;
			aMesh->mat.transparency = aMat.transparency;
			aMesh->mat.texCount = aMat.texCount;

			GLuint VboId[2];

			glGenBuffers(1, VboId);
			glBindBuffer(GL_UNIFORM_BUFFER, VboId[0]);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(aMat), (void *)(&aMat), GL_STATIC_DRAW);

			::translate(MODEL, 0, 1.0f, 0);
			::rotate(MODEL, -90, 1, 0, 0);
			::scale(MODEL, 1.5, 1.5, 1.5);
		};
		drawObject(aMat, test);
	}
}


bool Car::isInside(vec3 p, float radius) {
	float distance = sqrt(
		pow(p.x - position.x, 2) +
		pow(p.y - position.y, 2) +
		pow(p.z - position.z, 2)
	);

	return distance < radius;
}