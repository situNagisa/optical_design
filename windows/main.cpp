#include "optical_design/optical_design.h"
#include "gui/gui.h"
#include "global.h"
#include "b2d/b2d.h"


static void glfw_error_callback(int error, const char* description)
{
	NGS_LOGFL(error, "glfw error: %d, %s", error, description);
}
namespace gl = ngl::gl;

void mainUI() {
#if defined(_WIN32)
	// Enable memory-leak reports
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif

	s_settings.Load();
	SortTests();

	g_camera.m_width = s_settings.m_windowWidth;
	g_camera.m_height = s_settings.m_windowHeight;

	glfwSetErrorCallback(glfw_error_callback);

	ngl::GLFW& glfw = ngl::GLFW::I();
	ngl::Window window(ngs::Format("Box2D Testbed Version %d.%d.%d", b2_version.major, b2_version.minor, b2_version.revision), 1200, 900);

	g_debugDraw.Create();

	ImageGUI imgui("#version 330", window);

	s_settings.m_testIndex = b2Clamp(s_settings.m_testIndex, 0, g_testCount - 1);
	s_testSelection = s_settings.m_testIndex;
	s_test = g_testEntries[s_settings.m_testIndex].createFcn();

	std::chrono::duration<double> frameTime(0.0);
	std::chrono::duration<double> sleepAdjust(0.0);

	while (!window.ShouldClose())
	{
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

		ngl::Renderer::I().Viewport(window);
		ngl::Renderer::I().Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ngl::Renderer::I().FullColor(ngs::StdRGBA{0x80, 0x80, 0x80, 0xFF});

		imgui.NewFrame();
		if (g_debugDraw.m_showUI)
		{
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(ImVec2(float(g_camera.m_width), float(g_camera.m_height)));
			ImGui::SetNextWindowBgAlpha(0.0f);
			ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
			ImGui::End();

			const TestEntry& entry = g_testEntries[s_settings.m_testIndex];
			s_test->DrawTitle(ngs::Format("%s : %s", entry.category, entry.name).c_str());
		}

		s_test->Step(s_settings);
		imgui.UpdateUI();
		if (g_debugDraw.m_showUI)
			g_debugDraw.DrawString(5, g_camera.m_height - 20, ngs::Format("%.1f ms", 1000.0 * frameTime.count()).c_str());

		imgui.Render();

		window.SwapBuffer();

		if (s_testSelection != s_settings.m_testIndex)
		{
			s_settings.m_testIndex = s_testSelection;
			delete s_test;
			s_test = g_testEntries[s_settings.m_testIndex].createFcn();
			g_camera.ResetView();
		}

		glfw.PollEvents();

		std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
		std::chrono::duration<double> target(1.0 / 60.0);
		std::chrono::duration<double> timeUsed = t2 - t1;
		std::chrono::duration<double> sleepTime = target - timeUsed + sleepAdjust;
		if (sleepTime > std::chrono::duration<double>(0))
		{
			std::this_thread::sleep_for(sleepTime);
		}

		std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
		frameTime = t3 - t1;

		// Compute the sleep adjustment using a low pass filter
		sleepAdjust = 0.9 * sleepAdjust + 0.1 * (target - frameTime);
	}
	delete s_test;
	s_test = nullptr;

	g_debugDraw.Destroy();
	s_settings.Save();
}

int main() {
	std::jthread thread{mainUI};

	optical_design();
	thread.join();

	return 0;
}
