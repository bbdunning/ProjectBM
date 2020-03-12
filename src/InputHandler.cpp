#include "WindowManager.h"
#include "InputHandler.h"


// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

InputHandler::InputHandler() {
        bool Wflag = false;
        bool Sflag = false;
        bool Aflag = false;
        bool Dflag = false;
        bool Spaceflag = false;
        bool Ctrlflag = false;
        bool Shiftflag = false;
	    float lightX = .4;
}

void InputHandler::setKeyFlags(int key, int action) {
		if (key == GLFW_KEY_A && action == GLFW_PRESS) { Aflag = true; }
		if (key == GLFW_KEY_A && action == GLFW_RELEASE) { Aflag = false; }
		if (key == GLFW_KEY_D && action == GLFW_PRESS) { Dflag = true; }
		if (key == GLFW_KEY_D && action == GLFW_RELEASE) { Dflag = false; }
		if (key == GLFW_KEY_W && action == GLFW_PRESS) { Wflag = true; }
		if (key == GLFW_KEY_W && action == GLFW_RELEASE) { Wflag = false; }
		if (key == GLFW_KEY_S && action == GLFW_PRESS) { Sflag = true; }
		if (key == GLFW_KEY_S && action == GLFW_RELEASE) { Sflag = false; }
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { Spaceflag = true; }
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) { Spaceflag = false; }
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) { Ctrlflag = true; }
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) { Ctrlflag = false; }
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) { Shiftflag = true; }
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) { Shiftflag = false; }
		if (key == GLFW_KEY_Q ) { lightX -= 0.3; }
		if (key == GLFW_KEY_E ) { lightX += 0.3; }
		if (key == GLFW_KEY_UP && action == GLFW_PRESS) { Upflag = true; }
		if (key == GLFW_KEY_UP && action == GLFW_RELEASE) { Upflag = true; }
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) { Downflag = true; }
		if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) { Downflag = true; }
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) { Leftflag = true; }
		if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) { Leftflag = true; }
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { Rightflag = true; }
		if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) { Rightflag = true; }
}