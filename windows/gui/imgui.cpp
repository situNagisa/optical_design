#include "optical_design/optical_design.h"
#include "NGS/imgui/imgui.h"
#include "data_manager.h"

static void AllocatorWindow() {
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

static void UpdateUI()
{
	auto& data = DataManager::I();
	float menuWidth = 180.0f * data.display_scale;
	if (data.debug_draw.m_showUI)
	{
		ImGui::SetNextWindowPos({ data.camera.m_width - menuWidth - 10.0f, 10.0f });
		ImGui::SetNextWindowSize({ menuWidth, data.camera.m_height - 20.0f });

		ImGui::Begin("Tools", &data.debug_draw.m_showUI, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		if (ImGui::BeginTabBar("ControlTabs", ImGuiTabBarFlags_None))
		{
			data.settings.UpdateUI();
			box2d::TestSelector::I().UpdateUI(data.settings);

			ImGui::EndTabBar();
		}

		ImGui::End();

		box2d::TestSelector::I().current_test->UpdateUI();
	}
}


void ngs::ImageGUI::UpdateUI()
{
	//AllocatorWindow();
	RunningDataWindow();
	::UpdateUI();
}
