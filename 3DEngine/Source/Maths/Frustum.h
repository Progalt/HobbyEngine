#pragma once

#include "BoundingBox.h"
#include <glm/glm.hpp>

class Frustum
{
public:

	Frustum(const glm::mat4& view_proj)
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

		mPoints[0] = intersection<Left, Bottom, Near>(crosses);
		mPoints[1] = intersection<Left, Top, Near>(crosses);
		mPoints[2] = intersection<Right, Bottom, Near>(crosses);
		mPoints[3] = intersection<Right, Top, Near>(crosses);
		mPoints[4] = intersection<Left, Bottom, Far>(crosses);
		mPoints[5] = intersection<Left, Top, Far>(crosses);
		mPoints[6] = intersection<Right, Bottom, Far>(crosses);
		mPoints[7] = intersection<Right, Top, Far>(crosses);
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

		// check frustum outside/inside box
		int out;
		out = 0; for (int i = 0; i < 8; i++) out += ((mPoints[i].x > bb.max.x) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((mPoints[i].x < bb.min.x) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((mPoints[i].y > bb.max.y) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((mPoints[i].y < bb.min.y) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((mPoints[i].z > bb.max.z) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((mPoints[i].z < bb.min.z) ? 1 : 0); if (out == 8) return false;

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

	template<Planes i, Planes j>
	struct ij2k
	{
		enum { k = i * (9 - i) / 2 + j - 1 };
	};

	template<Planes a, Planes b, Planes c>
	glm::vec3 intersection(const glm::vec3* crosses) const
	{
		float D = glm::dot(glm::vec3(mPlanes[a]), crosses[ij2k<b, c>::k]);
		glm::vec3 res = glm::mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) *
			glm::vec3(mPlanes[a].w, mPlanes[b].w, mPlanes[c].w);
		return res * (-1.0f / D);
	}

	glm::mat4 mViewProj;

	glm::vec4 mPlanes[Count];
	glm::vec3 mPoints[8];

};