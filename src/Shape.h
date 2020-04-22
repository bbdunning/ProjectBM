
#pragma once

#ifndef LAB471_SHAPE_H_INCLUDED
#define LAB471_SHAPE_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/mesh.h>
#include <assimp/types.h>
#include "Texture.h"
#include "Material.h"

class Program;


class Shape
{

public:

	virtual void createShape(aiMesh* inMesh);
	virtual void init();
	void measure();
	virtual void draw(const std::shared_ptr<Program> prog) const;

	glm::vec3 min = glm::vec3(0);
	glm::vec3 max = glm::vec3(0);
	std::string texturePath;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<Material> material;

protected:

	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned int eleBufID = 0;
	unsigned int posBufID = 0;
	unsigned int norBufID = 0;
	unsigned int texBufID = 0;
	unsigned int vaoID = 0;

};

#endif // LAB471_SHAPE_H_INCLUDED
