#include "stat_reader.h"

void info::Get(transport::Catalogue& catalogue, std::istream& input, std::ostream& output) {
	std::string line;
	std::getline(input, line);
	int count = stoi(line);
	for (int i = 0; i < count; ++i) {
		std::getline(input, line);
		if (line[0] == 'B') {
			line = line.substr(4);
			try {
				const Bus& bus = catalogue.GetBusInfo(line);
				info::PrintBus(bus, output);
			}
			catch (std::out_of_range const& exc) {
				output << "Bus " << line << ": not found" << std::endl;
			}
		}	
		else if (line[0] == 'S') {
			line = line.substr(5);
			try {
				const Stop& stop = catalogue.GetStopInfo(line);
				info::PrintStop(stop, output);
			}
			catch (std::out_of_range const& exc) {
				output << "Stop " << line << ": not found" << std::endl;
			}
		}
	}
}

void info::PrintStop(const Stop& stop, std::ostream& output) {
	output << "Stop " << stop.name__ << ": ";
	if (stop.cross_buses__.empty()) {
		output << "no buses";
	}
	else {
		output << "buses ";
		for (const auto& it : stop.cross_buses__) {
			output << it << ' ';
		}
	}
	output << std::endl;
}

void info::PrintBus(const Bus& bus, std::ostream& output){
	output << "Bus " << bus.name__ << ": " << bus.stops__.size() << " stops on route, "
		<< bus.unique_stops__ << " unique stops, "
		<< bus.route_length__ << " route length, "
		<< bus.route_length__ / bus.length__ << " curvature";
	output << std::endl;
}