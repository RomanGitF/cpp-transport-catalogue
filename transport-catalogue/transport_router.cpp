#include "transport_router.h"

using namespace graph;
using namespace transport;

BusGraph::BusGraph(Catalogue& catalogue)
	:graph::DirectedWeightedGraph<double>(catalogue.GetCountStops() * 2)
	, catalogue_(catalogue)
	, setting_(catalogue.GetSetting())
{
	BuildGraph();
}

const std::string BusGraph::GetNameBus(EdgeId edge_id) const {
	size_t index = items_.at(edge_id).index_bus__;
	return catalogue_.GetDequeBuses().at(index).name__;
}

int BusGraph::GetSpanCount(EdgeId edge_id) const {
	return items_.at(edge_id).span_count__;
}

double BusGraph::CalcTime(const domain::Stop& stop_from, const domain::Stop& stop_to) {
	size_t length;
	if (stop_from.route_length__.count(stop_to.name__)) {
		length = stop_from.route_length__.at(stop_to.name__);
	}
	else if (stop_to.route_length__.count(stop_from.name__)) {
		length = stop_to.route_length__.at(stop_from.name__);
	}
	else { return 0.; }
	double V = 1.0 * MInKm * setting_.bus_velocity__ / SInH;
	double time = 1.0 * length / V;
	return time;
}

void BusGraph::BuildGraph() {
	Buses buses = catalogue_.GetDequeBuses();
	for (const auto& bus : buses) {
		VectorStops stops = bus.stops__;

		if (!bus.is_roundtrip__) {
			stops.insert(stops.end(), stops.rbegin(), stops.rend());
		}

		for (size_t from = 0; from < stops.size(); ++from) {
			double time = setting_.bus_wait_time__;
			for (size_t to = from + 1; to < stops.size(); ++to) {
				graph::VertexId vertex_from = stops.at(from)->index__;
				graph::VertexId vertex_to = stops.at(to)->index__;
				time += CalcTime(*stops.at(to - 1), *stops.at(to));
				EdgeId key = this->AddEdge({ vertex_from, vertex_to, time });
				size_t span_count = to - from;
				items_[key] = { bus.index__ ,span_count };
			}
			time = setting_.bus_wait_time__;
		}
	}
}

void BusGraph::Serialize(TCProto::BusGraph& proto) {
	proto.set_bus_wait_time(setting_.bus_wait_time__);
	proto.set_bus_velocity(setting_.bus_velocity__);

	for (auto& [edge, item] : items_) {
		TCProto::Item& proto_item = *proto.add_items();
		proto_item.set_edge_id(edge);
		proto_item.set_edge_id(item.index_bus__);
		proto_item.set_edge_id(item.span_count__);
	}
}

void BusGraph::Deserialize(const TCProto::BusGraph & proto) {
	setting_ = { proto.bus_wait_time(), proto.bus_velocity() };

    for (const auto& proto_item : proto.items()) {
        Item item = { proto_item.index_bus(), proto_item.span_count() };
        EdgeId edge = proto_item.edge_id();
        items_.insert(std::make_pair(edge, item));
    }
}

////////////// class BusRouter
BusRouter::BusRouter(const BusGraph& bus_graph)
	:BaseRouter(bus_graph.GetGraph())
	, bus_graph_(bus_graph)
{}

const Edge<double>& BusRouter::GetEdge(EdgeId edge_id) const {
	return bus_graph_.GetEdge(edge_id);
}

const std::string BusRouter::GetNameBus(EdgeId edge_id) const {
	return bus_graph_.GetNameBus(edge_id);
}

int BusRouter::GetSpanCount(EdgeId edge_id) const {
	return bus_graph_.GetSpanCount(edge_id);
}

void BusRouter::Serialize(TCProto::BusGraph& proto) {
	bus_graph_.Serialize(proto);
}

void BusRouter::Deserialize(TCProto::BusGraph& proto) {
	bus_graph_.Deserialize(proto);

}