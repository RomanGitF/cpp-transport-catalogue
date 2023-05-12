#pragma once

#include "json.h"
#include "request_handler.h"

#include <iostream>
#include <string>

#include <list>
#include <memory> 

class JSONReader {
	using Request_Buses = std::list<std::tuple<std::string, std::list<std::string>, bool>>;

	json::Document input_;

	std::list<const json::Dict*> stops_;
	std::list<const json::Dict*> buses_;
	std::list<const json::Dict*> stats_;
	std::unique_ptr<json::Dict> settings_;  
	std::unique_ptr<json::Dict> serialization_settings_;     

	void Load();

public:

	JSONReader(std::istream& input);

	std::list<StopRequest> GetRequestStops();
	Request_Buses GetRequestBuses();
	std::list<const json::Dict*>& GetRequestStat();

	json::Dict GetRenderSetting();
    

	size_t GetSettingsBusVelocity();
	size_t GetSettingsBusWaitTime();
    const std::string& GetOutFile() const;
};