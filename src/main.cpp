/*
 * Program 2 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

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
	vec3 gMin;

	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> texture2;
	shared_ptr<Texture> texture3;
	shared_ptr<Texture> texture4;


	//animation data
	float moveVelocity = .04;
	float sTheta = 0;
	int m = 1;
	float lightX = .4;
	bool Wflag = false;
	bool Sflag = false;
	bool Aflag = false;
	bool Dflag = false;
	bool Spaceflag = false;
	bool Ctrlflag = false;
	bool Shiftflag = false;

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

	//skybox
	unsigned int skyboxTextureId = 0;
	vector<std::string> faces {           
		"interstellar_rt.tga",           
		"interstellar_lf.tga",           
		"interstellar_up.tga",           
		"interstellar_dn.tga",           
		"interstellar_bk.tga",           
		"interstellar_ft.tga"};

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			Aflag = true;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			Aflag = false;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			Dflag = true;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			Dflag = false;
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			Wflag = true;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			Wflag = false;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			Sflag = true;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			Sflag = false;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			Spaceflag = true;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			Spaceflag = false;
		}
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
			Ctrlflag = true;
		}
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
			Ctrlflag = false;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
			Shiftflag = true;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
			Shiftflag = false;
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
		if (Wflag) {*eye += float(moveVelocity)*u; lookAtOffset += float(moveVelocity)*u;}
		if (Sflag) {*eye -= float(moveVelocity)*u; lookAtOffset -= float(moveVelocity)*u;}
		if (Aflag) {*eye -= float(moveVelocity)*v; lookAtOffset -= float(moveVelocity)*v;}
		if (Dflag) {*eye += float(moveVelocity)*v; lookAtOffset += float(moveVelocity)*v;}
		if (Spaceflag) {*eye += .5f*float(moveVelocity)*(*up); lookAtOffset += .5f * float(moveVelocity)*(*up);}
		if (Ctrlflag) {*eye -= .5f*float(moveVelocity)*(*up); lookAtOffset -= .5f * float(moveVelocity)*(*up);}
		if (Shiftflag) {moveVelocity = .09;}
		if (!Shiftflag) {moveVelocity = .04;}

		return glm::lookAt(*eye, *lookAtPoint + lookAtOffset, *up);
   }

	unsigned int createSky(string dir, vector<string> faces) {   
		unsigned int textureID;   
		glGenTextures(1, &textureID);   
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);   
		int width, height, nrChannels;   
		stbi_set_flip_vertically_on_load(false);   
		for(GLuint i = 0; i < faces.size(); i++) {     
			unsigned char *data =   
			stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);  
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
				glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);       
				glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);       
				glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);       
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
		glUniform3f(prog->getUniform("LightPos"), 0+lightX, 1, 3);
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
		cubeProg->setShaderNames(resourceDirectory + "/shaders/cube_vert.glsl", resourceDirectory + "/shaders/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");

		skyboxTextureId = createSky(resourceDirectory + "/skybox/", faces);
	}

	void initTex(const std::string& resourceDirectory){  
		texture0 = make_shared<Texture>();  
		texture0->setFilename(resourceDirectory + "/textures/crate.jpg");  
		texture0->init();  texture0->setUnit(0);  
		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);  

		texture1 = make_shared<Texture>();  
		texture1->setFilename(resourceDirectory + "/textures/world.jpg");  
		texture1->init();  texture1->setUnit(1);  
		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);  

		texture2 = make_shared<Texture>();  
		texture2->setFilename(resourceDirectory + "/textures/streaks.jpg");  
		texture2->init();  texture2->setUnit(2);  
		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); 

		texture3 = make_shared<Texture>();  
		texture3->setFilename(resourceDirectory + "/textures/glass.jpg");  
		texture3->init();  texture2->setUnit(2);  
		texture3->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); 

		texture4 = make_shared<Texture>();  
		texture4->setFilename(resourceDirectory + "/textures/yellow.jpg");  
		texture4->init();  texture2->setUnit(2);  
		texture4->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); 
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize mesh
		// Load geometry
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
/* 		vector<string> meshes = {"melee/fod/FoD2.0", "melee/Captain_Falcon", "totodile",
			"melee/pikachu", "melee/Gamecube/gamecube", "melee/fod/beam", "melee/fod/platform3",
			"melee/fod/skyring1", "melee/fod/skyring2", "bunny_no_normals",  "sphere",
			"cube"}; */
		vector<string> meshes = {};

		vector<string> textures;

		//for every mesh in the scene
		for (int k = 0; k < meshes.size(); k++) {
        	shared_ptr<GameObject> mesh = make_shared<GameObject>();
			//load in the mesh and make the shape(s)
			bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, 
				(resourceDirectory + "/" + meshes[k] + ".obj").c_str(),
				(resourceDirectory + "/" + meshes[k] + ".mtl").c_str());

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
			(*objectList)[meshes[k]] = mesh;
			}
		}


		//load assimp
		createGameObject(resourceDirectory + "/cube.obj", "cube");
		createGameObject(resourceDirectory + "/totodile.obj", "totodile");
	}

	void createGameObject(string meshPath, string objName) {
		Assimp::Importer importer;
		shared_ptr<Shape> newShape;
        shared_ptr<GameObject> mesh = make_shared<GameObject>();

		const aiScene* scene = importer.ReadFile(
			meshPath, aiProcess_Triangulate | aiProcess_FlipUVs);

		cout << objName + " has: " << scene->mNumMeshes << " meshes" << endl;
		cout << objName + " has: " << scene->mNumMaterials << " materials" << endl;
		cout << objName + " has: " << scene->mNumTextures << " textures" << endl;

		for (int i=0; i< scene->mNumMeshes; i++) {
			newShape = make_shared<Shape>();
			newShape->createShapeFromAssimp(scene->mMeshes[i]);
			newShape->measure();
			newShape->init();
			mesh->shapeList.push_back(newShape);
		}
		(*objectList)[objName] = mesh;
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
		View->pushMatrix();
		Model->pushMatrix();
		//to draw the sky box bind the right shader 
		cubeProg->bind(); 
		//set the projection matrix - can use the same one 
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		//set the depth function to always draw the box! 
		glDisable(GL_DEPTH_TEST);
		//set up view matrix to include your view transforms  
		//(your code likely will be different depending 
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(getViewMatrix(&eye, &lookAtPoint, &up)));
		//set and send model transforms - likely want a bigger cube 
		Model->translate(eye);
		Model->scale(vec3(75,75,75));
		Model->rotate(PI/2, vec3(0,1,0));
		glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()));
		//bind the cube map texture 
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId); 
		(*objectList)["cube"]->draw(cubeProg); 
		//set the depth test back to normal! 
		glEnable(GL_DEPTH_TEST);
		Model->popMatrix();
		cubeProg->unbind();



		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(getViewMatrix(&eye, &lookAtPoint, &up)));

		vec3 vd = lookAtPoint - eye;
		glUniform3f(prog->getUniform("viewDirection"), vd.x, vd.y, vd.z);

		//set initial material and Light
		setLight();

		Model->pushMatrix();
		Model->scale(vec3(2,2,2));

			//draw totodile
			Model->pushMatrix();
				Model->translate(vec3(-.2, -.9, 1));
				Model->scale(vec3(.1, .1, .1));
				setMaterial(m);
				setModel(prog, Model);
				texture3->bind(prog->getUniform("Texture0"));
				(*objectList)["cube"]->draw(prog);
			Model->popMatrix();

			Model->pushMatrix();
				Model->translate(vec3(-.2, -.9, 1));
				Model->scale(vec3(.1, .1, .1));
				setMaterial(m);
				setModel(prog, Model);
				texture1->bind(prog->getUniform("Texture0"));
				(*objectList)["totodile"]->draw(prog);
			Model->popMatrix();

			//Main Stage
			Model->pushMatrix();
				Model->translate(vec3(0, 0, 0));
				Model->scale(vec3(0.2, 0.2, 0.2));
				setMaterial(3);
				texture3->bind(prog->getUniform("Texture0"));
				setModel(prog, Model);
