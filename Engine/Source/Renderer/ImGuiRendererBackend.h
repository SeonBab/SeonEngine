#pragma once

struct ImGuiContext;
struct ImDrawData;
struct ImTextureRef;
class UTexture;

/** ImGui의 GPU backend 계약이다. context는 호출자가 소유하며 backend 객체보다 오래 살아 있어야 한다. */
class IImGuiRendererBackend
{
public:
	virtual ~IImGuiRendererBackend() = default;

	IImGuiRendererBackend(const IImGuiRendererBackend&) = delete;
	IImGuiRendererBackend& operator=(const IImGuiRendererBackend&) = delete;
	IImGuiRendererBackend(IImGuiRendererBackend&&) = delete;
	IImGuiRendererBackend& operator=(IImGuiRendererBackend&&) = delete;

	/** 같은 스레드에서 빌린 context에 backend를 준비한다. context의 생성/파괴는 하지 않는다. 중복 초기화는 거부한다. */
	[[nodiscard]] virtual bool Initialize(ImGuiContext* context) = 0;

	/** 초기화된 context의 GPU backend 프레임 준비를 수행한다. ImGui::NewFrame은 호출하지 않는다. 기존 current context를 복원한다. */
	[[nodiscard]] virtual bool NewFrame() = 0;

	/**
	 * 같은 스레드에서 초기화된 backend와 호환되는 텍스처를 ImGui 이미지 참조로 변환한다.
	 * 실패하면 outTextureRef를 빈 참조로 만들고 false를 반환한다. current context는 변경하지 않는다.
	 * 참조는 자원을 소유하지 않는다. 호출자는 참조 획득부터 이를 사용한 RenderDrawData 반환까지
	 * 텍스처와 GPU 데이터를 유지하고 교체하지 않아야 한다. 프레임을 넘어 참조를 캐시하지 않는다.
	 * 이미지 제출 전 해당 텍스처의 출력 타깃 사용을 끝내야 한다. 변환은 바인딩이나 복사를 하지 않는다.
	 */
	[[nodiscard]] virtual bool GetTextureRef(const UTexture& texture, ImTextureRef& outTextureRef) = 0;

	/**
	 * 같은 context에서 ImGui::Render 직후 얻은 데이터를 호출 동안 빌려 현재 출력에 제출한다. nullptr는 거부한다.
	 * 기존 current context를 복원한다. true는 호출 흐름 성공이며 GPU 완료나 화면 표시를 보장하지 않는다.
	 * 지연 구현은 반환 전에 다음 NewFrame으로 무효화될 데이터와 필요한 자원 수명을 확보해야 한다.
	 */
	[[nodiscard]] virtual bool RenderDrawData(ImDrawData* drawData) = 0;

	/** context와 원본 Renderer가 살아 있을 때 정리한다. 빈 상태·부분 실패·반복 호출을 허용하고 기존 current context를 복원한다. */
	virtual void Shutdown() = 0;

protected:
	IImGuiRendererBackend() = default;
};
