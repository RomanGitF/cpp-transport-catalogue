#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"

#include <iostream>
#include <set>
#include <stdexcept>
#include <algorithm>

using namespace transport;
using namespace domain;

void Catalogue::AddStop(std::string name, geo::Coordinates& coordinate) {
	Stop result(name, coordinate.lat, coordinate.lng);
	result.index__ = stops_.size();
	stops_.push_back(result);
	std::string_view tmp = stops_.back().name__;
	all_stops_.insert(std::make_pair(tmp, &stops_.back()));
	
}

void Catalogue::AddStopRouteLength(std::string name, std::list<std::tuple<std::string, int>> info) {
	auto& stop = *all_stops_.at(name);
	for (auto& [name_, length] : info) {
		auto& stop_ = *all_stops_.at(name_);
		stop.route_length__[stop_.name__] = length;
	}
}

void Catalogue::AddBus(std::string name, std::list<std::string> stops, bool is_round) {
	std::set<std::string> uniq_stop(stops.cbegin(), stops.cend());
	std::vector<Stop*> stops__;
	stops__.reserve(stops.size());
	size_t size_vector = 0;
	for (auto& it : stops) {
		stops__.push_back(all_stops_.at(it));
		++size_vector;
	}
	stops__.resize(size_vector);
	int route_length = CountRouteLength(stops__);
	if (!is_round) {
		std::vector<Stop*> revers_stops(stops__.rbegin(), stops__.rend());
		route_length += CountRouteLength(revers_stops);
	}
	size_t index = buses_.size();
	Bus tmp(name, stops__, uniq_stop.size(), route_length, is_round, index);
	CountLength(tmp);
	if (!is_round) { tmp.length__ *= 2; }
	buses_.push_back(std::move(tmp));
	std::string name_bus = buses_.back().name__;
	AddCrossBusesToStop(buses_.back());
	all_buses_.insert(std::make_pair(name_bus, &buses_.back()));
}

int Catalogue::CountRouteLength(const std::vector<Stop*>& stops) const {  
	int result = 0;
	for (size_t i = 0, j = 1; j < stops.size(); ++i, ++j) {
		if (stops[i]->route_length__.count(stops[j]->name__)) {
			result += stops[i]->route_length__.at(stops[j]->name__);
		}
		else {
			result += stops[j]->route_length__.at(stops[i]->name__);
		}
	}
	return result;
}

void Catalogue::CountLength(Bus& bus) {
	for (size_t i = 1; i < bus.stops__.size(); ++i) {
		bus.length__ += geo::ComputeDistance(bus.stops__[i - 1]->coordinates__, bus.stops__[i]->coordinates__);
	}
}

void Catalogue::AddCrossBusesToStop(const Bus& bus) {
	for (Stop* stop : bus.stops__) {
		std::string tmp = bus.name__;
		stop->cross_buses__.insert(tmp);
	}
}

const std::optional<Stop*> Catalogue::GetStopInfo(std::string name) {
	if (all_stops_.count(name)) {
		return all_stops_.at(name);
	}
	else {
		return std::nullopt;
	}
}

const std::optional<Bus*> Catalogue::GetBusInfo(std::string name) {
	if (all_buses_.count(name)) {
		return all_buses_.at(name);
	}
	else {
		return std::nullopt;
	}
}

const std::map<std::string, Stop*>& Catalogue::GetAllStops() const {
	return all_stops_;
}

const std::map<std::string, Bus*>& Catalogue::GetAllBuses() const {
	return all_buses_;
}

const domain::Setting Catalogue::GetSetting() const {
	return setting_;
}

const std::deque<domain::Bus>& Catalogue::GetDequeBuses() const {
	return buses_;
}

const std::deque<domain::Stop>& Catalogue::GetDequeStops() const {
	return stops_;
}

size_t Catalogue::GetCountStops() const {
	return stops_.size();
}

void Catalogue::SetSetting(size_t bus_velocity, size_t bus_wait_time) {
	setting_.bus_velocity__ = bus_velocity;
	setting_.bus_wait_time__ = bus_wait_time;
}

std::string Catalogue::Serialize() {

	TCProto::TransportCatalogue db_proto;

	for (const auto& stop : stops_) {
		TCProto::Stop& proto_stop = *db_proto.add_map_stops();
		proto_stop.set_name(std::string(stop.name__));
		proto_stop.set_lat(stop.coordinates__.lat);
		proto_stop.set_lng(stop.coordinates__.lng);
        proto_stop.set_index(stop.index__);

		for (const std::string_view bus_name : stop.cross_buses__) {
			proto_stop.add_bus_name(std::string(bus_name));
		}

		for (const auto& [name, len] : stop.route_length__) {
			auto& road = *proto_stop.add_road_distanse();
			road.set_name(std::string(name));
			road.set_len(len);
		}
	}


	for (const auto& bus : buses_) {
		TCProto::Bus& proto_bus = *db_proto.add_map_buses();
		proto_bus.set_name(std::string(bus.name__));
		proto_bus.set_is_roundtrip(bus.is_roundtrip__);
		proto_bus.set_index(bus.index__);
		proto_bus.set_unique_stop_count(bus.unique_stops__);
		proto_bus.set_route_length(bus.route_length__);
		proto_bus.set_length(bus.length__);

		for (const domain::Stop* stop : bus.stops__) {
			proto_bus.add_stops_name(std::string(stop->name__));
		}
	}

	return db_proto.SerializeAsString();
}


void Catalogue::Deserialize(const std::string& data) {
	TCProto::TransportCatalogue proto;
	assert(proto.ParseFromString(data));

	for (const TCProto::Stop& proto_stop : proto.map_stops()) {
		domain::Stop stop;
		stop.name__ = proto_stop.name();
		stop.coordinates__ = { proto_stop.lat(), proto_stop.lng() };

		for (const auto& name : proto_stop.bus_name()) {
			stop.cross_buses__.insert(name);
		}

		for (const auto& road : proto_stop.road_distanse()) {
			stop.route_length__[road.name()] = road.len();
		}
        stop.index__ = proto_stop.index();
		stops_.push_back(std::move(stop));
		all_stops_.insert(std::make_pair(stops_.back().name__, &stops_.back()));
	}
        

	for (const TCProto::Bus& proto_bus : proto.map_buses()) {
		Bus bus;

		bus.name__ = proto_bus.name();
		bus.is_roundtrip__ = proto_bus.is_roundtrip();
		bus.index__ = proto_bus.index();
		bus.unique_stops__ = proto_bus.unique_stop_count();
		bus.route_length__ = proto_bus.route_length();
		bus.length__ = proto_bus.length();

        size_t size = proto_bus.stops_name_size();
        bus.stops__.reserve(size);
        
        buses_.push_back(std::move(bus));
        all_buses_.insert(std::make_pair(buses_.back().name__, &buses_.back()));    
        
		for (const auto& name : proto_bus.stops_name()){ 
            buses_.back().stops__.push_back(all_stops_.at(name));  ////////////////////////////
		}


	}
}