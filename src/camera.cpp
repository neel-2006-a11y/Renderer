#include "camera.h"
#include "Object.h"
#include "transform.h"
#include "app.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

glm::mat4 Camera::getView() const {
    if (!viewDirty)
        return view;

    // World transform of camera
    const Transform* t = owner->transform;

    glm::vec3 position = t->position;
    glm::quat rotation = t->rotation;

    // Camera looks down -Z in OpenGL
    glm::vec3 forward = rotation * glm::vec3(0, 0, -1);
    glm::vec3 up      = rotation * glm::vec3(0, 1,  0);

    view = glm::lookAt(position, position + forward, up);
    viewDirty = false;
    return view;
}

glm::mat4 Camera::getProjection() const {
    if (!projDirty)
        return projection;

    App app = App::instance();
    float aspect = float(app.width)/float(app.height);

    if (orthographic) {
        float h = orthoHeight;
        float w = h * aspect;
        projection = glm::ortho(
            -w, w,
            -h, h,
            nearPlane,
            farPlane
        );
    } else {
        projection = glm::perspective(
            glm::radians(fovY),
            aspect,
            nearPlane,
            farPlane
        );
    }

    projDirty = false;
    return projection;
}

glm::mat4 Camera::getVP() const {
    return getProjection() * getView();
}
