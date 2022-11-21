
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

#include "map_renderer.h"


#include <fstream>


int main() {

    std::ifstream fin("s10_final_opentest_1.json");

    JSONReader input(fin);
    transport::Catalogue cat;
    cat.CatalogRequest(input);

    MapRender rend(input.GetRenderSetting());

    RequestHandler handler(cat, rend);

    std::ofstream fout;
    fout.open("test.txt");
    handler.GetStat(input.GetRequestStat(), fout);
    fout.close();

    return 0;
}