#include "stat_reader.h"

void info::Get(transport::Catalogue& catalogue, std::istream& input) {
	std::string line;
	std::getline(input, line);
	int count = stoi(line);
	for (int i = 0; i < count; ++i) {
		std::getline(input, line);
		if (line[0] == 'B') {
			line = line.substr(4);
			try {
				const Bus& bus = catalogue.GetBusInfo(line);
				info::PrintBus(bus);
			}
			catch (std::out_of_range const& exc) {
				std::cout << "Bus " << line << ": not found" << std::endl;
			}
		}	
		else if (line[0] == 'S') {
			line = line.substr(5);
			try {
				const Stop& stop = catalogue.GetStopInfo(line);
				info::PrintStop(stop);
			}
			catch (std::out_of_range const& exc) {
				std::cout << "Stop " << line << ": not found" << std::endl;
			}
		}
	}
}

void info::PrintStop(const Stop& stop) {
	std::cout << "Stop " << stop.name__ << ": ";
	if (stop.cross_buses__.empty()) {
		std::cout << "no buses";
	}
	else {
		std::cout << "buses ";
		for (const auto& it : stop.cross_buses__) {
			std::cout << it << ' ';
		}
	}
	std::cout << std::endl;
}

void info::PrintBus(const Bus& bus){
	std::cout << "Bus " << bus.name__ << ": " << bus.stops__.size() << " stops on route, "
		<< bus.UniqueStops__ << " unique stops, "
		<< bus.route_length__ << " route length, "
		<< bus.route_length__ / bus.length__ << " curvature";
	std::cout << std::endl;
}