#include "optical_design/device/camera.h"

OPT_BEGIN

struct _CameraData {
	cv::VideoCapture capture;
};

bool Camera::Open()
{
	NGS_NEW(_data, _CameraData)();
	auto& data = *(_CameraData*)_data;

	if (!data.capture.open(0))goto err_open;
	return true;
err_open:;
	NGS_DELETE(&data);
	_data = nullptr;
	return false;
}
bool Camera::IsOpened()const { return _data && reinterpret_cast<_CameraData*>(_data)->capture.isOpened(); }
void Camera::Close() {
	if (!_data)return;
	auto& data = *(_CameraData*)_data;
	data.capture.release();
	NGS_DELETE(&data);
	_data = nullptr;
}

void Camera::Update() {}

cv::Mat Camera::Get() {
	auto& data = *(_CameraData*)_data;
	cv::Mat buffer;

	data.capture >> buffer;
	return buffer;
}


OPT_END
