#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/graph/neighbor_bfs.hpp>

#include "optical_design/defined.h"
#include "optical_design/main_data.h"
#include "optical_design/data/maze/maze.h"

OPT_BEGIN

class Route {
public:
	using segment_inflection_type = InflectionSegment;
	using segment_node_type = NodeSegment;

	Route(const std::ranges::forward_range auto& nodes) {
		std::ranges::copy(nodes, std::back_inserter(_nodes));

		auto& maze = *DataManager::I().running_data.maze;

		for (auto it = _nodes.begin(); it != _nodes.end() - 1; it++) {
			auto current_node = *it;
			auto next_node = *(it + 1);

			NGS_ASSERT(maze.GetNodeSegment().contains(segment_node_type(current_node, next_node)));
			auto segment = const_cast<segment_node_type*>(&*maze.GetNodeSegment().find(segment_node_type(current_node, next_node)));

			_segments.push_back(segment);
			_cost += segment->length;
		}
	}
	void Normalize() {
		for (auto it = _nodes.begin(); it != _nodes.end() - 1; it++) {
			auto current_node = *it;
			auto current_segment = _segments[it - _nodes.begin()];

			if (current_node != current_segment->GetStart())
				current_segment->Reverse();
			NGS_ASSERT(current_node == current_segment->GetStart());
		}
	}

	segment_node_type* IsOnRoute(const position_t& position) const { return IsOnRoute(position.x, position.y); }
	segment_node_type* IsOnRoute(size_t x, size_t y)const {
		for (auto& segment : _segments) {
			if (segment->IsOnRoute(x, y))return segment;
		}
		return nullptr;
	}
	size_t GetCost()const { return _cost; }
	const std::vector<Node*>& GetNodes()const { return _nodes; }
	const std::vector<segment_node_type*>& GetSegments()const { return _segments; }
private:
	std::vector<Node*> _nodes{};
	std::vector<segment_node_type*> _segments{};
	size_t _cost{};
};

class Path {
private:

public:
	using cost_type = size_t;
	using graph_type = boost::adjacency_list<
		boost::listS,
		boost::vecS,
		boost::undirectedS,
		boost::no_property,
		boost::property<boost::edge_weight_t, cost_type>
	>;
	using edge_type = std::pair<ngs::Point2s, ngs::Point2s>;
	using vertex_type = boost::graph_traits<graph_type>::vertex_descriptor;
public:
	Path() {
		CacheMaze();
	}
	~Path() { if (_graph)NGS_DELETE(_graph); }
	void CacheMaze() {
		if (_graph)NGS_DELETE(_graph);
		auto& maze = *DataManager::I().running_data.maze;
		auto& nodes = maze.GetNodes();
		std::vector<std::pair<ngs::uint32, ngs::uint32>> edges{};
		std::vector<size_t> weight{};
		{
			for (auto& segment : maze.GetNodeSegment()) {
				edges.emplace_back(segment.first->GetID(), segment.second->GetID());
				weight.emplace_back(segment.length);
			}
		}
		NGS_NEW(_graph, graph_type)(edges.begin(), edges.end(), weight.begin(), nodes.size());
	}
	Route Search(position_t start, position_t goal) {
		std::vector<Node*> nodes{};
		nodes.push_back(const_cast<Node*>(&DataManager::I().running_data.maze->GetNode(start)));
		std::ranges::transform(_AStar(start, goal), std::back_inserter(nodes), [](vertex_type id)-> Node* {
			return const_cast<Node*>(&DataManager::I().running_data.maze->GetNode(id));
			});
		return { nodes };
	}
private:
	struct _FoundGoal {};
	class _Visitor :public boost::default_astar_visitor
	{
	public:
		_Visitor(vertex_type goal)
			: _goal(goal)
		{}

		template<class _Graph>
		void examine_vertex(vertex_type v, _Graph& g) { if (v == _goal)throw _FoundGoal(); }
	private:
		vertex_type _goal;
	};

	std::deque<vertex_type> _AStar(const position_t& start_pos, const position_t& goal_pos) {
		class _DistanceHeuristic : public boost::astar_heuristic<graph_type, cost_type>
		{
		public:
			_DistanceHeuristic(vertex_type goal, graph_type& graph)
				: _goal(goal)
				, _graph(graph)
			{
			}
			cost_type operator()(vertex_type v)
			{
				return boost::get(boost::vertex_index, _graph, _goal) - boost::get(boost::vertex_index, _graph, v);
			}
		private:
			vertex_type _goal;
			graph_type& _graph;
		};

		auto& maze = *DataManager::I().running_data.maze;
		auto& graph = *_graph;

		auto start_id = maze.GetNode(start_pos).GetID();
		auto goal_id = maze.GetNode(goal_pos).GetID();
		vertex_type start = boost::vertex(start_id, graph);
		vertex_type goal = boost::vertex(goal_id, graph);

		std::vector<vertex_type> parents(boost::num_vertices(graph));
		std::vector<cost_type> distance(boost::num_vertices(graph));

		try {
			boost::astar_search_tree(
				graph, start_id,
				_DistanceHeuristic(goal_id, graph),
				boost::predecessor_map(&parents[0]).distance_map(&distance[0]).visitor(_Visitor(goal_id))
			);
		}
		catch (_FoundGoal) {
			std::deque<vertex_type> route;
			for (vertex_type v = goal_id; v != start_id; v = parents[v])
				route.push_front(v);

			return route;
		}
		return {};
	}
	std::vector<vertex_type> _BFS(const position_t& start_pos, const position_t& goal_pos) {
		struct _BFSVisitor : public boost::default_bfs_visitor {
			std::vector<vertex_type> path;

			void discover_vertex(vertex_type v, const graph_type& graph) {
				path.push_back(v);
			}
		};
		auto& maze = *DataManager::I().running_data.maze;
		auto& graph = *_graph;

		auto start_id = maze.GetNode(start_pos).GetID();
		auto goal_id = maze.GetNode(goal_pos).GetID();
		vertex_type start = boost::vertex(start_id, graph);
		vertex_type goal = boost::vertex(goal_id, graph);

		std::vector<vertex_type> path;
	}
private:
	graph_type* _graph = nullptr;
};

OPT_END
