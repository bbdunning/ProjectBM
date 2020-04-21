#include <memory>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

void Camera::init() {
	eye = vec3(0,0,5);
	lookAtPoint = vec3(
		cos(phi)*cos(theta),
		sin(phi),
		cos(phi)*cos((PI/2.0)-theta));
	lookAtOffset = vec3(0,0,0);
	up = vec3(0,1,0);
}