#pragma once

#include "input_reader.h"
#include "transport_catalogue.h"

#include <iostream>

namespace info {
	void Get(transport::Catalogue& catalogue, std::istream& input);

	void PrintStop(const Stop& stop);
	void PrintBus(const Bus& bus);
}