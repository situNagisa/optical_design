#pragma once

#include "NGS/ngl/NGL.h"
#include <box2d/box2d.h>

class DebugDraw;
class Test;
class Camera;
class TestEntry;
class Settings;

extern DebugDraw g_debugDraw;
extern Camera g_camera;
extern GLFWwindow* g_mainWindow;

extern int32 s_testSelection;
extern Test* s_test;
extern Settings s_settings;
extern bool s_rightMouseDown;
extern b2Vec2 s_clickPointWS;
extern float s_displayScale;
#define MAX_TESTS 256
extern TestEntry g_testEntries[MAX_TESTS];
extern int g_testCount;

bool CompareTests(const TestEntry& a, const TestEntry& b);
void SortTests();
void RestartTest();
