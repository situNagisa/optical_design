#pragma once

#include "optical_design/defined.h"
#include "optical_design/main_data.h"
#include "optical_design/data/maze/inflection_point.h"

OPT_BEGIN

struct Node {
	size_t GetNumWalkable()const { return next.size(); }
	position_id_t GetID()const { return ConvertPosition(position); }

	position_t position{};
	std::set <std::pair<size_t, Node*>> next{};

	friend size_t operator-(const Node& a, const Node& b) {
		for (auto& [cost, next] : a.next) {
			if (next == &b)return cost;
		}
		NGS_ASSERT(false);
		return 0;
	}
};

struct NodeSegment : public std::pair<Node*, Node*> {
	using std::pair<Node*, Node*>::pair;

	bool operator==(const NodeSegment& other)const {
		return
			(this->first == other.first && this->second == other.second)
			|| (this->first == other.second && this->second == other.first);
	}
	size_t length{};
	std::vector<InflectionPoint*> route{};

	bool IsOnRoute(const InflectionPoint* element) const { return std::ranges::find(route, element) != std::ranges::end(route); }
	std::optional<InflectionSegment> IsOnRoute(size_t x, size_t y) const {
		for (auto it = route.begin(); it != route.end() - 1; it++) {
			auto current_inflection = *it;
			auto next_inflection = *(it + 1);
			InflectionSegment segment{ current_inflection,next_inflection };
			if (segment.IsOnRoute(x, y))return segment;
		}
		return {};
	}
	auto IsOnRoute(const position_t& position) const { return IsOnRoute(position.x, position.y); }

	auto GetStart()const { return first; }
	auto GetEnd()const { return second; }

	void Reverse() {
		std::swap(first, second);
		std::ranges::reverse(route);
		NGS_ASSERT(route.front()->position == GetStart()->position && route.back()->position == GetEnd()->position);
	}
};

OPT_END

namespace std {
	template<>
	struct hash<opt::NodeSegment> {
		ngs::uint64 operator()(const opt::NodeSegment& s) const {
			auto a = s.first->GetID();
			auto b = s.second->GetID();
			if (a < b)return (ngs::uint64)b << 32 & a;
			return (ngs::uint64)a << 32 & b;
		}
	};
}