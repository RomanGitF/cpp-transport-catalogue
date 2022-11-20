#pragma once

#include <string_view> 
#include <deque>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <list>

#include "geo.h"
#include "domain.h"

namespace transport {

	class Catalogue {
		std::deque<domain::Stop> stops_;
		std::map<std::string_view, domain::Stop*> all_stops_;

		std::deque<domain::Bus> buses_;
		std::map<std::string_view, domain::Bus*> all_buses_;

		void AddStop(std::string_view name, geo::Coordinates& coordinate);
		void AddStopRouteLength(std::string_view name, std::list<std::tuple<std::string_view, int>> info);

		void AddBus(std::string_view name, std::list<std::string_view> stops, bool is_round);
		int CountRouteLength(const std::vector<domain::Stop*>& stops) const;
		void AddCrossBusesToStop(const domain::Bus& bus);
		void CountLength(domain::Bus& bus);

	public:
		template <typename InputReader>
		void CatalogRequest(InputReader& request);

		const std::optional<domain::Stop*> GetStopInfo(std::string_view name);
		const std::optional<domain::Bus*> GetBusInfo(std::string_view name);

		const std::map<std::string_view, domain::Stop*>& GetAllStops() const;		
		const std::map<std::string_view, domain::Bus*>& GetAllBuses() const;

	};

}  // namespace transport

template <typename InputReader>
void transport::Catalogue::CatalogRequest(InputReader& request) {
	auto stops = request.GetRequestStops();
	for (auto& stop : stops) {
		AddStop(stop.GetName(), stop.GetCoordinates());
	}
	for (auto& stop : stops) {
		AddStopRouteLength(stop.GetName(), stop.GetRouteLengthStop());
	}
	auto buses = request.GetRequestBuses();
	for (auto& [name, list_stops, is_round] : buses) {
		AddBus(name, list_stops, is_round);
	}

}