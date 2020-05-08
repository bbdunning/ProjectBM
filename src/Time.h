#pragma once
#ifndef TIME_H
#define TIME_H

float getDeltaTimeMicroseconds() {
    float currentTime = glfwGetTime()/1000;
    float deltaTime = currentTime - previousTime;
    previousTime = currentTime;
    return deltaTime;
}

#endif