#pragma once
#include <Nova/graphics/opengl/Buffer.hpp>
#include <vector>
#include <optional>
#include <glad/gl.h>
#include <glm/vec3.hpp>

namespace Nova
{
	struct ModelVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TextureCoords;
	};

	class Model
	{
	public:
		Model() = default;
		
		Model(size_t id, Buffer&& modelDataBuffer)
			: id_(id),
			  modelBuffer_(std::move(modelDataBuffer)),
			  indexBuffer_(std::nullopt) { }

		Model(size_t id, Buffer&& modelDataBuffer, Buffer&& indexBuffer)
			: id_(id),
			  modelBuffer_(std::move(modelDataBuffer)),
			  indexBuffer_(std::move(indexBuffer)) { }

		constexpr size_t GetID() const noexcept { return id_; }
		constexpr const Buffer& GetModelDataBuffer() const noexcept { return modelBuffer_; }
		constexpr const std::optional<Nova::Buffer>& GetIndexBuffer() const noexcept { return indexBuffer_; }
		constexpr bool UsesIndexBuffer() const noexcept { return indexBuffer_.has_value(); }
		constexpr size_t GetIndexDataSize() const noexcept { return indexBuffer_.has_value() ? indexBuffer_.value().GetSize() : 0; }
		constexpr size_t GetModelDataSize() const noexcept { return modelBuffer_.GetSize(); }
		constexpr GLenum GetPrimitiveMode() const noexcept { return primitiveMode_; }

	private:
		std::optional<Nova::Buffer> indexBuffer_;
		Nova::Buffer modelBuffer_;
		GLenum primitiveMode_ = GL_TRIANGLES;
		size_t id_;
	};
}