/* 				(*objectList)["melee/fod/FoD2.0"]->draw(prog); */
			Model->popMatrix();

			//Skyring 1
			Model->pushMatrix();
				Model->translate(vec3(-2.4, 2, -2));
				Model->rotate(.1, vec3(1,0,0));
				Model->rotate(2*sin(.2*glfwGetTime()), vec3(0,0,1));
				Model->scale(vec3(0.2, 0.2, 0.2));
				setMaterial(1);
				texture2->bind(prog->getUniform("Texture0"));
				setModel(prog, Model);
/* 				(*objectList)["melee/fod/skyring1"]->draw(prog); */
			Model->popMatrix();
			//Skyring2
			Model->pushMatrix();
				Model->translate(vec3(-2.8, 2.1, -.26));
				Model->rotate(-2*sin(.2*glfwGetTime()), vec3(0,0,1));
				Model->rotate(.3, vec3(1,0,0));
				Model->scale(vec3(0.2, 0.2, 0.2));
				setModel(prog, Model);
/* 				(*objectList)["melee/fod/skyring2"]->draw(prog); */
			Model->popMatrix();


			//draw Captain Falcon
			Model->pushMatrix();
				Model->translate(vec3(0, -.67, 1));
				Model->scale(vec3(0.02, 0.02, 0.02));
				setMaterial(3);
				texture0->bind(prog->getUniform("Texture0"));
				setModel(prog, Model);
/* 				(*objectList)["melee/Captain_Falcon"]->draw(prog); */
			Model->popMatrix();

			//draw Platform
			Model->pushMatrix();
				Model->translate(vec3(0, -.1-sTheta*.3, 0));
				Model->scale(vec3(0.2, 0.2, 0.2));
				setMaterial(3);
				texture3->bind(prog->getUniform("Texture0"));
				setModel(prog, Model);
/* 				(*objectList)["melee/fod/platform3"]->draw(prog); */

				//draw Pikachu
				Model->pushMatrix();
					Model->translate(vec3(4.68, -2.2, -.4));
					Model->scale(vec3(0.007, 0.007, 0.007));
					Model->rotate(PI/2, vec3(0,1,0));
					Model->rotate(sin(glfwGetTime()*2), vec3(1,0,0));
					Model->translate(vec3(-6, -4, 0));
					texture4->bind(prog->getUniform("Texture0"));
					setMaterial(2);
					setModel(prog, Model);
/* 					(*objectList)["melee/pikachu"]->draw(prog); */
				Model->popMatrix();
			Model->popMatrix();

			//draw GameCube
			Model->pushMatrix();
				Model->translate(vec3(0, .3, 5));
				Model->scale(vec3(0.08, 0.08, 0.08));
				Model->rotate(PI, vec3(0,1,0));
				setMaterial(0);
				texture1->bind(prog->getUniform("Texture0"));
				setModel(prog, Model);
/* 				(*objectList)["melee/Gamecube/gamecube"]->draw(prog); */
			Model->popMatrix();
		Model->popMatrix();

		prog->unbind();

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
