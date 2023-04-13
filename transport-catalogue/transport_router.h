#pragma once

#include <map>
#include <string_view>

#include "domain.h"
#include "graph.h"
#include "transport_catalogue.h"

#include "router.h"

using namespace graph;

using Buses = const std::deque<domain::Bus>&;
using VectorStops = std::vector<domain::Stop*>;

namespace transport {

	struct Item {
		std::size_t index_bus__;
		std::size_t span_count__;
	};

	class BusGraph : public graph::DirectedWeightedGraph<double> {
	public:
		BusGraph(Catalogue& catalogue);

		const std::string_view GetNameBus(EdgeId edge_id) const;	
		int GetSpanCount(EdgeId edge_id) const;

	private:
		Catalogue& catalogue_;
		std::map<graph::EdgeId, Item> items_;
		domain::Setting setting_;

		void BuildGraph();
		double CalcTime(const domain::Stop& stop_from, const domain::Stop& stop_to);
	};

} // namespace transport
