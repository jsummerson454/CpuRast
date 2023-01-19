#pragma once

#include <glm/glm.hpp>

glm::mat4 createProjectionMatrix(float fov, float aspect_ratio, float near, float far);
glm::vec3 perspectiveDivide(glm::vec4 hom);
