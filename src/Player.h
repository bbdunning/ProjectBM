#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Player {
public:
    vec3 velocity;
    vec3 location;

    Player();
    int update();
};