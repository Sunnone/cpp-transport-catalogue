#pragma once 
 
#include "geo.h" 
 
#include <set> 
#include <string> 
#include <string_view> 
#include <unordered_map> 
#include <vector> 
 
namespace transport_catalogue { 
 
namespace domain { 
 
struct Stop { 
    std::string name; 
    geo::Coordinates coordinates; 
    std::set<std::string_view> buses_by_stop; 
}; 
using StopPtr = const Stop*; 
 
enum RouteType { 
    Straight, 
    Round 
}; 
 
struct Bus { 
    std::string name; 
    std::vector<StopPtr> stops; 
    RouteType type = Straight; 
}; 
     
using BusPtr = const Bus*; 
 
struct BusStat { 
    size_t stops_count; 
    size_t unique_stops_count; 
    double route_length; 
    double curvature; 
}; 
 
} // namespace domain 
} // namespace transport_catalogue