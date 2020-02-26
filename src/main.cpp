/*
 * Program 2 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <unordered_map>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Mesh.h"
#include "Texture.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<unordered_map<string, shared_ptr<Mesh>>> meshList =
	  make_shared<unordered_map<string, shared_ptr<Mesh>>>();

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> texture2;


	//animation data
	float sTheta = 0;
	int m = 1;
	float gTrans = 0;
	float gZoom = 0;
	float lightX = 0;
	bool RPress= false;

	//view angles, from mouse
	float phi = 0;
	float theta = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_A ) {
			gTrans -= 0.2;
		}
		if (key == GLFW_KEY_D ) {
			gTrans += 0.2;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
			RPress = !RPress;
		}
		if (key == GLFW_KEY_W) {
			gZoom += 0.2;
		}
		if (key == GLFW_KEY_S) {
			gZoom -= 0.2;
		}
		if (key == GLFW_KEY_Q ) {
			lightX -= 0.3;
		}
		if (key == GLFW_KEY_E ) {
			lightX += 0.3;
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			m = (m+1) % 5;
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
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		glfwGetCursorPos(window, &posX, &posY);
		theta = (posX/width * 3.14159);
		phi = clamp(-posY/height * 3.14159, -3.14159 *.5, .5 * 3.14159);
	}

	mat4 getViewMatrix() {
		int radius = 100;
		vec3 eye = vec3(0,0,0);
		vec3 lookAtPoint = vec3(
			radius*cos(phi)*cos(theta),
			radius*sin(phi),
			radius*cos(phi)*cos((3.14159/2.0)-theta));
		vec3 up = vec3(0,1,0);


		return glm::lookAt(eye, lookAtPoint, up);
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
				glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);       
				glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);       
				glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);       
				glUniform1f(prog->getUniform("shine"), 4.0);
			break;    
			case 2: //
			 	glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);        
				glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);       
				glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);       
				glUniform1f(prog->getUniform("shine"), 27.9);
			break;  
			case 3:
			 	glUniform3f(prog->getUniform("MatAmb"), 0.1745, 0.01175, 0.01175);        
				glUniform3f(prog->getUniform("MatDif"), 0.61424, 0.04136, 0.04136);       
				glUniform3f(prog->getUniform("MatSpec"), 0.727811, 0.626959, 0.626959);       
				glUniform1f(prog->getUniform("shine"), 76.8);
			break;
			case 4:
			 	glUniform3f(prog->getUniform("MatAmb"), 0.05, 0.05, 0.05);        
				glUniform3f(prog->getUniform("MatDif"), 0.5, 0.5, 0.5);       
				glUniform3f(prog->getUniform("MatSpec"), 0.7, 0.7, 0.7);       
				glUniform1f(prog->getUniform("shine"), 10);
			break;
		}
	}

	void setLight() {
		glUniform3f(prog->getUniform("LightPos"), 0+lightX, 0, 3);
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
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/blinn-phong.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
			prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		prog->addUniform("CameraPos");
		prog->addUniform("LightPos");
		prog->addUniform("LightCol");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("Texture0");

	}

	void initTex(const std::string& resourceDirectory){  
		texture0 = make_shared<Texture>();  
		texture0->setFilename(resourceDirectory + "/crate.jpg");  
		texture0->init();  texture0->setUnit(0);  
		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);  

		texture1 = make_shared<Texture>();  
		texture1->setFilename(resourceDirectory + "/world.jpg");  
		texture1->init();  texture1->setUnit(1);  
		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);  

		texture2 = make_shared<Texture>();  
/* 		texture2->setFilename(resourceDirectory + "/melee/Charizard/charizard.png");   */
		texture2->setFilename(resourceDirectory + "/broken_scale.jpg");  
		texture2->init();  texture2->setUnit(2);  
		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); 
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		vector<string> meshes = {"melee/fod/FoD", "melee/Captain_Falcon", "melee/Fox", 
			"melee/pikachu", "melee/Charizard/charizard", "melee/Gamecube/gamecube", "melee/fod/beam", "melee/fod/platform3",
			"melee/fod/skyring1", "melee/fod/skyring2", "bunny_no_normals", "melee/Totodile/totodile", "sphere"};

		//for every mesh in the scene
		for (int k = 0; k < meshes.size(); k++) {
        	shared_ptr<Mesh> mesh = make_shared<Mesh>();
			//load in the mesh and make the shape(s)
			bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/" + meshes[k] + ".obj").c_str());
			if (!rc) {
				cerr << errStr << endl;
			} else {
				//for every shape in a mesh
				for (int i = 0; i < TOshapes.size(); i++) {
					shared_ptr<Shape> shape = make_shared<Shape>();
					shape->createShape(TOshapes[i]);
					shape->measure();
					shape->init();

					mesh->shapeList.push_back(shape);
				}
			(*meshList)[meshes[k]] = mesh;
			}
		}
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }


	void render() {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		// View is global translation along negative z for now
/* 		setViewAngles(window); */
		vec3 cameraPos = vec3(0,0,-5+gZoom);
		View->pushMatrix();
			View->loadIdentity();
			View->translate(cameraPos);
			View->rotate(fmod(gTrans,3.14159*2), vec3(0,1,0));
			if (RPress) {
				View->rotate((sin(glfwGetTime()*3)), vec3(0,1,0)); // circle around
			}

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
/* 		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix())); */
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(getViewMatrix()));

		//set initial material and Light
		setMaterial(4);
		setLight();

		Model->pushMatrix();
 			Model->translate(vec3(-.2, -.9, 1));
			Model->scale(vec3(1, 1, 1));
			setModel(prog, Model);
			texture0->bind(prog->getUniform("Texture0"));
			(*meshList)["bunny_no_normals"]->draw(prog);
		Model->popMatrix();

		//Main Stage
		Model->pushMatrix();
			Model->translate(vec3(0, 0, 0));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setMaterial(m);
			setModel(prog, Model);
			(*meshList)["melee/fod/FoD"]->draw(prog);
		Model->popMatrix();

		//Skyring 1
		Model->pushMatrix();
			Model->translate(vec3(-2, 2, 0));
			Model->rotate(.1, vec3(1,0,0));
			Model->rotate(2*sin(.1*glfwGetTime()), vec3(0,0,1));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setMaterial(0);
			setModel(prog, Model);
			(*meshList)["melee/fod/skyring1"]->draw(prog);
		Model->popMatrix();
		//Skyring2
		Model->pushMatrix();
			Model->translate(vec3(-2.4, 2.1, -.6));
			Model->rotate(-2*sin(.1*glfwGetTime()), vec3(0,0,1));
			Model->rotate(.3, vec3(1,0,0));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setModel(prog, Model);
			(*meshList)["melee/fod/skyring2"]->draw(prog);
		Model->popMatrix();


		//draw Captain Falcon
		Model->pushMatrix();
			Model->translate(vec3(0, -.58, 1));
			Model->scale(vec3(0.03, 0.03, 0.03));
			setMaterial(3);
			setModel(prog, Model);
			(*meshList)["melee/Captain_Falcon"]->draw(prog);
		Model->popMatrix();

		//draw Platform
		Model->pushMatrix();
			Model->translate(vec3(0, -.1-sTheta*.3, 0));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setMaterial(1);
			setModel(prog, Model);
			(*meshList)["melee/fod/platform3"]->draw(prog);


			//draw Pikachu
			Model->pushMatrix();
				Model->translate(vec3(4.68, -2.2, -.4));
				Model->scale(vec3(0.01, 0.01, 0.01));
				Model->rotate(3.14159/2, vec3(0,1,0));
				Model->rotate(sin(glfwGetTime()*2), vec3(1,0,0));
				Model->translate(vec3(-6, -4, 0));
				setMaterial(2);
				setModel(prog, Model);
				(*meshList)["melee/pikachu"]->draw(prog);
			Model->popMatrix();
		Model->popMatrix();

		//draw GameCube
		Model->pushMatrix();
			Model->translate(vec3(0, .3, 5));
			Model->scale(vec3(0.08, 0.08, 0.08));
			Model->rotate(3.14159, vec3(0,1,0));
			setMaterial(0);
			setModel(prog, Model);
			(*meshList)["melee/Gamecube/gamecube"]->draw(prog);
		Model->popMatrix();

		prog->unbind();
/* 		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(prog->getUniform("CameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

		//set initial material and Light
		setMaterial(4);
		setLight();

		//draw GameCube
		Model->pushMatrix();
			Model->translate(vec3(0,-.5,0));
			Model->scale(vec3(0.08,.08,0.08));
			setMaterial(0);
			texture1->bind(prog->getUniform("Texture0")); 
			setModel(prog, Model);
			(*meshList)["melee/Gamecube/gamecube"]->draw(prog);
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(1.5,-.5,0));
			Model->scale(vec3(0.08,.08,0.08));
			setMaterial(2);
			texture0->bind(prog->getUniform("Texture0")); 
			setModel(prog, Model);
			(*meshList)["melee/Totodile/totodile"]->draw(prog);
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(-1.5,-.5,0));
			Model->scale(vec3(0.08,.08,0.08));
			setMaterial(2);
			texture2->bind(prog->getUniform("Texture0")); 
			setModel(prog, Model);
			(*meshList)["melee/Charizard/charizard"]->draw(prog);
		Model->popMatrix();

		prog->unbind(); */

		//animation update example
		sTheta = sin(glfwGetTime());

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();
	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
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
