#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace zyh
{
	template<uint8_t _Size, typename _ScalerType>
	class TVector {
	public:
		using InstanceType = glm::vec<_Size, _ScalerType>;

	public:
		TVector() : mInstance_(InstanceType())
		{
		}

		template<typename ... Types>
		TVector(Types... args) : mInstance_(InstanceType())
		{
			uint8_t argsSize = sizeof...(args);
			CtorHelper(std::forward<Types>(args)...);
		}

		TVector(_ScalerType v)
		{
			mInstance_ = InstanceType(v);
		}

		template<typename ... Types>
		void CtorHelper(_ScalerType v, Types... args)
		{
			uint8_t argsSize = sizeof...(args);
			assert(argsSize < _Size);
			uint8_t index = _Size - argsSize - 1;
			mInstance_[index] = v;
			CtorHelper(std::forward<Types>(args)...);
		}

		void CtorHelper(_ScalerType v)
		{
			mInstance_[_Size - 1] = v;
		}

		TVector(InstanceType& instance) : mInstance_(instance)
		{
		}

		operator InstanceType()
		{
			return mInstance_;
		}

		InstanceType& getInstance()
		{
			return mInstance_;
		}

		const InstanceType& getInstance() const
		{
			return mInstance_;
		}

		uint8_t size() { return _Size; }

	public:
		_ScalerType operator [](int16_t index)
		{
			assert(std::abs(index) < _Size);
			if (index < 0)
				index = _Size + index;
			return mInstance_[index];
		}

		TVector operator +(const TVector& rhs)
		{
			assert(size() == rhs.size());
			TVector v;
			for (size_t i = 0; i < size(); ++i)
			{
				v[i] = (*this)[i] + rhs[i];
			}
			return v;
		}

	private:
		InstanceType mInstance_;
	};


	using Vector3 = TVector<3, float>;
}



