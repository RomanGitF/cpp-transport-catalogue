#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {

	InputReader request;
	request.ReadRequest(std::cin);

	transport::Catalogue tmp;
	tmp.CatalogRequest(request);

	GetInfo(tmp, std::cin);

}