#include "transport_catalogue.h"

#include <iostream>
#include <set>
#include <stdexcept>

using namespace transport;

Stop::Stop(std::string& name, double lat, double lng) {
	name__ = name;
	coordinates__.lat = lat;
	coordinates__.lng = lng;
}

Bus::Bus(std::string& name, const std::vector<Stop*>& stopes, size_t count_unique_stops, int route_length) {
	route_length__ = route_length;
	unique_stops__ = count_unique_stops;
	name__ = std::move(name);
	stops__.reserve(stopes.size());
	for (const auto& it : stopes) {
		stops__.push_back(it);
	}
}

void Catalogue::AddStop(std::string& name, Coordinates& coordinate) {
	Stop result(name, coordinate.lat, coordinate.lng);
	stops_.push_back(result);
	std::string_view tmp = stops_.back().name__;
	all_stops_.insert(std::make_pair(tmp, &stops_.back()));
}

void Catalogue::AddStopRouteLength(std::string_view name, std::list<std::tuple<std::string, int>>& info) {
	auto& stop = *all_stops_.at(name);
	for (auto& [name_, length] : info) {
		auto& stop_ = *all_stops_.at(name_);
		stop.route_length__[stop_.name__] = length;
	}
}

void Catalogue::AddBus(std::string& name, std::list<std::string>& stops) {
	std::set<std::string_view> uniq_stop(stops.cbegin(), stops.cend());
	std::vector<Stop*> stops__;
	stops__.reserve(stops.size());
	for (auto& it : stops) {
		stops__.push_back(all_stops_.at(it));
	}
	int route_length = CountRouteLength(stops__);
	Bus tmp(name, stops__, uniq_stop.size(), route_length);
	CountLength(tmp);
	buses_.push_back(std::move(tmp));
	std::string_view name_bus = buses_.back().name__;
	AddCrossBusesToStop(buses_.back());
	all_buses_.insert(std::make_pair(name_bus, &buses_.back()));
	}

int Catalogue::CountRouteLength(const std::vector<Stop*>& stops) const {
	int result = 0;
	for (size_t i = 1; i < stops.size(); ++i) {
		try {
			result += stops[i - 1]->route_length__.at(stops[i]->name__);
		}
		catch (std::out_of_range const& exc) {
			result += stops[i]->route_length__.at(stops[i - 1]->name__);
		}
	}
	return result;
}

void Catalogue::CountLength(Bus& bus) {
	for (size_t i = 1; i < bus.stops__.size(); ++i) {
		bus.length__ += ComputeDistance(bus.stops__[i - 1]->coordinates__, bus.stops__[i]->coordinates__);
	}
}

void Catalogue::AddCrossBusesToStop(const Bus& bus) {
	for (Stop* stop : bus.stops__) {
		std::string_view tmp = bus.name__;
		stop->cross_buses__.insert(tmp);
	}
}

const Stop& Catalogue::GetStopInfo(std::string_view name) {
	return *all_stops_.at(name);
}

const Bus& Catalogue::GetBusInfo(std::string_view name) {
	return *all_buses_.at(name);
}