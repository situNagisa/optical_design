#pragma once

#include "optical_design/ctrl/move.h"
#include "optical_design/ctrl/physic.h"

OPT_BEGIN

class Ctrl {
public:

	void Update() {
		physic.Update();
		move_ctrl.Update();
	}


public:
	Physic physic;
	MoveCtrl move_ctrl;
};

OPT_END
