#pragma once
#include <glad/gl.h>
#include <map>
#include <vector>
#include <string_view>
#include <stdexcept>

namespace Nova
{
	class _GLObjectBase
	{
	public:
		static void RegisterObject(GLenum objType, GLuint id) noexcept;

		static void UnregisterObject(GLenum objType, GLuint id) noexcept;

		static void DeleteAll() noexcept;

	private:
		static inline std::map<GLenum, std::vector<GLuint>> s_ObjectIDs;
	};

	template <GLenum objType, typename idType>
	class GLObject : public _GLObjectBase
	{
	public:
		GLObject() = default;

		constexpr GLObject(GLuint id) noexcept
			: m_ID(id) { }

		virtual void Delete() noexcept = 0;

		constexpr idType GetID() const noexcept { return idType(m_ID); }

		void SetDebugName(const std::string_view name) const
		{
			if (name.size() > (size_t)std::numeric_limits<GLsizei>::max())
				throw std::overflow_error("Name length exceeds maximum allowed by OpenGL.");

			glObjectLabel(objType, m_ID, (GLsizei)name.size(), name.data());
		}

	protected:
		GLuint m_ID = 0;
	};
}
