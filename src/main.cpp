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
#include "Camera.h"
#include "Totodile.h"

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

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> cubeProg;

	// Shape to be used (from  file) - modify to support multiple
	unordered_map<string, shared_ptr<GameObject>> objL;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	shared_ptr<Texture> texture_glass;

	shared_ptr<InputHandler> inputHandler = make_shared<InputHandler>();
	shared_ptr<Player> player1 = make_shared<Player>();
	std::vector<std::shared_ptr<Shape>> totoMesh;
	vector<shared_ptr<Totodile>> collectables;
	
	//animation data
	float sTheta = 0;
	int m = 1;
	float c = 0;
	int catchCount = 0;
	float secondCount = 2;

	//movement vectors
	Camera camera;

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
			glfwSetWindowShouldClose(window, GL_TRUE);
		if (key == GLFW_KEY_M && action == GLFW_PRESS)
			m = (m+1) % 5;
		if (key == GLFW_KEY_Z && action == GLFW_PRESS)
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		bool lbutton_down;

		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
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
			 	glUniform3f(prog->getUniform("MatAmb"), 0.15, 0.15, 0.15);        
				glUniform3f(prog->getUniform("MatDif"), 0.5, 0.5, 0.5);       
				glUniform3f(prog->getUniform("MatSpec"), .8, 0.8, 0.8);       
				glUniform1f(prog->getUniform("shine"), 80);
			break;
		}
	}

	void setLight() {
		glUniform3f(prog->getUniform("LightPos"), 50, 20, 3);
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
		camera.init();
		camera.setInputHandler(inputHandler);
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
		totoMesh = createGameObject(rDir + "melee/totodile/", "toto.dae", "totodile");
		createGameObject(rDir + "melee/fod/", "fountain.fbx", "FoD");
		createGameObject(rDir + "melee/fod/", "skyring1.fbx", "skyring1");
		createGameObject(rDir + "melee/fod/", "skyring2.fbx", "skyring2");
		createGameObject(rDir + "terrain/", "moon.fbx", "moon");
		createGameObject(rDir + "arms/source/", "arms1.fbx", "arms");
		createGameObject(rDir + "melee/falcon2/", "Captain Falcon.dae", "falcon");
		// createGameObject(rDir + "anim/", "model.dae", "animModel");
	}

	std::vector<std::shared_ptr<Shape>> createGameObject(string meshPath, string fileName, string objName) {
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
			if (texPath->C_Str() != "" && (texPath->C_Str() != "/" || texPath->C_Str() != "\\") && texPath->length != 0 && 
				((texPath->length > 5) && (temp != "none"))) {
				cout << "   " << objName << " has texture. Texture path: " << texPath->C_Str() << endl;
				if (texPath->C_Str()[0] != '/') {
					newShape->texture = createTexture(meshPath + texPath->C_Str());
				} else
					newShape->texture = createTexture(texPath->C_Str());
			}
		}

		objL[objName] = mesh;
		cout << "created " << objName << endl << endl;
		return mesh->shapeList;
	}

	shared_ptr<Texture> createTexture(string texturePath) {
		shared_ptr<Texture> tex = make_shared<Texture>();  
		tex->setFilename(texturePath);  
		tex->init();  tex->setUnit(1);  
		tex->setWrapModes(GL_REPEAT, GL_REPEAT);  
		return tex;
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
	}

	void drawSkybox(shared_ptr<MatrixStack> Model, shared_ptr<MatrixStack> Projection) {
		cubeProg->bind(); 
		glDisable(GL_DEPTH_TEST);
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(cubeProg->getUniform("V"), 1, GL_FALSE, value_ptr(camera.getViewMatrix()));
		Model->pushMatrix();
		Model->translate(camera.eye); //move to center around eye
		Model->scale(vec3(75,75,75));
		glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()));
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId); 
		objL["cube"]->draw(cubeProg); 
		glEnable(GL_DEPTH_TEST);
		Model->popMatrix();
		cubeProg->unbind();
	}

	// bool hitSpherePlane(float c, float r, float A, float B, float C, float D) {
	// 	return 0 < A*v.x + B*v.y + C*v.z + D;
	// }

	/*
	/ m - P*V (perspective matrix * view matrix)
	/ v - position of model to be checked against the frustum
	*/


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
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(camera.getViewMatrix()));

		vec3 vd = camera.lookAtPoint - camera.eye;
		glUniform3f(prog->getUniform("viewDirection"), vd.x, vd.y, vd.z);

		//set initial material and Light
		setLight();

		// camera.eye = vec3(player1->location.x, player1->location.y + .5, player1->location.z);


		//draw terrain
		Model->pushMatrix();
			Model->translate(vec3(0,-1.11,0));
			Model->scale(vec3(50,50,50));
			Model->rotate(-PI/2, vec3(1,0,0));
			setMaterial(1);
			setModel(prog, Model);
			objL["moon"]->draw(prog); 
		Model->popMatrix();

		//draw totodile
		player1->update(&camera);
		Model->pushMatrix();
			Model->translate(player1->location);
			// Model->scale(vec3(.35, .75, .35));
			if (!player1->isGrounded)
			{
				if (!player1->facingRight) {
					Model->rotate(-PI/2, vec3(0,1,0));
					Model->rotate(PI/8, vec3(1,0,0));
				}
				else {
					Model->rotate(PI/2, vec3(0,1,0));
					Model->rotate(PI/8, vec3(1,0,0));
				}
			}
			//facing left
			else if (!player1->facingRight) {
				if (abs(player1->velocity.x <= -.04f) || inputHandler->Downflag){
					Model->rotate(PI/4, vec3(0,0,1));
					Model->rotate(.5*sin(glfwGetTime()*12), vec3(1,0,0));
				}
				else if (abs(player1->velocity.x <= -.01f)) {
					Model->rotate(.5*sin(glfwGetTime()*10), vec3(1,0,0));
				}
				Model->rotate(-PI/2, vec3(0,1,0));
			}
			//facing right
			else {
				if (abs(player1->velocity.x >= .04f) || inputHandler->Downflag) {
					Model->rotate(-PI/4, vec3(0,0,1));
					Model->rotate(.5*sin(glfwGetTime()*12), vec3(1,0,0));
				}
				else if (abs(player1->velocity.x >= .01f)) {
					Model->rotate(.5*sin(glfwGetTime()*10), vec3(1,0,0));
				}
				Model->rotate(PI/2, vec3(0,1,0));
			}
			if (player1->standing)
				setMaterial(0);
			else if (player1->isGrounded)
				setMaterial(1);
			else if (!player1->isGrounded)
				setMaterial(3);

			Model->scale(vec3(.025, .025, .025));
			setModel(prog, Model);
			objL["totodile"]->draw(prog); 
		Model->popMatrix();

		// if (fmod(glfwGetTime(), 2.0) == 0.0) {
		setMaterial(4);
		if (glfwGetTime() > secondCount || inputHandler->Ctrlflag) {
		// if (inputHandler->Ctrlflag) {
			collectables.push_back(createTotodile());
			secondCount += 3;
		}

		auto it = collectables.begin();
		while (it != collectables.end()) {
			(*it)->update();
			(*it)->draw(prog);
			if (1.5 > distance((*it)->location, camera.eye)) {
				catchCount++;
				cout << "Catch count: " << catchCount << endl;
				it = collectables.erase(it);
			}
			else if ((abs((*it)->location.x) > 50) || (abs((*it)->location.z) > 50)) {
				(*it)->velocity = -(*it)->velocity;
				it++;
			}
			else {
				auto it2 = collectables.begin();
				while (it2 != collectables.end()) {
					if ((it != it2) && (.7 > distance((*it)->location, (*it2)->location)))
						// (*it)->velocity = -(*it)->velocity;
						// (*it)->velocity = .05f * normalize(vec3(rand()%5, 0, rand()%5));
						(*it)->velocity = .05f * normalize((*it)->location - (*it2)->location);
					it2++;
				}
				++it;
			}
		}

		prog->unbind();
		// cout << glfwGetTime() << endl;

		//animation update example
		sTheta = sin(glfwGetTime());

		// Pop matrix stacks.
		Projection->popMatrix();
	}

	shared_ptr<Totodile> createTotodile() {
		shared_ptr<Totodile> t = make_shared<Totodile>();
		t->init(totoMesh);
		t->location = vec3(((rand()%40)-20),-1,((rand()%40)-20));
		c += 1;
		return t;
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
	windowManager->init(1280, 900);
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
		application->camera.setViewAngles(windowManager->getHandle());
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
