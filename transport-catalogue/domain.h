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
	};

	struct Bus {
		Bus(std::string_view name, const std::vector<Stop*>& stopes, size_t CountUniqueStops, int route_length, bool is_round);

		int route_length__ = 0;
		double length__ = 0.0;
		size_t unique_stops__;
		bool is_roundtrip__;
		std::string_view name__;
		std::vector<Stop*> stops__;
	};

} // namespace domain