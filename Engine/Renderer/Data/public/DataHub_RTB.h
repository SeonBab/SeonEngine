#pragma once

#include "RenderOption.h"
#include "DataHub_Types.h"

// RenderTransferBox: 렌더러로 전송할 요청 단위 (드로우 리퀘스트 / 액션 / 레이어)
// Canvas(RT) - Layer(Pipeline) - Action(Pass) - DrawRequest(Data) 순 포함구조


namespace Hakko::DataHub
{

    #pragma region RenderTransferBox

    struct RenderTransferBox
    {
    protected:
        RenderTransferBox() = default;
    };
    typedef RenderTransferBox RTB;


    #pragma region DrawRequest

    #define DRAWREQ_DATABOX_MAX 5

    struct RenderTransferBox_DrawRequest : public RTB
    {
        RenderID renderId;      // RenderIDProvider 확장 클래스를 이용하세요
        MeshID meshId;          // renderId 안의 메쉬 비트는 정렬용, 조회는 여기서
        SubMeshID subMeshId;

        DataBoxID dataBoxIds[DRAWREQ_DATABOX_MAX];
    };
    typedef RenderTransferBox_DrawRequest RTB_DrawRequest;
    

    #pragma region BuiltInRequests

    struct RenderTransferBox_DrawRequestShadow : public RTB
    {
        // 알파(1) psoBody(16) 텍스처조합(16) 메쉬(16) 뎁스(8) 잉여(7)
        bool alphaClip;
        DrawPropID drawProp;
        TexPropID texProp;
        MeshID meshId;
        uint8_t depth;
        uint8_t iid;

        SubMeshID subMeshId;
        DataBoxID generalId;
        DataBoxID optionalId;
    };
    typedef RenderTransferBox_DrawRequestShadow RTB_DrawRequestShadow, RTB_DrawSDW;


    struct RenderTransferBox_DrawRequestOpaque : public RTB
    {
        // 알파(1) 레이어(7) psoBody(16) 텍스처조합(16) 메쉬(16) 뎁스(8)
        bool alphaClip;
        uint8_t layer;
        DrawPropID drawProp;
        TexPropID texProp;
        MeshID meshId;
        uint8_t depth;

        SubMeshID subMeshId;
        DataBoxID generalId;
        DataBoxID optionalId;
    };
    typedef RenderTransferBox_DrawRequestOpaque RTB_DrawRequestOpaque, RTB_DrawOPQ;

    
    struct RenderTransferBox_DrawRequestTranslucent : public RTB
    {
        // 뎁스(16) psoBody(16) 텍스처조합(16) 잉여(16)
        uint16_t depth;
        DrawPropID drawProp;
        TexPropID texProp;
        uint16_t iid;

        MeshID meshId;
        SubMeshID subMeshId;
        DataBoxID generalId;
        DataBoxID optionalId;
    };
    typedef RenderTransferBox_DrawRequestTranslucent RTB_DrawRequestTranslucent, RTB_DrawTP;

    
    struct RenderTransferBox_DrawRequestScreen : public RTB
    {
        // 레이어(16) 반투명(1) psoBody(16) 텍스처(16) 잉여(15)
        uint16_t layer;
        bool translucent;
        DrawPropID drawProp;
        TexID texture;
        uint16_t iid;

        DataBoxID screenId;
    };
    typedef RenderTransferBox_DrawRequestScreen RTB_DrawRequestScreen, RTB_DrawSC;

    #pragma endregion

    #pragma endregion



    #pragma region ResourceCommand

    // 비트 조합 가능 (예: RTBCMD_Upload | RTBCMD_Release_Prop)
    enum RTBCMD_UpdateResource : uint32_t
    {
        RTBCMD_Upload_Resource  = 1 << 0,       // 메쉬 / 텍스처 / 컴퓨트 버퍼 / 링크 / 셰이더
        RTBCMD_Upload_Prop      = 1 << 1,       // drawProp / texProp
        RTBCMD_Update_Resource  = 1 << 2,
        RTBCMD_Update_Prop      = 1 << 3,
        RTBCMD_Release_Resource = 1 << 4,
        RTBCMD_Release_Prop     = 1 << 5,

