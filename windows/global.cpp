#include "global.h"
#include "b2d/b2d.h"

GLFWwindow* g_mainWindow = nullptr;
int32 s_testSelection = 0;
Test* s_test = nullptr;
Settings s_settings;
bool s_rightMouseDown = false;
b2Vec2 s_clickPointWS = b2Vec2_zero;
float s_displayScale = 1.0f;

bool CompareTests(const TestEntry& a, const TestEntry& b)
{
	int result = strcmp(a.category, b.category);
	if (result == 0)
	{
		result = strcmp(a.name, b.name);
	}

	return result < 0;
}

void SortTests()
{
	std::sort(g_testEntries, g_testEntries + g_testCount, CompareTests);
}

void RestartTest()
{
	delete s_test;
	s_test = g_testEntries[s_settings.m_testIndex].createFcn();
}
