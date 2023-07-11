#pragma once

#include "optical_design/defined.h"

#include "NGS/base/boost/property_tree.h"
#include <boost/exception/all.hpp>

class b2World;

OPT_BEGIN

struct Setting {
	struct {
		ngs::Point2f size;
		float mass;
	}car;
	Team team;
	std::filesystem::path treasures_map;
#if NGS_PLATFORM == NGS_WINDOWS
	ngs::pin_t gyroscope_serial_port;
#elif NGS_PLATFORM == NGS_LINUX

#endif
#if NGS_BUILD_TYPE == NGS_DEBUG
	struct {
		bool debug_vision;

		bool show_grid;
		bool show_inflection;
		bool show_node;

		bool show_gyroscope_data;
		bool show_car_state;
	}debug;
#endif
};

class Maze;

struct RunningData {
	struct {
		position_t position;			//physic
		float angle;					//physic
		ngs::Point2f acceleration;		//gyroscope
		float linear_velocity;			//engine
		float angular_velocity;			//gyroscope
	}car;

	position_t next_position;			//move

	bool requires_recognize_treasure = false;
	std::optional<vision::Treasure> current_treasure;

	std::shared_ptr<Maze> maze = nullptr;

	ngs::StdTimer timer;

	bool is_finished = false;

	b2World* world = nullptr;

	float GetDeltaAngle()const {
		float aim = (next_position - car.position).ArcTan();
		float cur = car.angle;
		float delta = aim - cur;
		delta = fmod(delta, 2 * std::numbers::pi);
		if (delta > std::numbers::pi)
			delta -= 2 * std::numbers::pi;
		else if (delta < -std::numbers::pi)
			delta += 2 * std::numbers::pi;

		return delta;
	}
};

class DataManager : public ngs::Singleton<DataManager> {
public:

	Setting GetDefaultSetting() const {
		return {
			.car = {
				.size = {0.25f,0.25f},
				.mass = 2.0f,
			},
			.team = Team::_1,
			.treasures_map = "treasures_map",
#if NGS_PLATFORM == NGS_WINDOWS
			.gyroscope_serial_port = 3,
#elif NGS_PLATFORM == NGS_LINUX

#endif
#if NGS_BUILD_TYPE == NGS_DEBUG
			.debug = {
				.debug_vision = false,

				.show_grid = true,
				.show_inflection = false,
				.show_node = true,

				.show_gyroscope_data = false,
				.show_car_state = true,
			},
#endif
		};
	}

	bool LoadSetting(const std::filesystem::path& filename) {
		using boost::property_tree::ptree;
		ptree pt;

		setting = {};

		try {
			boost::property_tree::read_json(filename.string(), pt);
		}
		catch (boost::exception& e) {
			NGS_LOGL(warning, "configuration file not found on filename: ", ngs::TextColor::yellow, filename.string());
			return false;
		}

		setting.team = (Team)pt.get("setting.team", 0);

		setting.car.mass = pt.get("setting.car.mass", 0.0f);
		setting.car.size.x = pt.get("setting.car.width", 0.0f);
		setting.car.size.y = pt.get("setting.car.height", 0.0f);

		setting.treasures_map = pt.get("setting.treasures_map", "");

#if NGS_PLATFORM == NGS_WINDOWS
		setting.gyroscope_serial_port = pt.get("setting.gyroscope_serial_port", 0);
#elif NGS_PLATFORM == NGS_LINUX


#endif
#if NGS_BUILD_TYPE == NGS_DEBUG
		setting.debug.debug_vision = pt.get("setting.debug.debug_vision", false);

		setting.debug.show_grid = pt.get("setting.debug.show_grid", false);
		setting.debug.show_inflection = pt.get("setting.debug.show_inflection", false);
		setting.debug.show_node = pt.get("setting.debug.show_node", false);

		setting.debug.show_gyroscope_data = pt.get("setting.debug.show_gyroscope_data", false);
		setting.debug.show_car_state = pt.get("setting.debug.show_car_state", false);
#endif
		return true;
	}
	bool SaveSetting(const std::filesystem::path& filename) {
		using boost::property_tree::ptree;
		ptree pt;

		pt.put("setting.team", (int)setting.team);
		pt.put("setting.car.mass", setting.car.mass);
		pt.put("setting.car.width", setting.car.size.x);
		pt.put("setting.car.height", setting.car.size.y);
		pt.put("setting.treasures_map", setting.treasures_map.string());

#if NGS_PLATFORM == NGS_WINDOWS
		pt.put("setting.gyroscope_serial_port", setting.gyroscope_serial_port);
#elif NGS_PLATFORM == NGS_LINUX


#endif
#if NGS_BUILD_TYPE == NGS_DEBUG
		pt.put("setting.debug.debug_vision", setting.debug.debug_vision);

		pt.put("setting.debug.show_grid", setting.debug.show_grid);
		pt.put("setting.debug.show_inflection", setting.debug.show_inflection);
		pt.put("setting.debug.show_node", setting.debug.show_node);

		pt.put("setting.debug.show_gyroscope_data", setting.debug.show_gyroscope_data);
		pt.put("setting.debug.show_car_state", setting.debug.show_car_state);
#endif

		try {
			boost::property_tree::write_json(filename.string(), pt);
		}
		catch (boost::exception& e) {
			NGS_LOGL(error, "\n", boost::diagnostic_information(e));
			return false;
		}
		return true;
	}

	Setting setting = GetDefaultSetting();
	RunningData running_data = {};
};

OPT_END
