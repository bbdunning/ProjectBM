#include <iostream>
#include <cmath>
#include <unordered_map>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "GameObject.h"
#include "Texture.h"
#include "stb_image.h"
#include "InputHandler.h"
#include "Player.h"
#include "Animation/AnimatedShape.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/types.h>
#include <assimp/texture.h>
#include <assimp/postprocess.h>


using namespace std;
using namespace glm;

#define PI 3.14159
#define viewFactor .002

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> cubeProg;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<unordered_map<string, shared_ptr<GameObject>>> objectList =
	  make_shared<unordered_map<string, shared_ptr<GameObject>>>();

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape

	shared_ptr<Texture> texture_glass;

	shared_ptr<InputHandler> inputHandler = make_shared<InputHandler>();
	shared_ptr<Player> player1 = make_shared<Player>();
	
	//animation data
	float moveVelocity = .04;
	float sTheta = 0;
	int m = 1;

	//view angles, from mouse
	float phi = 0;
	float theta = PI;

	float prevX = 0;
	float prevY = 0;

	//movement vectors
	vec3 eye = vec3(0,0,5);
	vec3 lookAtPoint = vec3(
		cos(phi)*cos(theta),
		sin(phi),
		cos(phi)*cos((PI/2.0)-theta));
	vec3 lookAtOffset = vec3(0,0,0);
	vec3 up = vec3(0,1,0);

	vec3 playerLocation = vec3(0, -1.05, -2.1);
	float initialPlayerLocation = -1.05;

	//skybox
	unsigned int skyboxTextureId = 0;
	vector<std::string> faces {           
		"right.jpg",           
		"left.jpg",           
		"top.jpg",           
		"bottom.jpg",           
		"front.jpg",           
		"back.jpg"};


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		inputHandler->setKeyFlags(key, action);

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			m = (m+1) % 5;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		bool lbutton_down;

		if (action == GLFW_PRESS or action == GLFW_REPEAT)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void setViewAngles(GLFWwindow *window) {
		double posX, posY;
		glfwGetCursorPos(window, &posX, &posY);

		theta += viewFactor *(posX - prevX);
		phi -= (viewFactor * (posY - prevY));
		phi = fmax(phi, -PI/2 + 0.2);
		phi = fmin(phi, PI/2 - 0.2);

		prevX = posX;
		prevY = posY;
	}

	mat4 getViewMatrix(vec3 *eye, vec3 *lookAtPoint, vec3 *up) {
		int radius = 50;
		int step = .5;


		*lookAtPoint = vec3(
			radius*cos(phi)*cos(theta),
			radius*sin(phi),
			radius*cos(phi)*cos((PI/2.0)-theta));

		vec3 u = normalize((*lookAtPoint+lookAtOffset) - *eye);
		vec3 v = cross(u, *up);

		//move Eye + LookAtOffset
		if (inputHandler->Wflag) {*eye += float(moveVelocity)*u; lookAtOffset += float(moveVelocity)*u;}
		if (inputHandler->Sflag) {*eye -= float(moveVelocity)*u; lookAtOffset -= float(moveVelocity)*u;}
		if (inputHandler->Aflag) {*eye -= float(moveVelocity)*v; lookAtOffset -= float(moveVelocity)*v;}
		if (inputHandler->Dflag) {*eye += float(moveVelocity)*v; lookAtOffset += float(moveVelocity)*v;}
		if (inputHandler->Shiftflag) {*eye += .5f*float(moveVelocity)*(*up); lookAtOffset += .5f * float(moveVelocity)*(*up);}
		if (inputHandler->Ctrlflag) {*eye -= .5f*float(moveVelocity)*(*up); lookAtOffset -= .5f * float(moveVelocity)*(*up);}
		if (inputHandler->Shiftflag) {moveVelocity = .09;}
		if (!inputHandler->Shiftflag) {moveVelocity = .04;}

		return glm::lookAt(*eye, *lookAtPoint + lookAtOffset, *up);
   }

	unsigned int createSky(string dir, vector<string> faces) {   
		unsigned int textureID;   
		glGenTextures(1, &textureID);   
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);   
		int width, height, nrChannels;   
		stbi_set_flip_vertically_on_load(false);   
		for(GLuint i = 0; i < faces.size(); i++) {     
			unsigned char *data = stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);  
			if (data) {          
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,               
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);  
			} else {    
				cout << "failed to load: " << (dir+faces[i]).c_str() << endl;  
			}   
		}   
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);   
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);   
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);     
		cout << " creating cube map any errors : " << glGetError() << endl;   
		return textureID; 
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void setMaterial(int i) {  
		switch (i) {    
			case 0:
				 glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);        
				 glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);       
				 glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);       
				 glUniform1f(prog->getUniform("shine"), 120.0);
			break;    
			case 1: // 
				glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.13);       
				glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);       
				glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.3);       
				glUniform1f(prog->getUniform("shine"), 4.0);
			break;    
			case 2: //pikachu
			 	glUniform3f(prog->getUniform("MatAmb"), 0.1294, 0.0235, 0.02745);        
				glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);       
				glUniform3f(prog->getUniform("MatSpec"), 0.3922, 0.341176, 0.30784);       
				glUniform1f(prog->getUniform("shine"), 20);
			break;  
			case 3: //stage
			 	glUniform3f(prog->getUniform("MatAmb"), 0.00745, 0.1175, 0.3175);        
				glUniform3f(prog->getUniform("MatDif"), 0.61424, 0.04136, 0.04136);       
				glUniform3f(prog->getUniform("MatSpec"), 0.727811, 0.626959, 0.626959);       
				glUniform1f(prog->getUniform("shine"), 100);
			break;
			case 4:
			 	glUniform3f(prog->getUniform("MatAmb"), 0.1, 0.1, 0.1);        
				glUniform3f(prog->getUniform("MatDif"), 0.5, 0.5, 0.5);       
				glUniform3f(prog->getUniform("MatSpec"), .8, 0.8, 0.8);       
				glUniform1f(prog->getUniform("shine"), 100);
			break;
		}
	}

	void setLight() {
		glUniform3f(prog->getUniform("LightPos"), .3, 3, 3);
		glUniform3f(prog->getUniform("LightCol"), 1, 1, 1); 
	}


	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shaders/simple_vert.glsl", resourceDirectory + "/shaders/blinn-phong.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
			prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		prog->addUniform("viewDirection");
		prog->addUniform("LightPos");
		prog->addUniform("LightCol");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("Texture0");

		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames( resourceDirectory + "/shaders/cube_vert.glsl", resourceDirectory + "/shaders/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");

		inputHandler->init();
		player1->init(inputHandler);
		skyboxTextureId = createSky(resourceDirectory + "/skybox/", faces);
	}

	void initTex(const std::string& resourceDirectory){  
		texture_glass = make_shared<Texture>();  
		texture_glass->setFilename(resourceDirectory + "/textures/glass.jpg");  
		texture_glass->init();  
		texture_glass->setUnit(2);  
		texture_glass->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); 
	}

	void initGeom(const std::string& resourceDirectory)
	{
		string rDir = resourceDirectory + "/";
		// Initialize mesh
		// Load geometry

		//create objects
		createGameObject(rDir, "cube.obj", "cube");
		createGameObject(rDir + "melee/totodile/", "toto.dae", "totodile");
		createGameObject(rDir + "melee/fod/", "fountain.fbx", "FoD");
		createGameObject(rDir + "melee/fod/", "skyring1.fbx", "skyring1");
		createGameObject(rDir + "melee/fod/", "skyring2.fbx", "skyring2");
/* 		createGameObject(rDir + "melee/", "pikachu.obj", "pikachu"); */
//		createGameObject("/home/bbdunning/Desktop/Wii - Super Smash Bros Brawl - Captain Falcon/", "falcon.fbx", "falcon");
		createGameObject(rDir + "melee/falcon2/", "Captain Falcon.dae", "falcon");
		createGameObject(rDir + "anim/", "model.dae", "animModel");
	}

	void createGameObject(string meshPath, string fileName, string objName) {
		Assimp::Importer importer;
		shared_ptr<AnimatedShape> newShape;
		aiString* texPath;
        shared_ptr<GameObject> mesh = make_shared<GameObject>();
		mesh->name = objName;

		const aiScene* scene = importer.ReadFile(
			meshPath + fileName, aiProcess_Triangulate | aiProcess_FlipUVs);

		cout << "creating " << objName << endl;
		cout << "   " << objName + " has: " << scene->mNumMeshes << " meshes" << endl;
		cout << "   " << objName + " has: " << scene->mNumMaterials << " materials" << endl;
		cout << "   " << objName + " has: " << scene->mNumTextures << " textures" << endl;
		cout << "   " << objName + " has: " << scene->mNumAnimations << " animations" << endl;

		for (int i=0; i< scene->mNumMeshes; i++) {
			texPath = new aiString();
			newShape = make_shared<AnimatedShape>();
			newShape->createShapeFromAssimp(scene->mMeshes[i]);
			newShape->measure();
			newShape->init();
			mesh->shapeList.push_back(newShape);
			string temp;

			//load texture path into texPath
			scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, texPath);
			temp = texPath->C_Str();

			//if mesh has texture, create a texture from it
			if (texPath->C_Str() != "" and texPath->C_Str() != "/" and texPath->length != 0 and 
				((texPath->length > 5) and (temp != "none"))) {
				cout << "   " << objName << " has texture. Texture path: " << texPath->C_Str() << endl;
				if (texPath->C_Str()[0] != '/') {
					newShape->texture = createTexture(meshPath + texPath->C_Str());
				} else
					newShape->texture = createTexture(texPath->C_Str());
			}
		}

		(*objectList)[objName] = mesh;
		cout << "created " << objName << endl << endl;;
	}

	shared_ptr<Texture> createTexture(string texturePath) {
		shared_ptr<Texture> tex = make_shared<Texture>();  
		tex->setFilename(texturePath);  
		tex->init();  tex->setUnit(1);  
		tex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);  
		return tex;
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
	}

	void drawSkybox(shared_ptr<MatrixStack> Model, shared_ptr<MatrixStack> Projection) {
		cubeProg->bind(); 
		glDisable(GL_DEPTH_TEST);
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(cubeProg->getUniform("V"), 1, GL_FALSE, value_ptr(getViewMatrix(&eye, &lookAtPoint, &up)));
		Model->pushMatrix();
		Model->translate(eye); //move to center around eye
		Model->scale(vec3(75,75,75));
		glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()));
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId); 
		(*objectList)["cube"]->draw(cubeProg); 
		glEnable(GL_DEPTH_TEST);
		Model->popMatrix();
		cubeProg->unbind();
	}

	bool hitSpherePlane(float c, float r, float A, float B, float C, float D) {
		return 0 < A*v.x + B*v.y + C*v.z + D;
	}

	/*
	/ m - P*V (perspective matrix * view matrix)
	/ v - position of model to be checked against the frustum
	*/
	bool checkInFrustum(const mat4 m, vec4 v) {
		vec4 v2 = m*v; 
		bool l, r, b, t, n = 1, f = 1;

		//check against left plane
		float A = m[0][3] + m[0][0];
		float B = m[1][3] + m[1][0];
		float C = m[2][3] + m[2][0];
		float D = m[3][3] + m[3][0];
		l = (0 < A*v.x + B*v.y + C*v.z + D);

		//check against right plane
		A = m[0][3] - m[0][0];
		B = m[1][3] - m[1][0];
		C = m[2][3] - m[2][0];
		D = m[3][3] - m[3][0];
		r = (0 < A*v.x + B*v.y + C*v.z + D);

		//bottom
		A = m[0][3] + m[0][1];
		B = m[1][3] + m[1][1];
		C = m[2][3] + m[2][1];
		D = m[3][3] + m[3][1];
		b = (0 < A*v.x + B*v.y + C*v.z + D);

		//top
		A = m[0][3] - m[0][1];
		B = m[1][3] - m[1][1];
		C = m[2][3] - m[2][1];
		D = m[3][3] - m[3][1];
		t = (0 < A*v.x + B*v.y + C*v.z + D);

		//near
		A = m[0][3] + m[0][2];
		B = m[1][3] + m[1][2];
		C = m[2][3] + m[2][2];
		D = m[3][3] + m[3][2];
		n = (0 < A*v.x + B*v.y + C*v.z + D);

		//far
		A = m[0][3] - m[0][2];
		B = m[1][3] - m[1][2];
		C = m[2][3] - m[2][2];
		D = m[3][3] - m[3][2];
		f = (0 < A*v.x + B*v.y + C*v.z + D);

		return l && r && b && t && n && f;
	}


	void render() {
		// Get current frame buffer size.
		int width, height;

		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = width/(float)height;

		// Create the matrix stacks
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		drawSkybox(Model, Projection);

		/* bind standard program */
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(getViewMatrix(&eye, &lookAtPoint, &up)));

		vec3 vd = lookAtPoint - eye;
		glUniform3f(prog->getUniform("viewDirection"), vd.x, vd.y, vd.z);

		//set initial material and Light
		setLight();


		//draw player
		Model->pushMatrix();
			player1->update();
			Model->translate(player1->location);
			Model->rotate(-PI/2, vec3(1,0,0));
			checkInFrustum(Projection->topMatrix()*getViewMatrix(&eye, &lookAtPoint, &up), vec4(player1->location, 1));
			// checkInFrustum(Projection->topMatrix(), vec4(player1->location, 1));
			setMaterial(m);
			setModel(prog, Model);
			(*objectList)["animModel"]->draw(prog); 
		Model->popMatrix();

		Model->pushMatrix();
			player1->update();
			Model->translate(player1->location);
			Model->scale(vec3(.35, .75, .35));
			setMaterial(m);
			setModel(prog, Model);
			(*objectList)["cube"]->draw(prog); 
		Model->popMatrix();

		//draw totodile
		Model->pushMatrix();
			Model->translate(vec3(1.1, -.39, -2.1));
			Model->scale(vec3(.02, .02, .02));
			setMaterial(m);
			setModel(prog, Model);
			(*objectList)["totodile"]->draw(prog);
		Model->popMatrix();

		//Main Stage
		Model->pushMatrix();
			Model->translate(vec3(0, -1, -2));
			Model->scale(vec3(0.03, 0.03, 0.03));
			setMaterial(3);
			setModel(prog, Model);
			(*objectList)["FoD"]->draw(prog);
		Model->popMatrix();

		//Skyring 1
		Model->pushMatrix();
			Model->translate(vec3(-2.4, 2, -2));
			Model->rotate(.1, vec3(1,0,0));
			Model->rotate(2*sin(.2*glfwGetTime()), vec3(0,0,1));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setMaterial(1);
			setModel(prog, Model);
			(*objectList)["skyring1"]->draw(prog);
		Model->popMatrix();
		//Skyring2
		Model->pushMatrix();
			Model->translate(vec3(-2.8, 2.1, -.26));
			Model->rotate(-2*sin(.2*glfwGetTime()), vec3(0,0,1));
			Model->rotate(.3, vec3(1,0,0));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setModel(prog, Model);
			(*objectList)["skyring2"]->draw(prog);
		Model->popMatrix();


		//draw Captain Falcon
		Model->pushMatrix();
			// getPlayerDisplacement();
			Model->translate(playerLocation);
			Model->rotate(-PI/2, vec3(1, 0, 0));
			Model->rotate(-PI/2, vec3(0, 0, 1));
			Model->scale(vec3(0.035, 0.035, 0.035));
			setMaterial(1);
			setModel(prog, Model);
				(*objectList)["falcon"]->draw(prog);
		Model->popMatrix();

		//draw Platform
		Model->pushMatrix();
			Model->translate(vec3(0, -.1-sTheta*.3, 0));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setMaterial(3);
			texture_glass->bind(prog->getUniform("Texture0"));
			setModel(prog, Model);
