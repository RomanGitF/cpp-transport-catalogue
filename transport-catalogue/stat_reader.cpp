#include "stat_reader.h"

void GetInfo(transport::Catalogue& catalogue, std::istream& input) {
	std::string line;
	std::getline(input, line);
	int count = stoi(line);
	for (int i = 0; i < count; ++i) {
		std::getline(input, line);
		if (line[0] == 'B') {
			line = line.substr(4);
				catalogue.GetBusInfo(line);
		} else if (line[0] == 'S') {
			line = line.substr(5);
				catalogue.GetStopInfo(line);
			}
	}
}