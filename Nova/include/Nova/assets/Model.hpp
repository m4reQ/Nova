#pragma once
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/assets/Asset.hpp>
#include <vector>
#include <optional>
#include <glad/gl.h>
#include <glm/vec3.hpp>

namespace Nova
{

	class Model : public Asset
	{
	public:
		Model() = default;

		Model(const AssetSource &source, std::optional<std::string_view> name = std::nullopt) noexcept;

		constexpr static AssetType GetStaticAssetType() noexcept { return AssetType::Model; }

		constexpr AssetType GetType() const noexcept override { return AssetType::Model; }

		constexpr void SetModelBuffer(Buffer &&buffer) noexcept { modelBuffer_ = std::move(buffer); }

		constexpr void SetIndexBuffer(Buffer &&buffer) noexcept { indexBuffer_ = std::move(buffer); }

		constexpr const Buffer &GetModelDataBuffer() const noexcept { return modelBuffer_; }

		constexpr const std::optional<Nova::Buffer> &GetIndexBuffer() const noexcept { return indexBuffer_; }

		constexpr bool UsesIndexBuffer() const noexcept { return indexBuffer_.has_value(); }

		constexpr size_t GetIndexDataSize() const noexcept { return indexBuffer_.has_value() ? indexBuffer_.value().GetSize() : 0; }

		constexpr size_t GetModelDataSize() const noexcept { return modelBuffer_.GetSize(); }

		constexpr GLenum GetPrimitiveMode() const noexcept { return primitiveMode_; }

	private:
		std::optional<Nova::Buffer> indexBuffer_;
		Nova::Buffer modelBuffer_;
		GLenum primitiveMode_ = GL_TRIANGLES;
	};
}