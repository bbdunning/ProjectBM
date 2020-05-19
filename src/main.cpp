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
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/LinearMath/btAabbUtil2.h>

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
	float previousTime = 0.0f;

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new	btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btTransform groundTransform;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	shared_ptr<InputHandler> inputHandler = make_shared<InputHandler>();
	shared_ptr<CollisionDetector> cd = make_shared<CollisionDetector>();
	shared_ptr<Player> player1 = make_shared<Player>();
	shared_ptr<Sandbag> sandbag = make_shared<Sandbag>();
	vector<btRigidBody*> projectiles;
	btRigidBody* playerBody;
	btRigidBody* bokoBody;
	
	//animation data
	float sTheta = 0;
	int m = 1;
	float prevOmega = 0.0f;
	bool leftMouse = false;

	//Camera
	Camera camera;

	//skybox
	unsigned int skyboxTextureId = 0;
	// vector<std::string> faces {           
	// 	"Newdawn1_right.png",           
	// 	"Newdawn1_left.png",           
	// 	"Newdawn1_up.png",           
	// 	"Newdawn1_down.png",           
	// 	"Newdawn1_back.png",           
	// 	"Newdawn1_front.png"};
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
		// glUniform3f(prog->getUniform("LightPos"), .5, .5, 0);
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

	void initPhysics() {
		dynamicsWorld->setGravity(btVector3(0,-10,0));

		///-----initialization_end-----
		///create a few basic rigid bodies
		//keep track of the shapes, we release memory at exit.
		//make sure to re-use collision shapes among rigid bodies whenever possible!

		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(13.f),btScalar(1.f),btScalar(8.f)));
		collisionShapes.push_back(groundShape);

		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(-2,-1,-9));

		{
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
			// body->setFriction(1.0);
			body->setFriction(0.0);
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

			btScalar mass(2.f);

			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			bool isDynamic = (mass != 0.f);

			btVector3 localInertia(0,0,0);
			if (isDynamic)
				colShape->calculateLocalInertia(mass,localInertia);

				startTransform.setOrigin(btVector3(0,10,-1));
			
				//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
				btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
				btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
				btRigidBody* body = new btRigidBody(rbInfo);

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

			btScalar mass(2.f);

			//rigidbody is dynamic if and only if mass is non zero, otherwise static
			bool isDynamic = (mass != 0.f);

			btVector3 localInertia(0,0,0);
			if (isDynamic)
				colShape->calculateLocalInertia(mass,localInertia);

				startTransform.setOrigin(btVector3(0,10,-3));
			
				//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
				btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
				btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
				btRigidBody* body = new btRigidBody(rbInfo);

				dynamicsWorld->addRigidBody(body);
		}
		playerBody = createPlayerRigidBody(vec3(0,5,-5));
		bokoBody = createGeneralRigidBody(vec3(4,5,-5), vec3(.3, .5, .3));
	}

	btRigidBody* createPlayerRigidBody(vec3 location) {
		//create a dynamic rigidbody

		btCollisionShape* colShape = new btBoxShape(btVector3(.25,.25,.25));
		// btCollisionShape* colShape = new btSphereShape(btScalar(0.25f));
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
		body->setFriction(1.0f);
		// body->setDamping(0.8f, 1.0f);
		// body->setRestitution(.8);
		return body;
	}

	btRigidBody* createGeneralRigidBody(vec3 location, vec3 size) {
		//create a dynamic rigidbody

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

	btRigidBody* createRigidBody(vector<btRigidBody*> &projectiles, vec3 location, vec3 direction, float magnitude) {
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

	void renderProjectiles(shared_ptr<Program> prog, unordered_map<string, shared_ptr<GameObject>> &objL, vector<btRigidBody*> &projectiles) {
		setMaterial(1, prog);

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
			objL["sphere"]->setModel(prog);
			objL["sphere"]->draw(prog); 
		}
	}

	void initGeom(const std::string& resourceDirectory)
	{
		string rDir = resourceDirectory + "/";

		//load geometry, initialize meshes, create objects
		objL["cube"] = GameObject::create(rDir, "cube.obj", "cube");
		objL["sphere"] = GameObject::create(rDir + "general/", "waterball.dae", "sphere");
		objL["totodile"] = GameObject::create(rDir + "melee/totodile/", "toto.dae", "totodile");
		platforms["platform"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		platforms["platform2"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		platforms["platform3"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		platforms["platform4"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		platforms["platform5"] = GameObject::create(rDir + "melee/fod/", "platform.fbx", "platform");
		objL["ps2"] = GameObject::create(rDir + "melee/ps2/", "ps2.dae", "ps2");
		objL["sandbag"] = GameObject::create(rDir + "melee/Sandbag/", "sandbag.fbx", "sandbag");

		objL["animModel"] = GameObject::create(rDir + "anim/", "toto.dae", "animModel");
		objL["animModel"]->addAnimation("toto_walk.dae");
		objL["animModel"]->addAnimation("toto_watergun.dae");
		objL["animModel"]->addAnimation("toto_dab.dae");
		objL["animModel"]->addAnimation("toto_jump.dae");
		objL["animModel"]->doAnimation(0);

		objL["boko"] = GameObject::create(rDir + "/anim/", "toto.dae", "boko");
		// objL["boko"]->path = rDir + "/ya_boi/source/";
		objL["boko"]->addAnimation("toto_watergun.dae");
		objL["boko"]->doAnimation(0);
		// objL["animModel"]->addAnimation("toto_run.dae");
		// objL["animModel"]->addAnimation("toto_jump.dae");


		cd->environmentBoxes.push_back(make_shared<AABB>(vec3(-10, -2, -10), vec3(10, 0, 10)));

		platforms["platform"]->location = vec3(-1, .5, -2);
		vec3 *temp = &platforms["platform"]->location;
		platforms["platform"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform"]->hitboxes[0]));

		platforms["platform2"]->location = vec3(1, .5, -2);
		temp = &platforms["platform2"]->location;
		platforms["platform2"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform2"]->hitboxes[0]));

		platforms["platform3"]->location = vec3(0, 1.25, -2);
		temp = &platforms["platform3"]->location;
		platforms["platform3"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform3"]->hitboxes[0]));

		platforms["platform4"]->location = vec3(2, 1.25, -2);
		temp = &platforms["platform4"]->location;
		platforms["platform4"]->hitboxes.push_back(make_shared<AABB>(vec3(-.35f,-0.005,-1.f)+*temp, vec3(.35f,0.05f,1.f)+*temp));
		cd->environmentBoxes.push_back(dynamic_pointer_cast<AABB>(platforms["platform4"]->hitboxes[0]));

		platforms["platform5"]->location = vec3(3, .5, -2);
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

	void render() {
		// Get current frame buffer size.
		int width, height;

		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		float dt = getDeltaTimeSeconds();
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = width/(float)height;

		// Create the matrix stacks
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 10000.0f);

		//draw Skybox
		drawSkybox(Model, Projection);

		/* bind standard program */
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(camera.getViewMatrix()));
		vec3 vd = camera.lookAtPoint - camera.eye;
		glUniform3f(prog->getUniform("viewDirection"), vd.x, vd.y, vd.z);

		//set initial material and Light
		setLight(prog);

		//getPosition of object
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[1];
		btRigidBody* body = btRigidBody::upcast(obj);
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);
		vec3 physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
		btQuaternion btQ = body->getOrientation();

		//draw sandbag
		setMaterial(1, prog);
		objL["sphere"]->translate(physicsLoc); //+ vec3(0,.9,0));
		// objL["sphere"]->scale(.05);
		objL["sphere"]->rotate(-PI/2, vec3(1.f,0.f,0.f));
		objL["sphere"]->rotate(btQ.getAngle(), cons(btQ.getAxis()));
		objL["sphere"]->setModel(prog);
		objL["sphere"]->draw(prog); 


		//getPosition of object
		obj = dynamicsWorld->getCollisionObjectArray()[2];
		body = btRigidBody::upcast(obj);
		body->getMotionState()->getWorldTransform(trans);
		physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
		btQ = body->getOrientation();

		//draw sphere
		setMaterial(1, prog);
		objL["sphere"]->translate(physicsLoc); //+ vec3(0,.9,0));
		// objL["sphere"]->scale(.05);
		objL["sphere"]->rotate(-PI/2, vec3(1.f,0.f,0.f));
		objL["sphere"]->rotate(btQ.getAngle(), cons(btQ.getAxis()));
		objL["sphere"]->setModel(prog);
		objL["sphere"]->draw(prog); 

		//draw ps2
		objL["ps2"]->translate(vec3(5,-3.15f,0));
		objL["ps2"]->scale(vec3(.35f, .35f, .35f));
		objL["ps2"]->rotate(-PI/2, vec3(1.f, 0.f, 0.f));
		setMaterial(1, prog);
		objL["ps2"]->setModel(prog);
		objL["ps2"]->draw(prog);

		if (leftMouse && player1->projectileCooldown <= 0.0f) {
			player1->projectileCooldown = 1.0f;
			createRigidBody(projectiles, player1->location + player1->getForwardDir() + vec3(0,.5,0), player1->getForwardDir(), 10.f);
		}

		playerBody->forceActivationState(1);
		playerBody->setAngularVelocity(bt(vec3(0,0,0)));
		cout << playerBody->isActive() << endl;
		if (inputHandler->Wflag) {
			btVector3 dir = bt(normalize(player1->getForwardDir()));
			float magnitude = 0.2f;
			vec3 v = cons(playerBody->getLinearVelocity());
			// if (length(vec3(v.x, 0, v.z)) < 2)
				playerBody->applyCentralImpulse(dir * magnitude);
		}
		if (inputHandler->Dflag) {
			btVector3 dir = bt(normalize(player1->getRightDir()));
			float magnitude = 0.2f;
			vec3 v = cons(playerBody->getLinearVelocity());
			// if (length(vec3(v.x, 0, v.z)) < 2)
				playerBody->applyCentralImpulse(dir * magnitude);
		}
		if (inputHandler->Aflag) {
			btVector3 dir = bt(normalize(-player1->getRightDir()));
			float magnitude = 0.2f;
			vec3 v = cons(playerBody->getLinearVelocity());
			// if (length(vec3(v.x, 0, v.z)) < 2)
				playerBody->applyCentralImpulse(dir * magnitude);
		}
		if (inputHandler->Sflag) {
			btVector3 dir = bt(normalize(-player1->getForwardDir()));
			float magnitude = 0.2f;
			vec3 v = cons(playerBody->getLinearVelocity());
			// if (length(vec3(v.x, 0, v.z)) < 2)
				playerBody->applyCentralImpulse(dir * magnitude);
		}
		if (inputHandler->Spaceflag) {
			btVector3 dir = bt(vec3(0,1,0));
			float magnitude = 1.f;
			playerBody->applyCentralImpulse(dir * magnitude);
		}


		renderProjectiles(prog, objL, projectiles);


		//draw platforms
		for (map<string, shared_ptr<GameObject>>::iterator it=platforms.begin(); it!=platforms.end(); ++it) {
			it->second->translate(it->second->location);
			it->second->setModel(prog);
			it->second->draw(prog);
		}

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
		player1->update(dt);
		camera.lookAtPoint = player1->location - camera.eye + player1->getRightDir() *.5f + camera.elevation + player1->getForwardDir() * .5f;
		gethitBoxes(player1, playerHitboxes);
		//move this to player class
		float angle = -glm::orientedAngle(normalize(vec3(player1->lookAtPoint.x, 0, player1->lookAtPoint.z)), vec3(1, 0, 0), vec3(0,1,0));

		playerBody->getMotionState()->getWorldTransform(trans);
		physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
		player1->location = physicsLoc;
		objL["animModel"]->translate(player1->location);
		objL["animModel"]->scale(vec3(0.03, 0.03, 0.03));
		objL["animModel"]->rotate(PI/2 + angle, vec3(0, 1, 0));
		objL["animModel"]->rotate(-PI/2, vec3(1, 0, 0));
		if (inputHandler->n1)
			objL["animModel"]->doAnimation(0);
		if (inputHandler->n2)
			objL["animModel"]->doAnimation(1);
		if (inputHandler->n3)
			objL["animModel"]->doAnimation(2);
		if (inputHandler->n4)
			objL["animModel"]->doAnimation(3);
		setMaterial(1, animProg);
		objL["animModel"]->setModel(animProg);
		((shared_ptr<AnimatedShape>) (objL["animModel"]->shapeList[0]))->update();
		glUniformMatrix4fv(animProg->getUniform("jointTransforms"), 50, GL_FALSE, value_ptr(((shared_ptr<AnimatedShape>) (objL["animModel"]->shapeList[0]))->jointTransforms[0]));
		objL["animModel"]->draw(animProg);

		bokoBody->forceActivationState(1);
		bokoBody->getMotionState()->getWorldTransform(trans);
		btQ = body->getOrientation();
		physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
		objL["boko"]->translate(physicsLoc - vec3(0,.25,0));
		objL["boko"]->scale(vec3(0.05, 0.05, 0.05));
		// objL["boko"]->scale(vec3(0.005, 0.005, 0.005));
		// objL["boko"]->rotate(btQ.getAngle(), cons(btQ.getAxis()));
		objL["boko"]->rotate(-PI/2, vec3(0, 1, 0));
		objL["boko"]->rotate(-PI/2, vec3(1, 0, 0));
		objL["boko"]->doAnimation(0);
		setMaterial(1, animProg);
		objL["boko"]->setModel(animProg);
		((shared_ptr<AnimatedShape>) (objL["boko"]->shapeList[0]))->update();
		glUniformMatrix4fv(animProg->getUniform("jointTransforms"), 50, GL_FALSE, value_ptr(((shared_ptr<AnimatedShape>) (objL["boko"]->shapeList[0]))->jointTransforms[0]));
		objL["boko"]->draw(animProg);
		animProg->unbind();

		//animation update example
		sTheta = sin((float)glfwGetTime());

		// Pop matrix stacks.
		Projection->popMatrix();

		playerHitboxes.clear();

		dynamicsWorld->stepSimulation(dt);
		for (int i=0; i<projectiles.size(); i++) {
			btCollisionObject* obj = projectiles[i];
			btRigidBody* body = btRigidBody::upcast(obj);
		}
		// GLDebugDrawer debugDrawer;
		// dynamicsWorld->debugDrawWorld();
		
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
	windowManager->init(1280, 900);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initPhysics();

	// glfwSetWindowMonitor(windowManager->getHandle(), glfwGetPrimaryMonitor(), 0, 0, 2650, 1440, 0);

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
