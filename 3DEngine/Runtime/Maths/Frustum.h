#pragma once

#include "BoundingBox.h"
#include <glm/glm.hpp>


inline  glm::mat4 ReversedDepthPerspective(float fov, float aspect, float zNear)
{
	float f = 1.0f / tan(fov / 2.0f);
	return glm::mat4(
		f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, zNear, 0.0f);
}

class Frustum
{
public:

	Frustum () { }

	void Create(const glm::mat4& view_proj)
	{
		glm::mat4 m = glm::transpose(view_proj);
		mPlanes[Left] = m[3] + m[0];
		mPlanes[Right] = m[3] - m[0];
		mPlanes[Bottom] = m[3] + m[1];
		mPlanes[Top] = m[3] - m[1];
		mPlanes[Near] = m[3] + m[2];
		mPlanes[Far] = m[3] - m[2];

		glm::vec3 crosses[Combinations] = {
			glm::cross(glm::vec3(mPlanes[Left]),   glm::vec3(mPlanes[Right])),
			glm::cross(glm::vec3(mPlanes[Left]),   glm::vec3(mPlanes[Bottom])),
			glm::cross(glm::vec3(mPlanes[Left]),   glm::vec3(mPlanes[Top])),
			glm::cross(glm::vec3(mPlanes[Left]),   glm::vec3(mPlanes[Near])),
			glm::cross(glm::vec3(mPlanes[Left]),   glm::vec3(mPlanes[Far])),
			glm::cross(glm::vec3(mPlanes[Right]),  glm::vec3(mPlanes[Bottom])),
			glm::cross(glm::vec3(mPlanes[Right]),  glm::vec3(mPlanes[Top])),
			glm::cross(glm::vec3(mPlanes[Right]),  glm::vec3(mPlanes[Near])),
			glm::cross(glm::vec3(mPlanes[Right]),  glm::vec3(mPlanes[Far])),
			glm::cross(glm::vec3(mPlanes[Bottom]), glm::vec3(mPlanes[Top])),
			glm::cross(glm::vec3(mPlanes[Bottom]), glm::vec3(mPlanes[Near])),
			glm::cross(glm::vec3(mPlanes[Bottom]), glm::vec3(mPlanes[Far])),
			glm::cross(glm::vec3(mPlanes[Top]),    glm::vec3(mPlanes[Near])),
			glm::cross(glm::vec3(mPlanes[Top]),    glm::vec3(mPlanes[Far])),
			glm::cross(glm::vec3(mPlanes[Near]),   glm::vec3(mPlanes[Far]))
		};

		glm::vec3 frustumCorners[8] = {
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
		};

		// Project frustum corners into world space
		glm::mat4 invCam = glm::inverse(view_proj);
		for (uint32_t i = 0; i < 8; i++) {
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
			mCorners[i] = invCorner / invCorner.w;
		}

		for (uint32_t i = 0; i < 8; i++) {
			mCenter += frustumCorners[i];
		}
		mCenter /= 8.0f;

	}

	Frustum(const glm::mat4& view_proj)
	{
		Create(view_proj);
	}

	bool Test(const BoundingBox& bb)
	{
		for (int i = 0; i < Count; i++)
		{
			if ((glm::dot(mPlanes[i], glm::vec4(bb.min.x, bb.min.y, bb.min.z, 1.0f)) < 0.0) &&
				(glm::dot(mPlanes[i], glm::vec4(bb.max.x, bb.min.y, bb.min.z, 1.0f)) < 0.0) &&
				(glm::dot(mPlanes[i], glm::vec4(bb.min.x, bb.max.y, bb.min.z, 1.0f)) < 0.0) &&
				(glm::dot(mPlanes[i], glm::vec4(bb.max.x, bb.max.y, bb.min.z, 1.0f)) < 0.0) &&
				(glm::dot(mPlanes[i], glm::vec4(bb.min.x, bb.min.y, bb.max.z, 1.0f)) < 0.0) &&
				(glm::dot(mPlanes[i], glm::vec4(bb.max.x, bb.min.y, bb.max.z, 1.0f)) < 0.0) &&
				(glm::dot(mPlanes[i], glm::vec4(bb.min.x, bb.max.y, bb.max.z, 1.0f)) < 0.0) &&
				(glm::dot(mPlanes[i], glm::vec4(bb.max.x, bb.max.y, bb.max.z, 1.0f)) < 0.0))
			{
				return false;
			}
		}

		return true;
	}

private:

	enum Planes
	{
		Left = 0,
		Right,
		Bottom,
		Top,
		Near,
		Far,
		Count,
		Combinations = Count * (Count - 1) / 2
	};


	glm::mat4 mViewProj;

	glm::vec4 mPlanes[Count];

	glm::vec3 mCorners[8];
	glm::vec3 mCenter;

};