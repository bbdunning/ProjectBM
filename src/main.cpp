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
#include "Camera.h"
#include "Hitbox.h"
#include "Particle.h"
#include "ParticleSystem.h"
#include "ParticleRenderer.h"
#include "CollisionDetector.h"
#include "DynamicCharacterController.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/LinearMath/btAabbUtil2.h>

#include <irrklang/irrKlang.h>
#include "ik_ISoundEngine.h"

#include <imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/projection.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/types.h>
#include <assimp/texture.h>
#include <assimp/postprocess.h>
// #include <AL/al.h>

using namespace std;
using namespace glm;
using namespace irrklang;

#define PI 3.14159

class Application : public EventCallbacks
{

public:

	//shadow data
	bool SHADOW = true;
	GLuint depthMapFBO;
	GLuint depthMap;
	const GLuint S_WIDTH = 4096, S_HEIGHT = 4096;
	float dx = 0; //screen ratio

	//create GLFW Window
	WindowManager * windowManager = nullptr;
	float previousTime = 0.0f;
	float dt = 0.0f;

	// create shaders
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> noShadeProg;
	std::shared_ptr<Program> cubeProg;
	std::shared_ptr<Program> animProg;
	std::shared_ptr<Program> DepthProg;
	std::shared_ptr<Program> outlineProg;

	//mesh data
	unordered_map<string, shared_ptr<GameObject>> objL;
	vector<HitSphere> playerHitboxes;

	//sound engine
	ISoundEngine* IRengine = createIrrKlangDevice();
	string audioDir = "D:/source/ProjectBM/resources/";
	ISound* bgTheme;
	bool sound;

	//physics data
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new	btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	GLuint VertexArrayID;
	GLuint VertexBufferID;

	//input data
	shared_ptr<InputHandler> inputHandler = make_shared<InputHandler>();

	//game data
	shared_ptr<Player> player1 = make_shared<Player>();
	vector<btRigidBody*> projectiles;
	vector<float> projectileTimes;
	float projectileLifeTime = 4.0f;
	btRigidBody* playerBody;
	btRigidBody* bokoBody;
	btRigidBody* pokeballBody;
	bool previousLand = false;
	bool leftMouse = false;
	int player1Lives = 3;
	int player2Lives = 3;
	float rTimer = 4.0f;
	
	//animation data
	int m = 1;

	//Camera
	Camera camera;

	//particle data
	vector<Particle> particles;
	vector<ParticleSystem> particleSystems;
	ParticleRenderer pr = ParticleRenderer(prog);
	float walkingParticleSystemElapsedTime = 0.0f;
	float walkingParticleSystemCooldown = .1f;

	//skybox
	unsigned int skyboxTextureId = 0;
	vector<std::string> faces {           
		"Newdawn1_left.png",           
		"Newdawn1_right.png",           
		"Newdawn1_up.png",           
		"Newdawn1_down.png",           
		"Newdawn1_front.png",           
		"Newdawn1_back.png"};          


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
		leftMouse = true;

