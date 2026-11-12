#include "DataHub.h"
#include "DataHub_Resource.h"


namespace Hakko::DataHub
{

    #pragma region Resource

    #pragma region Variables

    std::vector<Mesh> uploadMeshList;
    std::vector<Texture> uploadTextureList;  
    std::vector<ComputeBuffer> uploadCSBufferList;  
    std::vector<ShaderInfo> loadShaderList;
    std::vector<DrawProp> uploadDrawPropList;
    std::vector<TextureProp> uploadTexPropList;

    std::vector<uint16_t> meshSubMeshCountList;
    
    std::vector<CSMeshLinkData> linkCSBufferAsMeshList;


    std::unordered_map<HashedString, MeshID> meshIdLUT;
    std::unordered_map<HashedString, TexID> textureIdLUT;
    std::unordered_map<HashedString, CSBufferID> csBufferIdLUT;
    std::unordered_map<HashedString, ShaderID> shaderIdLUT;
    std::unordered_map<HashedString, DrawPropID> drawPropIdLUT;
    std::unordered_map<HashedString, TexPropID> texPropIdLUT;


    std::vector<std::pair<MeshID, Mesh>> updateMeshList;
    std::vector<std::pair<TexID, Texture>> updateTextureList;
    std::vector<std::pair<CSBufferID, ComputeBuffer>> updateCSBufferList;
    
    std::vector<std::pair<DrawPropID, DrawProp>> updateDrawPropList;
    std::vector<std::pair<TexPropID, TextureProp>> updateTexPropList;


    std::vector<MeshID> ReleaseMeshList;
    std::vector<TexID> ReleaseTextureList;
    std::vector<CSBufferID> ReleaseCSBufferList;

    std::vector<DrawPropID> ReleaseDrawPropList;
    std::vector<TexPropID> ReleaseTexPropList;

    #pragma endregion


    static bool IsDepthFormat(TextureReadFormat f)
    {
        return f >= TextureReadFormat::D16_UNorm && f <= TextureReadFormat::D32_Float_S8X24_UInt;
    }

    static bool IsBlockCompressedFormat(TextureReadFormat f)
    {
        return f >= TextureReadFormat::BC1_UNorm && f <= TextureReadFormat::BC7_SRGB;
    }

    static bool IsSRGBFormat(TextureReadFormat f)
    {
        switch (f)
        {
        case TextureReadFormat::RGBA8_SRGB:
        case TextureReadFormat::BGRA8_SRGB:
        case TextureReadFormat::BC1_SRGB:
        case TextureReadFormat::BC2_SRGB:
        case TextureReadFormat::BC3_SRGB:
        case TextureReadFormat::BC7_SRGB:
            return true;

        default:
            return false;
        }
    }

    static bool ValidateTextureUsage(const Texture& tex)
    {
        const TextureReadFormat f = tex.data.format;

        if (f == TextureReadFormat::Unknown)
            return false;

        switch (tex.usage)
        {
        case TextureUsage_DS:
            return IsDepthFormat(f);

        case TextureUsage_RT:
            return !IsDepthFormat(f) && !IsBlockCompressedFormat(f) && f != TextureReadFormat::RGB9E5_SharedExp;

        case TextureUsage_Compute:
            return !IsDepthFormat(f) && !IsBlockCompressedFormat(f) && !IsSRGBFormat(f) && f != TextureReadFormat::RGB9E5_SharedExp;

        default:
            return !IsDepthFormat(f);
        }
    }

    static bool ValidateComputeBuffer(const ComputeBuffer& desc)
    {
        if (desc.stride == 0 || desc.stride % 4 != 0 || desc.count == 0)
            return false;

        return desc.data.empty() || desc.data.size() == (size_t)desc.stride * desc.count;
    }

