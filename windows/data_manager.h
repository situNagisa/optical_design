#pragma once

#include <box2d/test/test.h>

struct DataManager : public ngs::Singleton<DataManager> {
private:
	friend class base;
	DataManager() = default;
public:

	box2d::Settings settings{};
	box2d::Camera camera{};
	box2d::DebugDraw debug_draw{&camera};

	float display_scale = 1.0f;
};
