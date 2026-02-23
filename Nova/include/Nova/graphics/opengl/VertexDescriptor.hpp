#pragma once
#include <glad/gl.h>

namespace Nova
{
	enum class AttributeType
	{
		Byte = GL_BYTE,
		UnsignedByte = GL_UNSIGNED_BYTE,
		Short = GL_SHORT,
		UnsignedShort = GL_UNSIGNED_SHORT,
		Int = GL_INT,
		UnsignedInt = GL_UNSIGNED_INT,
		Fixed = GL_FIXED,
		Float = GL_FLOAT,
		HalfFloat = GL_HALF_FLOAT,
		Double = GL_DOUBLE,
		Int2101010Rev = GL_INT_2_10_10_10_REV,
		UnsignedInt2101010Rev = GL_UNSIGNED_INT_2_10_10_10_REV,
		UnsignedInt10F11F11FRev = GL_UNSIGNED_INT_10F_11F_11F_REV,
	};

	struct VertexDescriptor
	{
		GLuint AttributeIndex;
		AttributeType AttributeType;
		GLint Count;
		GLint Rows = 1;
		bool IsNormalized = false;

		GLuint GetTypeSize() const noexcept;
		GLuint GetSize() const noexcept;
		GLuint GetRowSize() const noexcept;
	};
}