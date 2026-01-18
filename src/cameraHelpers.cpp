#include "cameraHelpers.h"
#include "app.h"
std::vector<glm::vec3> getCameraFrustumCorners(const Camera& cam) {
    std::vector<glm::vec3> corners;
    glm::mat4 invVP = glm::inverse(cam.getVP());

    std::vector<glm::vec4> ndcCorners = {
        {-1.0f, -1.0f, -1.0f, 1.0f}, // Near Bottom Left
        { 1.0f, -1.0f, -1.0f, 1.0f}, // Near Bottom Right
        { 1.0f,  1.0f, -1.0f, 1.0f}, // Near Top Right
        {-1.0f,  1.0f, -1.0f, 1.0f}, // Near Top Left
        {-1.0f, -1.0f,  1.0f, 1.0f}, // Far Bottom Left
        { 1.0f, -1.0f,  1.0f, 1.0f}, // Far Bottom Right
        { 1.0f,  1.0f,  1.0f, 1.0f}, // Far Top Right
        {-1.0f,  1.0f,  1.0f, 1.0f}  // Far Top Left
    };

    for (const auto& corner : ndcCorners) {
        glm::vec4 worldPos = invVP * corner;
        worldPos /= worldPos.w; // Perspective divide
        corners.push_back(glm::vec3(worldPos));
    }

    return corners;
}

void getCameraFrustumCorners(const Camera& cam, float nearPlane, float farPlane, std::vector<glm::vec3>& outCorners) {
    outCorners.clear();
    App& app = App::instance();
    float aspectRatio = static_cast<float>(app.width) / static_cast<float>(app.height);
    glm::mat4 proj = glm::perspective(
        glm::radians(cam.fovY),
        aspectRatio,
        nearPlane,
        farPlane
    );
    glm::mat4 invVP = glm::inverse(proj * cam.getView());

    const glm::vec4 ndcCorners[8] = {
        {-1, -1, -1, 1}, { 1, -1, -1, 1},
        { 1,  1, -1, 1}, {-1,  1, -1, 1},
        {-1, -1,  1, 1}, { 1, -1,  1, 1},
        { 1,  1,  1, 1}, {-1,  1,  1, 1},
    };

    for (const auto& c : ndcCorners) {
        glm::vec4 world = invVP * c;
        outCorners.push_back(glm::vec3(world) / world.w);
    }
}