#pragma once

#include "optical_design/defined.h"

OPT_BEGIN

class Gyroscope {
public:
	bool Open();
	void Close();

	void Update();
	bool IsOpened()const;

	float GetAccelerationX()const;
	float GetAccelerationY()const;
	float GetAccelerationZ()const;

	float GetAngularVelocityX()const;
	float GetAngularVelocityY()const;
	float GetAngularVelocityZ()const;
private:
	ngs::void_ptr _data = nullptr;
};

OPT_END