		if (action == GLFW_RELEASE)
			leftMouse = false;

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
					0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);  
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
		// glUniform3f(prog->getUniform("LightPos"), 5.f, 3.f, -.4f);
		glUniform3f(prog->getUniform("LightPos"), 5.f, 20.f, 15.f);
		glUniform3f(prog->getUniform("LightCol"), 1.f, 1.f, 1.f); 
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shaders/simple_vert.glsl", resourceDirectory + "/shaders/blinn-phong.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("LS");
		prog->addUniform("shadowDepth");
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
		prog->addUniform("aspectRatioX");
		prog->addUniform("aspectRatioY");
		prog->addUniform("cameraPos");

		noShadeProg = make_shared<Program>();
		noShadeProg->setVerbose(true);
		noShadeProg->setShaderNames(resourceDirectory + "/shaders/simple_vert.glsl", resourceDirectory + "/shaders/noShadow_frag.glsl");
		noShadeProg->init();
		noShadeProg->addUniform("P");
		noShadeProg->addUniform("V");
		noShadeProg->addUniform("M");
		noShadeProg->addUniform("LS");
		noShadeProg->addUniform("shadowDepth");
		noShadeProg->addAttribute("vertPos");
		noShadeProg->addAttribute("vertNor");
		noShadeProg->addAttribute("vertTex");

		noShadeProg->addUniform("viewDirection");
		noShadeProg->addUniform("LightPos");
		noShadeProg->addUniform("LightCol");
		noShadeProg->addUniform("MatAmb");
		noShadeProg->addUniform("MatDif");
		noShadeProg->addUniform("MatSpec");
		noShadeProg->addUniform("shine");
		noShadeProg->addUniform("Texture0");
		noShadeProg->addUniform("aspectRatioX");
		noShadeProg->addUniform("aspectRatioY");
		noShadeProg->addUniform("cameraPos");

		animProg = make_shared<Program>();
		animProg->setVerbose(true);
		animProg->setShaderNames(resourceDirectory + "/shaders/anim_vert.glsl", resourceDirectory + "/shaders/blinn-phong.glsl");
		animProg->init();
		animProg->addUniform("P");
		animProg->addUniform("V");
		animProg->addUniform("M");
		animProg->addUniform("LS");
		animProg->addAttribute("vertPos");
		animProg->addAttribute("vertNor");
		animProg->addAttribute("vertTex");		
		animProg->addAttribute("jointIndices");		
		animProg->addAttribute("jointWeights");		
		animProg->addUniform("shadowDepth");
		animProg->addUniform("aspectRatioX");
		animProg->addUniform("aspectRatioY");
		animProg->addUniform("cameraPos");

		animProg->addUniform("viewDirection");
		animProg->addUniform("LightPos");
		animProg->addUniform("LightCol");
		animProg->addUniform("MatAmb");
		animProg->addUniform("MatDif");
		animProg->addUniform("MatSpec");
		animProg->addUniform("shine");
		animProg->addUniform("Texture0");
		animProg->addUniform("jointTransforms");

		outlineProg = make_shared<Program>();
		outlineProg->setVerbose(true);
		outlineProg->hasTexture = false;
		outlineProg->setShaderNames(resourceDirectory + "/shaders/outline_vert.glsl", resourceDirectory + "/shaders/outline_frag.glsl");
		outlineProg->init();
		outlineProg->addUniform("P");
		outlineProg->addUniform("V");
		outlineProg->addUniform("M");
		outlineProg->addUniform("outlineOffset");
		outlineProg->addAttribute("vertPos");
		outlineProg->addAttribute("vertNor");
		outlineProg->addAttribute("vertTex");

		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames( resourceDirectory + "/shaders/cube_vert.glsl", resourceDirectory + "/shaders/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");

		DepthProg = make_shared<Program>();
		DepthProg->hasTexture = false;
		DepthProg->setVerbose(true);
		DepthProg->setShaderNames(resourceDirectory + "/shaders/depth_vert.glsl", resourceDirectory + "/shaders/depth_frag.glsl");
		DepthProg->init();
		DepthProg->addUniform("LP");
		DepthProg->addUniform("LV");
		DepthProg->addUniform("M");
		DepthProg->addAttribute("vertPos");
		//un-needed, better solution to modifying shape
		DepthProg->addAttribute("vertNor");
		DepthProg->addAttribute("vertTex");

		//IJSDFOIJSDPIOFJ:SLID JFILK:SDJIL:KJIO:EWASJPOWEJFO
		// partSys.emitParticle();

		inputHandler->init();
		camera.init();
		camera.setInputHandler(inputHandler);
		player1->init(inputHandler);
		// player1->playerBody = playerBody;
		skyboxTextureId = createSky(resourceDirectory + "/skybox/", faces);
		initShadow();
		// IRengine->play2D("D:/source/ProjectBM/resources/audio/ps2.mp3", true);
		// IRengine->play2D("D:/source/ProjectBM/resources/audio/main_theme.mp3", true);
		// bgTheme = IRengine->play2D("D:/source/ProjectBM/resources/audio/wild_battle_brawl.mp3", true, false, true);
		bgTheme = IRengine->play2D("D:/source/ProjectBM/resources/audio/ps2.mp3", true, false, true);
		if (bgTheme)
			bgTheme->setVolume(.24);
	}

	void initShadow() {

		//generate the FBO for the shadow depth
		glGenFramebuffers(1, &depthMapFBO);

		//generate the texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, S_WIDTH, S_HEIGHT, 
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//bind with framebuffer's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	//CREATE BALL
	btRigidBody* createBall() {
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);
		btTransform startTransform;
		startTransform.setIdentity();
		btScalar mass(.75f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);
		startTransform.setOrigin(btVector3(0,10,-10));
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		dynamicsWorld->addRigidBody(body);
		body->setRestitution(.5);
		body->setGravity(btVector3(0,-15,0));
		return body;
	}

	void initPhysics() {
		dynamicsWorld->setGravity(btVector3(0,-10,0));

		{  //CREATE GROUND
			btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(13.f),btScalar(1.f),btScalar(8.4f)));
			collisionShapes.push_back(groundShape);
			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(0,-1,-10));
			btScalar mass(0.);
			bool isDynamic = (mass != 0.f);
			btVector3 localInertia(0,0,0);
			if (isDynamic)
				groundShape->calculateLocalInertia(mass,localInertia);
			btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setFriction(1.0);
			body->setRestitution(1.0);
			dynamicsWorld->addRigidBody(body);
		}
		{  //CREATE GROUND
			btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(13.f),btScalar(100.f),btScalar(1.f)));
			collisionShapes.push_back(groundShape);
			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(0,-1,-18.4));
			btScalar mass(0.);
			bool isDynamic = (mass != 0.f);
			btVector3 localInertia(0,0,0);
			if (isDynamic)
				groundShape->calculateLocalInertia(mass,localInertia);
			btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setFriction(1.0);
			body->setRestitution(1.0);
			dynamicsWorld->addRigidBody(body);
		}
		{  //CREATE GROUND
			btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(13.f),btScalar(100.f),btScalar(1.f)));
			collisionShapes.push_back(groundShape);
			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(0,-1,-1.6));
			btScalar mass(0.);
			bool isDynamic = (mass != 0.f);
			btVector3 localInertia(0,0,0);
			if (isDynamic)
				groundShape->calculateLocalInertia(mass,localInertia);
			btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setFriction(1.0);
			body->setRestitution(1.0);
			dynamicsWorld->addRigidBody(body);
		}
		{  //CREATE GROUND
			btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(1.f),btScalar(100.f),btScalar(8.4f)));
			collisionShapes.push_back(groundShape);
			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(-12.4,-1,-10));
			btScalar mass(0.);
			bool isDynamic = (mass != 0.f);
			btVector3 localInertia(0,0,0);
			if (isDynamic)
				groundShape->calculateLocalInertia(mass,localInertia);
			btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setFriction(1.0);
			body->setRestitution(1.0);
			dynamicsWorld->addRigidBody(body);
		}
		{  //CREATE GROUND
			btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(1.f),btScalar(100.f),btScalar(8.4f)));
			collisionShapes.push_back(groundShape);
			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(12.4,-1,-10));
			btScalar mass(0.);
			bool isDynamic = (mass != 0.f);
			btVector3 localInertia(0,0,0);
			if (isDynamic)
				groundShape->calculateLocalInertia(mass,localInertia);
			btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setFriction(1.0);
			body->setRestitution(1.0);
			dynamicsWorld->addRigidBody(body);
		}
		pokeballBody = createBall();

		playerBody = createPlayerRigidBody(vec3(0,5,-5));
		bokoBody = createRigidBody(vec3(7,5,-10), vec3(.4, .4, .4));
	}

	btRigidBody* createPlayerRigidBody(vec3 location) {
		btCollisionShape* colShape = new btSphereShape(btScalar(0.25f));
		collisionShapes.push_back(colShape);
		btTransform startTransform;
		startTransform.setIdentity();
		btScalar mass(1.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);
		startTransform.setOrigin(bt(location));
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		dynamicsWorld->addRigidBody(body);
		body->setFriction(1.5f);
		return body;
	}

	//create a dynamic rigidbody
	btRigidBody* createRigidBody(vec3 location, vec3 size) {
		// btCollisionShape* colShape = new btBoxShape(bt(size));
		btCollisionShape* colShape = new btSphereShape(size.x);
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(2.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);

			startTransform.setOrigin(bt(location));
		
			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);

			dynamicsWorld->addRigidBody(body);
		body->setFriction(1.5f);
		return body;
	}

	btRigidBody* createProjectile(vector<btRigidBody*> &projectiles, vec3 location, vec3 direction, float magnitude) {
		//create a dynamic rigidbody

		// btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(0.25f));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);

			startTransform.setOrigin(bt(location));
		
			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);

			dynamicsWorld->addRigidBody(body);
		body->applyCentralImpulse(bt(direction) * magnitude);
		body->setFriction(1.0f);
		body->setDamping(0.2f, 0.5f);
		body->setRestitution(.8);
		projectiles.push_back(body);
		projectileTimes.push_back(0.0f);
		return body;
	}

	void renderProjectiles(shared_ptr<Program> currentShader, unordered_map<string, shared_ptr<GameObject>> &objL, vector<btRigidBody*> &projectiles) {
		// setMaterial(5, currentShader);

		for (int i=0; i<projectiles.size(); i++) {
			btCollisionObject* obj = projectiles[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;
			body->getMotionState()->getWorldTransform(trans);
			vec3 physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
			btQuaternion btQ = body->getOrientation();

			objL["sphere"]->translate(physicsLoc); //+ vec3(0,.9,0));
			objL["sphere"]->scale(.25f);
			objL["sphere"]->rotate(-PI/2, vec3(1.f,0.f,0.f));
			objL["sphere"]->rotate(btQ.getAngle(), cons(btQ.getAxis()));
			objL["sphere"]->setModel(currentShader);
			objL["sphere"]->draw(currentShader); 
		}
	}

	void removeProjectiles() {
		for (int i=0; i< projectiles.size(); i++) {
			btCollisionObject* obj = projectiles[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			deletePhysicsObject(body);
		}
		projectiles.clear();
		projectileTimes.clear();
	}

	void initGeom(const std::string& resourceDirectory)
	{
		string rDir = resourceDirectory + "/";

		//load geometry, initialize meshes, create objects
		objL["cube"] = GameObject::create(rDir, "cube.obj", "cube");
		// objL["quad"] = GameObject::create(rDir, "quad.dae", "quad");
		objL["quad"] = GameObject::create(rDir, "bubble.dae", "quad");
		objL["dustcloud"] = GameObject::create(rDir, "dustcloud.dae", "dustcloud");
		objL["cube2"] = GameObject::create(rDir, "cube.obj", "cube");
		objL["sphere"] = GameObject::create(rDir + "general/", "waterball.dae", "sphere");
		objL["ps2"] = GameObject::create(rDir + "melee/ps2/", "ps2_stage.dae", "ps2");
		objL["ps2_backdrop"] = GameObject::create(rDir + "melee/ps2/", "ps2_backdrop.dae", "ps2_backdrop");
		objL["pokeball"] = GameObject::create(rDir + "pokeball/", "pokeball.dae", "pokeball");
		objL["animModel"] = GameObject::create(rDir + "anim/", "toto.dae", "animModel");
			objL["animModel"]->addAnimation("toto_walk.dae");
			objL["animModel"]->addAnimation("toto_watergun.dae");
			objL["animModel"]->addAnimation("toto_dab.dae");
			objL["animModel"]->addAnimation("toto_jump.dae");
			objL["animModel"]->doAnimation(0);
		objL["boko"] = GameObject::create(rDir + "/anim/", "toto.dae", "boko");
		objL["boko"]->addAnimation("toto_watergun.dae");
		objL["boko"]->doAnimation(0);
		// objL["animModel"]->addAnimation("toto_run.dae");
		// objL["animModel"]->addAnimation("toto_jump.dae");

	}

	float getDeltaTimeSeconds() {
		float currentTime = glfwGetTime();
		float deltaTime = currentTime - previousTime;
		previousTime = currentTime;
		return deltaTime;
	}

	btVector3 bt(vec3 v) {
		return btVector3(v.x, v.y, v.z);
	}

	vec3 cons(btVector3 v) {
		return vec3(v.getX(), v.getY(), v.getZ());
	}

	void setCameraEye() {
		if (inputHandler->R)
			camera.eye = player1->location + normalize(player1->lookAtPoint - player1->location) * camera.distance + player1->getRightDir() * .6f  + camera.elevation;
		else
			camera.eye = player1->location - normalize(player1->lookAtPoint - player1->location) * camera.distance + player1->getRightDir() * .6f + camera.elevation;
		camera.lookAtPoint = player1->location - camera.eye + player1->getRightDir() *.5f + camera.elevation + player1->getForwardDir() * .5f;
	}

	void sendUniforms(shared_ptr<Program> prog, const mat4 &proj, const mat4 &view) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		glUniform1f(prog->getUniform("aspectRatioX"), (float) width/S_WIDTH);
		glUniform1f(prog->getUniform("aspectRatioY"), (float) height/S_HEIGHT);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(proj));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(view));
		vec3 vd = camera.lookAtPoint - camera.eye;
		glUniform3f(prog->getUniform("viewDirection"), vd.x, vd.y, vd.z);
		setLight(prog);
	}
	
	void checkAbilities() {
		player1->charging = leftMouse;

		//charge projectile
		if (player1->charging) {
			player1->projectileChargeTime += dt;
		}

		//launch projectile
		if (!player1->charging && player1->prevCharge && (player1->projectileCooldown <= 0)) {
			float magnitude = clamp((player1->projectileChargeTime/player1->projectileMaxChargeTime), 0.4f, 1.f) * player1->maxMagnitude;
			vec3 launchpos =player1->location + player1->getForwardDir() + vec3(0,.5,0);
			createProjectile(projectiles, launchpos, player1->getForwardDir(), magnitude);
			IRengine->play2D("D:/source/ProjectBM/resources/audio/squirt.wav", false);
			cout << "shoot" << endl;
			player1->projectileChargeTime = 0.0f;
			player1->projectileChargeTime += player1->projectileCooldown * .4;
			player1->projectileCooldown = .5f;

			particleSystems.push_back(ParticleSystem(launchpos, 0, 15, 1, objL["quad"]));
			projectileTimes.push_back(0.0f);
		}
		player1->prevCharge = player1->charging;

		if (player1->projectileCooldown > 0) {
			player1->projectileCooldown -= dt;
		}
	}

	void removeOneProjectile(vector<btRigidBody*> &projectiles) {
		if (projectiles.size() > 0) {
			btCollisionObject* obj = projectiles[0];
			btRigidBody* body = btRigidBody::upcast(obj);
			deletePhysicsObject(body);
			projectiles.erase(projectiles.begin());
		}
		if (projectileTimes.size() > 0) {
			projectileTimes.erase(projectileTimes.begin());
		}
	}

	//render player in correct position & animation
	void setPlayer() {
		objL["animModel"]->translate(player1->location - vec3(0,.25,0));
		objL["animModel"]->scale(vec3(0.04, 0.04, 0.04));
		// objL["animModel"]->rotate(-8, player1->getRightDir());
		objL["animModel"]->rotate(PI/2 + player1->getFacingAngle(), vec3(0, 1, 0));
		objL["animModel"]->rotate(-PI/2, vec3(1, 0, 0));
		if (inputHandler->n1)
			objL["animModel"]->doAnimation(0);
		if (inputHandler->n2)
			objL["animModel"]->doAnimation(1);
		if (inputHandler->n3)
			objL["animModel"]->doAnimation(2);
		if (inputHandler->n4)
			objL["animModel"]->doAnimation(3);
		setMaterial(5, animProg);
		objL["animModel"]->setModel(animProg);

		//update armature position
		((shared_ptr<AnimatedShape>) (objL["animModel"]->shapeList[0]))->update();
		//set joint transforms
		glUniformMatrix4fv(animProg->getUniform("jointTransforms"), 50, GL_FALSE, 
			value_ptr(((shared_ptr<AnimatedShape>) (objL["animModel"]->shapeList[0]))->jointTransforms[0]));

		if (!player1->isGrounded) {
			objL["animModel"]->doAnimation(3);
		}
		else if (player1->isGrounded && length(cons(playerBody->getLinearVelocity())) > 1) {
			objL["animModel"]->doAnimation(0);
			if (walkingParticleSystemElapsedTime <= 0) {
				particleSystems.push_back(ParticleSystem(player1->location - vec3(0,.1,0), 0, 1, .3, objL["dustcloud"]));
				walkingParticleSystemElapsedTime = walkingParticleSystemCooldown;
			}
		}
		else {
			objL["animModel"]->doAnimation(1);
		}
		walkingParticleSystemElapsedTime -= dt;
	}

	void setBoko() {
		btTransform trans;
		vec3 physicsLoc;
		bokoBody->forceActivationState(1);
		bokoBody->getMotionState()->getWorldTransform(trans);
		physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
		objL["boko"]->translate(physicsLoc - vec3(0,.26,0));
		objL["boko"]->scale(vec3(0.05, 0.05, 0.05));
		objL["boko"]->rotate(-PI/2, vec3(0, 1, 0));
		objL["boko"]->rotate(-PI/2, vec3(1, 0, 0));
		objL["boko"]->doAnimation(0);
		setMaterial(5, animProg);
		objL["boko"]->setModel(animProg);
		((shared_ptr<AnimatedShape>) (objL["boko"]->shapeList[0]))->update();
		glUniformMatrix4fv(animProg->getUniform("jointTransforms"), 50, GL_FALSE, value_ptr(((shared_ptr<AnimatedShape>) (objL["boko"]->shapeList[0]))->jointTransforms[0]));
	}

	void drawObjects(shared_ptr<Program> currentShader) {
		//getPosition of object
		btTransform trans;
		pokeballBody->getMotionState()->getWorldTransform(trans);
		vec3 physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
		btQuaternion btQ = pokeballBody->getOrientation();

		//draw sphere
		// setMaterial(1, currentShader);
		objL["pokeball"]->translate(physicsLoc); //+ vec3(0,.9,0));
		objL["pokeball"]->scale(.006); //+ vec3(0,.9,0));
		objL["pokeball"]->rotate(btQ.getAngle(), cons(btQ.getAxis()));
		objL["pokeball"]->setModel(currentShader);
		objL["pokeball"]->draw(currentShader); 

		//draw ps2
		objL["ps2"]->translate(vec3(6.5,-3.21f,-.7));
		objL["ps2"]->scale(vec3(.35f, .35f, .35f));
		objL["ps2"]->rotate(-PI/2, vec3(1.f, 0.f, 0.f));
		// setMaterial(5, currentShader);
		objL["ps2"]->setModel(currentShader);
		objL["ps2"]->draw(currentShader);

		//renderProjectiles
		renderProjectiles(currentShader, objL, projectiles);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

	mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
		float edge = 20.f;
		// float edge = 15.f;
		mat4 ortho = glm::ortho(-edge, edge, -edge, edge, 0.1f, 2*edge);
		glUniformMatrix4fv(curShade->getUniform("LP"), 1, GL_FALSE, value_ptr(ortho));
		return ortho;
	}

	mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up) {
		mat4 Cam = glm::lookAt(pos, LA, up);
		glUniformMatrix4fv(curShade->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
		//fill in the glUniform call to send to the right shader!
		return Cam;
	}

	void deletePhysicsObject(btRigidBody* rigidBody) {
		delete rigidBody->getMotionState();
		delete rigidBody->getCollisionShape();
		dynamicsWorld->removeRigidBody(rigidBody);
		delete rigidBody;
	}

	void resetPhysicsObjects() {
		deletePhysicsObject(pokeballBody);
		pokeballBody = createBall();
		removeProjectiles();
		deletePhysicsObject(playerBody);
		playerBody = createPlayerRigidBody(vec3(-7,5,-10));
		deletePhysicsObject(bokoBody);
		bokoBody = createRigidBody(vec3(7,5,-10), vec3(.4, .4, .4));
	}

	void updateParticleSystems(float dt) {
		//update particle systems
		for (int i=0; i<particleSystems.size(); i++) {
			particleSystems[i].update(dt);
		}

		//render particle systems
		for (int i=0; i<particleSystems.size(); i++) {
			pr.render(prog, particleSystems[i].particles, particleSystems[i].obj, camera.getViewMatrix());
		}

		//remove dead particle systems
		for (int i=0; i<particleSystems.size(); i++) {
			if (particleSystems[i].timeElapsed >= particleSystems[i].lifeLength)
				particleSystems.erase(particleSystems.begin() + i);
		}
	}

	void detectBallLand(btRigidBody *pokeBody) {
		//raycast straight down
		btTransform trans;
		pokeBody->getMotionState()->getWorldTransform(trans);
		btVector3 location = trans.getOrigin();
		btVector3 btTo(location.getX(), location.getY() - 1.2f, location.getZ());
		btVector3 particleloc(location.getX(), location.getY() - 1.f, location.getZ());
		btCollisionWorld::ClosestRayResultCallback res(location, btTo);
		dynamicsWorld->rayTest(location, btTo, res);
		if(res.hasHit() && !previousLand){
			IRengine->play2D("D:/source/ProjectBM/resources/audio/metal.wav", false);
			particleSystems.push_back(ParticleSystem(cons(particleloc), 0, 15, .75, objL["dustcloud"]));
		} else {
			cout << "air" << endl;
		}
		previousLand = res.hasHit();
	}

	void render() {
		// Get current frame buffer size & dt
		dt = getDeltaTimeSeconds();
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		float aspect = width/(float)height;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear FrameBuffer

		// initialize matrices
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 10000.0f);
		setCameraEye();

		//draw Skybox
		drawSkybox(Model, Projection);

		pokeballBody->forceActivationState(1);

		if (inputHandler->Cflag) {
			resetPhysicsObjects();
		}

		int pcount = 0;
		for (int i=0; i < projectiles.size(); i++) {
			projectileTimes[i] += dt;
			if (projectileTimes[i] >= projectileLifeTime)
				pcount++;
		}
		for (int i=0; i < pcount; i++) {
			removeOneProjectile(projectiles);
		}
		// if (rTimer <= 0) {
		// 	removeOneProjectile(projectiles);
		// 	rTimer = 4.0f;
		// } else {
		// 	rTimer -= dt;
		// }

		if (inputHandler->Q) {
			IRengine->setSoundVolume(0);
		} else if (inputHandler->E) {
			IRengine->setSoundVolume(1);
		}

		//check Pokeball collision
		detectBallLand(pokeballBody);
		{
			btTransform trans;
			vec3 bokoLoc;
			bokoBody->getMotionState()->getWorldTransform(trans);
			bokoLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));

			vec3 pokeLoc;
			pokeballBody->getMotionState()->getWorldTransform(trans);
			pokeLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));

			if (length(bokoLoc-pokeLoc) < 1.5) {
				resetPhysicsObjects();
				player2Lives -= 1;
			}
		}

		//shadow Data
		mat4 LP, LV, LS;
		vec3 lightPos = vec3(5,15 + dx,15);
		vec3 lightLA = lightPos + vec3(0,-10,-10);
		vec3 lightUp = vec3(0,1,0);

		//generate shadow map
		glViewport(0, 0, S_WIDTH, S_WIDTH);

		//sets up the output to be out FBO
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		
		//Second pass, now draw the scene
		glViewport(0, 0, width, height);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//set up shadow shader and render the scene
		DepthProg->bind();
			LP = SetOrthoMatrix(DepthProg);
			LV = SetLightView(DepthProg, lightPos, lightLA, lightUp);
			LS = LP*LV;
			drawObjects(DepthProg);
			objL["sphere"]->translate(player1->location);
			objL["sphere"]->scale(vec3(.2,.1,.2));
			objL["sphere"]->setModel(DepthProg);
			objL["sphere"]->draw(DepthProg);
		DepthProg->unbind();

		//set culling back to normal
		glCullFace(GL_BACK);

		//this sets the output back to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		

		//update player
		player1->update(dt);
		player1->move(dt, playerBody, dynamicsWorld);
		checkAbilities();

		/* bind & initialize standard program */
		prog->bind();
			setMaterial(5, prog);
			sendUniforms(prog, Projection->topMatrix(), camera.getViewMatrix());
			glUniform3f(prog->getUniform("cameraPos"), camera.eye.x, camera.eye.y, camera.eye.z);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			glUniform1i(prog->getUniform("shadowDepth"), 2);
			glUniformMatrix4fv(prog->getUniform("LS"), 1, GL_FALSE, value_ptr(LS));
			drawObjects(prog);
		prog->unbind();

		noShadeProg->bind();
			setMaterial(5, noShadeProg);
			sendUniforms(noShadeProg, Projection->topMatrix(), camera.getViewMatrix());
			glUniform3f(noShadeProg->getUniform("cameraPos"), camera.eye.x, camera.eye.y, camera.eye.z);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			glUniform1i(noShadeProg->getUniform("shadowDepth"), 2);
			glUniformMatrix4fv(noShadeProg->getUniform("LS"), 1, GL_FALSE, value_ptr(LS));

			objL["ps2_backdrop"]->translate(vec3(5,-3.15f,0));
			objL["ps2_backdrop"]->scale(vec3(.35f, .35f, .35f));
			objL["ps2_backdrop"]->rotate(-PI/2, vec3(1.f, 0.f, 0.f));
			objL["ps2_backdrop"]->setModel(noShadeProg);
			objL["ps2_backdrop"]->draw(noShadeProg);
		noShadeProg->unbind();


		animProg->bind();
			sendUniforms(animProg, Projection->topMatrix(), camera.getViewMatrix());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			glUniform1i(animProg->getUniform("shadowDepth"), 2);
			glUniform3f(animProg->getUniform("cameraPos"), camera.eye.x, camera.eye.y, camera.eye.z);
			glUniformMatrix4fv(animProg->getUniform("LS"), 1, GL_FALSE, value_ptr(LS));
			setPlayer();
			objL["animModel"]->draw(animProg);
			setBoko();
			objL["boko"]->draw(animProg);
		animProg->unbind();

		updateParticleSystems(dt);
		// outlineProg->bind();
		// 	glUniformMatrix4fv(outlineProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		// 	glUniformMatrix4fv(outlineProg->getUniform("V"), 1, GL_FALSE, value_ptr(camera.getViewMatrix()));
		// 	glUniform1f(outlineProg->getUniform("outlineOffset"), 0.0);
		// 	drawObjects(outlineProg);
		// outlineProg->unbind();

		player1->updateLocation(playerBody);

		// Pop matrix stacks.
		Projection->popMatrix();

		//step physics simulation
		dynamicsWorld->stepSimulation(dt);
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
	windowManager->init(1520, 1080);
	// windowManager->init(1024, 1024);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initPhysics();

	glfwSetWindowMonitor(windowManager->getHandle(), glfwGetPrimaryMonitor(), 0, 0, 2650, 1440, 144);
	#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
		bool err = gl3wInit() != 0;
	#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
		bool err = gladLoadGL() == 0;
	#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
		bool err = false;
		glbinding::Binding::initialize();
	#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
		bool err = false;
		glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
	#endif
	    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
	// const char* glsl_version = "#version 330 core";
	const char* glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL(windowManager->getHandle(), true);
    ImGui_ImplOpenGL3_Init(glsl_version);
	bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	shared_ptr<Texture> totoPic = createTexture(resourceDir + "/toto.png");
	GLuint toto_id = totoPic->getID();

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		if (!application->inputHandler->Ctrlflag)
			glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else 
			glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		application->camera.setViewAngles(windowManager->getHandle());
		application->player1->setViewAngles(windowManager->getHandle());
		application->render();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

			const ImVec2 size = ImVec2(100, 100);
			ImGui::SetNextWindowSize(size);
            ImGui::Begin("Hello, world!", (bool*) nullptr, ImGuiWindowFlags_NoDecoration);

            ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
            ImGui::End();
        }

        {
			const ImVec2 size = ImVec2(100, 100);
			ImGui::SetNextWindowSize(size);
            ImGui::Begin("Player 1", (bool*) nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
			// ImGui::SetWindowPos(wndPos);

			// GLuint id = 6;
			ImGui::Image((void*)(intptr_t) toto_id, ImVec2(64, 64));
			ImGui::SameLine();
			ImGui::Text("x%d", application->player1Lives);
            ImGui::End();
        }

        {
			const ImVec2 size = ImVec2(100, 100);
			ImGui::SetNextWindowSize(size);
            ImGui::Begin("Player 2", (bool*) nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);

			ImGui::Image((void*)(intptr_t) toto_id, ImVec2(64, 64));
			ImGui::SameLine();
			ImGui::Text("x%d", application->player2Lives);
            ImGui::End();
        }

		if (application->player2Lives <= 0)
        {
			const ImVec2 size = ImVec2(256, 140);
			ImGui::SetNextWindowSize(size);
            ImGui::Begin("Win", (bool*) nullptr, ImGuiWindowFlags_NoDecoration);

			ImGui::SameLine();
			ImGui::Text("Player 1 Wins!");
            ImGui::End();
        }


		
		ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// ImGui::EndFrame();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
