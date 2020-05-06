#include "WindowManager.h"
#include "InputHandler.h"


// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace std;
using namespace glm;

InputHandler::InputHandler() {
}

void InputHandler::init() {
        this->Wflag = false;
        this->Sflag = false;
        this->Aflag = false;
        this->Dflag = false;
        this->Cflag = false;
        this->Spaceflag = false;
        this->Ctrlflag = false;
        this->Shiftflag = false;
	    this->lightX = .4;
	    this->up_time = 0;
		this->Upflag = false;
        this->Leftflag = false;
        this->Rightflag = false;
		this->Downflag = false;

		this->kp5 = false;
        this->kp1 = false;
        this->kp2 = false;
		this->kp3 = false;
		this->R = false;
		this->Q = false;

		// controllerPresent = glfwJoystickIsGamepad(GLFW_JOYSTICK_1);
		// if (controllerPresent)
		// 	std::cout << "controller is connected!" << std::endl;
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
		if (key == GLFW_KEY_C && action == GLFW_PRESS) { Cflag = true; }
		if (key == GLFW_KEY_C && action == GLFW_RELEASE) { Cflag = false; }
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { 
			Spaceflag = true; 
			if (!jump)
				space_start_time = glfwGetTime();
			jump = true;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) { 
			Spaceflag = false; 
			space_time = 0;
		}
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) { Ctrlflag = true; }
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) { Ctrlflag = false; }
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) { 
			Shiftflag = true; 
			shift_start_time = glfwGetTime();
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) { 
			Shiftflag = false; 
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS) { 
			Upflag = true; 
		}
		if (key == GLFW_KEY_UP && action == GLFW_RELEASE) { 
			Upflag = false; 
			up_time = 0;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) { 
			Downflag = true; 
			down_start_time = glfwGetTime();
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) { 
			Downflag = false; 
			up_time = 0;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) { 
			Leftflag = true; 
			left_start_time = glfwGetTime();
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) { 
			Leftflag = false; 
			left_time = 0;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { 
			Rightflag = true; 
			right_start_time = glfwGetTime();
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) { 
			Rightflag = false; 
			right_time = 0;
		}

		if (key == GLFW_KEY_KP_5 && action == GLFW_PRESS) { kp5 = true;}
		if (key == GLFW_KEY_KP_5 && action == GLFW_RELEASE) { kp5 = false;}
		if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS) { kp2 = true;}
		if (key == GLFW_KEY_KP_2 && action == GLFW_RELEASE) { kp2 = false;}
		if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS) { kp1 = true;}
		if (key == GLFW_KEY_KP_1 && action == GLFW_RELEASE) { kp1 = false;}
		if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS) { kp3 = true;}
		if (key == GLFW_KEY_KP_3 && action == GLFW_RELEASE) { kp3 = false;}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) { Q = true; }
		if (key == GLFW_KEY_Q && action == GLFW_RELEASE) { Q = false; }
		if (key == GLFW_KEY_R && action == GLFW_PRESS) { R = true; }
		if (key == GLFW_KEY_R && action == GLFW_RELEASE) { R = false; }
		if (key == GLFW_KEY_E && action == GLFW_PRESS) { E = true; }
		if (key == GLFW_KEY_E && action == GLFW_RELEASE) { E = false; }
}