#include "gui/imgui.h"
#include "optical_design/optical_design.h"
#include "global.h"
#include "b2d/b2d.h"

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
	float menuWidth = 180.0f * s_displayScale;
	if (g_debugDraw.m_showUI)
	{
		ImGui::SetNextWindowPos({ g_camera.m_width - menuWidth - 10.0f, 10.0f });
		ImGui::SetNextWindowSize({ menuWidth, g_camera.m_height - 20.0f });

		ImGui::Begin("Tools", &g_debugDraw.m_showUI, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		if (ImGui::BeginTabBar("ControlTabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Controls"))
			{
				ImGui::SliderInt("Vel Iters", &s_settings.m_velocityIterations, 0, 50);
				ImGui::SliderInt("Pos Iters", &s_settings.m_positionIterations, 0, 50);
				ImGui::SliderFloat("Hertz", &s_settings.m_hertz, 5.0f, 120.0f, "%.0f hz");

				ImGui::Separator();

				ImGui::Checkbox("Sleep", &s_settings.m_enableSleep);
				ImGui::Checkbox("Warm Starting", &s_settings.m_enableWarmStarting);
				ImGui::Checkbox("Time of Impact", &s_settings.m_enableContinuous);
				ImGui::Checkbox("Sub-Stepping", &s_settings.m_enableSubStepping);

				ImGui::Separator();

				ImGui::Checkbox("Shapes", &s_settings.m_drawShapes);
				ImGui::Checkbox("Joints", &s_settings.m_drawJoints);
				ImGui::Checkbox("AABBs", &s_settings.m_drawAABBs);
				ImGui::Checkbox("Contact Points", &s_settings.m_drawContactPoints);
				ImGui::Checkbox("Contact Normals", &s_settings.m_drawContactNormals);
				ImGui::Checkbox("Contact Impulses", &s_settings.m_drawContactImpulse);
				ImGui::Checkbox("Friction Impulses", &s_settings.m_drawFrictionImpulse);
				ImGui::Checkbox("Center of Masses", &s_settings.m_drawCOMs);
				ImGui::Checkbox("Statistics", &s_settings.m_drawStats);
				ImGui::Checkbox("Profile", &s_settings.m_drawProfile);

				ImVec2 button_sz = ImVec2(-1, 0);
				if (ImGui::Button("Pause (P)", button_sz))
				{
					s_settings.m_pause = !s_settings.m_pause;
				}

				if (ImGui::Button("Single Step (O)", button_sz))
				{
					s_settings.m_singleStep = !s_settings.m_singleStep;
				}

				if (ImGui::Button("Restart (R)", button_sz))
				{
					RestartTest();
				}

				if (ImGui::Button("Quit", button_sz))
				{
					glfwSetWindowShouldClose(g_mainWindow, GL_TRUE);
				}

				ImGui::EndTabItem();
			}

			ImGuiTreeNodeFlags leafNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			leafNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

			if (ImGui::BeginTabItem("Tests"))
			{
				int categoryIndex = 0;
				const char* category = g_testEntries[categoryIndex].category;
				int i = 0;
				while (i < g_testCount)
				{
					bool categorySelected = strcmp(category, g_testEntries[s_settings.m_testIndex].category) == 0;
					ImGuiTreeNodeFlags nodeSelectionFlags = categorySelected ? ImGuiTreeNodeFlags_Selected : 0;
					bool nodeOpen = ImGui::TreeNodeEx(category, nodeFlags | nodeSelectionFlags);

					if (nodeOpen)
					{
						while (i < g_testCount && strcmp(category, g_testEntries[i].category) == 0)
						{
							ImGuiTreeNodeFlags selectionFlags = 0;
							if (s_settings.m_testIndex == i)
							{
								selectionFlags = ImGuiTreeNodeFlags_Selected;
							}
							ImGui::TreeNodeEx((void*)(intptr_t)i, leafNodeFlags | selectionFlags, "%s", g_testEntries[i].name);
							if (ImGui::IsItemClicked())
							{
								s_testSelection = i;
							}
							++i;
						}
						ImGui::TreePop();
					}
					else
					{
						while (i < g_testCount && strcmp(category, g_testEntries[i].category) == 0)
						{
							++i;
						}
					}

					if (i < g_testCount)
					{
						category = g_testEntries[i].category;
						categoryIndex = i;
					}
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();

		s_test->UpdateUI();
	}
}


void ImageGUI::UpdateUI()
{
	AllocatorWindow();
	RunningDataWindow();
	UpdateUI();
}
