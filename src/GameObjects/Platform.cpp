#include "Platform.h"
#include <string>

using namespace std;

// std::shared_ptr<Platform> Platform::create(string meshPath, string fileName, string objName) {
//     return (shared_ptr<Platform>) super::create(meshPath, fileName, objName);
// }

shared_ptr<Platform> Platform::create(string meshPath, string fileName, string objName) {
    // shared_ptr<Platform> temp = ((shared_ptr<Platform>) (GameObject::create(meshPath, fileName, objName)));
    shared_ptr<GameObject> temp = GameObject::create(meshPath, fileName, objName);
    shared_ptr<Platform> temp2 = dynamic_pointer_cast<Platform> (temp);
    return temp2;
}