#pragma once

#include "optical_design/defined.h"

OPT_BEGIN

class Camera {
public:
public:
	bool Open();
	bool IsOpened()const;
	void Close();

	void Update();

	cv::Mat Get();
private:
	ngs::void_ptr _data = nullptr;
};

OPT_END
