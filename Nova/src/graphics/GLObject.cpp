#include <Nova/graphics/opengl/GLObject.hpp>
#include <Nova/core/Build.hpp>
#include <Nova/debug/Log.hpp>
#include <algorithm>
#include <limits>

using namespace Nova;

template <typename TReturn, typename TContainer>
static TReturn GetSizeChecked(const TContainer &container) noexcept
{
	const auto maxVal = std::numeric_limits<TReturn>::max();
	const auto size = container.size();
	if (size > maxVal)
		NV_LOG_WARNING("Container size exceeds max type number. Some objects will not be deleted.");

	return size > maxVal
			   ? (TReturn)maxVal
			   : (TReturn)size;
}

void _GLObjectBase::RegisterObject(GLenum objType, GLuint id) noexcept
{
	s_ObjectIDs[objType].emplace_back(id);
}

void _GLObjectBase::UnregisterObject(GLenum objType, GLuint id) noexcept
{
	auto &objects = s_ObjectIDs[objType];
	objects.erase(std::remove(objects.begin(), objects.end(), id));
}

void _GLObjectBase::DeleteAll() noexcept
{
	for (const auto &[objType, objectsList] : s_ObjectIDs)
	{
		// We can't really do anything about overflows at this point
		const auto objectsSize = GetSizeChecked<GLsizei>(objectsList);

		switch (objType)
		{
		case GL_BUFFER:
			glDeleteBuffers(objectsSize, objectsList.data());
			break;
		case GL_PROGRAM:
			for (const auto programID : objectsList)
				glDeleteProgram(programID);
			break;
		case GL_VERTEX_ARRAY:
			glDeleteVertexArrays(objectsSize, objectsList.data());
			break;
		case GL_TEXTURE:
			glDeleteTextures(objectsSize, objectsList.data());
			break;
		default:
			NV_DEBUG_FAIL("Invalid object type.");
		}
	}

	s_ObjectIDs.clear();
}
