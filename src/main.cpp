#include <iostream>
#include <cmath>
#include <unordered_map>
#include <glad/glad.h>
#include <memory>

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
#include "Animation/Animator.h"
#include "GameObjects/Platform.h"
#include "Camera.h"
#include "Hitbox.h"
#include "CollisionDetector.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
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

	//create GLFW Window
	WindowManager * windowManager = nullptr;

	// create shaders
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> cubeProg;
	std::shared_ptr<Program> animProg;

	// Shape to be used (from  file) - modify to support multiple
	unordered_map<string, shared_ptr<GameObject>> objL;
	map<string, shared_ptr<GameObject>> platforms;
	vector<HitSphere> playerHitboxes;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	shared_ptr<Texture> texture_glass;

	shared_ptr<InputHandler> inputHandler = make_shared<InputHandler>();
	shared_ptr<CollisionDetector> cd = make_shared<CollisionDetector>();
	shared_ptr<Player> player1 = make_shared<Player>();
	shared_ptr<Sandbag> sandbag = make_shared<Sandbag>();
	
	//animation data
	float sTheta = 0;
	int m = 1;
	float prevOmega = 0.0f;

	//Camera
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

		if (action == GLFW_PRESS or action == GLFW_REPEAT)
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


	void setLight(shared_ptr<Program> prog) {
		// glUniform3f(prog->getUniform("LightPos"), .5, .5, 0);
		glUniform3f(prog->getUniform("LightPos"), 5, 3, -.4);
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

		animProg = make_shared<Program>();
		animProg->setVerbose(true);
		animProg->setShaderNames(resourceDirectory + "/shaders/anim_vert.glsl", resourceDirectory + "/shaders/blinn-phong.glsl");
		animProg->init();
		animProg->addUniform("P");
		animProg->addUniform("V");
		animProg->addUniform("M");
		animProg->addAttribute("vertPos");
		animProg->addAttribute("vertNor");
		animProg->addAttribute("vertTex");		
		animProg->addAttribute("jointIndices");		
		animProg->addAttribute("jointWeights");		

		animProg->addUniform("viewDirection");
		animProg->addUniform("LightPos");
		animProg->addUniform("LightCol");
		animProg->addUniform("MatAmb");
		animProg->addUniform("MatDif");
		animProg->addUniform("MatSpec");
		animProg->addUniform("shine");
		animProg->addUniform("Texture0");
		animProg->addUniform("jointTransforms");

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
		player1->cd = cd;
		sandbag->init(inputHandler);
		sandbag->cd = cd;
		skyboxTextureId = createSky(resourceDirectory + "/skybox/", faces);
	}

	void drawSkybox(shared_ptr<MatrixStack> Model, shared_ptr<MatrixStack> Projection) {
		cubeProg->bind(); 
		glDisable(GL_DEPTH_TEST);
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(cubeProg->getUniform("V"), 1, GL_FALSE, value_ptr(camera.getViewMatrix()));
		Model->pushMatrix();
		Model->translate(camera.eye); //move to center around eye
		Model->scale(vec3(100,100,100));
		glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()));
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureId); 
		objL["cube"]->draw(cubeProg); 
		glEnable(GL_DEPTH_TEST);
		Model->popMatrix();
		cubeProg->unbind();
	}

	void initGeom(const std::string& resourceDirectory)
	{
		string rDir = resourceDirectory + "/";

		//load geometry, initialize meshes, create objects
		objL["cube"] = GameObject::create(rDir, "cube.obj", "cube");
		objL["totodile"] = GameObject::create(rDir + "melee/totodile/", "toto.dae", "totodile");
		objL["FoD"] = GameObject::create(rDir + "melee/fod/", "fountain.fbx", "FoD");
		objL["skyring1"] = GameObject::create(rDir + "melee/fod/", "skyring1.fbx", "skyring1");
		objL["skyring2"] = GameObject::create(rDir + "melee/fod/", "skyring2.fbx", "skyring2");
		platforms["platform"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		platforms["platform2"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		platforms["platform3"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		platforms["platform4"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		platforms["platform5"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		objL["moon"] = GameObject::create(rDir + "melee/ps2/", "ps2.dae", "moon");
		objL["sandbag"] = GameObject::create(rDir + "melee/Sandbag/", "sandbag.fbx", "sandbag");
		// GameObject::create(rDir + "melee/falcon2/", "Captain Falcon.dae", "falcon");
		// objL["animModel"] = GameObject::create(rDir + "anim/", "model.dae", "animModel");
		objL["animModel"] = GameObject::create(rDir + "anim/", "toto_walk.dae", "animModel");
		objL["animModel"]->addAnimation("toto_dab.dae");
		objL["animModel"]->addAnimation("toto_jump.dae");
		// objL["animModel"]->addAnimation("toto_run.dae");
		// objL["animModel"]->addAnimation("toto_jump.dae");


		cd->environmentBoxes.push_back(make_shared<AABB>(vec3(-10, -2, -10), vec3(10, -1, 10)));

		platforms["platform"]->location = vec3(-1, -.5, -2);
		vec3 *temp = &platforms["platform"]->location;
		platforms["platform"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform"]->hitboxes[0]));

		platforms["platform2"]->location = vec3(1, -.5, -2);
		temp = &platforms["platform2"]->location;
		platforms["platform2"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform2"]->hitboxes[0]));

		platforms["platform3"]->location = vec3(0, 0.25, -2);
		temp = &platforms["platform3"]->location;
		platforms["platform3"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform3"]->hitboxes[0]));

		platforms["platform4"]->location = vec3(2, 0.25, -2);
		temp = &platforms["platform4"]->location;
		platforms["platform4"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform4"]->hitboxes[0]));

		platforms["platform5"]->location = vec3(3, -.5, -2);
		temp = &platforms["platform5"]->location;
		platforms["platform5"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform5"]->hitboxes[0]));
		// cd->environmentBoxes.push_back(make_shared<AABB>(vec3(-.35f, -.11f, -1.f), vec3(.35f, -0.1f, 1.0f)));
	}
	
	void gethitBoxes(shared_ptr<Player> player, vector<HitSphere> &hitboxes) {
		vec3 offset = vec3(.05, .05, 0);
		if (!player->facingRight) {
			offset = -offset;
		}
		if (player->isAttacking)
			hitboxes.push_back(HitSphere(player->location+offset, .3));
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
		Projection->perspective(45.0f, aspect, 0.01f, 200.0f);

		//draw SKybox
		drawSkybox(Model, Projection);

		/* bind standard program */
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(camera.getViewMatrix()));
		vec3 vd = camera.lookAtPoint - camera.eye;
		glUniform3f(prog->getUniform("viewDirection"), vd.x, vd.y, vd.z);

		//set initial material and Light
		setLight(prog);

		
		sandbag->update(playerHitboxes);
		setMaterial(1, prog);
		objL["sandbag"]->translate(sandbag->location + vec3(0,.9,0));
		objL["sandbag"]->scale(.05);
		objL["sandbag"]->rotate(-PI/2, vec3(1,0,0));
		objL["sandbag"]->setModel(prog);
		objL["sandbag"]->draw(prog); 

		//Main Stage
		objL["FoD"]->translate(vec3(0, -1, -2));
		objL["FoD"]->scale(vec3(0.03f, 0.03f, 0.03f));
		setMaterial(3, prog);
		objL["FoD"]->setModel(prog);
		// objL["FoD"]->draw(prog);

		objL["moon"]->translate(vec3(0,-2.5,0));
		objL["moon"]->scale(vec3(.2f, .2f, .2f));
		objL["moon"]->rotate(-PI/2, vec3(1, 0, 0));
		setMaterial(1, prog);
		objL["moon"]->setModel(prog);
		objL["moon"]->draw(prog);

		//platform
		// cout << platforms["platform"]->hitboxes[0]->checkCollision(player1->environmentalHbox)<< endl;
		// platforms["platform"]->translate(vec3(0, -.5, -2));
		for (map<string, shared_ptr<GameObject>>::iterator it=platforms.begin(); it!=platforms.end(); ++it) {
			it->second->translate(it->second->location);
			it->second->setModel(prog);
			it->second->draw(prog);
		}

		// platforms["platform2"]->translate(platforms["platform2"]->location);
		// platforms["platform2"]->setModel(prog);
		// platforms["platform2"]->draw(prog);

		// platforms["platform3"]->translate(platforms["platform3"]->location);
		// platforms["platform3"]->setModel(prog);
		// platforms["platform3"]->draw(prog);

		//Skyring 1
		setMaterial(3, prog);
		objL["skyring1"]->translate(vec3(-2.4, 2, -2));
		objL["skyring1"]->rotate(.1, vec3(1,0,0));
		objL["skyring1"]->rotate(2*sin(.2*glfwGetTime()), vec3(0,0,1));
		objL["skyring1"]->scale(vec3(0.2, 0.2, 0.2));
		objL["skyring1"]->setModel(prog);
		// objL["skyring1"]->draw(prog);
		//Skyring2
		objL["skyring2"]->translate(vec3(-2.8, 2.1, -.26));
		objL["skyring2"]->rotate(-2*sin(.2*glfwGetTime()), vec3(0,0,1));
		objL["skyring2"]->rotate(.3, vec3(1,0,0));
		objL["skyring2"]->scale(vec3(0.2, 0.2, 0.2));
		objL["skyring2"]->setModel(prog);
		// objL["skyring2"]->draw(prog);

		// //draw Captain Falcon
		// Model->pushMatrix();
		// 	// getPlayerDisplacement();
		// 	Model->rotate(-PI/2, vec3(1, 0, 0));
		// 	Model->rotate(-PI/2, vec3(0, 0, 1));
		// 	Model->scale(vec3(0.035, 0.035, 0.035));
		// 	setMaterial(1, prog);
		// 	setModel(prog, Model);
		// 	// objL["falcon"]->draw(prog);
		// Model->popMatrix();

		prog->unbind();

		animProg->bind();
		glUniformMatrix4fv(animProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(animProg->getUniform("V"), 1, GL_FALSE, value_ptr(camera.getViewMatrix()));
		glUniform3f(animProg->getUniform("viewDirection"), vd.x, vd.y, vd.z);

		//set initial material and Light
		setLight(animProg);

		if (inputHandler->R)
			camera.eye = player1->location + normalize(player1->lookAtPoint - player1->location) * camera.distance + player1->getRightDir() * .6f  + camera.elevation;
		else
			camera.eye = player1->location - normalize(player1->lookAtPoint - player1->location) * camera.distance + player1->getRightDir() * .6f + camera.elevation;
		player1->update();
		camera.lookAtPoint = player1->location - camera.eye + player1->getRightDir() *.5f + camera.elevation + player1->getForwardDir() * .5f;
		gethitBoxes(player1, playerHitboxes);
		//move this to player class
		float angle = -glm::orientedAngle(normalize(vec3(player1->lookAtPoint.x, 0, player1->lookAtPoint.z)), vec3(1, 0, 0), vec3(0,1,0));

		objL["animModel"]->translate(player1->location);
		objL["animModel"]->scale(vec3(0.03, 0.03, 0.03));
		objL["animModel"]->rotate(PI/2 + angle, vec3(0, 1, 0));
		objL["animModel"]->rotate(-PI/2, vec3(1, 0, 0));
		if (inputHandler->Q)
			objL["animModel"]->doAnimation(0);
		if (inputHandler->E)
			objL["animModel"]->doAnimation(1);
		if (inputHandler->R)
			objL["animModel"]->doAnimation(2);
		setMaterial(1, animProg);
		objL["animModel"]->setModel(animProg);
		((shared_ptr<AnimatedShape>) (objL["animModel"]->shapeList[0]))->update();
		glUniformMatrix4fv(animProg->getUniform("jointTransforms"), 50, GL_FALSE, value_ptr(((shared_ptr<AnimatedShape>) (objL["animModel"]->shapeList[0]))->jointTransforms[0]));
		objL["animModel"]->draw(animProg);

		animProg->unbind();

		//animation update example
		sTheta = sin(glfwGetTime());

		// Pop matrix stacks.
		Projection->popMatrix();
		playerHitboxes.clear();
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

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// glfwSetWindowMonitor(windowManager->getHandle(), glfwGetPrimaryMonitor(), 0, 0, 1980, 1080, 0);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		application->camera.setViewAngles(windowManager->getHandle());
		application->player1->setViewAngles(windowManager->getHandle());
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
