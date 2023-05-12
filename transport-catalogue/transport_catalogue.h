#pragma once

#include <string> 
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
		std::map<std::string, domain::Stop*> all_stops_;

		std::deque<domain::Bus> buses_;
		std::map<std::string, domain::Bus*> all_buses_;

		domain::Setting setting_;

		void AddStop(std::string name, geo::Coordinates& coordinate);
		void AddStopRouteLength(std::string name, std::list<std::tuple<std::string, int>> info);

		void AddBus(std::string name, std::list<std::string> stops, bool is_round);
		int CountRouteLength(const std::vector<domain::Stop*>& stops) const;
		void AddCrossBusesToStop(const domain::Bus& bus);
		void CountLength(domain::Bus& bus);

		void SetSetting(size_t bus_velocity, size_t bus_wait_time);

	public:
		template <typename InputReader>
		void CatalogRequest(InputReader& request);

		const std::optional<domain::Stop*> GetStopInfo(std::string name);
		const std::optional<domain::Bus*> GetBusInfo(std::string name);

		const std::map<std::string, domain::Stop*>& GetAllStops() const;		
		const std::map<std::string, domain::Bus*>& GetAllBuses() const;

		const domain::Setting GetSetting() const;
		const std::deque<domain::Bus>& GetDequeBuses() const;
		const std::deque<domain::Stop>& GetDequeStops() const;
		size_t GetCountStops() const;
        
        void Deserialize(const std::string& data);
        std::string Serialize();
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
	SetSetting(request.GetSettingsBusVelocity(), request.GetSettingsBusWaitTime());

}