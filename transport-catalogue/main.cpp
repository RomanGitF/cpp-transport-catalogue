#include <fstream>
#include <sstream>
#include <iostream>
#include <string_view>
#include <string>

#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

#include "map_renderer.h"

using namespace std::literals;

std::string ReadFile(const std::string& file_name) {
	std::ifstream file(file_name, std::ios::binary | std::ios::ate);
	const std::ifstream::pos_type end = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string str(end, '\0');
	file.read(&str[0], end);
	return str;
}

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        std::ifstream fin("s14_3_opentest_3_make_base.json");
        JSONReader input(std::fin);
        transport::Catalogue cat;
        cat.CatalogRequest(input);
        MapRenderer rend(input.GetRenderSetting());
        RequestHandler handler(cat, rend);  

        const std::string& file_name = input.GetOutFile();
        std::ofstream file(file_name);
        file << cat.Serialize();
        // make base here

    }
    else if (mode == "process_requests"sv) {

       //std::string file_name = input.GetOutFile();
        std::ifstream fin("s14_3_opentest_3_process_requests.json");
        JSONReader input(std::fin);
        transport::Catalogue cat;

        cat.Deserialize(ReadFile(input.GetOutFile()));
        MapRenderer rend(input.GetRenderSetting());
        RequestHandler handler(cat, rend);
        
        std::ofstream fout;
        fout.open("test.txt");
        
        handler.GetStat(input.GetRequestStat(), std::fout);

    }
    else {
        PrintUsage();
        return 1;
    }
}