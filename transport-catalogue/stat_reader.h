#pragma once

#include "input_reader.h"
#include "transport_catalogue.h"

#include <iostream>

namespace info {
	void Get(transport::Catalogue& catalogue, std::istream& input, std::ostream& output);


	void PrintStop(const Stop& stop, std::ostream& output);
	void PrintBus(const Bus& bus, std::ostream& output);
}