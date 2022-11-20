#include "transport_catalogue.h"

#include <iostream>
#include <set>
#include <stdexcept>
#include <algorithm>

using namespace transport;
using namespace domain;

void Catalogue::AddStop(std::string_view name, geo::Coordinates& coordinate) {
	Stop result(name, coordinate.lat, coordinate.lng);
	stops_.push_back(result);
	std::string_view tmp = stops_.back().name__;
	all_stops_.insert(std::make_pair(tmp, &stops_.back()));
}

void Catalogue::AddStopRouteLength(std::string_view name, std::list<std::tuple<std::string_view, int>> info) {
	auto& stop = *all_stops_.at(name);
	for (auto& [name_, length] : info) {
		auto& stop_ = *all_stops_.at(name_);
		stop.route_length__[stop_.name__] = length;
	}
}

void Catalogue::AddBus(std::string_view name, std::list<std::string_view> stops, bool is_round) {
	std::set<std::string_view> uniq_stop(stops.cbegin(), stops.cend());
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
	Bus tmp(name, stops__, uniq_stop.size(), route_length, is_round);
	CountLength(tmp);
	if (!is_round) { tmp.length__ *= 2; }
	buses_.push_back(std::move(tmp));
	std::string_view name_bus = buses_.back().name__;
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
		std::string_view tmp = bus.name__;
		stop->cross_buses__.insert(tmp);
	}
}

const std::optional<Stop*> Catalogue::GetStopInfo(std::string_view name) {
	if (all_stops_.count(name)) {
		return all_stops_.at(name);
	}
	else {
		return std::nullopt;
	}
}

const std::optional<Bus*> Catalogue::GetBusInfo(std::string_view name) {
	if (all_buses_.count(name)) {
		return all_buses_.at(name);
	}
	else {
		return std::nullopt;
	}
}

const std::map<std::string_view, Stop*>& Catalogue::GetAllStops() const {
	return all_stops_;
}

const std::map<std::string_view, Bus*>& Catalogue::GetAllBuses() const {
	return all_buses_;
}