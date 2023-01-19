#include <glm/glm.hpp>
#include "vertex_processing.h"

// Model matrix converts model space -> world space
// View matrix converts world space -> camera space
// Projection matrix converts camera space -> clip space
// Conversion from homogenous to cartesian (division by w) converts clip space -> NDC ([-1, 1])
// THIS STEP ALSO APPLIES PERSPECTIVE DIVIDE, since projection leaves w coordinate as -z

glm::mat4 createProjectionMatrix(float fov, float aspect_ratio, float near, float far) {
	glm::mat4 projection = glm::mat4(0.0);

	float f = 1.0 / glm::tan(glm::radians(fov) / 2.0);

	projection[0][0] = f / aspect_ratio;
	projection[1][1] = f;
	// NOTE - glm uses column-major matrices, so indexing is [col][row]
	projection[2][2] = (near + far) / (near - far);
	projection[3][2] = (2 * near * far) / (near - far);
	projection[2][3] = -1.0;

	return projection;
}

glm::vec3 perspectiveDivide(glm::vec4 hom) {
	return glm::vec3(hom) / hom.w;
}