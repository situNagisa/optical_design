#include "b2d/test.h"
#include "optical_design/optical_design.h"


class GameMainLogic : public Test {
public:

	static Test* Create() {
		return new GameMainLogic();
	}
public:
	GameMainLogic()
	{
		m_world->SetGravity({ 0,0 });

		g_camera.m_center = opt_b2::Transform(opt_vision::Transform(vision::CVUtil::GET_MAZE_CENTER()));
		g_camera.m_zoom = 0.1f;
	}
	void Step(Settings& settings) override {
		auto& running_data = opt::DataManager::I().running_data;
		g_debugDraw.DrawSegment(opt_b2::Transform(running_data.car.position), opt_b2::Transform(running_data.next_position), { 0.5,0.5,0.5 });
		g_debugDraw.DrawPoint(opt_b2::Transform(running_data.car.position), 2, { 1,1,1 });
		g_debugDraw.DrawPoint(opt_b2::Transform(running_data.next_position), 2, { 1,1,1 });
		for (auto& i : running_data.maze->GetTreasures()) {
			g_debugDraw.DrawPoint(opt_b2::Transform(i), 5, { 0xFF,0,1 });
		}
		Test::Step(settings);
	}
	b2World* NewWorld(const b2Vec2&)override {
		return opt::DataManager::I().running_data.world;
	}
	void DestroyWorld(b2World*)override {

	}
	void StepWorld(float timeStep, int32 velocityIterations, int32 positionIterations)override
	{

	}
private:
};

static int testIndex = RegisterTest("ngs", "Game Main Logic", GameMainLogic::Create);