/* 				(*objectList)["melee/fod/platform3"]->draw(prog); */

			//draw Pikachu
			Model->pushMatrix();
				Model->translate(vec3(4.68, -2.2, -.4));
				Model->scale(vec3(0.007, 0.007, 0.007));
				Model->rotate(PI/2, vec3(0,1,0));
				Model->rotate(sin(glfwGetTime()*2), vec3(1,0,0));
				Model->translate(vec3(-6, -4, 0));
				setMaterial(2);
				setModel(prog, Model);
/* 					(*objectList)["pikachu"]->draw(prog); */
			Model->popMatrix();
		Model->popMatrix();

		//draw GameCube
		Model->pushMatrix();
			Model->translate(vec3(0, .3, 5));
			Model->scale(vec3(0.08, 0.08, 0.08));
			Model->rotate(PI, vec3(0,1,0));
			setMaterial(0);
			setModel(prog, Model);
/* 				(*objectList)["melee/Gamecube/gamecube"]->draw(prog); */
		Model->popMatrix();

		prog->unbind();

		//animation update example
		sTheta = sin(glfwGetTime());

		// Pop matrix stacks.
		Projection->popMatrix();
	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
		resourceDir = argv[1];

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
//	windowManager->init(640, 480);
	windowManager->init(1280, 960);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initTex(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		application->setViewAngles(windowManager->getHandle());
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
