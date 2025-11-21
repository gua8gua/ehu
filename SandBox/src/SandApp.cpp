#include <Ehu.h>

class ExampleLayer : public Ehu::Layer {
public:
	ExampleLayer():
	Layer("Example")
	{}
	void OnUpdate() override {
		EHU_INFO("Example:OnUpdate");
	}
	void OnEvent(Ehu::Event& event) override {
		EHU_INFO("Example:OnEvent");
	}
};

class SandApp : public Ehu::Application {
	public:
		SandApp() {
			PushLayer(new ExampleLayer());
		}	
		~SandApp() {
		}
};

Ehu::Application* Ehu::CreateApplication() {
	return new SandApp();
}