#pragma once

#include "DataHub.h"

// 데이터허브 사용에 도움을 주는 조작 헬퍼들입니다
// 선택 기능 헤더


namespace Hakko::DataHub
{
    
    #pragma region Interface

    template <std::derived_from<RTB> T>
    void SendRTB(const T& RTB, uint8_t layer = 0U) = delete;
    
    template <std::derived_from<RTB> T>
    void SendRTB(const T& RTB, ActionID action, uint8_t layer = 0U) = delete;


    // (미구현) 커스텀 파이프라인용 액션 노드 추가
    template <std::derived_from<RTB_Action> T>
    bool AddActionNode(const T& Action, ActionID frontNode, uint8_t layer = 0U)
    {
        Internal::PushHubReport(HubReport_Contract, L"[Hub] AddActionNode: 커스텀 파이프라인 미구현");
        return false;
    }


    // SendRTB 로 전송
    #pragma region LayerCommand

    struct RenderTransferBox_SetRenderCondition : public RTB
    {
        RenderCondition condition;
    };
    typedef RenderTransferBox_SetRenderCondition RTB_SetRenderCondition, RTB_SetCondition;


    struct RenderTransferBox_SetRenderCamera : public RTB
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX viewProj;
        DirectX::XMFLOAT3 eyePos;
    };
    typedef RenderTransferBox_SetRenderCamera RTB_SetRenderCamera, RTB_SetCamera;


    struct RenderTransferBox_SetRenderGlobalLight : public RTB
    {
        DirectX::XMFLOAT4 ambient;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT3 direction;
    };
    typedef RenderTransferBox_SetRenderGlobalLight RTB_SetRenderGlobalLight, RTB_SetGlobalLight;


    struct RenderTransferBox_SetRenderTime : public RTB
    {
        float totalTime;
    };
    typedef RenderTransferBox_SetRenderTime RTB_SetRenderTime, RTB_SetTime;



    struct RenderTransferBox_SetRenderLight : public RTB
    {
        DataBoxID light;
        uint16_t slot;
    };
    typedef RenderTransferBox_SetRenderLight RTB_SetRenderLight, RTB_SetLight;



    struct RenderTransferBox_SetRenderLightMatrix : public RTB
    {
        DataBoxID lightMatrix;
        uint16_t slot;
    };
    typedef RenderTransferBox_SetRenderLightMatrix RTB_SetRenderLightMatrix, RTB_SetLightMatrix;

    #pragma endregion


    void SetLayerBackground(TexID texId, uint8_t layer = 0U);       // 0 이면 안 그림

    void SetLayerBackgroundColor(float r, float g, float b, float a, uint8_t layer = 0U);

    void SetLayerEnvMap(TexID texId, uint8_t layer = 0U);           // 큐브맵만, 0 이면 없음

    void SetLayerViewport(const Layer_Viewport& viewport, uint8_t layer = 0U);


    #pragma region MakeDrawRequest

    // 간편 사용용: 빌트인 3종 (Opaque / Translucent / Screen) 이 쓰는 속성만 받음
    // 나머지 drawProp 칸과 밉 레벨은 알아서 기본값으로, 세부 조정은 CreateDrawProp / CreateTexProp 직접
    struct DrawOption
    {
        DrawProp_CullType cullType = DrawProp_CullType_Back;
        DrawProp_BlendType blendType = DrawProp_BlendType_None;     // Opaque 요청은 None 만
        bool alphaClipOrTranslucent = false;

        VertexType vertexType = VertexType_Static;
        ShaderID VSId = 0;      // 0 이면 기본 제공 셰이더 (Default_World_VS). 스크린은 기본 제공이 없어서 필수
        ShaderID PSId = 0;      // 0 이면 기본 제공 셰이더 (Opaque = Default_World_OPQ_PS / Translucent = Default_World_TP_PS)

        TexID diffuseTexId = RESERVED_TEX_NONE_DIFFUSE_ID;
        TexID normalTexId = RESERVED_TEX_NONE_NORMAL_ID;
        TexID ORMTexId = RESERVED_TEX_NONE_ORM_ID;
        TexID emissionTexId = RESERVED_TEX_NONE_EMISSION_ID;
        TexID flexibleTexId[TEXPROP_SLOT_COUNT - 4] = {0};
    };

    bool MakeDrawRequestAndProps(
        DrawOption option, 
        const char* drawPropName, const char* texPropName, 
        RTB_DrawOPQ& rtb,
        MeshID meshId = 0U, DataBoxID generalId = 0U, DataBoxID optionalId = 0U
    );

    bool MakeDrawRequestAndProps(
        DrawOption option, 
        const char* drawPropName, const char* texPropName, 
        RTB_DrawTP& rtb,
        MeshID meshId = 0U, DataBoxID generalId = 0U, DataBoxID optionalId = 0U
    );


    bool MakeDrawRequestAndProps(
        DrawOption option, 
        const char* drawPropName, 
        RTB_DrawSC& rtb,
        TexID texId, DataBoxID screenId = 0U
    );

    #pragma endregion


    #pragma region ClearRequests

    // 드로우 요청 비우기 (요청 목록은 서비스가 비우는 계약)
    // 빌트인 파이프라인: SendRTB 로 넣은 요청 종류별로
    void ClearOpaqueDrawRequests(uint8_t layer = 0U);

    void ClearTranslucentDrawRequests(uint8_t layer = 0U);

    void ClearScreenDrawRequests(uint8_t layer = 0U);


    // (미구현) 커스텀 파이프라인용: 액션 id 로 지정
    void ClearDrawRequests(ActionID action, uint8_t layer = 0U);

    #pragma endregion

    #pragma endregion
    


#pragma region Template

template <>
void DataHub::SendRTB<DataHub::RTB_UpdateResources>(const RTB_UpdateResources& RTB, uint8_t layer);


template <>
void DataHub::SendRTB<DataHub::RTB_SetCondition>(const RTB_SetCondition& RTB, uint8_t layer);

template <>
void DataHub::SendRTB<DataHub::RTB_SetCamera>(const RTB_SetCamera& RTB, uint8_t layer);

template <>
void DataHub::SendRTB<DataHub::RTB_SetGlobalLight>(const RTB_SetGlobalLight& RTB, uint8_t layer);

template <>
void DataHub::SendRTB<DataHub::RTB_SetTime>(const RTB_SetTime& RTB, uint8_t layer);

template <>
void DataHub::SendRTB<DataHub::RTB_SetLight>(const RTB_SetLight& RTB, uint8_t layer);

template <>
void DataHub::SendRTB<DataHub::RTB_SetLightMatrix>(const RTB_SetLightMatrix& RTB, uint8_t layer);


template <>
void DataHub::SendRTB<DataHub::RTB_DrawOPQ>(const RTB_DrawOPQ& RTB, uint8_t layer);

template <>
void DataHub::SendRTB<DataHub::RTB_DrawTP>(const RTB_DrawTP& RTB, uint8_t layer);

template <>
void DataHub::SendRTB<DataHub::RTB_DrawSC>(const RTB_DrawSC& RTB, uint8_t layer);

#pragma endregion

}
