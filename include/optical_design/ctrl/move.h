#pragma once

#include "optical_design/defined.h"
#include "optical_design/data/maze/maze.h"
#include "optical_design/main_data.h"
#include "optical_design/data/path.h"

OPT_BEGIN

class MoveCtrl {
public:
	void Update() {
		using segment_inflection_type = InflectionSegment;
		auto current_position = DataManager::I().running_data.car.position;

		if (!_inflection_segment || !_inflection_segment.value().IsOnRoute(current_position)) {
			_node_segment = nullptr;
			if (!_route || !(_node_segment = _route.value().IsOnRoute(current_position))) {
				_SearchNearestTreasureRoute();
				_node_segment = _route.value().IsOnRoute(current_position);
			}
			_inflection_segment = _node_segment->IsOnRoute(current_position);
		}
		auto next_position = _inflection_segment.value().GetEnd()->position;
		if (current_position == next_position) {
			if (_IsLastInflectionSegment()) {
				if (_IsLastNodeSegment()) {
					_SearchNearestTreasureRoute();
				}
				_NextNodeSegment();
			}
			_NextInflectionSegment();
		}

		DataManager::I().running_data.next_position = next_position;
	}
private:
	void _SearchNearestTreasureRoute() {
		auto& treasures = DataManager::I().running_data.maze->GetTreasures();
		for (auto treasure : treasures) {
			auto&& route = _path.Search(DataManager::I().running_data.car.position, treasure);
			if (!_route || route.GetCost() < _route.value().GetCost())
				_route = route;
		}
		_route.value().Normalize();
#if NGS_BUILD_TYPE == NGS_DEBUG
		NGS_LOGF(debug, "(%d, %d)", _route.value().GetNodes().front()->position.x, _route.value().GetNodes().front()->position.y);
		for (auto it = _route.value().GetNodes().begin() + 1; it != _route.value().GetNodes().end(); it++) {
			NGS_PRINTF(" -> (%d, %d)", (*it)->position.x, (*it)->position.y);
		}
		NGS_PRINTL();
#endif
		NGS_ASSERT(_route && _route.value().IsOnRoute(DataManager::I().running_data.car.position));
	}
	bool _IsLastInflectionSegment()const { return _inflection_segment && _node_segment->route.back()->position == _inflection_segment.value().GetEnd()->position; }
	bool _IsLastNodeSegment()const { return _node_segment && _node_segment == _route.value().GetSegments().back(); }

	void _NextInflectionSegment() {
		_inflection_segment = _node_segment->IsOnRoute(DataManager::I().running_data.car.position);
	}
	void _NextNodeSegment() {
		_node_segment = _route.value().IsOnRoute(DataManager::I().running_data.car.position);
	}
private:
	Path _path{};
	std::optional<InflectionSegment> _inflection_segment{};
	const NodeSegment* _node_segment{};
	std::optional<Route> _route{};
};

OPT_END
