#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Vector.h"

namespace zyh
{
	template<uint8_t _RowSize, uint8_t _ColSize, typename _ScalerType>
	class TMatrix
	{
	public:
		using InstanceType = glm::mat<_ColSize, _RowSize, _ScalerType>;

	public:
		TMatrix() : mInstance_(InstanceType(1.0f)) {}
		TMatrix(InstanceType& instance) : mInstance_(instance) {}
		TMatrix(InstanceType&& instance) : mInstance_(instance) {}

		InstanceType& getInstance()
		{
			return mInstance_;
		}

		const InstanceType& getInstance() const
		{
			return mInstance_;
		}

	protected:
		InstanceType mInstance_;
	};


	class Matrix4x4 : public TMatrix<4, 4, float>
	{
		using SuperType = TMatrix<4, 4, float>;

	public:
		static Matrix4x4 GenerateLookAt(const Vector3& position, const Vector3& target, const Vector3& up)
		{
			return glm::lookAt(position.getInstance(), target.getInstance(), up.getInstance());
		}

		static Matrix4x4 GenerateRotate(const Matrix4x4& baseMatrix, const float& angle, const Vector3& axis)
		{
			return glm::rotate(baseMatrix.getInstance(), angle, axis.getInstance());
		}

		static Matrix4x4 GeneratePerspective(const float& angle, const float& aspect, const float& _near, const float& _far)
		{
			return glm::perspective(angle, aspect, _near, _far);
		}


	public:
		Matrix4x4() : SuperType() {}
		Matrix4x4(InstanceType& instance) : SuperType(instance) {}
		Matrix4x4(InstanceType&& instance) : SuperType(std::forward<InstanceType>(instance)) {}

		operator InstanceType() {
			return mInstance_;
		}

	public:
		void LookAt(const Vector3& position, const Vector3& target, const Vector3& up)
		{
			mInstance_ = GenerateLookAt(position, target, up);
		}
	};
}