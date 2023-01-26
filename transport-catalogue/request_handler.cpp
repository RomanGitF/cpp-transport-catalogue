#include "request_handler.h"
#include "json_builder.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace domain;

StopRequest::StopRequest(std::string_view name, geo::Coordinates coordinates, std::list<std::tuple<std::string_view, int>> length) {
    name_ = name;
    coordinates_ = std::move(coordinates);
    route_length_stops_ = length;
}

StatRequest::StatRequest(int id, std::string_view type, std::string_view name)
    :id_(id),
    type_(type),
    name_(name) {}


std::string_view StopRequest::GetName() {
    return name_;
}

geo::Coordinates& StopRequest::GetCoordinates() {
    return coordinates_;
}

std::list<std::tuple<std::string_view, int>> StopRequest::GetRouteLengthStop() {
    return route_length_stops_;
}

/////////////////////  RequestHandler
RequestHandler::RequestHandler(transport::Catalogue& catalogue, MapRenderer& render)
    :catalogue_(catalogue),
    render_(render)
{}

json::Array RequestHandler::StatHandler(std::list<const json::Dict*> requests) {
    json::Array nodes;
    for (const auto& stat : requests) {
        if (stat->at("type").AsString() == "Bus") {
            nodes.push_back(GetBusStat(stat));
        }
        if (stat->at("type").AsString() == "Stop") {
            nodes.push_back(GetStopStat(stat));
        }
        if (stat->at("type").AsString() == "Map") {
            nodes.push_back(GetMap(stat->at("id").AsInt()));
        }
    }
    return nodes;
}

json::Node RequestHandler::GetMap(int id) {
    using namespace std::literals;
    std::ostringstream sout;
    render_.Draw(catalogue_, sout);
    json::Node dict_node = json::Builder{}
        .StartDict()
        .Key("map"s).Value(sout.str())
        .Key("request_id"s).Value(id)
        .EndDict()
        .Build();
        return dict_node;
}

json::Node RequestHandler::GetStopStat(const json::Dict* request) {
    using namespace std::literals;
    std::optional<Stop*> stop = catalogue_.GetStopInfo(request->at("name"s).AsString());
    if (stop) {
        auto& info = stop.value();
        json::Array buses;
        buses.reserve(info->cross_buses__.size());
        for (const auto& it : info->cross_buses__) {
            std::string s = it.data();
            buses.emplace_back(s);
        }
        json::Node dict_node = json::Builder{}
            .StartDict()
            .Key("buses"s).Value(buses)
            .Key("request_id"s).Value(request->at("id").AsInt())
            .EndDict()
            .Build();

        return dict_node;
    }
    else {
        json::Node dict_node = json::Builder{}
            .StartDict()
            .Key("error_message"s).Value("not found"s)
            .Key("request_id"s).Value(request->at("id"s).AsInt())
            .EndDict()
            .Build();

        return dict_node;
    }
}

json::Node RequestHandler::GetBusStat(const json::Dict* request) {
    using namespace std::literals;
    std::optional<Bus*> bus = catalogue_.GetBusInfo(request->at("name").AsString());
    if (bus) {
        auto& info = bus.value();
        int n = info->stops__.size();
        if (!info->is_roundtrip__ && n>1) {
            n += (n - 1);
        }
        json::Node dict_node = json::Builder{}
            .StartDict()
            .Key("curvature"s).Value(info->route_length__ / info->length__)
            .Key("route_length"s).Value(info->route_length__)
            .Key("stop_count"s).Value(n)
            .Key("unique_stop_count"s).Value(static_cast<int>(info->unique_stops__))
            .Key("request_id"s).Value(request->at("id").AsInt())
            .EndDict()
            .Build();
        return dict_node;
    }
    else {
        json::Node dict_node = json::Builder{}
            .StartDict()
            .Key("error_message"s).Value("not found"s)
            .Key("request_id"s).Value(request->at("id").AsInt())
            .EndDict()
            .Build();
        return dict_node;
    }
}

void RequestHandler::GetStat(std::list<const json::Dict*> requests, std::ostream& out) {
    json::Document stats(StatHandler(requests));
    json::Print(stats, out);
}
