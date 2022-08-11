#include <iostream>
#include "Matrix4x3.h"


namespace zyh
{
	std::ostream& operator<<(std::ostream& stream, const Matrix4x3& m)
	{
		stream
			<< m.m00 << ' ' << m.m01 << ' ' << m.m02 << ' '
			<< m.m10 << ' ' << m.m11 << ' ' << m.m12 << ' '
			<< m.m20 << ' ' << m.m21 << ' ' << m.m22 << ' '
			<< m.m30 << ' ' << m.m31 << ' ' << m.m32;
		return stream;
	}

	std::istream& operator>>(std::istream& stream, Matrix4x3& m)
	{
		stream >> m.m00 >> m.m01 >> m.m02 >> m.m10 >> m.m11 >> m.m12 >> m.m20 >> m.m21 >> m.m22 >> m.m30 >> m.m31 >> m.m32;
		return stream;
	}
}