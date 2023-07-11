#pragma once

#include "optical_design/defined.h"
#include "optical_design/main_data.h"

OPT_BEGIN

struct InflectionPoint {

	size_t GetNumWalkable()const {
		size_t count = 0;
		if (up)count++;
		if (down)count++;
		if (left)count++;
		if (right)count++;
		return count;
	}
	std::vector<InflectionPoint*> GetWalkable()const {
		std::vector<InflectionPoint*> walkable;
		if (up)walkable.push_back(up);
		if (down)walkable.push_back(down);
		if (left)walkable.push_back(left);
		if (right)walkable.push_back(right);
		return walkable;
	}
	bool IsDeadEnd()const { return GetNumWalkable() == 1; }
	bool IsFork()const { return GetNumWalkable() > 2; }
	bool IsCorner()const { return GetNumWalkable() == 2; }
	bool IsNode()const { return IsDeadEnd() || IsFork(); }

	position_id_t GetID()const { return ConvertPosition(position); }

	position_t position{};
	InflectionPoint* up = nullptr;
	InflectionPoint* down = nullptr;
	InflectionPoint* left = nullptr;
	InflectionPoint* right = nullptr;

	friend size_t operator-(const InflectionPoint& a, const InflectionPoint& b) {
		if (a.position.x == b.position.x)
			return std::abs((int)b.position.y - (int)a.position.y);
		if (a.position.y == b.position.y)
			return std::abs((int)b.position.x - (int)a.position.x);
		NGS_ASSERT(false);
		return 0;
	}
};

struct InflectionSegment : public std::pair<InflectionPoint*, InflectionPoint*> {
	InflectionSegment(InflectionPoint* a, InflectionPoint* b)
		: std::pair<InflectionPoint*, InflectionPoint*>(a, b)
	{
		auto a_walkable = a->GetWalkable();
		auto b_walkable = b->GetWalkable();
		NGS_ASSERT(std::ranges::find(a_walkable, b) != a_walkable.end());
		NGS_ASSERT(std::ranges::find(b_walkable, a) != b_walkable.end());
	}

	bool operator==(const InflectionSegment& other)const {
		return
			(this->first == other.first && this->second == other.second)
			|| (this->first == other.second && this->second == other.first);
	}
	size_t length{};

	bool IsOnRoute(position_t position) const {
		return IsOnRoute(position.x, position.y);
	}
	bool IsOnRoute(size_t x, size_t y) const {
		if (first->position.x == second->position.x) {
			return x == first->position.x;
		}
		else if (first->position.y == second->position.y) {
			return y == first->position.y;
		}
		NGS_ASSERT(false);
		return false;
	}
	auto GetStart()const { return first; }
	auto GetEnd()const { return second; }

	void Reverse() {
		std::swap(first, second);
	}
};

OPT_END


namespace std {
	template<>
	struct hash<opt::InflectionSegment> {
		ngs::uint64 operator()(const opt::InflectionSegment& s) const {
			auto a = s.first->GetID();
			auto b = s.second->GetID();
			if (a < b)return (ngs::uint64)b << 32 & a;
			return (ngs::uint64)a << 32 & b;
		}
	};
}
