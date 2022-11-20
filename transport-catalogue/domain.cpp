#include "domain.h"

using namespace domain;

Stop::Stop(std::string_view name, double lat, double lng) {
	name__ = name;
	coordinates__.lat = lat;
	coordinates__.lng = lng;
}

Bus::Bus(std::string_view name, const std::vector<Stop*>& stopes, size_t count_unique_stops, int route_length, bool is_round) {
	route_length__ = route_length;
	unique_stops__ = count_unique_stops;
	name__ = name;
	is_roundtrip__ = is_round;
	stops__.reserve(stopes.size());
	for (const auto& it : stopes) {
		stops__.push_back(it);
	}
}