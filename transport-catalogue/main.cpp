#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>

using namespace std;
using namespace transport_catalogue;
using namespace input;
using namespace query;

int main() {
    TransportCatalogue tc;
    InputReader ir;
    ir.ParseInput(std::cin);
    ir.Load(tc);
    QueryReader sr;
    sr.ParseQuery();
    sr.Load(tc, std::cout);

    return 0;
}