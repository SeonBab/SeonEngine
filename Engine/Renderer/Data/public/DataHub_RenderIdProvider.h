#pragma once

#include "DataHub.h"


namespace Hakko::DataHub
{
    enum RenderId_ParseFlag
    {
        RenderId_ParseFlag_OK = 0,
        RenderId_ParseFlag_NotUse = 1,
        RenderId_ParseFlag_OnHanded = 2,    // (미구현) DtB칸 활용, GetMask시 리턴으로 슬롯 인덱스
    };
    

    class RenderIdProvider
    {
    protected:
        RenderIdProvider(uint64_t dpm, uint64_t tpm, uint64_t tm) : 
            DRAWPROP_MASK(dpm), TEXPROP_MASK(tpm), TEX_MASK(tm) {}
    public:
        virtual ~RenderIdProvider() = default;

    protected:
        const uint64_t DRAWPROP_MASK;
        const uint64_t TEXPROP_MASK;
        const uint64_t TEX_MASK;

    public:
        virtual operator RenderID() const = 0;

        virtual bool IsAscendingSort() const { return true; }
        virtual uint64_t GetDrawPropIdMask(RenderId_ParseFlag& flag) const { flag = RenderId_ParseFlag_OK; return DRAWPROP_MASK; }
        virtual uint64_t GetTexPropIdMask(RenderId_ParseFlag& flag) const { flag = RenderId_ParseFlag_OK; return TEXPROP_MASK; }
        virtual uint64_t GetTexIdMask(RenderId_ParseFlag& flag) const { flag = RenderId_ParseFlag_OK; return TEX_MASK; }
    };


    class RenderIdNone : public RenderIdProvider
    {
    public:
        RenderIdNone() : RenderIdProvider(0xFFFF000000000000, 0x0000FFFF00000000, 0) {}

        // psoBody(16) 텍스처조합(16) iid(32!) 지만 마스크만 제공합니다 에딧용
    public:
        
        operator RenderID() const override
        {
            return 0;
        }

        uint64_t GetTexIdMask(RenderId_ParseFlag& flag) const
        {
            flag = RenderId_ParseFlag_NotUse;
            return 0;
        }
    };

    class RenderIdOpaque : public RenderIdProvider
    {
    public:
        RenderIdOpaque() : RenderIdProvider(0x00FFFF0000000000, 0x000000FFFF000000, 0), 
            alphaClip(0), layer(0), drawProp(0), texProp(0), meshId(0), depth(0) {}

        // 알파(1) 레이어(7) psoBody(16) 텍스처조합(16) 메쉬(16) 뎁스(8)
    public:
        bool alphaClip;
        uint8_t layer;
        DrawPropID drawProp;
        TexPropID texProp;
        MeshID meshId;
        uint8_t depth;
        

        operator RenderID() const override
        {
            RenderID id = 
				(RenderID)alphaClip << 63 |
				(RenderID)(layer & 0b01111111) << 56 |
				(RenderID)drawProp << 40 |
				(RenderID)texProp << 24 |
				(RenderID)meshId << 8 |
				(RenderID)depth;

            return id;
        }

        uint64_t GetTexIdMask(RenderId_ParseFlag& flag) const
        {
            flag = RenderId_ParseFlag_NotUse;
            return 0;
        }
    };

    class RenderIdTranslucent : public RenderIdProvider
    {
    public:
        RenderIdTranslucent() : RenderIdProvider(0x0000FFFF00000000, 0x00000000FFFF0000, 0), 
            depth(0), drawProp(0), texProp(0), iid(0) {}

        // 뎁스(16) psoBody(16) 텍스처조합(16) 잉여(16)
    public:
        uint16_t depth;
        DrawPropID drawProp;
        TexPropID texProp;
        uint16_t iid;


        operator RenderID() const override
        {
			RenderID id = 
				(RenderID)depth << 48 |
				(RenderID)drawProp << 32 |
				(RenderID)texProp << 16 |
				(RenderID)iid;

            return id;
        }

        bool IsAscendingSort() const { return false; }

        uint64_t GetTexIdMask(RenderId_ParseFlag& flag) const
        {
            flag = RenderId_ParseFlag_NotUse;
            return 0;
        }
        

    };

    class RenderIdShadow : public RenderIdProvider
    {
    public:
        RenderIdShadow() : RenderIdProvider((0xFFFF000000000000 >> 1), (0x0000FFFF00000000 >> 1), 0), 
            alphaClip(0), drawProp(0), texProp(0), meshId(0), depth(0) {}

        // 알파(1) psoBody(16) 텍스처조합(16) 메쉬(16) 뎁스(8) 잉여(7)
    public:
        bool alphaClip;
        DrawPropID drawProp;
        TexPropID texProp;
        MeshID meshId;
        uint8_t depth;
        uint8_t iid;
        

        operator RenderID() const override
        {
            RenderID id = 
				(RenderID)alphaClip << 63 |
				(RenderID)drawProp << 47 |
				(RenderID)texProp << 31 |
				(RenderID)meshId << 15 |
				(RenderID)depth << 7 |
				(RenderID)(iid & 0b01111111);

            return id;
        }

        uint64_t GetTexIdMask(RenderId_ParseFlag& flag) const
        {
            flag = RenderId_ParseFlag_NotUse;
            return 0;
        }
    };

    class RenderIdScreen : public RenderIdProvider
    {
    public:
        RenderIdScreen() : RenderIdProvider((0x0000FFFF00000000 >> 1), 0, (0x00000000FFFF0000 >> 1)), 
            layer(0), translucent(0), drawProp(0), texture(0), iid(0) {}

        // 레이어(16) 반투명(1) psoBody(16) 텍스처(16) 잉여(15)
    public:
        uint16_t layer;
        bool translucent;
        DrawPropID drawProp;
        TexID texture;
        uint16_t iid;


        operator RenderID() const override
        {
			RenderID id = 
				(RenderID)layer << 48 |
				(RenderID)translucent << 47 |
				(RenderID)drawProp << 31 |
				(RenderID)texture << 15 |
				(RenderID)(iid & 0b0111111111111111);

            return id;
        }

        uint64_t GetTexPropIdMask(RenderId_ParseFlag& flag) const
        {
            flag = RenderId_ParseFlag_NotUse;
            return 0;
        }
    };

}
