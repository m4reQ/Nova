#include <Nova/assets/ModelLoader.hpp>
#include <Nova/assets/Model.hpp>
#include <Nova/debug/Profile.hpp>
#include <Nova/debug/Log.hpp>
#include <Nova/core/File.hpp>
#include <Nova/core/Utility.hpp>
#include <Nova/graphics/opengl/Buffer.hpp>

using namespace Nova;

static void LoadFromFile(ModelLoadingData *loadingData, const std::filesystem::path &filepath)
{
    File file(filepath, "rS");

    std::vector<glm::vec3> positionData;
    std::vector<glm::vec2> textureCoordData;
    std::vector<glm::vec3> normalData;

    std::string line;
    while (file.GetLine(line))
    {
        switch (line[0])
        {
        case 'v':
            if (line[1] == 't')
            {
                glm::vec2 data;
                std::sscanf(
                    line.data(),
                    "vt %f %f\n",
                    &data.x,
                    &data.y);
                textureCoordData.emplace_back(data);
            }
            else if (line[1] == 'n')
            {
                glm::vec3 data;
                std::sscanf(
                    line.data(),
                    "vn %f %f %f\n",
                    &data.x,
                    &data.y,
                    &data.z);
                normalData.emplace_back(data);
            }
            else
            {
                glm::vec3 data;
                std::sscanf(
                    line.data(),
                    "v %f %f %f\n",
                    &data.x,
                    &data.y,
                    &data.z);
                positionData.emplace_back(data);
            }

            break;
        case 'f':
            int vertexIndices[3];
            int textureIndices[3];
            int normalIndices[3];
            std::sscanf(
                line.data(),
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertexIndices[0],
                &textureIndices[0],
                &normalIndices[0],
                &vertexIndices[1],
                &textureIndices[1],
                &normalIndices[1],
                &vertexIndices[2],
                &textureIndices[2],
                &normalIndices[2]);

            for (int i = 0; i < 3; i++)
            {
                loadingData->ModelVertices.emplace_back(
                    Nova::ModelVertex{
                        .Position = positionData[vertexIndices[i] - 1],
                        .Normal = normalData[normalIndices[i] - 1],
                        .TextureCoords = textureCoordData[textureIndices[i] - 1],
                    });
            }

            break;
        }
    }
}

static void LoadFromMemory(ModelLoadingData *, std::span<const uint8_t>)
{
    // TODO Implement
}

void OBJModelLoader::Load(std::shared_ptr<Asset> asset, void *loadingData) noexcept
{
    NV_PROFILE_FUNC;

    auto loadingData_ = static_cast<ModelLoadingData *>(loadingData);

    std::visit(
        Overloaded{
            [=](const std::filesystem::path &filepath)
            {
                LoadFromFile(loadingData_, filepath);
            },
            [](const std::vector<std::filesystem::path> &)
            {
                NV_LOG_ERROR("Loading models from file list is not supported.");
            },
            [=](std::span<const uint8_t> data)
            {
                LoadFromMemory(loadingData_, data);
            }},
        asset->GetSource());
}

void OBJModelLoader::PostLoad(std::shared_ptr<Asset> asset, void *loadingData) noexcept
{
    NV_PROFILE_FUNC;

    auto loadingData_ = static_cast<ModelLoadingData *>(loadingData);
    auto asset_ = std::static_pointer_cast<Model>(asset);

    asset_->SetModelBuffer(
        std::make_shared<Buffer>(
            sizeof(ModelVertex) * loadingData_->ModelVertices.size(),
            BufferAccessFlags::None,
            loadingData_->ModelVertices.data()));
}