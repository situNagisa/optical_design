#pragma once
#include "optical_design/main_data.h"
#include "optical_design/data/maze/maze.h"
#include "optical_design/device/device.h"
#include "optical_design/ctrl/ctrl.h"

inline void optical_design() {
	auto& data_main = opt::DataManager::I();
	auto& setting = data_main.setting;
	auto& running_data = data_main.running_data;

	if (!data_main.LoadSetting("setting.json")) {
		setting = data_main.GetDefaultSetting();
		data_main.SaveSetting("setting.json");
	}

	auto& devices = opt::Device::I();
	while (!devices.Open());

	struct TeamColor {
		vision::ColorRange start_color;
		vision::ColorRange end_color;
		vision::ColorRange card_range;
		vision::ColorRange pattern_range;
	};
	TeamColor team_1 = {
		.start_color = vision::color_defined::red,
		.end_color = vision::color_defined::blue,
		.card_range = vision::color_defined::red,
		.pattern_range = vision::color_defined::green,
	};
	TeamColor team_2 = {
		.start_color = vision::color_defined::blue,
		.end_color = vision::color_defined::red,
		.card_range = vision::color_defined::blue,
		.pattern_range = vision::color_defined::yellow,
	};
	TeamColor* team_current = nullptr;
	TeamColor* team_target = nullptr;
	switch (setting.team)
	{
	case opt::Team::_1:
		team_current = &team_1;
		team_target = &team_2;
		break;
	case opt::Team::_2:
		team_current = &team_2;
		team_target = &team_1;
		break;
	}

	{
		vision::MazeGrid maze_grid[vision::GRID_NUM.x][vision::GRID_NUM.y] = {};
		std::optional<std::tuple<vision::position_t, vision::position_t, std::vector<vision::position_t>>> optional;

		try {
			NGS_LOG_SCOPE_DISABLE;
			while (true) {
				cv::Mat view;
				if (setting.treasures_map.empty())
					view = devices.camera.Get();
				else
					view = cv::imread(setting.treasures_map.string());

				cv::Mat maze_image = vision::CVUtil::RECOGONIZE_MAZE(view, 350, 350);
				if (maze_image.empty()) {
					devices.screen.Show(view);
					continue;
				}
				optional = vision::CVUtil::CONVERT_IMAGE_TO_MAZE(maze_image, team_current->start_color, team_current->end_color, maze_grid);
				if (optional)break;
			}
		}
		catch (cv::Exception& e) {
			NGS_ASSERT(false, e.what());
		}

		auto& [start_point, end_point, treasures] = optional.value();
		maze_grid[(size_t)start_point.x][(size_t)start_point.y] = vision::MazeGrid::start;
		maze_grid[(size_t)end_point.x][(size_t)end_point.y] = vision::MazeGrid::end;

		running_data.maze = std::make_shared<opt::Maze>(maze_grid);
		auto& maze = *running_data.maze;

		maze.SetStartPoint(start_point.x, start_point.y);
		maze.SetEndPoint(end_point.x, end_point.y);
		maze.SetTreasures(treasures);

		NGS_DEBUG_EXPR(if (setting.debug.show_grid)maze.PrintGrids());
	}
	opt::Ctrl ctrl;

	while (!running_data.is_finished) {
		running_data.timer.Update();
		{
			devices.Update();
			running_data.car.acceleration.Set(devices.gyroscope.GetAccelerationX(), devices.gyroscope.GetAccelerationY());
			//running_data.car.linear_velocity = devices.engine.GetLinearVelocity();
			//running_data.car.angular_velocity = devices.gyroscope.GetAngularVelocityZ();

			if (running_data.requires_recognize_treasure) {
				auto view = devices.camera.Get();
				running_data.current_treasure = vision::CVUtil::RECOGNIZE_TREASURE(view, team_current->card_range, team_current->pattern_range);
				if (!running_data.current_treasure)
					vision::CVUtil::RECOGNIZE_TREASURE(view, team_target->card_range, team_target->pattern_range);
			}
		}
		ctrl.Update();
	}
	devices.Close();

	data_main.SaveSetting("setting.json");
}
