#pragma once

#include "json.h"
#include "request_handler.h"

#include <iostream>
#include <string_view>
#include <list>
//#include <memory>

class JSON_Reader {
	using Request_Buses = std::list<std::tuple<std::string_view, std::list<std::string_view>, bool>>;

	json::Document input_;

	std::list<const json::Dict*> stops_;
	std::list<const json::Dict*> buses_;
	std::list<const json::Dict*> stats_;

	void Load();

public:

	JSON_Reader(std::istream& input);

	std::list<StopRequest> GetRequestStops();
	Request_Buses GetRequestBuses();
	std::list<const json::Dict*>& GetRequestStat();
	json::Dict GetRenderSetting();
};

