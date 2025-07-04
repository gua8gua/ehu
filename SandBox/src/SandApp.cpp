#include <Ehu.h>

class SandApp : public Ehu::Application {
	public:
		SandApp() {
		}	
		~SandApp() {
		}
};

Ehu::Application* Ehu::CreateApplication() {
	return new SandApp();
}