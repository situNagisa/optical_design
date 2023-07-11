#pragma once

#include "optical_design/device/camera.h"
#include "optical_design/device/engine.h"
#include "optical_design/device/gyroscope.h"
#include "optical_design/device/screen.h"

OPT_BEGIN

struct Device : public ngs::Singleton<Device> {

	bool Open() {
		if (!camera.IsOpened()) {
			NGS_LOGL(info, "open camera...");
			if (!camera.Open())return false;
			NGS_LOGL(info, "open camera success!");
		}
		if (!screen.IsOpened()) {
			NGS_LOGL(info, "open screen...");
			if (!screen.Open())return false;
			NGS_LOGL(info, "open screen success!");
		}
		if (!engine.IsOpened()) {
			NGS_LOGL(info, "open engine...");
			if (!engine.Open())return false;
			NGS_LOGL(info, "open engine success!");
		}
		if (!gyroscope.IsOpened()) {
			NGS_LOGL(info, "open gyroscope...");
			if (!gyroscope.Open())return false;
			NGS_LOGL(info, "open gyroscope success!");
		}

		NGS_LOGL(info, "devices all opened successfully!");
		_is_opened = true;
		return true;
	}
	bool IsOpened()const { return _is_opened; }
	void Close() {
		camera.Close();
		screen.Close();
		engine.Close();
		gyroscope.Close();

		_is_opened = false;
	}
	void Update() {
		camera.Update();
		screen.Update();
		engine.Update();
		gyroscope.Update();
	}
public:
	Camera camera;
	Screen screen;
	Engine engine;
	Gyroscope gyroscope;
private:
	bool _is_opened = false;
};

OPT_END