        RTBCMD_Upload  = RTBCMD_Upload_Resource  | RTBCMD_Upload_Prop,
        RTBCMD_Update  = RTBCMD_Update_Resource  | RTBCMD_Update_Prop,
        RTBCMD_Release = RTBCMD_Release_Resource | RTBCMD_Release_Prop,
    };

    struct RenderTransferBox_UpdateResources : public RTB
    {
        uint32_t command;       // RTBCMD_UpdateResource 조합
    };
    typedef RenderTransferBox_UpdateResources RTB_UpdateResources;


    extern uint32_t flag_updateResources;

    #pragma endregion



    #pragma region Action

    struct Action_Viewport
    {
        float originX;
        float originY;
        float width;
        float height;
        float minZ = 0.0f;
        float maxZ = 1.0f;
    };

    struct Action_Rect
    {
        int32_t originX;
        int32_t originY;
        int32_t width;
        int32_t height;
    };


    #pragma region IOSignature

    // 선행 액션에서 받은 임시 출력 자원의 사용방식
    enum Action_InputType
    {
        Action_InputType_NoConvert = 0,      // 그대로 이어그리기
        Action_InputType_Convert = 0x01,     // 읽기용 변환 (복사 우선)
        Action_InputType_Copy = 0x02,        // 읽기용 복사
        Action_InputType_Detach = 0x04,      // 임시 자원 폐기 (변환, 복사 우선) 후 새 자원으로
        
        Action_InputType_ClearSR = 0x08,     // 이전 임시 SR 폐기

        Action_Input_DrawDefault = Action_InputType_NoConvert,
        Action_Input_EditDefault = Action_InputType_Convert,
        Action_Input_ComputeDefault = Action_InputType_Detach,
    };


    #define RENDER_TARGET_MAX 8

    struct Action_Input
    {
        std::array<Action_InputType, RENDER_TARGET_MAX> RTInputType;
        Action_InputType DSInputType;
    };


    enum Action_Output_BlendOption
    {
        Action_Output_BlendOption_ByBlendType,

        Action_Output_BlendOption_ForceNone,
        Action_Output_BlendOption_ForceAlpha,
        Action_Output_BlendOption_ForceAddtive,
        Action_Output_BlendOption_ForceMultiply,
    };

    enum Action_Output_DepthOption
    {
        Action_Output_DepthOption_ByDepthUsage,

        Action_Output_DepthOption_ForceDepthRead,
    };

    enum Action_Output_LoadOption
    {
        Action_Output_LoadOption_Default,

        Action_Output_LoadOption_Load,
        Action_Output_LoadOption_Clear,
    };

    // 출력 레이아웃 (의미 정보): PS가 어느 칸에 뭘 쓰는지, 셰이더 변형 컴파일용
    enum Action_OutputLayout
    {
        OutputLayout_None = 0,      // 매크로/검증 없이 원본 PS 그대로 (Edit / Compute / PP 경로)
        OutputLayout_DepthOnly,     // DS 만, PS 없음 (LATER 알파클립 drawProp 은 clip 전용 PS)
        OutputLayout_Color,         // RT0 = 최종 색 (포워드 / 스크린 / 디버그)
        OutputLayout_ColorId,       // RT0 = 색, RT1 = id (UI / 피킹)
        OutputLayout_GBuffer,       // RT0 알베도, RT1 노멀, RT2 ORM, RT3 이미션 (a 채널 예약)

        OutputLayout_Count,
    };

    inline uint8_t OutputLayoutTargetMask(Action_OutputLayout layout)
    {
        switch (layout)
        {
        case OutputLayout_Color:    return 0b0001;
        case OutputLayout_ColorId:  return 0b0011;
        case OutputLayout_GBuffer:  return 0b1111;
        default:                    return 0;
        }
    }

