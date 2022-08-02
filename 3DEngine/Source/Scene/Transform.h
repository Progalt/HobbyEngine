#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
public:

	Transform()
	{
		mPosition = glm::vec3(0.0f);
		mScale = glm::vec3(1.0f);
		mRotation = glm::quat();

		mCachedMatrix = glm::mat4(1.0f);
	}

	void SetPosition(const glm::vec3& pos)
	{
		mPosition = pos;
		mUpdate = true;
	}

	const glm::vec3& GetPosition()
	{
		return mPosition;
	}

	void SetScale(const glm::vec3& s)
	{
		mScale = s;
		mUpdate = true;
	}

	glm::vec3& GetScale()
	{
		return mScale;
	}

	// Rotation Euler Functions

	void SetEuler(const glm::vec3& eulerAngles)
	{
		mRotation = glm::quat(glm::radians(eulerAngles));
		mUpdate = true;
	}

	const glm::vec3& GetEuler()
	{
		return glm::eulerAngles(mRotation);
	}

	const glm::mat4 ComputeMatrix(const glm::mat4& parent)
	{
		if (mUpdate)
		{
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), this->mPosition);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), this->mScale);
			glm::mat4 rotation = glm::toMat4(this->mRotation);

			glm::mat4 transform = translation * scale * rotation;

			mCachedMatrix = transform;
			return parent * transform;
		}
		
		return parent * mCachedMatrix;
	}

private:

	glm::vec3 mPosition;
	glm::vec3 mScale;
	glm::quat mRotation;

	bool mUpdate = true;

	// This is the cached local matrix
	glm::mat4 mCachedMatrix;
};