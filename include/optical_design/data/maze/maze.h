#pragma once

#include "optical_design/defined.h"
#include "optical_design/main_data.h"
#include "optical_design/data/maze/grid.h"
#include "optical_design/data/maze/inflection_point.h"
#include "optical_design/data/maze/node.h"

OPT_BEGIN

class Maze {
private:

public:
	using segment_inflection_type = InflectionSegment;
	using segment_node_type = NodeSegment;

	Maze(const vision::MazeGrid(&maze)[vision::GRID_NUM.x][vision::GRID_NUM.y]) {
		_InitGrid(maze);
		_InitInflectionPoint();
		NGS_DEBUG_EXPR(if (DataManager::I().setting.debug.show_inflection)PrintInflectionPoints());
		_InitNode();
		NGS_DEBUG_EXPR(if (DataManager::I().setting.debug.show_node) PrintNodes());
	}
public:
	const Grid& GetGrid(size_t x, size_t y) const {
		NGS_ASSERT(x < vision::GRID_NUM.x && y < vision::GRID_NUM.y);
		return _grids[x][y];
	}
	const Grid& GetGrid(const ngs::Point2s& position)const { return GetGrid(position.x, position.y); }
	const InflectionPoint& GetInflectionPoint(size_t x, size_t y) const {
		NGS_ASSERT(_inflection_points.contains(x) && _inflection_points.at(x).contains(y));
		return _inflection_points.at(x).at(y);
	}
	const InflectionPoint& GetInflectionPoint(const ngs::Point2s& position)const { return GetInflectionPoint(position.x, position.y); }
	const Node& GetNode(size_t x, size_t y) const {
		NGS_ASSERT(_nodes.contains(x) && _nodes.at(x).contains(y));
		return _nodes.at(x).at(y);
	}
	const Node& GetNode(const position_t& position)const { return GetNode(position.x, position.y); }
	const Node& GetNode(position_id_t id)const { return GetNode(id >> 16, id & 0xFFFF); }

	const auto& GetNodes()const { return _nodes; }

	void SetStartPoint(size_t x, size_t y) { _grids[x][y].type = vision::MazeGrid::start; _start.Set(x, y); }
	void SetEndPoint(size_t x, size_t y) { _grids[x][y].type = vision::MazeGrid::end; _end.Set(x, y); }
	const position_t& GetStartPoint()const { return _start; }
	const position_t& GetEndPoint()const { return _end; }

	void SetTreasures(const std::ranges::forward_range auto& treasures) {
		std::ranges::transform(treasures, std::back_inserter(_treasures), (position_t(*)(const vision::position_t&))opt_vision::Transform);
		for (auto pos : _treasures) {
			auto& grid = _grids[pos.x][pos.y];
			grid.type = vision::MazeGrid::treasure_unknown;
		}
	}
	const auto& GetTreasures()const { return _treasures; }

