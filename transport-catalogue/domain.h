#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include "geo.h"

namespace domain {

	struct Stop {
		Stop(std::string_view name, double lat, double lng);

		std::unordered_map<std::string_view, int> route_length__;
		std::set<std::string_view> cross_buses__;
		std::string_view name__;
		geo::Coordinates coordinates__;
		size_t index__;
	};

	struct Bus {
		Bus(std::string_view name, const std::vector<Stop*>& stopes, size_t CountUniqueStops, int route_length, bool is_round, size_t index);

		int route_length__ = 0;
		double length__ = 0.0;
		size_t unique_stops__;
		bool is_roundtrip__;
		std::string_view name__;
		std::vector<Stop*> stops__;
		size_t index__;

	};

	struct Setting {
		Setting();
		Setting(size_t bus_velocity, size_t bus_wait_time);

		size_t bus_velocity__ = 0;
		size_t bus_wait_time__ = 0;
	};

} // namespace domain