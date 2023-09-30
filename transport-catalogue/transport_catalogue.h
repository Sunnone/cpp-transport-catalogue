#pragma once

#include "domain.h" 
#include "geo.h" 

#include <algorithm>
#include <deque> 
#include <iostream> 
#include <map> 
#include <optional> 
#include <set> 
#include <stdexcept> 
#include <string> 
#include <unordered_map> 
#include <unordered_set> 
#include <vector> 

namespace transport_catalogue {
    
using namespace domain;

class TransportCatalogue {
public:
    class StopHasher { 
    public: 
        size_t operator()(std::pair<StopPtr, StopPtr> stop) const { 
            return hasher_(stop.first) + 47 * hasher_(stop.second); 
        } 
 
    private: 
        std::hash<StopPtr> hasher_; 
    };

    void AddStop(std::string_view stop_name, const geo::Coordinates coordinates);
    void AddRoute(std::string_view bus_name, const std::vector<StopPtr> stops, bool is_circle);
    
    BusPtr GetRoute(const std::string_view& bus_name) const; 
    StopPtr GetStop(const std::string_view& stop_name) const;
    
    void SetStopDistance(StopPtr from, StopPtr to, const int distance); 
    int GetStopDistance(StopPtr from, StopPtr to) const;
    
    std::optional<BusStat> GetRouteStatistics(const std::string_view& bus_name) const;
    
    const std::map<std::string_view, BusPtr> SortBuses() const;

private:
    std::deque<Stop> stops_; 
    std::deque<Bus> buses_; 
     
    std::unordered_map<std::string_view, StopPtr> stopname_to_stop_; 
    std::unordered_map<std::string_view, BusPtr> busname_to_bus_; 
 
    std::unordered_map<std::pair<StopPtr, StopPtr>, int, StopHasher> stops_distances_;
};

} // namespace transport_catalogue