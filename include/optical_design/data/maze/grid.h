#pragma once

#include "optical_design/defined.h"
#include "optical_design/main_data.h"

OPT_BEGIN

struct Grid {
	size_t GetNumWalkable()const {
		size_t count = 0;
		if (walkable.up)count++;
		if (walkable.down)count++;
		if (walkable.left)count++;
		if (walkable.right)count++;
		return count;
	}
	bool IsStraight()const {
		return
			(walkable.up && walkable.down && !walkable.left && !walkable.right)
			|| (!walkable.up && !walkable.down && walkable.left && walkable.right);
	}
	bool IsDeadEnd()const { return GetNumWalkable() == 1; }
	bool IsFork()const { return GetNumWalkable() > 2; }
	bool IsCorner()const { return GetNumWalkable() == 2 && !IsStraight(); }
	bool IsInflection()const { return IsCorner() || IsFork() || IsDeadEnd(); }

	bool IsTreasure()const {
		return
			type == vision::MazeGrid::treasure
			|| type == vision::MazeGrid::treasure_target
			|| type == vision::MazeGrid::treasure_unknown;
	}
	bool IsEnd()const { return type == vision::MazeGrid::end; }
	bool IsAim()const { return IsTreasure() || IsEnd(); }

	vision::MazeGrid type = vision::MazeGrid::unknown;
	struct {
		bool up : 1;
		bool down : 1;
		bool left : 1;
		bool right : 1;
	}walkable = {};
};

OPT_END
