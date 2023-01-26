#pragma once

#include <string>
#include <list>
#include <iostream>
#include <optional>

#include "geo.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"


class StopRequest {
    std::string_view name_;
    geo::Coordinates coordinates_;
    std::list<std::tuple<std::string_view, int>> route_length_stops_ = {};

public:
    StopRequest(std::string_view name, geo::Coordinates coordinates, std::list<std::tuple<std::string_view, int>> length);

    std::string_view GetName();
    geo::Coordinates& GetCoordinates();
    std::list<std::tuple<std::string_view, int>> GetRouteLengthStop();
};

struct StatRequest {
    StatRequest(int id, std::string_view type, std::string_view name);

    int id_;
    std::string_view type_;
    std::string_view name_;
};

class RequestHandler {
    transport::Catalogue& catalogue_;
    MapRenderer& render_;

    json::Array StatHandler(std::list<const json::Dict*> requests);
    json::Node GetStopStat(const json::Dict* request);
    json::Node GetBusStat(const json::Dict* request);
    json::Node GetMap(int id);

public:
    RequestHandler(transport::Catalogue& catalogue, MapRenderer& render);
    void GetStat(std::list<const json::Dict*> requests, std::ostream& out);
};