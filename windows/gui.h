#pragma once


#include "NGL/NGL.h"
#include "imgui/imgui.h"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

class ImageGUI {
public:

public:
	ImageGUI(std::string_view glsl_version, ngl::Window& window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		bool success;
		success = ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true);
		NGS_ASSERT(success);
		success = ImGui_ImplOpenGL3_Init(glsl_version.data());
		NGS_ASSERT(success);
	}
	~ImageGUI() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	void Render() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void NewFrame() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
private:

private:
};

void AllocatorWindow() {
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

	ImGui::Begin("Allocator");

	ImGui::Text("allocated memory record (%d bytes)", ngs::Allocator::I().GetUsedBytes());

	if (ImGui::CollapsingHeader("allocated list"))
	{
		for (const auto& [block, info] : ngs::Allocator::I())
		{
			auto [call, return_type, function_id, params] = ngs::ParseIdFactor::ParseFunction(info.function_name);
			if (ImGui::TreeNode(ngs::Format("%s %d bytes", (function_id + "::" + info.id).c_str(), info.size * info.count).c_str())) {
				ImGui::Text("address %p", block);
				ImGui::Text("type %s", info.type_name.c_str());
				ImGui::Text("size %d * %d", info.size, info.count);
				ImGui::Text("call on %s", info.function_name.c_str());
				ImGui::Text("line & column %d : %d", info.line, info.column);

				ImGui::TreePop();
				ImGui::Spacing();
			}
		}
	}

	ImGui::End();
}
