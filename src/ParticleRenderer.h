#pragma once
#ifndef PARTICLE_RENDERER_H 
#define PARTICLE_RENDERER_H

#include <string>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "Program.h"
#include "Particle.h"
#include "Shape.h"

class ParticleRenderer
{
public:
    std::shared_ptr<Program> shader;
    // GLuint quad_vertexArrayID;
    // GLuint quad_vertexBuffer;

    ParticleRenderer(std::shared_ptr<Program> prog) {
        this->shader = prog;
    }

    void render(std::shared_ptr<Program> shader, std::vector<Particle> &particles, std::shared_ptr<GameObject> obj, 
        const glm::mat4 &viewMatrix) {
        shader->bind();
        for (Particle p : particles) {
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.f), p.position);
            modelMatrix[0][0] = viewMatrix[0][0];
            modelMatrix[0][1] = viewMatrix[1][0];
            modelMatrix[0][2] = viewMatrix[2][0];
            modelMatrix[1][0] = viewMatrix[0][1];
            modelMatrix[1][1] = viewMatrix[1][1];
            modelMatrix[1][2] = viewMatrix[2][1];
            modelMatrix[2][0] = viewMatrix[0][2];
            modelMatrix[2][1] = viewMatrix[1][2];
            modelMatrix[2][2] = viewMatrix[2][2];
            modelMatrix *= glm::scale(glm::vec3(p.scale));
            glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(viewMatrix));
            obj->draw(shader);
        }
        shader->unbind();
    }

    void prepare() {

    }

    // void updateModelViewMatrix(glm::vec3 position, float rotation, float scale, glm::mat4 viewMatrix) {
    //     glm::mat4 modelMatrix = glm::mat4(1);
    //     glm::mat4 inv = transpose(modelMatrix);
    // }

//     void initQuad() {
//         glGenVertexArrays(1, &quad_vertexArrayID);
//         glBindVertexArray(quad_vertexArrayID);
//         static const GLfloat g_quad_vertex_buffer_data[] = {
//             -1.0f, -1.0f, 0.0f,
//             1.0f, -1.0f, 0.0f,
//             -1.0f,  1.0f, 0.0f,
//             -1.0f,  1.0f, 0.0f,
//             1.0f, -1.0f, 0.0f,
//             1.0f,  1.0f, 0.0f,
//         };
//         glGenBuffers(1, &quad_vertexBuffer);
//         glBindBuffer(GL_ARRAY_BUFFER, quad_vertexBuffer);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
//   }
};

#endif