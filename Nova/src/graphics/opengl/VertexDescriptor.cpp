#include <Nova/graphics/opengl/VertexDescriptor.hpp>
#include <Nova/core/Build.hpp>

using namespace Nova;

GLuint VertexDescriptor::GetTypeSize() const noexcept
{
	switch (AttributeType)
	{
	case AttributeType::Byte:
		return sizeof(GLbyte);
	case AttributeType::UnsignedByte:
		return sizeof(GLubyte);
	case AttributeType::Short:
		return sizeof(GLshort);
	case AttributeType::UnsignedShort:
		return sizeof(GLushort);
	case AttributeType::Int:
		return sizeof(GLint);
	case AttributeType::UnsignedInt:
		return sizeof(GLuint);
	case AttributeType::Fixed:
		return sizeof(GLfixed);
	case AttributeType::Float:
		return sizeof(GLfloat);
	case AttributeType::HalfFloat:
		return sizeof(GLhalf);
	case AttributeType::Double:
		return sizeof(GLdouble);
	case AttributeType::Int2101010Rev:
		return sizeof(GLint);
	case AttributeType::UnsignedInt2101010Rev:
		return sizeof(GLuint);
	case AttributeType::UnsignedInt10F11F11FRev:
		return sizeof(GLuint);
	}

	NV_UNREACHABLE;
}

GLuint VertexDescriptor::GetSize() const noexcept
{
	return GetRowSize() * Rows;
}

GLuint VertexDescriptor::GetRowSize() const noexcept
{

	return GetTypeSize() * Count;
}