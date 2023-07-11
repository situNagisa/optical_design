#pragma once

#include <box2d/b2_api.h>
#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_polygon_shape.h>

#include "optical_design/defined.h"
#include "optical_design/main_data.h"
#include "optical_design/data/maze/maze.h"

namespace opt_b2 {
	using box2d_pos_t = ngs::Point2f;

	constexpr box2d_pos_t _transformant{ 0.2f, -0.2f };

	inline b2Vec2 Convert(const box2d_pos_t& pos) { return b2Vec2(pos.x, pos.y); }
	inline box2d_pos_t Convert(const b2Vec2& pos) { return ngs::Point2f(pos.x, pos.y); }

	inline b2Vec2 Transform(const opt::position_t& pos) { return Convert(box2d_pos_t(pos.x, pos.y) * _transformant); }
	inline opt::position_t Transform(const b2Vec2& pos) {
		auto result = Convert(pos) / _transformant;
		return { (opt::position_t::type)std::round(result.x),(opt::position_t::type)std::round(result.y) };
	}
}

OPT_BEGIN

class Physic {
private:

public:
	Physic() {
		NGS_NEW(_world, b2World)({ 0,0 });
		_InitWall();
		_InitBody();
		DataManager::I().running_data.world = _world;

		NGS_LOGL(info, "the physical engine has been initialized successfully!");
	}
	~Physic() {
		NGS_DELETE(_world);
		DataManager::I().running_data.world = nullptr;
		_body = nullptr;
	}

	void Update() {
		auto lv = data_manager.running_data.car.linear_velocity;
		_body->SetLinearVelocity({ lv * std::cos(_body->GetAngle()),lv * std::sin(_body->GetAngle()) });
		_body->SetAngularVelocity(data_manager.running_data.car.angular_velocity);

		_world->Step((float)data_manager.running_data.timer.GetDuration().count() / 1'000'000'000, 8, 2);

		data_manager.running_data.car.angle = _body->GetAngle();
		data_manager.running_data.car.position = opt_b2::Transform(_body->GetPosition());

		NGS_DEBUG_EXPR(
			if (DataManager::I().setting.debug.show_car_state)
				NGS_LOGFL(debug, "[%fs] angle: %d, pos: (%d,%d)", (float)data_manager.running_data.timer.GetDuration().count() / 1'000'000'000, ngs::AsDegree(data_manager.running_data.car.angle), data_manager.running_data.car.position.x, data_manager.running_data.car.position.y)
				);
	}
private:
	void _InitWall() {

	}
	void _InitBody() {
		auto& running_data = data_manager.running_data;
		b2BodyDef bd;
		auto start = running_data.maze->GetStartPoint();
		bd.type = b2_dynamicBody;
		bd.position = opt_b2::Transform(start);

		if (start.x == 0)
			bd.angle = 0;
		else if (start.x == vision::GRID_NUM.x - 1)
			bd.angle = std::numbers::pi * 0.5;
		else if (start.y == 0)
			bd.angle = -std::numbers::pi;
		else if (start.y == vision::GRID_NUM.y - 1)
			bd.angle = std::numbers::pi;
		else
			NGS_LOGL(error, "the start point is not on the edge of the maze");

		bd.allowSleep = false;
		_body = _world->CreateBody(&bd);
		b2PolygonShape shape;
		shape.SetAsBox(data_manager.setting.car.size.x * 0.5, data_manager.setting.car.size.y * 0.5);
		_body->CreateFixture(&shape, data_manager.setting.car.mass / (data_manager.setting.car.size.x * data_manager.setting.car.size.y));
	}
private:
	b2World* _world = nullptr;
	b2Body* _body = nullptr;
	DataManager& data_manager = DataManager::I();
};

OPT_END