    // 액션에서 사용할 출력 정보
    struct Action_Output
    {
        std::array<bool, RENDER_TARGET_MAX> useRT = {0};
        std::array<TexID, RENDER_TARGET_MAX> RTTex = {0};     // 0이면 임시 리소스 사용, InputType 적용
        std::array<Action_Output_BlendOption, RENDER_TARGET_MAX> blendOpt = {(Action_Output_BlendOption)0};
        std::array<Action_Output_LoadOption, RENDER_TARGET_MAX> loadOpt = {(Action_Output_LoadOption)0};

        bool useDS = 0;
        TexID DSTex = 0;
        Action_Output_DepthOption depthOpt = {(Action_Output_DepthOption)0};
        Action_Output_LoadOption dsLoadOpt = {(Action_Output_LoadOption)0};

        Action_OutputLayout outputLayout = OutputLayout_None;   // PS 변형 생성용 키 (상기 정보와 일치해야 함)
    };

    #define RENDER_ACTION_NODE_MAX 8    

    struct RenderTransferBox_Action : public RTB
    {
    public:
        std::array<Action_Input, RENDER_ACTION_NODE_MAX> inputs;
        Action_Output output;

        Action_Viewport viewport;
        Action_Rect rect;

        DataBoxTID actionDataBoxTypeId;
        DataBoxID actionDataBoxId;

    protected:
        RenderTransferBox_Action(Action_InputType defaultInput)
        {
            for (auto& node : inputs)
            {
                node.RTInputType.fill(defaultInput);
                node.DSInputType = defaultInput;
            }
            viewport = {};  rect = {};
            actionDataBoxTypeId = 0;    actionDataBoxId = 0;
        }
    };
    typedef RenderTransferBox_Action RTB_Action;    

    typedef uint32_t ActionID;

    #pragma endregion


    #pragma region ActionVariant

    struct RenderTransferBox_ActionDraw : public RTB_Action
    {
        RenderIDType renderIdType;
        std::array<DataBoxTID, DRAWREQ_DATABOX_MAX> dataBoxTypeIds;
        std::vector<RTB_DrawRequest> drawReqList;

        RenderTransferBox_ActionDraw() : RenderTransferBox_Action(Action_Input_DrawDefault)
        {
            output.useRT[0] = true;
            output.useDS = true;
            output.outputLayout = OutputLayout_Color;
            renderIdType = RenderIdType_None;
            dataBoxTypeIds.fill(0);
        }
    };
    typedef RenderTransferBox_ActionDraw RTB_ActionDraw;


    struct RenderTransferBox_ActionEdit : public RTB_Action
    {
        DrawPropID drawProp;
        TexPropID texProp;

        RenderTransferBox_ActionEdit() : RenderTransferBox_Action(Action_Input_EditDefault)
        {
            drawProp = 0;   texProp = 0;
        }
    };
    typedef RenderTransferBox_ActionEdit RTB_ActionEdit;
    

    enum ActionCompute_Binding_Type
    {
        ActionCompute_Binding_Type_None,
        ActionCompute_Binding_Type_Buffer,
        ActionCompute_Binding_Type_Texture,
    };

    struct ActionCompute_Binding
    {
        ActionCompute_Binding_Type type;
        uint16_t resId;
    };

    #define COMPUTE_BINDING_MAX 4

    struct RenderTransferBox_ActionCompute : public RTB_Action
    {
        // 미구현 (컴퓨트 경로)
        ShaderID csId;
        uint32_t dispatchX;
        uint32_t dispatchY;
        uint32_t dispatchZ;

        std::array<ActionCompute_Binding, COMPUTE_BINDING_MAX> writes;
        std::array<ActionCompute_Binding, COMPUTE_BINDING_MAX> reads;

        RenderTransferBox_ActionCompute() : RenderTransferBox_Action(Action_Input_ComputeDefault)
        {
            csId = 0;
            dispatchX = dispatchY = dispatchZ = 0;
            writes.fill({});
            reads.fill({});
        }
    };
    typedef RenderTransferBox_ActionCompute RTB_ActionCompute;


