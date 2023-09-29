#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

using namespace std;
using namespace transport_catalogue;
using namespace map_renderer;
using namespace request_handler;
using namespace transport_catalogue::detail::json;

int main() {
    vector<StatRequest> stat_request;
    RenderSettings render_settings;
    TransportCatalogue catalogue;

    JSONReader json_reader;
    RequestHandler request_handler;

    json_reader = JSONReader(cin);
    json_reader.parse(catalogue, stat_request, render_settings);

    request_handler = RequestHandler();
    request_handler.execute_queries(catalogue, stat_request, render_settings);
    transport_catalogue::detail::json::print(request_handler.get_document(), cout);
}