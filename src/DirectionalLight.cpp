#include "DirectionalLight.h"
#include "Object.h"

glm::vec3 DirectionalLight::getDirection() const {
    // forward = -Z
    return owner->transform->forward();
}

void DirectionalLight::updateLightMatrices(const glm::vec3& sceneCenter) {
    glm::vec3 dir = glm::normalize(getDirection());


    glm::vec3 lightPos = sceneCenter - dir * 100.0f;

    glm::vec3 up = owner->transform->up();
    lightView = glm::lookAt(
        lightPos,
        sceneCenter,
        up
    );

    lightProj = glm::ortho(
        -shadowSize, shadowSize,
        -shadowSize, shadowSize,
        shadowNear,
        shadowFar
    );

    lightVP = lightProj * lightView;
}

void DirectionalLight::updateLightMatrices(const std::vector<glm::vec3>& frustumCorners) {
    glm::vec3 dir = glm::normalize(getDirection());

    // Compute the center of the frustum
    glm::vec3 frustumCenter(0.0f);
    for (const auto& corner : frustumCorners) {
        frustumCenter += corner;
    }
    frustumCenter /= static_cast<float>(frustumCorners.size());

    glm::vec3 lightPos = frustumCenter - dir * shadowFar;

    glm::vec3 up = owner->transform->up();
    // glm::vec3 up = glm::vec3(1.0f, 0.0f, 0.0f);
    lightView = glm::lookAt(
        lightPos,
        frustumCenter,
        up
    );

    // Find the AABB of the frustum corners in light space
    glm::vec3 minBounds( std::numeric_limits<float>::max());
    glm::vec3 maxBounds(-std::numeric_limits<float>::max());

    for (const auto& corner : frustumCorners) {
        glm::vec4 lightSpaceCorner = lightView * glm::vec4(corner, 1.0f);
        minBounds = glm::min(minBounds, glm::vec3(lightSpaceCorner));
        maxBounds = glm::max(maxBounds, glm::vec3(lightSpaceCorner));
    }

    float worldUnitsPerTexel =
    (maxBounds.x - minBounds.x) / shadowResolution;

    glm::vec3 centerLS = (minBounds+ maxBounds) * 0.5f;

    centerLS.x = floor(centerLS.x / worldUnitsPerTexel) * worldUnitsPerTexel;
    centerLS.y = floor(centerLS.y / worldUnitsPerTexel) * worldUnitsPerTexel;

    float halfW = (maxBounds.x - minBounds.x) * 0.5f;
    float halfH = (maxBounds.y - minBounds.y) * 0.5f;

    float left   = centerLS.x - halfW;
    float right  = centerLS.x + halfW;
    float bottom = centerLS.y - halfH;
    float top    = centerLS.y + halfH;

    // minBounds.x = floor(minBounds.x / worldUnitsPerTexel) * worldUnitsPerTexel;
    // maxBounds.x = minBounds.x + worldUnitsPerTexel * shadowResolution;

    // worldUnitsPerTexel = (maxBounds.y - minBounds.y) / shadowResolution;    
    // minBounds.y = floor(minBounds.y / worldUnitsPerTexel) * worldUnitsPerTexel;
    // maxBounds.y = minBounds.y + worldUnitsPerTexel * shadowResolution;

    lightProj = glm::ortho(
        left, right,
        bottom, top,
        -maxBounds.z, -minBounds.z
    );

    lightVP = lightProj * lightView;
}