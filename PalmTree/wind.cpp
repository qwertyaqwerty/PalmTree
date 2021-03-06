#include "wind.h"

#include <cmath>

const int PI = asinf(-1.0);

glm::vec3 getWind(GLfloat time)
{
	float strength = sinf(2 * PI * time * 0.3) + 0.8;
	//float strength = 1.0;
	//return glm::vec3(strength, 0, 0);
	return glm::vec3(strength * 1.2, 0, 0);
}

float getDetailBendScale(GLfloat time)
{
	float strength = sinf(2 * PI * time * 0.3) + 0.8;
	return 2.0 + strength;
}
