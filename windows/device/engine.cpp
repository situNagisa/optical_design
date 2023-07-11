#include "optical_design/device/engine.h"

OPT_BEGIN

bool Engine::Open() { return true; }
void Engine::Close() {}
bool Engine::IsOpened() const { return true; }
void Engine::Update() {}
void Engine::SetLinearVelocityPercent(ngs::float32 percent) {}
void Engine::SetAngularVelocityPercent(ngs::float32 percent) {}
ngs::float32 Engine::GetLinearVelocity()const { return 0.0f; }

OPT_END
