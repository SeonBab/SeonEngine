#include "DataHub_Interface.h"

#include <utility>


namespace Hakko::DataHub
{

    #pragma region Interface

    template <>
    void DataHub::SendRTB<DataHub::RTB_UpdateResources>(const RTB_UpdateResources& RTB, uint8_t layer)
    {
        flag_updateResources |= RTB.command;
    }

    #pragma region SetLayerOption

    void SetLayerBackground(TexID texId, uint8_t layer)
    {
        if (layer >= LayerList.size() || texId >= textureIdLUT.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SetLayerBackground: 레이어 / 텍스처 id 범위 밖 (무시됨)");
            return;
        }

        LayerList[layer].backGround.bgTexId = texId;
    }

    void SetLayerBackgroundColor(float r, float g, float b, float a, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SetLayerBackgroundColor: 레이어 범위 밖 (무시됨)");
            return;
        }

        float* color = LayerList[layer].backGround.color;
        color[0] = r;   color[1] = g;   color[2] = b;   color[3] = a;
    }

    void SetLayerEnvMap(TexID texId, uint8_t layer)
    {
        if (layer >= LayerList.size() || texId >= textureIdLUT.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SetLayerEnvMap: 레이어 / 텍스처 id 범위 밖 (무시됨)");
            return;
        }

        LayerList[layer].envTexId = texId;
    }

    void SetLayerViewport(const Layer_Viewport& viewport, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SetLayerViewport: 레이어 범위 밖 (무시됨)");
            return;
        }

        LayerList[layer].viewport = viewport;
    }

    #pragma endregion


    #pragma region SetLayerData

    template <>
    void DataHub::SendRTB<DataHub::RTB_SetCondition>(const RTB_SetCondition& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        LayerList[layer].condition = RTB.condition;
    }

    template <>
    void DataHub::SendRTB<DataHub::RTB_SetCamera>(const RTB_SetCamera& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        RenderCondition& condition = LayerList[layer].condition;
        condition.camera_View = RTB.view;
        condition.camera_Proj = RTB.proj;
        condition.camera_ViewProj = RTB.viewProj;
        condition.camera_EyePos = RTB.eyePos;
    }

    template <>
    void DataHub::SendRTB<DataHub::RTB_SetGlobalLight>(const RTB_SetGlobalLight& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        RenderCondition& condition = LayerList[layer].condition;
        condition.global_LightAmbient = RTB.ambient;
        condition.global_LightColor = RTB.color;
        condition.global_LightDirection = RTB.direction;
    }

    template <>
    void DataHub::SendRTB<DataHub::RTB_SetTime>(const RTB_SetTime& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        LayerList[layer].condition.global_TotalTime = RTB.totalTime;
    }

    template <>
    void DataHub::SendRTB<DataHub::RTB_SetLight>(const RTB_SetLight& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        if (RTB.slot >= RENDER_LIGHT_MAX || RTB.light >= RenderLightList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SetLight: 슬롯 / 조명 id 범위 밖");
            return;
        }

        LayerList[layer].lights[RTB.slot] = RTB.light;
    }

    template <>
    void DataHub::SendRTB<DataHub::RTB_SetLightMatrix>(const RTB_SetLightMatrix& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        if (RTB.slot >= RENDER_LIGHT_MAX || RTB.lightMatrix >= RenderLightMatrixList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SetLightMatrix: 슬롯 / 행렬 id 범위 밖");
            return;
        }

        LayerList[layer].lightMatrixes[RTB.slot] = RTB.lightMatrix;
    }

    #pragma endregion


    #pragma region SendDrawRequest
    
    template <>
    void DataHub::SendRTB<DataHub::RTB_DrawOPQ>(const RTB_DrawOPQ& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        if (RTB.meshId >= meshIdLUT.size() || RTB.subMeshId >= meshSubMeshCountList[RTB.meshId])
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] DrawRequest: 메쉬 / 서브메쉬 id 범위 밖 (요청 버림)");
            return;
        }
        if (RTB.drawProp >= drawPropIdLUT.size() || RTB.texProp >= texPropIdLUT.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] DrawRequest: drawProp / texProp id 범위 밖 (요청 버림)");
            return;
        }

        RenderIdOpaque idp;
        idp.alphaClip = RTB.alphaClip;  idp.layer = RTB.layer;  idp.drawProp = RTB.drawProp;
        idp.texProp = RTB.texProp;      idp.meshId = RTB.meshId;  idp.depth = RTB.depth;

        RTB_DrawRequest req = {};
        req.renderId = idp;
        req.meshId = RTB.meshId;
        req.subMeshId = RTB.subMeshId;
        req.dataBoxIds[0] = RTB.generalId;
        req.dataBoxIds[1] = RTB.optionalId;
        LayerList[layer].actionOPQ.drawReqList.push_back(req);
    }

    template <>
    void DataHub::SendRTB<DataHub::RTB_DrawTP>(const RTB_DrawTP& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        if (RTB.meshId >= meshIdLUT.size() || RTB.subMeshId >= meshSubMeshCountList[RTB.meshId])
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] DrawRequest: 메쉬 / 서브메쉬 id 범위 밖 (요청 버림)");
            return;
        }
        if (RTB.drawProp >= drawPropIdLUT.size() || RTB.texProp >= texPropIdLUT.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] DrawRequest: drawProp / texProp id 범위 밖 (요청 버림)");
            return;
        }

        RenderIdTranslucent idp;
        idp.depth = RTB.depth;  idp.drawProp = RTB.drawProp;  idp.texProp = RTB.texProp;  idp.iid = RTB.iid;

        RTB_DrawRequest req = {};
        req.renderId = idp;
        req.meshId = RTB.meshId;
        req.subMeshId = RTB.subMeshId;
        req.dataBoxIds[0] = RTB.generalId;
        req.dataBoxIds[1] = RTB.optionalId;
        LayerList[layer].actionTP.drawReqList.push_back(req);
    }

    template <>
    void DataHub::SendRTB<DataHub::RTB_DrawSC>(const RTB_DrawSC& RTB, uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] SendRTB: 레이어 범위 밖");
            return;
        }

        if (RTB.drawProp >= drawPropIdLUT.size() || RTB.texture >= textureIdLUT.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] DrawRequest(SC): drawProp / 텍스처 id 범위 밖 (요청 버림)");
            return;
        }

        RenderIdScreen idp;
        idp.layer = RTB.layer;  idp.translucent = RTB.translucent;  idp.texture = RTB.texture;
        idp.drawProp = RTB.drawProp;  idp.iid = RTB.iid;

        RTB_DrawRequest req = {};
        req.renderId = idp;
        req.meshId = 1;
        req.subMeshId = 0;
        req.dataBoxIds[0] = RTB.screenId;
        LayerList[layer].actionSC.drawReqList.push_back(req);
    }

    #pragma endregion


    #pragma region MakeDrawRequest

    #pragma region Helper

    static DrawProp MakeDrawProp(const DrawOption& option, DrawProp_DepthUsage depthUsage, ShaderID defaultPSId)
    {
        DrawProp dp = {};
        dp.depthUsage = depthUsage;
        dp.cullType = option.cullType;
        dp.blendType = option.blendType;
        dp.fillType = DrawProp_FillType_Solid;
        dp.depthBias = DrawProp_DepthBias_None;
        dp.stecilOpt.useStencil = false;
        dp.topology = DrawProp_Topology_TriangleList;
        dp.vertexType = option.vertexType;
        dp.VSId = option.VSId != 0 ? option.VSId : RESERVED_SHADER_WORLD_VS_ID;
        dp.PSId = option.PSId != 0 ? option.PSId : defaultPSId;

        return dp;
    }

    static TextureProp MakeTexProp(const DrawOption& option)
    {
        TextureProp tp = {};
        tp.texId[0] = option.diffuseTexId;
        tp.texId[1] = option.normalTexId;
        tp.texId[2] = option.ORMTexId;
        tp.texId[3] = option.emissionTexId;
        for (size_t i = 4; i < TEXPROP_SLOT_COUNT; i++)
            tp.texId[i] = option.flexibleTexId[i - 4];

        return tp;
    }

    static bool RegisterProps(
        const char* drawPropName, const DrawProp& dp, DrawPropID& dpId,
        const char* texPropName, const TextureProp* tp, TexPropID& tpId)
    {
        if (dp.VSId >= shaderIdLUT.size() || dp.PSId >= shaderIdLUT.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] MakeDrawRequestAndProps: 셰이더 id 범위 밖");
            return false;
        }

        const auto dpKey = HashString(std::string_view(drawPropName));
        const auto tpKey = tp != nullptr ? HashString(std::string_view(texPropName)) : 0U;

        if (drawPropIdLUT.contains(dpKey) || (tp != nullptr && texPropIdLUT.contains(tpKey)))
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] MakeDrawRequestAndProps: 이미 있는 이름");
            return false;
        }

        if (drawPropIdLUT.size() >= 0xFFFF || (tp != nullptr && texPropIdLUT.size() >= 0xFFFF))
        {
            Internal::PushHubReport(HubReport_Limit, L"[Hub] MakeDrawRequestAndProps: id 소진 (16비트)");
            return false;
        }

        dpId = static_cast<DrawPropID>(drawPropIdLUT.size());
        drawPropIdLUT.emplace(dpKey, dpId);
        uploadDrawPropList.emplace_back(dp);

        if (tp != nullptr)
        {
            tpId = static_cast<TexPropID>(texPropIdLUT.size());
            texPropIdLUT.emplace(tpKey, tpId);
            uploadTexPropList.emplace_back(*tp);
        }

        return true;
    }

    #pragma endregion


    bool MakeDrawRequestAndProps(
        DrawOption option,
        const char* drawPropName, const char* texPropName,
        RTB_DrawOPQ& rtb,
        MeshID meshId, DataBoxID generalId, DataBoxID optionalId)
    {
        if (option.blendType != DrawProp_BlendType_None)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] MakeDrawRequestAndProps(OPQ): 블렌드 옵션은 Translucent 요청으로");
            return false;
        }

        const DrawProp dp = MakeDrawProp(option, DrawProp_DepthUsage_Opaque, RESERVED_SHADER_WORLD_OPQ_PS_ID);
        const TextureProp tp = MakeTexProp(option);

        DrawPropID dpId = 0;
        TexPropID tpId = 0;
        if (!RegisterProps(drawPropName, dp, dpId, texPropName, &tp, tpId))
            return false;


        rtb.alphaClip = option.alphaClipOrTranslucent;
        rtb.layer = 0U;
        rtb.drawProp = dpId;
        rtb.texProp = tpId;
        rtb.meshId = meshId;
        rtb.depth = 0U;
        rtb.subMeshId = 0U;
        rtb.generalId = generalId;
        rtb.optionalId = optionalId;


        return true;
    }

    bool MakeDrawRequestAndProps(
        DrawOption option,
        const char *drawPropName, const char *texPropName,
        RTB_DrawTP &rtb,
        MeshID meshId, DataBoxID generalId, DataBoxID optionalId)
    {
        const DrawProp dp = MakeDrawProp(option, DrawProp_DepthUsage_Translucent, RESERVED_SHADER_WORLD_TP_PS_ID);
        const TextureProp tp = MakeTexProp(option);

        DrawPropID dpId = 0;
        TexPropID tpId = 0;
        if (!RegisterProps(drawPropName, dp, dpId, texPropName, &tp, tpId))
            return false;


        rtb.depth = 0U;
        rtb.drawProp = dpId;
        rtb.texProp = tpId;
        rtb.iid = 0U;
        rtb.meshId = meshId;
        rtb.subMeshId = 0U;
        rtb.generalId = generalId;
        rtb.optionalId = optionalId;


        return true;
    }

    bool MakeDrawRequestAndProps(
        DrawOption option,
        const char* drawPropName,
        RTB_DrawSC& rtb,
        TexID texId, DataBoxID screenId)
    {
        if (option.VSId == 0 || option.PSId == 0)
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] MakeDrawRequestAndProps(SC): 스크린은 기본 제공 셰이더가 없음 (VSId / PSId 필수)");
            return false;
        }

        DrawProp dp = MakeDrawProp(option, DrawProp_DepthUsage_Screen, 0);
        dp.vertexType = VertexType_Simple;

        DrawPropID dpId = 0;
        TexPropID tpId = 0;
        if (!RegisterProps(drawPropName, dp, dpId, nullptr, nullptr, tpId))
            return false;


        rtb.layer = 0U;
        rtb.translucent = option.alphaClipOrTranslucent;
        rtb.texture = texId;
        rtb.drawProp = dpId;
        rtb.iid = 0U;
        rtb.screenId = screenId;


        return true;
    }

    #pragma endregion


    #pragma region ClearRequests

    void ClearOpaqueDrawRequests(uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ClearDrawRequests: 레이어 범위 밖");
            return;
        }

        LayerList[layer].actionOPQ.drawReqList.clear();
    }

    void ClearTranslucentDrawRequests(uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ClearDrawRequests: 레이어 범위 밖");
            return;
        }

        LayerList[layer].actionTP.drawReqList.clear();
    }

    void ClearScreenDrawRequests(uint8_t layer)
    {
        if (layer >= LayerList.size())
        {
            Internal::PushHubReport(HubReport_Contract, L"[Hub] ClearDrawRequests: 레이어 범위 밖");
            return;
        }

        LayerList[layer].actionSC.drawReqList.clear();
    }


    void ClearDrawRequests(ActionID action, uint8_t layer)
    {
        Internal::PushHubReport(HubReport_Contract, L"[Hub] ClearDrawRequests: 액션 id 지정은 커스텀 파이프라인용 (미구현), 빌트인은 Clear~DrawRequests 사용");
    }

    #pragma endregion

    #pragma endregion

    

}
