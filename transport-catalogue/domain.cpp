#include "domain.h"

using namespace domain;

Stop::Stop(std::string name, double lat, double lng) {
	name__ = name;
	coordinates__.lat = lat;
	coordinates__.lng = lng;
}

Bus::Bus(std::string name, const std::vector<Stop*>& stopes, size_t count_unique_stops, int route_length, bool is_round, size_t index) {
	route_length__ = route_length;
	unique_stops__ = count_unique_stops;
	name__ = name;
	is_roundtrip__ = is_round;
	stops__.reserve(stopes.size());
	for (const auto& it : stopes) {
		stops__.push_back(it);
	}
	index__ = index;
}

Setting::Setting(size_t bus_velocity, size_t bus_wait_time)
	:bus_velocity__(bus_velocity)
	, bus_wait_time__(bus_wait_time)
{}

Setting::Setting()
	:bus_velocity__(0)
	, bus_wait_time__(0)
{}