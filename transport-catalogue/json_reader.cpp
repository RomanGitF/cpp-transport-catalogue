#include "json_reader.h"

JSON_Reader::JSON_Reader(std::istream& input) {
	input_ = json::Load(input);
	Load();
}

void JSON_Reader::Load() {
	const auto& requests = input_.GetRoot().AsMap();
	try {
		const auto& base = requests.at("base_requests").AsArray();
		for (const auto& it : base) {
			if (it.AsMap().at("type").AsString() == "Stop") {
				stops_.emplace_front(&it.AsMap());
			}
			else if (it.AsMap().at("type").AsString() == "Bus") {
				buses_.emplace_front(&it.AsMap());
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
			stats_.emplace_back(&it.AsMap());
		}
	}
	catch (std::out_of_range const& exc) {
		std::cerr << "Error Load Stat" << std::endl;
	}
}

std::list<StopRequest> JSON_Reader::GetRequestStops() {
	std::list<StopRequest> stops_request;
	for (const auto& stop : stops_) {
		std::string_view name = stop->at("name").AsString();
		geo::Coordinates coordinates(stop->at("latitude").AsDouble(), stop->at("longitude").AsDouble());
		std::list<std::tuple<std::string_view, int>> length;
		for (const auto& other_stop : stop->at("road_distances").AsMap()) {
			length.emplace_front(other_stop.first, other_stop.second.AsInt());
		}
		stops_request.emplace_front(name, coordinates, length);
	}
	return stops_request;
}

JSON_Reader::Request_Buses JSON_Reader::GetRequestBuses() {
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

std::list<const json::Dict*>& JSON_Reader::GetRequestStat() {
	return stats_;
}

json::Dict JSON_Reader::GetRenderSetting() {
	return input_.GetRoot().AsMap().at("render_settings").AsMap();
}