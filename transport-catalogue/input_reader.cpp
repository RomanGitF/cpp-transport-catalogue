#include "input_reader.h"

StopRequest::StopRequest(std::string_view name, Coordinates coordinates, std::list<std::tuple<std::string, int>> length) {
    name_ = move(name);
    coordinates_ = std::move(coordinates);
    route_length_stops_ = std::move(length);
}

std::string* StopRequest::GetName() {
    return &name_;
}

Coordinates* StopRequest::GetCoordinates() {
    return &coordinates_;
}

std::list<std::tuple<std::string, int>>* StopRequest::GetRouteLengthStop() {
    return &route_length_stops_;
}

void InputReader::ReadRequest(std::istream& input) {
    int CountInputReader = detail::ReadLineWithNumber(input);
    for (int i = 0; i < CountInputReader; ++i) {
        AddInputRequest(input);
    }
}

void InputReader::AddInputRequest(std::istream& input) {
    std::string line;
    std::getline(input, line);
    size_t pos = 0;
    pos = line.find_first_not_of(' ', 0);
    if (line[pos] == 'B') {
        AddBus(line);
    }
    else if (line[pos] == 'S') {
        AddStop(line);
    }
    else {
        std::cout << "invalid request" << std::endl;
    }
}

void InputReader::AddBus(std::string& line) {
    size_t pos = line.find_first_not_of(' ', 0) + 4;
    line = line.substr(pos);
    auto [bus, stops] = detail::Split(detail::Lstrip(line), ':');
    std::list <std::string> list_stops;
    if (stops.find('>') != stops.npos) {
        while (!stops.empty()) {
            auto [stop, tail] = detail::Split(stops, '>');
            list_stops.push_back(std::string(detail::DeleteSpace(stop)));
            stops = tail;
        }
    }
    else {
        std::list <std::string> list_stops_return;
        while (!stops.empty()) {
            auto [stop, tail] = detail::Split(stops, '-');
            list_stops.push_back(std::string(detail::DeleteSpace(stop)));
            list_stops_return.push_front(std::string(detail::DeleteSpace(stop)));
            stops = tail;
        }
        list_stops_return.pop_front();
        list_stops.splice(list_stops.end(), list_stops_return);
    }
    buses_request_.push_front(std::make_tuple(move(std::string(detail::DeleteSpace(bus))), move(list_stops)));
}

void InputReader::AddStop(std::string& line) {
    size_t pos = line.find_first_not_of(' ', 0) + 5;
    line = line.substr(pos);
    auto [stop, geo] = detail::Split(detail::DeleteSpace(line), ':');
    stop = detail::DeleteSpace(stop);
    auto [lat_sv, tmp_tail] = detail::Split(geo, ',');
    auto [lng_sv, route_lengths] = detail::Split(tmp_tail, ',');
    double lat = std::stod(std::string(lat_sv));
    double lng = std::stod(std::string(lng_sv));
    Coordinates coordinates(lat, lng);

    std::list<std::tuple<std::string, int>> route_length;
    while (!route_lengths.empty()) {
        auto [length, tail] = detail::Split(route_lengths, ',');
        pos = length.find_first_of('m', 0);
        auto tmp_length = length.substr(0, pos);
        int lnght = std::stoi(std::string(tmp_length));
        pos = length.find_first_of(' ', pos + 2);
        length = length.substr(pos + 1);
        route_length.push_front({ move(std::string(length)), lnght });
        route_lengths = tail;
    }
    stops_request_.push_front({ std::string(stop), coordinates,  route_length });
}

std::list<StopRequest>* InputReader::GetRequestStops() {
    return &stops_request_;
}

std::list<std::tuple<std::string, std::list<std::string>>>* InputReader::GetRequestBuses() {
    return &buses_request_;
}

std::pair<std::string_view, std::string_view> detail::Split(std::string_view line, char by) {
    size_t pos = line.find(by);
    std::string_view left = line.substr(0, pos);

    if (pos < line.size() && pos + 1 < line.size()) {
        return { left, line.substr(pos + 1) };
    }
    else {
        return { left, std::string_view() };
    }
}

std::string_view detail::Lstrip(std::string_view line) {
    while (!line.empty() && isspace(line[0])) {
        line.remove_prefix(1);
    }
    return line;
}

std::string_view detail::DeleteSpace(std::string_view line) {
    while (isspace(line[0])) {
        line.remove_prefix(1);
    }
    while (line.back() == ' ') {
        line.remove_suffix(1);
    }
    return line;
}

int detail::ReadLineWithNumber(std::istream& input) {
    std::string line;
    std::getline(input, line);
    int result = stoi(line);
    return result;
}