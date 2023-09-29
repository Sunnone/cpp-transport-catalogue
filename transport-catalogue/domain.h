#pragma once

#include <algorithm>
#include <vector>
#include <string>

#include "geo.h"

namespace domain {

    struct StatRequest {
        int id;
        std::string type;
        std::string name;
    };

    struct Bus;

    struct Stop {
        std::string name;
        double latitude;
        double longitude;

        std::vector<Bus*> buses;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        bool is_roundtrip;
        size_t route_length;
    };

    struct Distance {
        const Stop* start;
        const Stop* end;
        int distance;
    };

    struct BusQueryResult {
        std::string_view name;
        bool not_found;
        int stops_on_route;
        int unique_stops;
        int route_length;
        double curvature;
    };

    struct StopQueryResult {
        std::string_view name;
        bool not_found;
        std::vector <std::string> buses_name;
    };

}//end namespace domain
