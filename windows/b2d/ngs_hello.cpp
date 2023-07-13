#include <box2d/test/test.h>
#include "optical_design/optical_design.h"
#include "data_manager.h"

class GameMainLogic : public box2d::Test {
public:

	static Test* Create() {
		return new GameMainLogic(&DataManager::I().debug_draw);
	}
public:
	GameMainLogic(box2d::DebugDraw* debug_draw)
		: box2d::Test(opt::DataManager::I().running_data.world, debug_draw)
	{
		m_world->SetGravity({ 0,0 });

		debug_draw->camera->m_center = opt_b2::Transform(opt_vision::Transform(vision::CVUtil::GET_MAZE_CENTER()));
		debug_draw->camera->m_zoom = 0.1f;
	}
	void Step(box2d::Settings& settings) override {
		auto& running_data = opt::DataManager::I().running_data;
		debug_draw->DrawSegment(opt_b2::Transform(running_data.car.position), opt_b2::Transform(running_data.next_position), { 0.5,0.5,0.5 });
		debug_draw->DrawPoint(opt_b2::Transform(running_data.car.position), 2, { 1,1,1 });
		debug_draw->DrawPoint(opt_b2::Transform(running_data.next_position), 2, { 1,1,1 });
		for (auto& i : running_data.maze->GetTreasures()) {
			debug_draw->DrawPoint(opt_b2::Transform(i), 5, { 0xFF,0,1 });
		}
		Test::Step(settings);
	}
	void StepWorld(float timeStep, int32 velocityIterations, int32 positionIterations)override
	{

	}

	;
private:
};

static int testIndex = box2d::TestSelector::I().Register("ngs", "Game Main Logic", GameMainLogic::Create);