    enum BuiltinActionType
    {
        BuiltinAction_Background = 0,
    };

    struct RenderTransferBox_ActionBuiltin : public RTB_Action
    {
        BuiltinActionType builtinType;

        RenderTransferBox_ActionBuiltin() : RenderTransferBox_Action(Action_Input_DrawDefault)
        {
            output.useRT[0] = true;
            output.useDS = true;
            output.outputLayout = OutputLayout_Color;
            builtinType = BuiltinAction_Background;
        }
    };
    typedef RenderTransferBox_ActionBuiltin RTB_ActionBuiltin;

    #pragma endregion

    #pragma endregion



    #pragma region Layer

    #define RENDER_LAYER_MAX 8
    #define RENDER_LIGHT_MAX 64

    #define RENDER_BUILTIN_PRECOMPUTE_MAX 4
    #define RENDER_BUILTIN_PP_MAX 16

    #define RENDER_BUILTIN_AO_SLOT 4            // AO 결과가 흐르는 RT 칸 (GBuffer 가 안 쓰는 칸, 포워드 / 디퍼드 공통)
    
    struct Layer_BackGround
    {
        TexID bgTexId;
        float color[4];
    };

    struct Layer_Viewport
    {
        float originX;
        float originY;
        float width;
        float height;
        float minZ = 0.0f;
        float maxZ = 1.0f;
    };


    // 매크로는 RenderOption.h 에 정의되어 있습니다
    #pragma region LayerPipelineVariant

    #if RENDER_PIPELINE_MODE == RENDER_USE_PIPELINE_BUILTIN

    // 미구현: 디퍼드 경로 기본 셰이더 없음, 포워드로 처리
    enum Layer_PipelineType
    {
        Layer_PipelineType_Forward,
        Layer_PipelineType_Deferred,    // 미구현
    };


    struct RenderTransferBox_LayerBuiltIn : public RTB
    {
        Layer_PipelineType pipelineType;

        Layer_BackGround backGround;
        TexID envTexId;
        Layer_Viewport viewport;
        

        RenderCondition condition;
        std::array<DataBoxID, RENDER_LIGHT_MAX> lights;
        std::array<DataBoxID, RENDER_LIGHT_MAX> lightMatrixes;

        DataBoxTID layerDataBoxTid;
        DataBoxID layerDataBoxId;


        // 미구현 섀도우맵 고정칸: 레이어 조명 환경으로 읽힘 (0 = 없음)
        TexID globalShadowTexId;
        TexID localShadowTexId;


        std::array<RTB_ActionCompute, RENDER_BUILTIN_PRECOMPUTE_MAX> actionPreComputes;     // 미구현 스키닝 / 파티클 시뮬

        RTB_ActionDraw actionEarlyZ;                    // 포워드용
        RTB_ActionDraw actionGBuffer;                   // 디퍼드용

        RTB_ActionEdit actionAO;                        // 미구현 (기본 셰이더 전)
        RTB_ActionEdit actionAOBlur;                    // 미구현 (기본 셰이더 전)

        std::array<RTB_ActionDraw, RENDER_BUILTIN_CSM_MAX> actionGlobalSDWs;    // 미구현 캐스케이드별, 아틀라스 타일은 액션 뷰포트로
        std::array<RTB_ActionDraw, RENDER_LIGHT_MAX> actionLocalSDWs;   // 미구현

        RTB_ActionDraw actionOPQ;                       // 포워드용
        RTB_ActionEdit actionLighting;                  // 미구현 디퍼드용 
        RTB_ActionBuiltin actionBackground;             // OPQ 뒤 고정 (스카이박스 / 배경판)
        RTB_ActionDraw actionTP;

        std::array<RTB_ActionEdit, RENDER_BUILTIN_PP_MAX> actionPPs;

        RTB_ActionDraw actionSC;                        // 미구현