    static bool ValidateSubMeshes(size_t vertexCount, const std::vector<VIndex>& indices, const std::vector<SubMesh>& subMeshes)
    {
        if (subMeshes.empty())
        {
            uint32_t maxIdx = 0;
            for (size_t i = 0; i < indices.size(); i++)
                if (indices[i] > maxIdx) maxIdx = indices[i];
            return maxIdx < vertexCount;
        }

        for (size_t s = 0; s < subMeshes.size(); s++)
        {
            const SubMesh& sub = subMeshes[s];
            const size_t end = (size_t)sub.indexStart + sub.indexCount;

            if (end > indices.size())
                return false;

            uint32_t maxIdx = 0;
            for (size_t i = sub.indexStart; i < end; i++)
                if (indices[i] > maxIdx) maxIdx = indices[i];

            if ((size_t)sub.baseVertex + maxIdx >= vertexCount)
                return false;
        }

        return true;
    }


    MeshID CreateMesh(const wchar_t* name, Mesh&& mesh)
    {
        size_t vertexCount = 0;
        switch (mesh.vertexType)
        {
        case VertexType_Simple:  vertexCount = mesh.verticesSimple.size();  break;
        case VertexType_Static:  vertexCount = mesh.verticesStatic.size();  break;
        case VertexType_Skinned: vertexCount = mesh.verticesSkinned.size(); break;
        }

        if (vertexCount == 0 || mesh.indices.empty() || !ValidateSubMeshes(vertexCount, mesh.indices, mesh.subMeshes))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateMesh: 빈 데이터 / 서브메쉬 범위 밖");
            return 0;
        }

        auto key = HashString(std::wstring_view(name));

        if (meshIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateMesh: 이미 있는 이름");
            return 0;
        }

        if (meshIdLUT.size() >= 0xFFFF)
        {
            Internal::PushHubReport(HubReport_Limit, L"[Hub] CreateMesh: 메쉬 id 소진 (16비트)");
            return 0;
        }

        MeshID id = static_cast<MeshID>(meshIdLUT.size());
        meshIdLUT.emplace(key, id);
        meshSubMeshCountList.push_back(mesh.subMeshes.empty() ? (uint16_t)1 : (uint16_t)mesh.subMeshes.size());
        uploadMeshList.emplace_back(std::move(mesh));

