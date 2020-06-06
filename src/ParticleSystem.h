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
    float lifeLength;
    std::vector<Particle> particles;
    glm::vec3 systemCenter;

    ParticleSystem(glm::vec3 position, float pps, float lifeLength) {
         this->pps = pps;
         this->lifeLength = lifeLength;
         this->systemCenter = position;
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
		particles.push_back(Particle(systemCenter, glm::vec3(0,1,0), 0, 1, 0, .35f));
    }

    void update(float dt) {
        generateParticles(dt);
        for (int i=0; i < particles.size(); i++) {
            particles[i].update(dt);
        }
        cullParticles();
        std::cout << particles.size() << std::endl;
    }

    void cullParticles() {
        for (int i=0; i < particles.size(); i++) {
            if (particles[i].elapsedTime >= particles[i].life)
                particles.erase(particles.begin() + i);
        }
    }

};

#endif