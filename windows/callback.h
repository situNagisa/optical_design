#pragma once
#include "NGS/ngl/NGL.h"
#include "data_manager.h"

static void ResizeWindowCallback(GLFWwindow*, int width, int height)
{
	DataManager::I().camera.m_width = width;
	DataManager::I().camera.m_height = height;
	DataManager::I().settings.m_windowWidth = width;
	DataManager::I().settings.m_windowHeight = height;
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		return;
	}

	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			// Quit
			ngl::targets::GLFW::I().GetCurrentState()->ShouldClose();
			break;

		case GLFW_KEY_LEFT:
			// Pan left
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(2.0f, 0.0f);
				box2d::TestSelector::I().current_test->ShiftOrigin(newOrigin);
			}
			else
			{
				DataManager::I().camera.m_center.x -= 0.5f;
			}
			break;

		case GLFW_KEY_RIGHT:
			// Pan right
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(-2.0f, 0.0f);
				box2d::TestSelector::I().current_test->ShiftOrigin(newOrigin);
			}
			else
			{
				DataManager::I().camera.m_center.x += 0.5f;
			}
			break;

		case GLFW_KEY_DOWN:
			// Pan down
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(0.0f, 2.0f);
				box2d::TestSelector::I().current_test->ShiftOrigin(newOrigin);
			}
			else
			{
				DataManager::I().camera.m_center.y -= 0.5f;
			}
			break;

		case GLFW_KEY_UP:
			// Pan up
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(0.0f, -2.0f);
				box2d::TestSelector::I().current_test->ShiftOrigin(newOrigin);
			}
			else
			{
				DataManager::I().camera.m_center.y += 0.5f;
			}
			break;

		case GLFW_KEY_HOME:
			DataManager::I().camera.ResetView();
			break;

		case GLFW_KEY_Z:
			// Zoom out
			DataManager::I().camera.m_zoom = std::min(1.1f * DataManager::I().camera.m_zoom, 20.0f);
			break;

		case GLFW_KEY_X:
			// Zoom in
			DataManager::I().camera.m_zoom = std::max(0.9f * DataManager::I().camera.m_zoom, 0.02f);
			break;

		case GLFW_KEY_R:
			box2d::TestSelector::I().Restart();
			break;

		case GLFW_KEY_SPACE:
			// Launch a bomb.
			if (box2d::TestSelector::I().current_test)
			{
				box2d::TestSelector::I().current_test->LaunchBomb();
			}
			break;

		case GLFW_KEY_O:
			DataManager::I().settings.m_singleStep = true;
			break;

		case GLFW_KEY_P:
			DataManager::I().settings.m_pause = !DataManager::I().settings.m_pause;
			break;

		case GLFW_KEY_LEFT_BRACKET:
			// Switch to previous test
			box2d::TestSelector::I().Prev();
			break;

		case GLFW_KEY_RIGHT_BRACKET:
			// Switch to next test
			box2d::TestSelector::I().Next();
			break;

		case GLFW_KEY_TAB:
			DataManager::I().debug_draw.m_showUI = !DataManager::I().debug_draw.m_showUI;

		default:
			if (box2d::TestSelector::I().current_test)
			{
				box2d::TestSelector::I().current_test->Keyboard(key);
			}
		}
	}
	else if (action == GLFW_RELEASE)
	{
		box2d::TestSelector::I().current_test->KeyboardUp(key);
	}
	// else GLFW_REPEAT
}

static void CharCallback(GLFWwindow* window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
}

inline static b2Vec2 s_clickPointWS;
inline static bool s_rightMouseDown;

static void MouseButtonCallback(GLFWwindow* window, int32 button, int32 action, int32 mods)
{
	//ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

	double xd, yd;
	glfwGetCursorPos(ngl::targets::GLFW::I().GetCurrentState()->GetContext(), &xd, &yd);
	b2Vec2 ps((float)xd, (float)yd);

	// Use the mouse to move things around.
	if (button == GLFW_MOUSE_BUTTON_1)
	{
		//<##>
		//ps.Set(0, 0);
		b2Vec2 pw = DataManager::I().camera.ConvertScreenToWorld(ps);
		if (action == GLFW_PRESS)
		{
			if (mods == GLFW_MOD_SHIFT)
			{
				box2d::TestSelector::I().current_test->ShiftMouseDown(pw);
			}
			else
			{
				box2d::TestSelector::I().current_test->MouseDown(pw);
			}
		}

		if (action == GLFW_RELEASE)
		{
			box2d::TestSelector::I().current_test->MouseUp(pw);
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_2)
	{
		if (action == GLFW_PRESS)
		{
			s_clickPointWS = DataManager::I().camera.ConvertScreenToWorld(ps);
			s_rightMouseDown = true;
		}

		if (action == GLFW_RELEASE)
		{
			s_rightMouseDown = false;
		}
	}
}

static void MouseMotionCallback(GLFWwindow*, double xd, double yd)
{
	if (!box2d::TestSelector::I().current_test)return;
	b2Vec2 ps((float)xd, (float)yd);

	b2Vec2 pw = DataManager::I().camera.ConvertScreenToWorld(ps);
	box2d::TestSelector::I().current_test->MouseMove(pw);

	if (s_rightMouseDown)
	{
		b2Vec2 diff = pw - s_clickPointWS;
		DataManager::I().camera.m_center.x -= diff.x;
		DataManager::I().camera.m_center.y -= diff.y;
		s_clickPointWS = DataManager::I().camera.ConvertScreenToWorld(ps);
	}
}

static void ScrollCallback(GLFWwindow* window, double dx, double dy)
{
	//ImGui_ImplGlfw_ScrollCallback(window, dx, dy);
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	if (dy > 0)
	{
		DataManager::I().camera.m_zoom /= 1.1f;
	}
	else
	{
		DataManager::I().camera.m_zoom *= 1.1f;
	}
}
