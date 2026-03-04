#include "Ehu.h"
#include "Renderer/RendererModule.h"
#include "Platform/RenderContext.h"

// 3D 示例层：透视相机 + 多个四边形表现深度与透视
class Example3DLayer : public Ehu::Layer {
public:
	Example3DLayer() : Layer("Example3D"), m_Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f) {
		m_Camera.SetPosition({ 0.0f, 0.0f, 6.0f });
		m_Camera.SetRotation({ 0.0f, 0.0f, 0.0f });
	}

	void OnUpdate() override {
		Ehu::RenderContext::GetAPI().SetDepthTest(true);
		Ehu::Renderer2D::BeginScene(m_Camera);
		// 前排（z=0）：红、绿
		Ehu::Renderer2D::DrawQuad({ -1.2f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.9f, 0.2f, 0.2f, 1.0f });
		Ehu::Renderer2D::DrawQuad({  1.2f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.2f, 0.9f, 0.2f, 1.0f });
		// 后排（z=-2）：蓝、黄，尺寸略大以体现透视
		Ehu::Renderer2D::DrawQuad({ -0.8f, 0.0f, -2.0f }, { 1.4f, 1.4f }, { 0.2f, 0.3f, 0.9f, 1.0f });
		Ehu::Renderer2D::DrawQuad({  0.8f, 0.0f, -2.0f }, { 1.4f, 1.4f }, { 0.95f, 0.9f, 0.2f, 1.0f });
		// 最远（z=-3.5）：紫
		Ehu::Renderer2D::DrawQuad({ 0.0f, 0.0f, -3.5f }, { 1.8f, 1.8f }, { 0.6f, 0.2f, 0.8f, 1.0f });
		Ehu::Renderer2D::EndScene();
	}
	void OnEvent(Ehu::Event& event) override { (void)event; }

private:
	Ehu::PerspectiveCamera m_Camera;
};

class SandApp : public Ehu::Application {
public:
	SandApp() {
		PushLayer(new Example3DLayer());
	}
	~SandApp() {}
};

Ehu::Application* Ehu::CreateApplication() {
	return new SandApp();
}
