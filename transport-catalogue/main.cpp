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
    std::istream& input(cin);
    ir.ParseInput(input, tc);
    QueryReader sr;
    std::ostream& out(cout);
    sr.ParseQuery(tc, out);

    return 0;
}