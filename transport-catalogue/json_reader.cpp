#include "json_reader.h"



JSONReader::JSONReader(std::istream& input) {
	input_ = json::Load(input);
	Load();
}

void JSONReader::Load() {
	const auto& requests = input_.GetRoot().AsDict();
	try {
		const auto& base = requests.at("base_requests").AsArray();
		for (const auto& it : base) {
			if (it.AsDict().at("type").AsString() == "Stop") {
				stops_.emplace_front(&it.AsDict());
			}
			else if (it.AsDict().at("type").AsString() == "Bus") {
				buses_.emplace_front(&it.AsDict());
			}
			else {
				std::cerr << "Error Node" << std::endl;
			}
		}
	}
	catch (std::out_of_range const& exc) {
		std::cerr << "Error Load Base" << std::endl;
	}
	try {
		const auto& stat = requests.at("stat_requests").AsArray();
		for (const auto& it : stat) {
			stats_.emplace_back(&it.AsDict());
		}
	}
	catch (std::out_of_range const& exc) {
		std::cerr << "Error Load Stat" << std::endl;
	}

	try { 
		settings_ = std::make_unique<json::Dict>(requests.at("routing_settings").AsDict());
	}
	catch (std::out_of_range const& exc) {
		std::cerr << "Error Load Settings" << std::endl;
	}

}

std::list<StopRequest> JSONReader::GetRequestStops() {
	std::list<StopRequest> stops_request;
	for (const auto& stop : stops_) {
		std::string_view name = stop->at("name").AsString();
		geo::Coordinates coordinates(stop->at("latitude").AsDouble(), stop->at("longitude").AsDouble());
		std::list<std::tuple<std::string_view, int>> length;
		for (const auto& other_stop : stop->at("road_distances").AsDict()) {
			length.emplace_front(other_stop.first, other_stop.second.AsInt());
		}
		stops_request.emplace_front(name, coordinates, length);
	}
	return stops_request;
}

JSONReader::Request_Buses JSONReader::GetRequestBuses() {
	Request_Buses buses_request;
	for (const auto& bus : buses_) {
		std::string_view name = bus->at("name").AsString();
		bool is_round = bus->at("is_roundtrip").AsBool();
		std::list<std::string_view> stops;
		std::list<std::string_view> revers_stops;
		for (const auto& stop : bus->at("stops").AsArray()) {
			stops.push_back(stop.AsString());
		}
		buses_request.emplace_front(make_tuple(name, stops, is_round));
	}
	return buses_request;
}

std::list<const json::Dict*>& JSONReader::GetRequestStat() {
	return stats_;
}

json::Dict JSONReader::GetRenderSetting() {
	return input_.GetRoot().AsDict().at("render_settings").AsDict();
}


size_t  JSONReader::GetSettingsBusVelocity() {
	return settings_.get()->at("bus_velocity").AsInt();
}

size_t  JSONReader::GetSettingsBusWaitTime() {
	return settings_.get()->at("bus_wait_time").AsInt();
}