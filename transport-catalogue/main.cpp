
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

#include "map_renderer.h"

int main() {

    //заменил несортированый словарь на обычный в каталоге
    JSON_Reader input(std::cin);
    transport::Catalogue cat;
    cat.CatalogRequest(input);
    MapRender rend(input.GetRenderSetting());
    RequestHandler handler(cat, rend);
    handler.GetStat(input.GetRequestStat(), std::cout);
    return 0;
}
