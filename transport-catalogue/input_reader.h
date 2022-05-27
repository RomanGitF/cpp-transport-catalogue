#pragma once

#include <iostream>
#include <string>
#include <list>
#include <unordered_map>

#include "transport_catalogue.h"
#include "geo.h"

class StopRequest {
	std::string name_;
	Coordinates coordinates_;
	std::list<std::tuple<std::string, int>> route_length_stops_ = {};

public:
	StopRequest(std::string_view name, Coordinates coordinates, std::list<std::tuple<std::string, int>> length);

	std::string* GetName();
	Coordinates* GetCoordinates();
	std::list<std::tuple<std::string, int>>* GetRouteLengthStop();
};

class InputReader {
	std::list<std::tuple<std::string, std::list<std::string>>> buses_request_;
	std::list<StopRequest> stops_request_;

	void AddInputRequest(std::istream& input);
	void AddBus(std::string& line);
	void AddStop(std::string& line);
	
public:
	void ReadRequest(std::istream& input);

	std::list<StopRequest>* GetRequestStops();
	std::list<std::tuple<std::string, std::list<std::string>>>* GetRequestBuses();
};

namespace detail {
	std::pair<std::string_view, std::string_view> Split(std::string_view line, char by);
	std::string_view Lstrip(std::string_view line);
	std::string_view DeleteSpace(std::string_view line);
	int ReadLineWithNumber(std::istream& input);
}

