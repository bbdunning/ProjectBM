#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "Program.h"

class Material
{
    public:
        glm::vec3 matSpec;
        glm::vec3 matAmb;
        float shine;

        void setMaterial(std::shared_ptr<Program> prog);
};