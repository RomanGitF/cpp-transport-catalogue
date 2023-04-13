#include "request_handler.h"
#include "json_builder.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace domain;
using namespace transport;

StopRequest::StopRequest(std::string_view name, geo::Coordinates coordinates, std::list<std::tuple<std::string_view, int>> length) {
    name_ = name;
    coordinates_ = std::move(coordinates);
    route_length_stops_ = length;
}

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
    render_(render),
    graph_(catalogue),
    router_(graph_)
{
}

json::Array RequestHandler::StatHandler(std::list<const json::Dict*> requests) {
    json::Array nodes;
    for (const auto& stat : requests) {
        if (stat->at("type").AsString() == "Bus") {
            nodes.push_back(GetBusStat(stat));
        }
        if (stat->at("type").AsString() == "Stop") {
            nodes.push_back(GetStopStat(stat));
        }
        if (stat->at("type").AsString() == "Route") {
            nodes.push_back(GetRoute(stat));
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

json::Node RequestHandler::GetRoute(const json::Dict* request) {

    auto [from, to] = FindIndexStops(request->at("from").AsString(), request->at("to").AsString());
    std::optional<graph::Router<double>::RouteInfo> route = router_.BuildRoute(from, to);

    if (route) {
        auto& info = route.value();
        json::Node items = GetArrayItems(info.edges);
        json::Node dict_node = json::Builder{}
            .StartDict()
            .Key("total_time"s).Value(info.weight)
            .Key("request_id"s).Value(request->at("id").AsInt())
            .Key("items"s).Value(items)
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

json::Node RequestHandler::GetArrayItems(const std::vector<graph::EdgeId> edges) {
    const auto& stops = catalogue_.GetDequeStops();
    json::Array items;

    for (auto edge : edges) {
        auto it = router_.GetEdge(edge);
        if (it.weight == 0)
        {
            return items;
        }
        string_view stop = stops.at(it.from).name__;
        int time_for_wait = catalogue_.GetSetting().bus_wait_time__;
        json::Node item_wait = json::Builder{}
            .StartDict()
            .Key("stop_name"s).Value(string(stop))
            .Key("time"s).Value(double(time_for_wait))
            .Key("type"s).Value("Wait"s)
            .EndDict()
            .Build();

        double time_for_bus = it.weight - time_for_wait;
        string_view bus = router_.GetNameBus(it.from);
        int span_count = router_.GetSpanCount(it.from);
        json::Node item_bus = json::Builder{}
            .StartDict()
            .Key("bus"s).Value(string(bus))
            .Key("span_count").Value(span_count)
            .Key("time").Value(double(time_for_bus))
            .Key("type").Value("Bus"s)
            .EndDict()
            .Build();
        items.push_back(item_wait);
        items.push_back(item_bus);
    }
    return items;
}

std::pair<size_t, size_t> RequestHandler::FindIndexStops(std::string_view from, string_view to) {
    size_t index_from = catalogue_.GetAllStops().at(from)->index__;
    size_t index_to = catalogue_.GetAllStops().at(to)->index__;
    return make_pair(index_from, index_to);
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
