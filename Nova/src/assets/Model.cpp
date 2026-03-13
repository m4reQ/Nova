#include <Nova/assets/Model.hpp>
#include <Nova/core/File.hpp>
#include <Nova/debug/Profile.hpp>

using namespace Nova;

static void LoadModelFromWavefrontOBJFile(const std::filesystem::path &filepath, ModelLoadingData *loadingData)
{
    NV_PROFILE_FUNC;

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

static void LoadModelFromFile(const std::filesystem::path &filepath, ModelLoadingData *loadingData)
{
    NV_PROFILE_FUNC;

    if (filepath.extension() == ".obj")
        LoadModelFromWavefrontOBJFile(filepath, loadingData);
}

void Model::Load(void *loadingData)
{
    NV_PROFILE_FUNC;

    auto loadingData_ = static_cast<ModelLoadingData *>(loadingData);

    if (source_.Type == AssetSourceType::File)
        LoadModelFromFile(source_.Filepath, loadingData_);
}

void Model::PostLoad(void *loadingData)
{
    NV_PROFILE_FUNC;

    ModelLoadingData *_loadingData = static_cast<ModelLoadingData *>(loadingData);
    Buffer buffer(
        sizeof(ModelVertex) * _loadingData->ModelVertices.size(),
        false,
        false,
        _loadingData->ModelVertices.data());

    {
        std::lock_guard<std::mutex> lock(loadingMutex_);
        modelBuffer_ = std::move(buffer);
    }
}