#pragma once

#include "optical_design/defined.h"

OPT_BEGIN

class Screen {
public:
	bool Open();
	void Close();

	void Update();
	bool IsOpened()const;

	void Show(const cv::Mat&);
private:
	ngs::void_ptr _data = nullptr;
};

OPT_END
