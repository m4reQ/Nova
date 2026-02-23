#pragma once
#include <Nova/core/Utility.hpp>
#include <Nova/graphics/opengl/GLObject.hpp>
#include <type_traits>
#include <concepts>

namespace Nova
{
	struct TextureID : public StrongTypedef<TextureID, GLuint>
	{
		using StrongTypedef::StrongTypedef;
	};

	class Texture : public GLObject<GL_TEXTURE, TextureID>
	{
	public:
		constexpr static Texture &GetWhiteTexture() noexcept
		{
			return s_WhiteTexture;
		}

		static void InitializeWhiteTexture() noexcept;

		template <HasArrayInterface<GLuint> T>
		static void BindTextures(const T &textureIDs, GLuint first = 0) noexcept
		{
			glBindTextures(first, textureIDs.size(), textureIDs.data());
		}

		template <size_t N>
		static void BindTextures(const std::array<Texture, N> &textures, GLuint first = 0) noexcept
		{
			std::array<GLuint, N> textureIDs;
			for (const auto &texture : textures)
			{
				textureIDs.emplace_back((GLuint)texture.GetID());
			}

			BindTextures(textureIDs, first);
		}

		Texture() = default;

		void BindToUnit(GLuint unit) const noexcept;

		void Delete() noexcept override;

	private:
		static Texture s_WhiteTexture;
	};
}
