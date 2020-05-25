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
#include "CollisionDetector.h"
#include "DynamicCharacterController.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/LinearMath/btAabbUtil2.h>

#include <irrklang/irrKlang.h>
#include "ik_ISoundEngine.h"

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
	const GLuint S_WIDTH = 1024, S_HEIGHT = 1024;

	//create GLFW Window
	WindowManager * windowManager = nullptr;
	float previousTime = 0.0f;

	// create shaders
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> cubeProg;
	std::shared_ptr<Program> animProg;
	std::shared_ptr<Program> DepthProg;
	std::shared_ptr<Program> DepthProgDebug;

	//mesh data
	unordered_map<string, shared_ptr<GameObject>> objL;
	vector<HitSphere> playerHitboxes;

	//sound engine
	// ISoundEngine* engine = createIrrKlangDevice();

	//physics data
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new	btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	shared_ptr<InputHandler> inputHandler = make_shared<InputHandler>();
	shared_ptr<Player> player1 = make_shared<Player>();
	vector<btRigidBody*> projectiles;
	btRigidBody* playerBody;
	btRigidBody* bokoBody;
	
	//animation data
	int m = 1;
	float prevOmega = 0.0f;
	bool leftMouse = false;

	//Camera
	Camera camera;

	//skybox
	unsigned int skyboxTextureId = 0;
	vector<std::string> faces {           
		"posx.jpg",           
		"negx.jpg",           
		"posy.jpg",           
		"negy.jpg",           
		"posz.jpg",           
		"negz.jpg"};


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


	void setLight(shared_ptr<Program> prog) {
		glUniform3f(prog->getUniform("LightPos"), 5.f, 3.f, -.4f);
		glUniform3f(prog->getUniform("LightCol"), 1.f, 1.f, 1.f); 
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

		DepthProg = make_shared<Program>();
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

		DepthProgDebug = make_shared<Program>();
		DepthProgDebug->setVerbose(true);
		DepthProgDebug->setShaderNames(resourceDirectory + "/shaders/depth_vertDebug.glsl", resourceDirectory + "/shaders/depth_fragDebug.glsl");
		DepthProgDebug->init();
		DepthProgDebug->addUniform("LP");
		DepthProgDebug->addUniform("LV");
		DepthProgDebug->addUniform("M");
		DepthProgDebug->addAttribute("vertPos");
		//un-needed, better solution to modifying shape
		DepthProgDebug->addAttribute("vertNor");
		DepthProgDebug->addAttribute("vertTex");

		inputHandler->init();
		camera.init();
		camera.setInputHandler(inputHandler);
		player1->init(inputHandler);
		// player1->playerBody = playerBody;
		skyboxTextureId = createSky(resourceDirectory + "/skybox/", faces);
		initShadow();
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

	void initPhysics() {
		dynamicsWorld->setGravity(btVector3(0,-10,0));

		{
			btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(13.f),btScalar(1.f),btScalar(8.f)));
			collisionShapes.push_back(groundShape);

			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(-2,-1,-9));

			btScalar mass(0.);

			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			bool isDynamic = (mass != 0.f);

			btVector3 localInertia(0,0,0);
			if (isDynamic)
				groundShape->calculateLocalInertia(mass,localInertia);

			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setFriction(1.0);
			body->setRestitution(1.0);

			//add the body to the dynamics world
			dynamicsWorld->addRigidBody(body);
		}

		{
			//create a dynamic rigidbody
			// btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
			btCollisionShape* colShape = new btSphereShape(btScalar(1.));
			collisionShapes.push_back(colShape);

			/// Create Dynamic Objects
			btTransform startTransform;
			startTransform.setIdentity();

			btScalar mass(4.f);

			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			bool isDynamic = (mass != 0.f);

			btVector3 localInertia(0,0,0);
			if (isDynamic)
				colShape->calculateLocalInertia(mass,localInertia);

				startTransform.setOrigin(btVector3(-1,10,-10));
			
				//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
				btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
				btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
				btRigidBody* body = new btRigidBody(rbInfo);

				dynamicsWorld->addRigidBody(body);
		}
		playerBody = createPlayerRigidBody(vec3(0,5,-5));
		bokoBody = createRigidBody(vec3(7,5,-10), vec3(.25, .25, .25));
	}

	btRigidBody* createPlayerRigidBody(vec3 location) {
		//create a dynamic rigidbody

		// btCollisionShape* colShape = new btBoxShape(btVector3(.25,.25,.25));
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
		body->setFriction(1.5f);
		// body->setGravity(btVector3(0, -15, 0));
		// body->setDamping(0.8f, 1.0f);
		// body->setRestitution(.2);
		return body;
	}

	//create a dynamic rigidbody
	btRigidBody* createRigidBody(vec3 location, vec3 size) {
		btCollisionShape* colShape = new btBoxShape(bt(size));
		// btCollisionShape* colShape = new btSphereShape(btScalar(0.25f));
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
		body->setFriction(1.0f);
		// body->setDamping(0.8f, 1.0f);
		// body->setRestitution(.8);
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

	void initGeom(const std::string& resourceDirectory)
	{
		string rDir = resourceDirectory + "/";

		//load geometry, initialize meshes, create objects
		objL["cube"] = GameObject::create(rDir, "cube.obj", "cube");
		objL["sphere"] = GameObject::create(rDir + "general/", "waterball.dae", "sphere");
		objL["ps2"] = GameObject::create(rDir + "melee/ps2/", "ps2.dae", "ps2");
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
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(proj));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(view));
		vec3 vd = camera.lookAtPoint - camera.eye;
		glUniform3f(prog->getUniform("viewDirection"), vd.x, vd.y, vd.z);
		setLight(prog);
	}
	
	void checkAbilities() {
		//launch projectile
		if (leftMouse && player1->projectileCooldown <= 0.0f) {
			player1->projectileCooldown = 1.0f;
			createProjectile(projectiles, player1->location + player1->getForwardDir() + vec3(0,.5,0), player1->getForwardDir(), 10.f);
		}
	}

	//render player in correct position & animation
	void setPlayer() {
		objL["animModel"]->translate(player1->location - vec3(0,.1,0));
		objL["animModel"]->scale(vec3(0.03, 0.03, 0.03));
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

		if (player1->isGrounded) {
			objL["animModel"]->doAnimation(0);
		}
		else {
			objL["animModel"]->doAnimation(3);
		}
	}

	void setBoko() {
		btTransform trans;
		vec3 physicsLoc;
		bokoBody->forceActivationState(1);
		bokoBody->getMotionState()->getWorldTransform(trans);
		physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
		objL["boko"]->translate(physicsLoc - vec3(0,.2,0));
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
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1];
		btRigidBody* body = btRigidBody::upcast(obj);
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);
		vec3 physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
		btQuaternion btQ = body->getOrientation();

		//draw sphere
		// setMaterial(1, currentShader);
		objL["sphere"]->translate(physicsLoc); //+ vec3(0,.9,0));
		objL["sphere"]->rotate(btQ.getAngle(), cons(btQ.getAxis()));
		objL["sphere"]->setModel(currentShader);
		objL["sphere"]->draw(currentShader); 

		//draw ps2
		objL["ps2"]->translate(vec3(5,-3.15f,0));
		objL["ps2"]->scale(vec3(.35f, .35f, .35f));
		objL["ps2"]->rotate(-PI/2, vec3(1.f, 0.f, 0.f));
		// setMaterial(5, currentShader);
		objL["ps2"]->setModel(currentShader);
		objL["ps2"]->draw(currentShader);

		//renderProjectiles
		renderProjectiles(currentShader, objL, projectiles);
	}

	void drawAnim() {
	}
	/* TODO fix */
	mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
		float edge = 15.f;
		mat4 ortho = glm::ortho(-edge, edge, -edge, edge, 0.1f, 2*edge);
		//fill in the glUniform call to send to the right shader!
		glUniformMatrix4fv(curShade->getUniform("LP"), 1, GL_FALSE, value_ptr(ortho));
		return ortho;
	}

	/* TODO fix */
	mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up) {
		mat4 Cam = glm::lookAt(pos, LA, up);
		glUniformMatrix4fv(curShade->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
		//fill in the glUniform call to send to the right shader!
		return Cam;
	}

	void render() {
		// Get current frame buffer size & dt
		float dt = getDeltaTimeSeconds();
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

		//shadow Data
		mat4 LP, LV, LS;
		vec3 lightPos = vec3(5,3,-4);
		vec3 lightLA = lightPos + vec3(-4,-3,-20);
		vec3 lightUp = vec3(0,1,0);

		//generate shadow map
		if (SHADOW) {
			//set up light's depth map
			glViewport(0, 0, S_WIDTH, S_WIDTH);

			//sets up the output to be out FBO
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);

			//set up shadow shader and render the scene
			DepthProg->bind();
			LP = SetOrthoMatrix(DepthProg);
			LV = SetLightView(DepthProg, lightPos, lightLA, lightUp);
			LS = LP*LV;
			drawObjects(DepthProg);
			DepthProg->unbind();

			//set culling back to normal
			glCullFace(GL_BACK);

			//this sets the output back to the screen
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		
		// DepthProgDebug->bind();
		// 	//render scene from light's point of view
		// 	SetOrthoMatrix(DepthProgDebug);
		// 	SetLightView(DepthProgDebug, lightPos, lightLA, lightUp);
		// 	drawObjects(DepthProgDebug);
		// DepthProgDebug->unbind();

		//update player
		player1->update(dt);
		player1->move(dt, playerBody, dynamicsWorld);
		checkAbilities();

		/* bind & initialize standard program */
		prog->bind();
			setMaterial(5, prog);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			glUniform1i(prog->getUniform("shadowDepth"), 1);
			sendUniforms(prog, Projection->topMatrix(), camera.getViewMatrix());
			drawObjects(prog);
		prog->unbind();

		// animProg->bind();
		// 	sendUniforms(animProg, Projection->topMatrix(), camera.getViewMatrix());
		// 	setPlayer();
		// 	objL["animModel"]->draw(animProg);
		// 	setBoko();
		// 	objL["boko"]->draw(animProg);
		// animProg->unbind();

		player1->updateLocation(playerBody);

		// Pop matrix stacks.
		Projection->popMatrix();

		//step physics simulation
		dynamicsWorld->stepSimulation(dt);

		leftMouse = false;
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
	// windowManager->init(1520, 1080);
	windowManager->init(1024, 1024);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initPhysics();

	// glfwSetWindowMonitor(windowManager->getHandle(), glfwGetPrimaryMonitor(), 0, 0, 2650, 1440, 144);

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
