#pragma once

namespace GTS {
	std::string GetRawName(const void* obj_c);

	std::string Vector2Str(const hkVector4& vector);

	std::string Vector2Str(const hkVector4* vector);

	std::string Vector2Str(const NiPoint3& vector);

	std::string Vector2Str(const NiPoint3* vector);

	std::string Vector2Str(const NiQuaternion& vector);

	std::string Vector2Str(const NiQuaternion* vector);
}
