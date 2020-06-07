#pragma once
#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM__H 

#include <string>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

class ParticleSystem
{
public:
    float pps;
    float speed;
    float gravityComplient;
    int initialNum;
    float lifeLength;
    float timeElapsed = 0.0f;
    std::vector<Particle> particles;
    glm::vec3 systemCenter;
    std::shared_ptr<GameObject> obj;

    ParticleSystem(glm::vec3 position, float pps, int initialNum, float lifeLength, std::shared_ptr<GameObject> obj) {
         this->pps = pps;
         this->lifeLength = lifeLength;
         this->initialNum = initialNum;
         this->systemCenter = position;
         this->obj = obj;
    }

    void generateParticles(float dt) {
        float particlesToCreate = pps * dt;
        int count = floor(particlesToCreate);
        float partialParticle = fmod(particlesToCreate, 1);
        for (int i = 0; i < count; i++) {
            emitParticle();
        }
        if (fmod((float) rand()/100.f, 1) < partialParticle) {
            emitParticle();
        }
    }

    void emitParticle() {
        glm::vec3 emit_direction = glm::vec3(fmod((float)rand()/100.0f, 1) - .5, fmod((float)rand()/100.0f, 1) - .5, fmod((float)rand()/100.0f, 1) - .5);
        float speed = 4;
        glm::vec3 velocity = emit_direction * speed;
		particles.push_back(Particle(systemCenter, velocity, 0, 1 + fmod((float) rand()/100.0f, 1)*.8 - .4, 0, .1f));
    }

    void update(float dt) {
        if (timeElapsed == 0) {
            for (int i=0; i<initialNum; i++) {
                emitParticle();
            }
        }
        generateParticles(dt);
        for (int i=0; i < particles.size(); i++) {
            particles[i].update(dt);
        }
        cullParticles();
        timeElapsed += dt;
    }

    void cullParticles() {
        for (int i=0; i < particles.size(); i++) {
            if (particles[i].elapsedTime >= particles[i].life)
                particles.erase(particles.begin() + i);
        }
    }

};

#endif