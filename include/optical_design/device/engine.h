#pragma once

#include "optical_design/defined.h"

OPT_BEGIN

class Engine {
public:
	bool Open();
	void Close();

	void Update();
	bool IsOpened()const;

	void SetLinearVelocityPercent(float percent);
	void SetAngularVelocityPercent(float percent);

	ngs::float32 GetLinearVelocity()const;
private:
	ngs::void_ptr _data = nullptr;
};

OPT_END
