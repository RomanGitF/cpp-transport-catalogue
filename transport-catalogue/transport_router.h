#pragma once

#include <map>
#include <string_view>

#include "domain.h"
#include "graph.h"
#include "transport_catalogue.h"

#include "router.h"

using namespace graph;



namespace transport {

	using Buses = const std::deque<domain::Bus>&;
	using VectorStops = std::vector<domain::Stop*>;
	using BaseGraph = graph::DirectedWeightedGraph<double>;
	using BaseRouter = graph::Router<double>;

	struct Item {
		std::size_t index_bus__;
		std::size_t span_count__;
	};

	class BusGraph : public BaseGraph {
	public:
		BusGraph(Catalogue& catalogue);

		const std::string_view GetNameBus(EdgeId edge_id) const;	
		int GetSpanCount(EdgeId edge_id) const;

		const BaseGraph& GetGraph() const{
			return *this;
		}

	private:
		Catalogue& catalogue_;
		std::map<graph::EdgeId, Item> items_;
		domain::Setting setting_;

		void BuildGraph();
		double CalcTime(const domain::Stop& stop_from, const domain::Stop& stop_to);

		int MInKm = 1000;
		int SInH = 60;
	};

	class BusRouter : public BaseRouter {
		const BusGraph& bus_graph_;
	public:
		BusRouter(const BusGraph& bus_graph);

		const Edge<double>& GetEdge(EdgeId edge_id) const;
		const std::string_view GetNameBus(EdgeId edge_id) const;
		int GetSpanCount(EdgeId edge_id) const;
	};

} // namespace transport
