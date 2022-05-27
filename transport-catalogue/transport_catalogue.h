#pragma once

#include <string>
#include <string_view> 
#include <deque>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <iostream>

#include "geo.h"
#include "input_reader.h"

namespace transport {

	struct Stop {
		Stop(std::string& name, double lat, double lng);

		std::unordered_map<std::string_view, int> route_length__;
		std::set<std::string_view> cross_buses__;
		std::string name__;
		Coordinates coordinates__;
	};

	struct Bus {
		Bus(std::string& name, std::vector<Stop*> stopes, size_t CountUniqueStops, int route_length);

		int route_length__ = 0;
		double length__ = 0.0;
		size_t UniqueStops;
		std::string name__;
		std::vector<Stop*> stops__;
	};

	class Catalogue {

		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> all_stops_;

		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> all_buses_;
	
		void AddStop(std::string& name, Coordinates& coordinate);
		void AddStopRouteLength(std::string_view name, std::list<std::tuple<std::string, int>>& info);

		void AddBus(std::string& name, std::list<std::string>& stops);
		int CountRouteLength(const std::vector<Stop*>& stops) const;
		void AddCrossBusesToStop(const Bus& bus);
		void CountLenght(Bus& bus);

		void PrintStopInfo(const Stop& stop) const;
		void PrintBusInfo(const Bus& bus) const;

	public:

		template <typename InputReader>
		void CatalogRequest(InputReader& request);
		
		void GetStopInfo(std::string_view name);
		void GetBusInfo(std::string_view name);
	};
}

template <typename InputReader>
void transport::Catalogue::CatalogRequest(InputReader& request) {

	auto* stops = request.GetRequestStops();
	for (auto& stop : *stops) {
		AddStop(*stop.GetName(), *stop.GetCoordinates());

	}
	for (auto& stop : *stops) {
		AddStopRouteLength(*stop.GetName(), *stop.GetRouteLengthStop());
	}	
	stops->clear();
	auto* buses = request.GetRequestBuses();
	for (auto& [name, list_stops] : *buses) {
		AddBus(name, list_stops);
	}
	buses->clear();
}