        return id;
    }

    TexID CreateTexture(const wchar_t* name, Texture&& tex)
    {
        if (tex.usage != TextureUsage_ReadOnly)
        {
            Internal::PushHubReport(HubReport_Contract, L"[미지원] CreateTexture: 쓰기 텍스처 (usage RT / DS / Compute) 는 이 버전에서 막혀 있음");
            return 0;
        }

        if (!ValidateTextureUsage(tex))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateTexture: 포맷이 usage 와 안 맞음 (DS = 깊이 포맷만 / RT, Compute = 깊이, 블록 압축 불가 / Compute = sRGB 불가)");
            return 0;
        }

        auto key = HashString(std::wstring_view(name));

        if (textureIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateTexture: 이미 있는 이름");
            return 0;
        }

        if (textureIdLUT.size() >= 0xFFFF)
        {
            Internal::PushHubReport(HubReport_Limit, L"[Hub] CreateTexture: 텍스처 id 소진 (16비트)");
            return 0;
        }

        TexID id = static_cast<TexID>(textureIdLUT.size());
        textureIdLUT.emplace(key, id);
        uploadTextureList.emplace_back(std::move(tex));

        return id;
    }

    CSBufferID CreateComputeBuffer(const wchar_t* name, const ComputeBuffer& desc)
    {
        Internal::PushHubReport(HubReport_Contract, L"[미지원] CreateComputeBuffer: 컴퓨트 버퍼는 이 버전에서 막혀 있음");
        return 0;

        if (!ValidateComputeBuffer(desc))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateComputeBuffer: stride(4의 배수) / count 0 / data 크기 불일치");
            return 0;
        }

        auto key = HashString(std::wstring_view(name));

        if (csBufferIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateComputeBuffer: 이미 있는 이름");
            return 0;
        }

        if (csBufferIdLUT.size() >= 0xFFFF)
        {
            Internal::PushHubReport(HubReport_Limit, L"[Hub] CreateComputeBuffer: 컴퓨트 버퍼 id 소진 (16비트)");
            return 0;
        }

        CSBufferID id = static_cast<CSBufferID>(csBufferIdLUT.size());
        csBufferIdLUT.emplace(key, id);
        uploadCSBufferList.push_back(desc);

        return id;
    }


    ShaderID RegisterShader(const wchar_t* path, ShaderType shaderType)
    {
        if (shaderType != ShaderType_VS && shaderType != ShaderType_PS && shaderType != ShaderType_CS)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] RegisterShader: 셰이더 타입 잘못됨");
            return 0;
        }

        auto key = HashString(std::wstring_view(path));

        if (shaderIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] RegisterShader: 이미 있는 경로");
            return 0;
        }

        ShaderID id = static_cast<ShaderID>(shaderIdLUT.size());
        shaderIdLUT.emplace(key, id);
        loadShaderList.emplace_back(path, shaderType);

        return id;
    }

    DrawPropID CreateDrawProp(const char *name, DrawProp prop)
    {
        std::string_view sName(name);
        auto key = HashString(sName);

        
        if (drawPropIdLUT.size() >= 0xFFFF)
        {
            Internal::PushHubReport(HubReport_Limit, L"[Hub] CreateDrawProp: id 소진 (16비트)");
            return 0;
        }

        if (prop.VSId >= shaderIdLUT.size() || prop.PSId >= shaderIdLUT.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateDrawProp: 셰이더 id 범위 밖");
            return 0;
        }

        DrawPropID id = static_cast<DrawPropID>(drawPropIdLUT.size());

        auto result = drawPropIdLUT.try_emplace(key, id);

        if (!result.second)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateDrawProp: 이미 있는 이름");
            return 0;
        }

            
        uploadDrawPropList.emplace_back(prop);

        return id;
    }

    TexPropID CreateTexProp(const char *name, TextureProp prop)
    {
        std::string_view sName(name);
        auto key = HashString(sName);

        
        if (texPropIdLUT.size() >= 0xFFFF)
        {
            Internal::PushHubReport(HubReport_Limit, L"[Hub] CreateTexProp: id 소진 (16비트)");
            return 0;
        }

        for (size_t i = 0; i < TEXPROP_SLOT_COUNT; i++)
        {
            if (prop.texId[i] >= textureIdLUT.size())
            {
                Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateTexProp: 텍스처 id 범위 밖 (0 으로)");
                prop.texId[i] = 0;
            }
        }

        TexPropID id = static_cast<TexPropID>(texPropIdLUT.size());

        auto result = texPropIdLUT.try_emplace(key, id);

        if (!result.second)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] CreateTexProp: 이미 있는 이름");
            return 0;
        }


        uploadTexPropList.emplace_back(prop);

        return id;
    }


    MeshID LinkComputeBufferAsMesh(const wchar_t *csBufferName, const wchar_t *meshName, 
                                    std::vector<VIndex>&& indices, std::vector<SubMesh>&& subMeshes)
    {
        Internal::PushHubReport(HubReport_Contract, L"[미지원] LinkComputeBufferAsMesh: 컴퓨트 버퍼 링크 메쉬는 이 버전에서 막혀 있음");
        return 0;

        if (indices.empty())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] LinkComputeBufferAsMesh: 인덱스 정보 비어있음");
            return 0;
        }

        
        auto csBufferKey = HashString(std::wstring_view(csBufferName));

        if (!csBufferIdLUT.contains(csBufferKey))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] LinkComputeBufferAsMesh: 없는 이름");
            return 0;
        }

        auto csBufferId = csBufferIdLUT[csBufferKey];
        if (csBufferId < RESERVED_CSBUFFER_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] LinkComputeBufferAsMesh: 예약 컴퓨트 버퍼 연결 시도");
            return 0;
        }
        

        auto meshKey = HashString(std::wstring_view(meshName));

        if (meshIdLUT.contains(meshKey))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] LinkComputeBufferAsMesh: 이미 있는 이름");
            return 0;
        }

        if (meshIdLUT.size() >= 0xFFFF)
        {
            Internal::PushHubReport(HubReport_Limit, L"[Hub] LinkComputeBufferAsMesh: 메쉬 id 소진 (16비트)");
            return 0;
        }


        MeshID meshId = static_cast<MeshID>(meshIdLUT.size());
        meshIdLUT.emplace(meshKey, meshId);
        meshSubMeshCountList.push_back(subMeshes.empty() ? (uint16_t)1 : (uint16_t)subMeshes.size());
        uploadMeshList.emplace_back(Mesh{});

        linkCSBufferAsMeshList.emplace_back(meshId, csBufferId, std::move(indices), std::move(subMeshes));

        return meshId;
    }


    MeshID GetMeshId(std::wstring_view name)
    {
        auto key = HashString(name);

        if (!meshIdLUT.contains(key))
            return 0;
            
        return meshIdLUT[key];
    }

    TexID GetTexId(std::wstring_view name)
    {
        auto key = HashString(name);

        if (!textureIdLUT.contains(key))
            return 0;

        return textureIdLUT[key];
    }

    CSBufferID GetComputeBufferId(std::wstring_view name)
    {
        auto key = HashString(name);

        if (!csBufferIdLUT.contains(key))
            return 0;

        return csBufferIdLUT[key];
    }


    ShaderID GetShaderId(std::wstring_view name)
    {
        auto key = HashString(name);

        if (!shaderIdLUT.contains(key))
            return 0;

        return shaderIdLUT[key];
    }

    DrawPropID GetDrawPropId(std::string_view name)
    {
        auto key = HashString(name);

        if (!drawPropIdLUT.contains(key))
            return 0;

        return drawPropIdLUT[key];
    }

    TexPropID GetTexPropId(std::string_view name)
    {
        auto key = HashString(name);

        if (!texPropIdLUT.contains(key))
            return 0;

        return texPropIdLUT[key];
    }


    uint16_t GetSubMeshCount(MeshID id)
    {
        if (id >= meshSubMeshCountList.size())
            return 0;

        return meshSubMeshCountList[id];
    }


    bool UpdateMesh(std::wstring_view name, Mesh&& mesh)
    {
        auto key = HashString(name);

        if (!meshIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateMesh: 없는 이름");
            return false;
        }

        auto id = meshIdLUT[key];
        if (id < RESERVED_MESH_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateMesh: 예약 메쉬 업데이트 시도");
            return false;
        }

        updateMeshList.emplace_back(std::make_pair(id, std::move(mesh)));

        return true;
    }

    bool UpdateTexture(std::wstring_view name, Texture&& tex)
    {
        if (tex.usage != TextureUsage_ReadOnly)
        {
            Internal::PushHubReport(HubReport_Contract, L"[미지원] UpdateTexture: 쓰기 텍스처 (usage RT / DS / Compute) 는 이 버전에서 막혀 있음");
            return false;
        }

        if (!ValidateTextureUsage(tex))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateTexture: 포맷이 usage 와 안 맞음 (DS = 깊이 포맷만 / RT, Compute = 깊이, 블록 압축 불가 / Compute = sRGB 불가)");
            return false;
        }

        auto key = HashString(name);

        if (!textureIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateTexture: 없는 이름");
            return false;
        }

        auto id = textureIdLUT[key];
        if (id < RESERVED_TEXTURE_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateTexture: 예약 텍스처 업데이트 시도");
            return false;
        }

        updateTextureList.emplace_back(std::make_pair(id, std::move(tex)));

        return true;
    }

    bool UpdateComputeBuffer(std::wstring_view name, const ComputeBuffer& desc)
    {
        Internal::PushHubReport(HubReport_Contract, L"[미지원] UpdateComputeBuffer: 컴퓨트 버퍼는 이 버전에서 막혀 있음");
        return false;

        if (!ValidateComputeBuffer(desc))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateComputeBuffer: stride(4의 배수) / count 0 / data 크기 불일치");
            return false;
        }

        auto key = HashString(name);

        if (!csBufferIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateComputeBuffer: 없는 이름");
            return false;
        }

        auto id = csBufferIdLUT[key];
        if (id < RESERVED_CSBUFFER_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateComputeBuffer: 예약 컴퓨트 버퍼 업데이트 시도");
            return false;
        }

        updateCSBufferList.emplace_back(id, desc);

        return true;
    }


    bool UpdateDrawProp(std::string_view name, DrawProp prop)
    {
        auto key = HashString(name);

        if (!drawPropIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateDrawProp: 없는 이름");
            return false;
        }

        auto id = drawPropIdLUT[key];
        if (id < RESERVED_DRAWPROP_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateDrawProp: 빌트인 drawProp 업데이트 시도");
            return false;
        }

        updateDrawPropList.push_back(std::make_pair(id, prop));

        return true;
    }

    bool UpdateTexProp(std::string_view name, TextureProp prop)
    {
        auto key = HashString(name);

        if (!texPropIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateTexProp: 없는 이름");
            return false;
        }

        auto id = texPropIdLUT[key];
        if (id < RESERVED_TEXPROP_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] UpdateTexProp: 기본 texProp 업데이트 시도");
            return false;
        }

        updateTexPropList.push_back(std::make_pair(id, prop));

        return true;
    }


    bool ReleaseMesh(std::wstring_view name)
    {
        auto key = HashString(name);

        if (!meshIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseMesh: 없는 이름");
            return false;
        }

        auto id = meshIdLUT[key];
        if (id < RESERVED_MESH_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseMesh: 예약 메쉬 해제 시도");
            return false;
        }

        ReleaseMeshList.push_back(id);

        return true;
    }

    bool ReleaseTexture(std::wstring_view name)
    {
        auto key = HashString(name);

        if (!textureIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseTexture: 없는 이름");
            return false;
        }

        auto id = textureIdLUT[key];
        if (id < RESERVED_TEXTURE_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseTexture: 예약 텍스처 해제 시도");
            return false;
        }

        ReleaseTextureList.push_back(id);

        return true;
    }

    bool ReleaseComputeBuffer(std::wstring_view name)
    {
        auto key = HashString(name);

        if (!csBufferIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseComputeBuffer: 없는 이름");
            return false;
        }

        auto id = csBufferIdLUT[key];
        if (id < RESERVED_CSBUFFER_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseComputeBuffer: 예약 컴퓨트 버퍼 해제 시도");
            return false;
        }

        ReleaseCSBufferList.push_back(id);

        return true;
    }


    bool ReleaseDrawProp(std::string_view name)
    {
        auto key = HashString(name);

        if (!drawPropIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseDrawProp: 없는 이름");
            return false;
        }

        auto id = drawPropIdLUT[key];
        if (id < RESERVED_DRAWPROP_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseDrawProp: 빌트인 drawProp 해제 시도");
            return false;
        }

        ReleaseDrawPropList.push_back(id);

        return true;
    }
    
    bool ReleaseTexProp(std::string_view name)
    {
        auto key = HashString(name);

        if (!texPropIdLUT.contains(key))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseTexProp: 없는 이름");
            return false;
        }

        auto id = texPropIdLUT[key];
        if (id < RESERVED_TEXPROP_COUNT)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ReleaseTexProp: 기본 texProp 해제 시도");
            return false;
        }

        ReleaseTexPropList.push_back(id);

        return true;
    }

    #pragma endregion

}
