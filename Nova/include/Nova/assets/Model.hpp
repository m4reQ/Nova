#pragma once
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/assets/Asset.hpp>
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

	struct ModelLoadingData
	{
		std::vector<ModelVertex> ModelVertices;
		std::vector<uint32_t> ModelIndices;
	};

	class Model : public Asset
	{
	public:
		constexpr static AssetType GetStaticAssetType() noexcept { return AssetType::Model; }
		using Asset::Asset;

		void Load(void *loadingData) override;

		void PostLoad(void *loadingData) override;

		void *CreateLoadingData() const noexcept override { return new ModelLoadingData(); }

		void FreeLoadingData(void *loadingData) const override { delete static_cast<ModelLoadingData *>(loadingData); }

		constexpr bool RequiresPostLoad() const noexcept override { return true; }

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