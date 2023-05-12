#pragma once

#include <string>
#include <list>
#include <iostream>
#include <optional>

#include "geo.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "router.h"

class StopRequest {
    std::string name_;
    geo::Coordinates coordinates_;
    std::list<std::tuple<std::string, int>> route_length_stops_ = {};

public:
    StopRequest(std::string name, geo::Coordinates coordinates, std::list<std::tuple<std::string, int>> length);

    std::string GetName();
    geo::Coordinates& GetCoordinates();
    std::list<std::tuple<std::string, int>> GetRouteLengthStop();
};

class RequestHandler {

    transport::Catalogue& catalogue_;
    MapRenderer& render_;
    transport::BusGraph graph_;
    transport::BusRouter router_;

    json::Array StatHandler(std::list<const json::Dict*> requests);
    json::Node GetStopStat(const json::Dict* request);
    json::Node GetBusStat(const json::Dict* request);
    json::Node GetRoute(const json::Dict* request);

    json::Node GetMap(int id);

    std::pair<size_t, size_t> FindIndexStops(std::string from, std::string to);
    json::Node GetArrayItems(const std::vector<graph::EdgeId> edges);
public:
            //RequestHandler(transport::Catalogue& catalogue);
    RequestHandler(transport::Catalogue& catalogue, MapRenderer& render);
    void GetStat(std::list<const json::Dict*> requests, std::ostream& out);
};