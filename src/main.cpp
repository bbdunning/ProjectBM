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
#include "Animation/Animator.h"
#include "Camera.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
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

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	shared_ptr<Texture> texture_glass;

	shared_ptr<InputHandler> inputHandler = make_shared<InputHandler>();
	shared_ptr<Player> player1 = make_shared<Player>();
	
	//animation data
	float sTheta = 0;
	int m = 1;

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

	void setMaterial(int i, shared_ptr<Program> prog) {  
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

	void setLight(shared_ptr<Program> prog) {
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
		createGameObject(rDir + "melee/totodile/", "toto.dae", "totodile");
		createGameObject(rDir + "melee/fod/", "fountain.fbx", "FoD");
		createGameObject(rDir + "melee/fod/", "skyring1.fbx", "skyring1");
		createGameObject(rDir + "melee/fod/", "skyring2.fbx", "skyring2");
		createGameObject(rDir + "terrain/", "moon.fbx", "moon");
/* 		createGameObject(rDir + "melee/", "pikachu.obj", "pikachu"); */
//		createGameObject("/home/bbdunning/Desktop/Wii - Super Smash Bros Brawl - Captain Falcon/", "falcon.fbx", "falcon");
		// createGameObject(rDir + "melee/falcon2/", "Captain Falcon.dae", "falcon");
		createGameObject(rDir + "anim/", "model.dae", "animModel");
	}
	
	unsigned int jointCount=0;

	//creates joings and builds map of all joints
	void populateJointMap(shared_ptr<map<string, Joint>> jointMap, aiNode *node, const aiScene* scene) {
		for (int i=0; i<scene->mNumMeshes; i++) {
			aiMesh *mesh = scene->mMeshes[i];
			for (int j=0; j<mesh->mNumBones;j++) {
				aiBone *bone = mesh->mBones[j];
				string boneName = bone->mName.C_Str();
				if (jointMap->find(boneName) == jointMap->end()) {
					(*jointMap)[boneName] = Joint(jointCount++, boneName, make_shared<mat4>(mat4_cast(bone->mOffsetMatrix)));
				}
			}
		}
	}

	//adds children to Joints
	void buildJointHeirarchy(shared_ptr<map<string, Joint>> jointMap, aiNode *node, const aiScene* scene) {
		for (int i=0; i< node->mNumChildren; i++) {
			if (jointMap->find(node->mChildren[i]->mName.C_Str()) != jointMap->end()) {
				(*jointMap)[node->mName.C_Str()].children.push_back(&(*jointMap)[node->mChildren[i]->mName.C_Str()]);
				&(*jointMap)[node->mChildren[i]->mName.C_Str()];
			}
			buildJointHeirarchy(jointMap, node->mChildren[i], scene);
		}
	}

	Joint* getRootJoint(shared_ptr<map<string, Joint>> jointMap, aiNode *node) {
		if (node == nullptr || jointMap == nullptr) {
			return nullptr;
		}
		if (jointMap->find(node->mName.C_Str()) != jointMap->end()) {
			return &(*jointMap)[node->mName.C_Str()];
		}
		// for (int i=0; i< node->mNumChildren; i++) {
		// 	if (jointMap->find(node->mChildren[i]->mName.C_Str()) != jointMap->end()) {
		// 		return &(*jointMap)[node->mChildren[i]->mName.C_Str()];
		// 	}
		// }
		for (int i=0; i<node->mNumChildren; i++) {
			cout << "node name: " << node->mChildren[i]->mName.C_Str() << endl;
			string childname = node->mChildren[i]->mName.C_Str();
			if (childname == "Armature"){
				return getRootJoint(jointMap, node->mChildren[i]);
			}
		}
		return nullptr;
	}

	void printJoints(Joint *j)
	{
		if (j == nullptr) {
			cout << "nothing" << endl;
			return;
		}
		cout << "joint name: " << j->name << endl;
		for (int i=0;i<j->children.size();i++) {
		// if (j->name != "")
				// cout << to_string(*(j->localBindTransform)) << endl;
			printJoints((j->children[i]));
		}
	}

	void printAllJoints(shared_ptr<map<string, Joint>> jointMap) {
		for (map<string,Joint>::iterator it=jointMap->begin(); it != jointMap->end(); ++it) {
			Joint *j = &it->second;
			cout << it->first << " has children: " << j->children.size() << endl;
			for (int q=0; q<j->children.size();q++) {
				cout << "   child: " << j->children[q]->name.c_str() << endl;
			}
		}
	}

	shared_ptr<AnimatedShape> createShape(const aiScene * scene, string meshPath, 
		string fileName, string objName, shared_ptr<GameObject> obj, int i, Joint *rootJoint, shared_ptr<map<string, Joint>> jointMap) {
		shared_ptr<AnimatedShape> newShape;
		aiString* texPath;


		newShape = make_shared<AnimatedShape>();
		if (scene->mMeshes[i]->mNumBones > 0) {
			aiBone* j = scene->mMeshes[i]->mBones[0];
			newShape->isAnimated = true;
		} else {
			newShape->isAnimated = false;
		}
		texPath = new aiString();
		newShape->scene = scene;
		newShape->jointMap = jointMap;
		newShape->createShape(scene->mMeshes[i]);
		newShape->name = scene->mMeshes[i]->mName.C_Str();
		newShape->measure();
		//fix rootJoint
		newShape->init(rootJoint);
		string temp;

		//load texture path into texPath
		scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, texPath);
		temp = texPath->C_Str();

		//if mesh has texture, create a texture from it
		if (texPath->C_Str() != "" and texPath->C_Str() != "/" and texPath->length != 0 and 
			((texPath->length > 5) and (temp != "none"))) {
			// cout << "   " << objName << " has texture. Texture path: " << texPath->C_Str() << endl;
			if (texPath->C_Str()[0] != '/') {
				newShape->texture = createTexture(meshPath + texPath->C_Str());
			} else
				newShape->texture = createTexture(texPath->C_Str());
		}
		// mesh->shapeList.push_back(createShape(scene, meshPath, fileName, objName, mesh, i));
		return newShape;
	}

	Animation createAnimation(const aiAnimation *anim) {
		vector<KeyFrame> frames;
		shared_ptr<map<string, JointTransform>> pose = make_shared<map<string, JointTransform>>();
		map<double, shared_ptr<map<string, JointTransform>>> timePosMap;
		float time = 999999999;
		//
		for (int j=0; j<anim->mNumChannels; j++) {
			aiNodeAnim *anode = anim->mChannels[j];
			//
			for (int k=0; k<anode->mNumPositionKeys; k++) {
				time = anode->mPositionKeys[k].mTime;
				vec3 position = vec3_cast(anode->mPositionKeys[k].mValue);
				quat rotation = quat_cast(anode->mRotationKeys[k].mValue);
				if (timePosMap.find(time) == timePosMap.end()) {
					timePosMap[time] = pose;
					pose = make_shared<map<string, JointTransform>>(); //pose maps joint transforms to joint names
				}
				else {
					(*timePosMap[time])[anode->mNodeName.C_Str()] = JointTransform(position, rotation);
				}
			}
		}
		for (map<double, shared_ptr<map<string, JointTransform>>>::iterator it = timePosMap.begin(); it != timePosMap.end(); ++it)
			frames.push_back(KeyFrame(it->first, *(it->second)));
		cout << timePosMap.size() << endl;
		return Animation((float) (anim->mDuration*anim->mTicksPerSecond), frames);
	}

	void createAnimations(const aiScene *scene, vector<Animation> &animList) {
		for (int i=0; i<scene->mNumAnimations; i++) {
			animList.push_back(createAnimation(scene->mAnimations[i]));
		}
	}

	void printAnimations(vector<Animation> &animList) {
		for (int i=0; i<animList.size(); i++) {
			cout << "animation " << i << " length: " << animList[i].length << endl;
			for (int j=0; j<animList[i].frames.size(); j++) {
				cout << "keyframe " << j << " has " << animList[i].frames[j].pose.size() << " joints" << " timeStamp: " << animList[i].frames[j].timeStamp << endl;
				for (map<string, JointTransform>::iterator it = animList[i].frames[j].pose.begin(); it != animList[i].frames[j].pose.end(); ++it) {
					cout << "joint " << it->first << endl;
				}
			}
		}
	}

	void createGameObject(string meshPath, string fileName, string objName) {
		Assimp::Importer importer;
        shared_ptr<GameObject> mesh = make_shared<GameObject>();
		shared_ptr<map<string, Joint>> jointMap = make_shared<map<string, Joint>>();
		mesh->name = objName;
		Joint *rootJoint = nullptr;
		vector<Animation> animList; 

		const aiScene* scene = importer.ReadFile(
			meshPath + fileName, aiProcess_Triangulate | aiProcess_FlipUVs);

		cout << "creating " << objName << endl;
		populateJointMap(jointMap, scene->mRootNode, scene);
		// cout << "jointMap size: " << jointMap->size() << endl;
		buildJointHeirarchy(jointMap, scene->mRootNode, scene);
		createAnimations(scene, animList);
		// printAnimations(animList);
		// printAllJoints(jointMap);


		if (jointMap->size() > 0) {
			rootJoint = getRootJoint(jointMap, scene->mRootNode)->children[0];
			// printJoints(rootJoint);
		}

		for (int i=0; i< scene->mNumMeshes; i++) {
			mesh->shapeList.push_back(createShape(scene, meshPath, fileName, objName, mesh, i, rootJoint, jointMap));
		}

		objL[objName] = mesh;
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

		//draw totodile player
		Model->pushMatrix();
			player1->update();
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
				setMaterial(0, prog);
			else if (player1->isGrounded)
				setMaterial(1, prog);
			else if (!player1->isGrounded)
				setMaterial(3, prog);

			Model->scale(vec3(.025, .025, .025));
			// cout << camera.checkInFrustum(Projection->topMatrix()*camera.getViewMatrix(), vec4(player1->location, 1)) << endl;;
			setModel(prog, Model);
			objL["totodile"]->draw(prog); 
		Model->popMatrix();

		//Main Stage
		Model->pushMatrix();
			Model->translate(vec3(0, -1, -2));
			Model->scale(vec3(0.03, 0.03, 0.03));
			setMaterial(3, prog);
			setModel(prog, Model);
			objL["FoD"]->draw(prog);
		Model->popMatrix();

		//Skyring 1
		Model->pushMatrix();
			Model->translate(vec3(-2.4, 2, -2));
			Model->rotate(.1, vec3(1,0,0));
			Model->rotate(2*sin(.2*glfwGetTime()), vec3(0,0,1));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setMaterial(1, prog);
			setModel(prog, Model);
			objL["skyring1"]->draw(prog);
		Model->popMatrix();
		//Skyring2
		Model->pushMatrix();
			Model->translate(vec3(-2.8, 2.1, -.26));
			Model->rotate(-2*sin(.2*glfwGetTime()), vec3(0,0,1));
			Model->rotate(.3, vec3(1,0,0));
			Model->scale(vec3(0.2, 0.2, 0.2));
			setModel(prog, Model);
			objL["skyring2"]->draw(prog);
		Model->popMatrix();


		//draw Captain Falcon
		Model->pushMatrix();
			// getPlayerDisplacement();
			Model->rotate(-PI/2, vec3(1, 0, 0));
			Model->rotate(-PI/2, vec3(0, 0, 1));
			Model->scale(vec3(0.035, 0.035, 0.035));
			setMaterial(1, prog);
			setModel(prog, Model);
			// objL["falcon"]->draw(prog);
		Model->popMatrix();

		Model->pushMatrix();
			// getPlayerDisplacement();
			Model->rotate(-PI/2, vec3(1, 0, 0));
			Model->rotate(-PI/2, vec3(0, 0, 1));
			Model->scale(vec3(0.035, 0.035, 0.035));
			setMaterial(1, prog);
			setModel(prog, Model);
			objL["animModel"]->draw(prog);
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
