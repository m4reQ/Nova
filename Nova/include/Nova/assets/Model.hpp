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

		void SetModelBuffer(std::shared_ptr<Buffer> &&buffer) noexcept { modelBuffer_ = std::move(buffer); }

		void SetIndexBuffer(std::shared_ptr<Buffer> &&buffer) noexcept { indexBuffer_ = std::move(buffer); }

		const std::shared_ptr<Buffer> GetModelDataBuffer() const noexcept { return modelBuffer_; }

		const std::shared_ptr<Buffer> GetIndexBuffer() const noexcept { return indexBuffer_; }

		bool UsesIndexBuffer() const noexcept { return indexBuffer_ != nullptr; }

		constexpr size_t GetIndexDataSize() const noexcept { return indexBuffer_ != nullptr ? indexBuffer_->GetSize() : 0zu; }

		constexpr size_t GetModelDataSize() const noexcept { return modelBuffer_->GetSize(); }

		constexpr GLenum GetPrimitiveMode() const noexcept { return primitiveMode_; }

	private:
		std::shared_ptr<Nova::Buffer> indexBuffer_;
		std::shared_ptr<Nova::Buffer> modelBuffer_;
		GLenum primitiveMode_ = GL_TRIANGLES;
	};
}