        RTB_ActionDraw actionDebug;                     // 미구현 PP 안 먹는 3D 오버레이 (기즈모 / 와이어 등)


        RenderTransferBox_LayerBuiltIn();
    };
    typedef RenderTransferBox_LayerBuiltIn RenderTransferBox_Layer;

    inline RenderTransferBox_LayerBuiltIn::RenderTransferBox_LayerBuiltIn() : 
            backGround({0, {1.0f, 1.0f, 1.0f, 1.0f}}), envTexId(0), viewport(), 
            condition(), lights(), lightMatrixes(),
            layerDataBoxTid(0), layerDataBoxId(0),
            actionLocalSDWs({}), actionPPs({})
    {
        pipelineType = Layer_PipelineType_Forward;

        layerDataBoxTid = 0;
        layerDataBoxId = 0;

        globalShadowTexId = 0;
        localShadowTexId = 0;
        

        actionEarlyZ.renderIdType = RenderIdType_Opaque;
        actionEarlyZ.output.useRT[0] = false;
        actionEarlyZ.output.outputLayout = OutputLayout_DepthOnly;
        actionEarlyZ.dataBoxTypeIds[0] = DataBoxType_DataBox_Render3DGeneral;
        actionEarlyZ.dataBoxTypeIds[1] = DataBoxType_DataBox_Render3DOptional;

        actionGBuffer.renderIdType = RenderIdType_Opaque;
        actionGBuffer.output.useRT[1] = true;
        actionGBuffer.output.useRT[2] = true;
        actionGBuffer.output.useRT[3] = true;
        actionGBuffer.output.outputLayout = OutputLayout_GBuffer;
        actionGBuffer.dataBoxTypeIds[0] = DataBoxType_DataBox_Render3DGeneral;
        actionGBuffer.dataBoxTypeIds[1] = DataBoxType_DataBox_Render3DOptional;

        // AO: G 노드의 DS (디퍼드면 노멀도) 를 읽어서 AO 칸에. drawProp 을 넣어야 노드가 생김
        // DS 는 읽으면서 다음 노드로 넘김
        actionAO.output.useRT[RENDER_BUILTIN_AO_SLOT] = true;
        actionAO.output.useDS = true;
        actionAO.output.depthOpt = Action_Output_DepthOption_ForceDepthRead;

        actionAOBlur.output.useRT[RENDER_BUILTIN_AO_SLOT] = true;
        actionAOBlur.output.useDS = true;
        actionAOBlur.output.depthOpt = Action_Output_DepthOption_ForceDepthRead;

        // 섀도우는 기본(임시 리소스 경로) 출력 없음! 쓰려면 섀도우맵 (DS 텍스처) 을 직접 넣고, useDS 를 켜세요
        // 액션 데이터 = DataBox_RenderShadowView (그 패스의 라이트 뷰프로젝션)
        for (auto& action : actionGlobalSDWs)
        {
            action.output.useRT[0] = false;
            action.output.useDS = false;
            action.renderIdType = RenderIdType_Shadow;
            action.output.outputLayout = OutputLayout_DepthOnly;
            action.actionDataBoxTypeId = DataBoxType_DtB_ShadowView;
            action.dataBoxTypeIds[0] = DataBoxType_DataBox_Render3DGeneral;
            action.dataBoxTypeIds[1] = DataBoxType_DataBox_Render3DOptional;
        }

        for (auto& action : actionLocalSDWs)
        {
            action.output.useRT[0] = false;
            action.output.useDS = false;
            action.renderIdType = RenderIdType_Shadow;
            action.output.outputLayout = OutputLayout_DepthOnly;
            action.actionDataBoxTypeId = DataBoxType_DtB_ShadowView;
            action.dataBoxTypeIds[0] = DataBoxType_DataBox_Render3DGeneral;
            action.dataBoxTypeIds[1] = DataBoxType_DataBox_Render3DOptional;
        }
        
        // inputs[0] = EarlyZ (아무것도 안 받음, 지정만), inputs[1] = AO 체인 끝 (AO 칸 읽기 + DS 이어 그리기)
        actionOPQ.inputs[0].RTInputType[0] = Action_InputType_Detach;
        actionOPQ.inputs[0].DSInputType = Action_InputType_Detach;
        actionOPQ.inputs[1].RTInputType[0] = Action_InputType_Detach;
        actionOPQ.inputs[1].RTInputType[RENDER_BUILTIN_AO_SLOT] = Action_InputType_Convert;
        actionOPQ.renderIdType = RenderIdType_Opaque;
        actionOPQ.dataBoxTypeIds[0] = DataBoxType_DataBox_Render3DGeneral;
        actionOPQ.dataBoxTypeIds[1] = DataBoxType_DataBox_Render3DOptional;

        // 디퍼드 라이팅: inputs[0] = GBuffer (RT0~3 읽기), inputs[1] = AO 체인 끝 (AO 칸 읽기 + DS 읽으면서 넘김). 나머지는 Edit 기본값 (Convert)
        actionLighting.inputs[0].DSInputType = Action_InputType_Detach;
        actionLighting.output.useRT[0] = true;
        actionLighting.output.useDS = true;
        actionLighting.output.depthOpt = Action_Output_DepthOption_ForceDepthRead;

        // AO 는 OPQ | Lighting 까지만 읽음 (뒤로 안 물려줌)
        actionBackground.inputs[0].RTInputType[RENDER_BUILTIN_AO_SLOT] = Action_InputType_ClearSR;
        actionTP.inputs[0].RTInputType[RENDER_BUILTIN_AO_SLOT] = Action_InputType_ClearSR;

        // actionTP.inputs[0].RTInputType[0] = Action_InputType_NoConvert;
        actionTP.renderIdType = RenderIdType_Translucent;
        actionTP.dataBoxTypeIds[0] = DataBoxType_DataBox_Render3DGeneral;
        actionTP.dataBoxTypeIds[1] = DataBoxType_DataBox_Render3DOptional;

        // PP: drawProp 을 넣어야 노드가 생김, 기본 출력은 RT0
        for (auto& action : actionPPs)
            action.output.useRT[0] = true;

        // actionSC.inputs[0].RTInputType[0] = Action_InputType_NoConvert;
        actionSC.renderIdType = RenderIdType_Screen;
        actionSC.dataBoxTypeIds[0] = DataBoxType_DataBox_Render2DScreen;
    }
    

