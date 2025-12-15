#pragma once
#include <glm/glm.hpp>
#include "Component.h"

class PointLight : public Component {
  public:
    glm::vec3 color    = glm::vec3(1.0f);   // RGB color of the light

    float intensity    = 1.0f;              // overall brightness multiplier

    // Attenuation factors
    float constant     = 1.0f;              // usually 1.0
    float linear       = 0.09f;             // affects how quickly light fades
    float quadratic    = 0.032f;            // affects how quickly it fades farther away

    PointLight() = default;

    PointLight(const glm::vec3& col,
               float intens = 1.0f,
               float c = 1.0f, float l = 0.09f, float q = 0.032f)
        : color(col), intensity(intens), constant(c), linear(l), quadratic(q)
    {}


};
