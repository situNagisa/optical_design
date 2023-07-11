#include <wit/jy901b/jy901b.h>

#include "optical_design/main_data.h"
#include "optical_design/device/gyroscope.h"

OPT_BEGIN

using JY901B = wit::JY901B<ngs::SerialPort>;

struct _GyroscopeData {
	JY901B gyroscope;
	ngs::Point3f acceleration;
	ngs::Point3f angularVelocity;
};

bool Gyroscope::Open() {
	NGS_NEW(_data, _GyroscopeData);
	_GyroscopeData& data = *(_GyroscopeData*)_data;
	auto& gyroscope = data.gyroscope;

	if (!gyroscope.Open(DataManager::I().setting.gyroscope_serial_port))goto err_open;
	gyroscope.ActiveRegister(wit::Register::acceleratin_x);
	gyroscope.ReadTable(JY901B::ReadParsableStruct::acceleration, JY901B::ReadParsableStruct::gyroscope);

	gyroscope.Write(wit::Register::Table::save, wit::Register::Config::Save::reset);
	gyroscope.Write(wit::Register::Table::axis6, wit::Register::Config::AXIS6::_6);
	gyroscope.Write(wit::Register::Table::r_rate, wit::Register::Config::RRate::_200);
	gyroscope.Write(wit::Register::Table::band_width, wit::Register::Config::BandWidth::_256);
	gyroscope.Write(wit::Register::Table::filter_k, wit::Register::Config::FilterK::_2000);
	gyroscope.Write(wit::Register::Table::acceleration_filter, wit::Register::Config::AccelerationFilter::_2000);

	return true;
err_open:;
	NGS_DELETE(&data);
	_data = nullptr;
	return false;
}
void Gyroscope::Close() {
	auto& data = *(_GyroscopeData*)_data;
	data.gyroscope.Close();

	NGS_DELETE(&data);
	_data = nullptr;
}
bool Gyroscope::IsOpened() const { return _data && reinterpret_cast<_GyroscopeData*>(_data)->gyroscope.IsOpened(); }
void Gyroscope::Update() {
	auto& data = *(_GyroscopeData*)_data;
	data.gyroscope.Update();

	data.acceleration = data.gyroscope.GetAcceleration();
	data.angularVelocity = data.gyroscope.GetAngularVelocity();

#if NGS_BUILD_TYPE == NGS_DEBUG
	if (DataManager::I().setting.debug.show_gyroscope_data) {
		NGS_LOGF(debug, "angular velocity: %4.4f %4.4f %4.4f\r\n", data.angularVelocity.x, data.angularVelocity.y, data.angularVelocity.z);
		NGS_LOGFL(debug, "acceleration: %4.4f %4.4f %4.4f\r\n", data.acceleration.x, data.acceleration.y, data.acceleration.z);
	}
#endif
}
float Gyroscope::GetAccelerationX() const {
	auto& data = *(_GyroscopeData*)_data;
	return data.acceleration.x;
}
float Gyroscope::GetAccelerationY() const {
	auto& data = *(_GyroscopeData*)_data;
	return data.acceleration.y;
}
float Gyroscope::GetAccelerationZ() const {
	auto& data = *(_GyroscopeData*)_data;
	return data.acceleration.z;
}
float Gyroscope::GetAngularVelocityX() const {
	auto& data = *(_GyroscopeData*)_data;
	return data.angularVelocity.x;
}
float Gyroscope::GetAngularVelocityY() const {
	auto& data = *(_GyroscopeData*)_data;
	return data.angularVelocity.y;
}
float Gyroscope::GetAngularVelocityZ() const {
	auto& data = *(_GyroscopeData*)_data;
	return data.angularVelocity.z;
}

OPT_END