    #elif RENDER_PIPELINE_MODE == RENDER_USE_PIPELINE_CUSTOM
    
    struct RenderTransferBox_LayerCustom : public RTB
    {
        Layer_BackGround backGround;
        TexID envTexId;
        Layer_Viewport viewport;
        
        RenderCondition condition;
        std::array<DataBoxID, RENDER_LIGHT_MAX> lights;
        std::array<DataBoxId, RENDER_LIGHT_MAX> lightMatrixes;

        DataBoxTID layerDataBoxTid;
        DataBoxID layerDataBoxId;


        std::vector<std::array<ActionID, RENDER_ACTION_NODE_MAX>> actionNodeList;       // 아래 리스트와 일대일대응, 본인의 선행 노드를 가리킴
        std::vector<std::variant<RTB_ActionDraw, RTB_ActionEdit, RTB_ActionCompute, RTB_ActionBuiltin>> actionList;    // 0번 빈칸 (Base)

        RenderTransferBox_LayerCustom() : 
            backGround({0, {1.0f, 1.0f, 1.0f, 1.0f}}), envTexId(0), viewport(), 
            condition(), lights(), lightMatrixes(),
            layerDataBoxTid(0), layerDataBoxId(0), actionNodeList(1), actionList(1) {}
    };
    typedef RenderTransferBox_LayerCustom RenderTransferBox_Layer;

    #endif

    #pragma endregion


    typedef RenderTransferBox_Layer RTB_Layer;

    extern std::vector<RTB_Layer> LayerList;

    #pragma endregion

    #pragma endregion

}
