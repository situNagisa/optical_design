#include "optical_design/device/screen.h"

OPT_BEGIN

bool Screen::Open() { return true; }
bool Screen::IsOpened()const { return true; }
void Screen::Close() {}

void Screen::Update() {}

void Screen::Show(const cv::Mat& image) {
	if (image.empty())return;
	cv::imshow("screen", image);
	cv::waitKey(30);
}

OPT_END
