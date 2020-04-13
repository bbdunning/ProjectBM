#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <memory>
#include "Material.h"
#include "Program.h"

void Material::setMaterial(std::shared_ptr<Program> prog) {
    glUniform3f(prog->getUniform("MatAmb"), matAmb.x, matAmb.y, matAmb.z);        
/*     glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);        */
    glUniform3f(prog->getUniform("MatSpec"), matSpec.x, matSpec.y, matSpec.z);       
    glUniform1f(prog->getUniform("shine"), shine);
}
