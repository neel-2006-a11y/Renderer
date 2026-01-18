#include "DirectionalLightCSM.h"
#include "cameraHelpers.h"
#include <iostream>

glm::vec3 DirectionalLightCSM::getDirection() const {
    return owner->transform->forward();
}

// calculates cascade splits of camera frustum and stores in light's cascades
void computeCascadeSplits(DirectionalLightCSM& light, float camNear, float camFar){
    float range = camFar - camNear;
    float ratio = camFar / camNear;

    for (int i = 0; i < DirectionalLightCSM::NUM_CASCADES; ++i) {
        float p = (i + 1) / static_cast<float>(DirectionalLightCSM::NUM_CASCADES);
        float log = camNear * std::pow(ratio, p);
        float lin = camNear + range * p;
        float d = light.lambda * (log - lin) + lin;
        light.cascades[i].splitNear = (i == 0) ? camNear : light.cascades[i - 1].splitFar;
        light.cascades[i].splitFar = d;
    }
}

void updateCascadeLightMatrices(DirectionalLightCSM& light, const Camera& cam) {
    for(int i = 0; i < DirectionalLightCSM::NUM_CASCADES; i++){
        std::vector<glm::vec3> frustumCorners;
        getCameraFrustumCorners(cam, light.cascades[i].splitNear, light.cascades[i].splitFar, frustumCorners);
        // update light matrices for this cascade
        glm::vec3 dir = glm::normalize(light.getDirection());

        // Compute center of frustum
        glm::vec3 frustumCenter(0.0f);
        for(const auto& corner : frustumCorners) {
            frustumCenter += corner;
        }
        frustumCenter /= static_cast<float>(frustumCorners.size());

        if(i==0){
            // std::cout << "Frustum center cascade 0: " << frustumCenter.x << ", " << frustumCenter.y << ", " << frustumCenter.z << std::endl;
        }
        glm::vec3 lightPos = frustumCenter - dir * 1.0f;

        // glm::vec3 up = glm::vec3(0, 1, 0);

        // if (abs(glm::dot(up, dir)) > 0.99f)
        //     up = glm::vec3(1, 0, 0);
        glm::vec3 up = light.owner->transform->up();

        glm::mat4 lightView = glm::lookAt(
            lightPos,
            frustumCenter,
            up
        );

        // find AABB of frustum corners in light space
        glm::vec3 minBounds(std::numeric_limits<float>::max());
        glm::vec3 maxBounds(-std::numeric_limits<float>::max());

        for(const auto& corner : frustumCorners) {
            glm::vec4 lightSpaceCorner = lightView * glm::vec4(corner, 1.0f);
            minBounds = glm::min(minBounds, glm::vec3(lightSpaceCorner));
            maxBounds = glm::max(maxBounds, glm::vec3(lightSpaceCorner));
        }

        // // create ortho projection for AABB
        // float zPad = 50.0f * i; // extra padding to avoid caster clipping

        // glm::mat4 lightProj = glm::ortho(
        //     minBounds.x, maxBounds.x,
        //     minBounds.y, maxBounds.y,
        //     -maxBounds.z - zPad, -minBounds.z + zPad
        // );
        
        // glm::mat4 lightProj = glm::ortho(
        //     minBounds.x, maxBounds.x,
        //     minBounds.y, maxBounds.y,
        //     -2*maxBounds.z, -2*minBounds.z
        // );

        glm::mat4 lightProj = glm::ortho(
            minBounds.x, maxBounds.x,
            minBounds.y, maxBounds.y,
            -light.halfRange, light.halfRange
        );
        light.cascades[i].lightVP = lightProj * lightView;

        /////////////////////////
        // float worldUnitsPerTexel =
        // (maxBounds.x - minBounds.x) / 2048.0f;

        // glm::vec3 centerLS = (minBounds+ maxBounds) * 0.5f;

        // centerLS.x = floor(centerLS.x / worldUnitsPerTexel) * worldUnitsPerTexel;
        // centerLS.y = floor(centerLS.y / worldUnitsPerTexel) * worldUnitsPerTexel;

        // float halfW = (maxBounds.x - minBounds.x) * 0.5f;
        // float halfH = (maxBounds.y - minBounds.y) * 0.5f;

        // float left   = centerLS.x - halfW;
        // float right  = centerLS.x + halfW;
        // float bottom = centerLS.y - halfH;
        // float top    = centerLS.y + halfH;

        // glm::mat4 lightProj = glm::ortho(
        //     left, right,
        //     bottom, top,
        //     -maxBounds.z, -minBounds.z
        // );

        // light.cascades[i].lightVP = lightProj * lightView;
    }
}