	void PrintGrids() {
		for (size_t y = 0; y < vision::GRID_NUM.y; y++)
		{
			for (size_t x = 0; x < vision::GRID_NUM.x; x++)
			{
				auto& grid = _grids[x][y];
				switch (grid.type) {
				case vision::MazeGrid::wall:
					NGS_PRINT(ngs::TextColor::black, "墙");
					break;
				case vision::MazeGrid::road:
					NGS_PRINT(ngs::TextColor::white, "路");
					break;
				case vision::MazeGrid::treasure:
				case vision::MazeGrid::treasure_target:
				case vision::MazeGrid::treasure_unknown:
					NGS_PRINT(ngs::TextColor::green, "宝");
					break;
				case vision::MazeGrid::start:
					NGS_PRINT(ngs::TextColor::red, "起");
					break;
				case vision::MazeGrid::end:
					NGS_PRINT(ngs::TextColor::blue, "终");
					break;
				}
			}
			NGS_PRINTL(ngs::TextColor::reset);
		}
	}
	void PrintInflectionPoints() {
		for (size_t y = 0; y < vision::GRID_NUM.y; y++)
		{
			for (size_t x = 0; x < vision::GRID_NUM.x; x++)
			{
				auto& grid = _grids[x][y];
				if (_inflection_points.contains(x) && _inflection_points.at(x).contains(y)) {
					NGS_PRINT(ngs::TextColor::magenta, "拐");
					continue;
				}
				switch (grid.type) {
				case vision::MazeGrid::wall:
					NGS_PRINT(ngs::TextColor::black, "墙");
					break;
				case vision::MazeGrid::road:
					NGS_PRINT(ngs::TextColor::white, "路");
					break;
				case vision::MazeGrid::treasure:
				case vision::MazeGrid::treasure_target:
				case vision::MazeGrid::treasure_unknown:
					NGS_PRINT(ngs::TextColor::green, "宝");
					break;
				case vision::MazeGrid::start:
					NGS_PRINT(ngs::TextColor::red, "起");
					break;
				case vision::MazeGrid::end:
					NGS_PRINT(ngs::TextColor::blue, "终");
					break;
				}
			}
			NGS_PRINTL(ngs::TextColor::reset);
		}
	}
	void PrintNodes() {
		for (size_t y = 0; y < vision::GRID_NUM.y; y++)
		{
			for (size_t x = 0; x < vision::GRID_NUM.x; x++)
			{
				auto& grid = _grids[x][y];
				if (_nodes.contains(x) && _nodes.at(x).contains(y)) {
					NGS_PRINT(ngs::TextColor::magenta, "节");
					continue;
				}
				switch (grid.type) {
				case vision::MazeGrid::wall:
					NGS_PRINT(ngs::TextColor::black, "墙");
					break;
				case vision::MazeGrid::road:
					NGS_PRINT(ngs::TextColor::white, "路");
					break;
				case vision::MazeGrid::treasure:
				case vision::MazeGrid::treasure_target:
				case vision::MazeGrid::treasure_unknown:
					NGS_PRINT(ngs::TextColor::green, "宝");
					break;
				case vision::MazeGrid::start:
					NGS_PRINT(ngs::TextColor::red, "起");
					break;
				case vision::MazeGrid::end:
					NGS_PRINT(ngs::TextColor::blue, "终");
					break;
				}
			}
			NGS_PRINTL(ngs::TextColor::reset);
		}
	}

