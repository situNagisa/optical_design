#include "optical_design/optical_design.h"
#include "gui.h"

void RunningDataWindow() {
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);

	ImGui::Begin("running data");

	auto& running_data = opt::DataManager::I().running_data;

	ImGui::Text("linear velocity %f", running_data.car.linear_velocity);
	ImGui::Text("angular velocity %f", running_data.car.angular_velocity);
	ImGui::Text("acceleration (%f, %f)", running_data.car.acceleration.x, running_data.car.acceleration.y);
	ImGui::Text("angle %d : %f", ngs::AsDegree(running_data.car.angle), running_data.car.angle);
	ImGui::Text("position (%d, %d)", running_data.car.position.x, running_data.next_position.y);
	ImGui::Text("next position (%d, %d)", running_data.next_position.x, running_data.next_position.y);

	ImGui::Separator();

	ImGui::SliderFloat("linear", &running_data.car.linear_velocity, -1.0f, 21.0f, "%.0f m");
	ImGui::SliderFloat("angular", &running_data.car.angular_velocity, -1.0f, 21.0f, "%.0f m");

	ImGui::End();
}


static void glfw_error_callback(int error, const char* description)
{
	NGS_LOGFL(error, "glfw error: %d, %s", error, description);
}
namespace gl = ngl::gl;

void ShowUI() {
	glfwSetErrorCallback(glfw_error_callback);
	ngl::GLFW& glfw = ngl::glfw;
	ngl::Window window("666", 1200, 900);

	ImageGUI imgui("#version 330", window);

	while (!window.ShouldClose())
	{
		ngl::Renderer::I().Viewport(window);
		ngl::Renderer::I().Clear(GL_COLOR_BUFFER_BIT);
		ngl::Renderer::I().FullColor(ngs::StdRGBA{0x80, 0x80, 0x80, 0xFF});

		imgui.NewFrame();
		AllocatorWindow();
		RunningDataWindow();
		imgui.Render();

		window.SwapBuffer();
		glfw.PollEvents();
	}


}

int main() {
	std::jthread thread{ShowUI};

	optical_design();
	thread.join();

	return 0;
}
