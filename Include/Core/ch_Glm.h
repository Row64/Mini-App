#ifndef CH_GLM_H
#define CH_GLM_H


#define _USE_MATH_DEFINES

#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan [0, 1] depth range, instead of OpenGL [-1, +1]
#define GLM_FORCE_RIGHT_HANDED // Vulkan has a left handed coordinate system (same as DirectX), OpenGL is right handed
#define GLM_FORCE_RADIANS

#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/vec2.hpp>
#include <glm/ext.hpp>

#define EPS 1e-9 

using namespace glm;

#endif // CH_GLM_H