	const auto& GetInflectionSegments()const { return _inflection_segments; }
	const auto& GetNodeSegment()const { return _node_segments; }

private:
	void _InitGrid(const vision::MazeGrid(&maze)[vision::GRID_NUM.x][vision::GRID_NUM.y]) {
		for (size_t x = 0; x < vision::GRID_NUM.x; x++)
		{
			for (size_t y = 0; y < vision::GRID_NUM.y; y++)
			{
				auto& grid = _grids[x][y] = { .type = maze[x][y],.walkable = {} };
				if (grid.type == vision::MazeGrid::wall) continue;
				grid.walkable.up = (y == 0) ? false : maze[x][y - 1] != vision::MazeGrid::wall;
				grid.walkable.down = (y == vision::GRID_NUM.y - 1) ? false : maze[x][y + 1] != vision::MazeGrid::wall;
				grid.walkable.left = (x == 0) ? false : maze[x - 1][y] != vision::MazeGrid::wall;
				grid.walkable.right = (x == vision::GRID_NUM.x - 1) ? false : maze[x + 1][y] != vision::MazeGrid::wall;
			}
		}
	}
	void _InitInflectionPoint() {
		for (size_t x = 0; x < vision::GRID_NUM.x; x++)
		{
			for (size_t y = 0; y < vision::GRID_NUM.y; y++)
			{
				auto& grid = _grids[x][y];
				if (!grid.IsInflection())continue;
				auto& inflection_point = _inflection_points[x][y] = {};
				inflection_point.position.Set(x, y);
			}
		}
		for (auto it_x = _inflection_points.begin(); it_x != _inflection_points.end(); it_x++) {
			auto& ys = it_x->second;
			for (auto it_y = ys.begin(); it_y != ys.end(); it_y++) {
				auto inflection_point = &it_y->second;
				auto& [position, up, down, left, right] = *inflection_point;
				auto& grid = _grids[position.x][position.y];


				if (grid.walkable.up) up = &std::ranges::prev(it_y)->second;

				if (grid.walkable.down) down = &std::ranges::next(it_y)->second;

				if (grid.walkable.left) {
					auto prev = it_x;
					do {
						prev--;
					} while (!prev->second.contains(position.y));
					left = &prev->second[position.y];
				}
				if (grid.walkable.right) {
					auto next = it_x;
					do {
						next++;
					} while (!next->second.contains(position.y));
					right = &next->second[position.y];
				}
			}
		}
		for (auto it_x = _inflection_points.begin(); it_x != _inflection_points.end(); it_x++) {
			auto& ys = it_x->second;
			for (auto it_y = ys.begin(); it_y != ys.end(); it_y++) {
				auto inflection_point = &it_y->second;
				auto& [position, up, down, left, right] = *inflection_point;
				if (up) {
					segment_inflection_type segment{ inflection_point, up };
					segment.length = *up - *inflection_point;
					_inflection_segments.emplace(segment);
				}
				if (down) {
					segment_inflection_type segment{ inflection_point, down };
					segment.length = *down - *inflection_point;
					_inflection_segments.emplace(segment);
				}
				if (left) {
					segment_inflection_type segment{ inflection_point, left };
					segment.length = *left - *inflection_point;
					_inflection_segments.emplace(segment);
				}
				if (right) {
					segment_inflection_type segment{ inflection_point, right };
					segment.length = *right - *inflection_point;
					_inflection_segments.emplace(segment);
				}
			}
		}
	}
	void _InitNode() {
		std::queue<Node*> queue = {};
		for (auto xs : _inflection_points) {
			for (auto& [y, inflection_point] : xs.second | std::views::filter([](const std::pair<size_t, InflectionPoint>& pair) ->bool {
				return pair.second.IsNode();
				})) {
				auto& node = _nodes[inflection_point.position.x][inflection_point.position.y] = {};
				node.position = inflection_point.position;
				node.next.clear();
				queue.push(&node);
			}
		}
		while (!queue.empty()) {
			auto& node = *queue.front();
			queue.pop();
			auto& inflection_point = _inflection_points[node.position.x][node.position.y];
			auto walkable = inflection_point.GetWalkable();
			for (auto next_inflections : walkable) {
				size_t cost = 0;
				std::vector<InflectionPoint*> route_inflection{};
				route_inflection.push_back(&inflection_point);

				cost += *next_inflections - inflection_point;
				route_inflection.push_back(next_inflections);

				auto current_inflection = next_inflections;
				opt::InflectionPoint* prev_inflection = &inflection_point;

				while (!current_inflection->IsNode()) {
					NGS_ASSERT(current_inflection->IsCorner());
					auto current_walkable = current_inflection->GetWalkable();
					auto next_inflection = (current_walkable[0] == prev_inflection) ? current_walkable[1] : current_walkable[0];

					cost += *next_inflection - *current_inflection;
					route_inflection.push_back(next_inflection);

					prev_inflection = current_inflection;
					current_inflection = next_inflection;
				}
				NGS_ASSERT(_nodes.contains(current_inflection->position.x) && _nodes[current_inflection->position.x].contains(current_inflection->position.y));
				auto next_node = &_nodes[current_inflection->position.x][current_inflection->position.y];
				node.next.insert({ cost,next_node });

				segment_node_type segment = { &node,next_node };
				segment.route = route_inflection;
				segment.length = cost;
				_node_segments.insert(segment);
			}
			NGS_ASSERT(node.next.size() <= walkable.size());
		}
	}
private:
	Grid _grids[vision::GRID_NUM.x][vision::GRID_NUM.y]{};
	std::map<size_t, std::map<size_t, InflectionPoint>> _inflection_points{};
	std::unordered_set<segment_inflection_type> _inflection_segments{};

	std::map<size_t, std::map<size_t, Node>> _nodes{};
	std::unordered_set<segment_node_type> _node_segments{};

	std::vector<position_t> _treasures{};

	position_t _start{}, _end{};
};

OPT_END
