#pragma once

#include "DataHub_Types.h"


namespace Hakko::DataHub
{

    #pragma region ResourceControl

    #define RESERVED_MESH_COUNT 2
    #define RESERVED_TEXTURE_COUNT 6
    #define RESERVED_CSBUFFER_COUNT 1
    #define RESERVED_SHADER_COUNT 9
    #define RESERVED_DRAWPROP_COUNT 7
    #define RESERVED_TEXPROP_COUNT 1


    #define RESERVED_MESH_QUAD_ID 1

    #define RESERVED_TEX_ERROR_ID 0
    #define RESERVED_TEX_TRANSPARENT_ID 1
    #define RESERVED_TEX_NONE_DIFFUSE_ID 2
    #define RESERVED_TEX_NONE_NORMAL_ID 3
    #define RESERVED_TEX_NONE_ORM_ID 4
    #define RESERVED_TEX_NONE_EMISSION_ID 5

    #define RESERVED_TEX_NONE_DIFFUSE_NAME L"None_Diffuse"
    #define RESERVED_TEX_NONE_NORMAL_NAME L"None_Normal"
    #define RESERVED_TEX_NONE_ORM_NAME L"None_ORM"
    #define RESERVED_TEX_NONE_EMISSION_NAME L"None_Emission"
    

    #define RESERVED_SHADER_WORLD_VS_ID 6
    #define RESERVED_SHADER_WORLD_OPQ_PS_ID 7
    #define RESERVED_SHADER_WORLD_TP_PS_ID 8

    #define RESERVED_SHADER_WORLD_VS_NAME L"Default_World_VS"
    #define RESERVED_SHADER_WORLD_OPQ_PS_NAME L"Default_World_OPQ_PS"
    #define RESERVED_SHADER_WORLD_TP_PS_NAME L"Default_World_TP_PS"
    

    #define RESERVED_DRAWPROP_SKYBOX_ID 1
    #define RESERVED_DRAWPROP_BG_ID 2
    #define RESERVED_DRAWPROP_OPAQUE_ID 3
    #define RESERVED_DRAWPROP_TRANSLUCENT_ID 4
    #define RESERVED_DRAWPROP_OPAQUE_SKINNED_ID 5
    #define RESERVED_DRAWPROP_TRANSLUCENT_SKINNED_ID 6
    
    #define RESERVED_DRAWPROP_OPAQUE_NAME "Default_Opaque"
    #define RESERVED_DRAWPROP_TRANSLUCENT_NAME "Default_Translucent"
    #define RESERVED_DRAWPROP_OPAQUE_SKINNED_NAME "Default_Opaque_Skinned"
    #define RESERVED_DRAWPROP_TRANSLUCENT_SKINNED_NAME "Default_Translucent_Skinned"


    
    // Create / Update / Link 는 만든 프레임에 해당 RTB (RTBCMD_Upload / RTBCMD_Update) 를 보내야 함
    // 미루면 그 사이에 실행된 해제 때문에 대기 중인 id 가 어긋남... (적어도 해제를 동일 프레임에 하면 문제 x)
    MeshID CreateMesh(const wchar_t* name, Mesh&& mesh);
    TexID CreateTexture(const wchar_t* name, Texture&& tex);
    CSBufferID CreateComputeBuffer(const wchar_t* name, const ComputeBuffer& desc);    // 미구현

    ShaderID RegisterShader(const wchar_t* path, ShaderType shaderType);
    DrawPropID CreateDrawProp(const char* name, DrawProp prop);
    TexPropID CreateTexProp(const char* name, TextureProp prop);

    MeshID LinkComputeBufferAsMesh(const wchar_t* csBufferName, const wchar_t* meshName, 
                                    std::vector<VIndex>&& indices, std::vector<SubMesh>&& subMeshes);     // 미구현


    MeshID GetMeshId(std::wstring_view name);
    TexID GetTexId(std::wstring_view name);
    CSBufferID GetComputeBufferId(std::wstring_view name);  // 미구현

    ShaderID GetShaderId(std::wstring_view name);           // 해제가 없어서 그냥 아이디 저장해도 무방하긴 함
    DrawPropID GetDrawPropId(std::string_view name);
    TexPropID GetTexPropId(std::string_view name);

    uint16_t GetSubMeshCount(MeshID id);

    
    bool UpdateMesh(std::wstring_view name, Mesh&& mesh);
    bool UpdateTexture(std::wstring_view name, Texture&& tex);
    bool UpdateComputeBuffer(std::wstring_view name, const ComputeBuffer& desc);   // 미구현

    bool UpdateDrawProp(std::string_view name, DrawProp prop);
    bool UpdateTexProp(std::string_view name, TextureProp prop);


    bool ReleaseMesh(std::wstring_view name);
    bool ReleaseTexture(std::wstring_view name);
    bool ReleaseComputeBuffer(std::wstring_view name);   // 미구현

    bool ReleaseDrawProp(std::string_view name);
    bool ReleaseTexProp(std::string_view name);



    extern std::vector<Mesh> uploadMeshList;
    extern std::vector<Texture> uploadTextureList;
    extern std::vector<ComputeBuffer> uploadCSBufferList;
    extern std::vector<ShaderInfo> loadShaderList;
    extern std::vector<DrawProp> uploadDrawPropList;
    extern std::vector<TextureProp> uploadTexPropList;

    extern std::vector<uint16_t> meshSubMeshCountList;
    
    extern std::vector<CSMeshLinkData> linkCSBufferAsMeshList;

    
    extern std::unordered_map<HashedString, MeshID> meshIdLUT;
    extern std::unordered_map<HashedString, TexID> textureIdLUT;
    extern std::unordered_map<HashedString, CSBufferID> csBufferIdLUT;
    extern std::unordered_map<HashedString, ShaderID> shaderIdLUT;
    extern std::unordered_map<HashedString, DrawPropID> drawPropIdLUT;
    extern std::unordered_map<HashedString, TexPropID> texPropIdLUT;


    extern std::vector<std::pair<MeshID, Mesh>> updateMeshList;
    extern std::vector<std::pair<TexID, Texture>> updateTextureList;
    extern std::vector<std::pair<CSBufferID, ComputeBuffer>> updateCSBufferList;
    
    extern std::vector<std::pair<DrawPropID, DrawProp>> updateDrawPropList;
    extern std::vector<std::pair<TexPropID, TextureProp>> updateTexPropList;


    extern std::vector<MeshID> ReleaseMeshList;
    extern std::vector<TexID> ReleaseTextureList;
    extern std::vector<CSBufferID> ReleaseCSBufferList;
    // 셰이더는 현재 내리기 지원 안함
    extern std::vector<DrawPropID> ReleaseDrawPropList;
    extern std::vector<TexPropID> ReleaseTexPropList;

    #pragma endregion

}
