#include "transport_catalogue.h" 
 
namespace transport_catalogue { 
 
void TransportCatalogue::AddStop(std::string_view stop_name, const geo::Coordinates coordinates) { 
    stops_.push_back({ std::string(stop_name), coordinates, {} }); 
    stopname_to_stop_[stops_.back().name] = &stops_.back(); 
} 
 
void TransportCatalogue::AddRoute(std::string_view bus_name, const std::vector<StopPtr> stops, bool is_circle) { 
    RouteType type = RouteType::Straight; 
    if (is_circle) { 
        type = RouteType::Round; 
    } 
    buses_.push_back({ std::string(bus_name), stops, type }); 
    busname_to_bus_[buses_.back().name] = &buses_.back(); 
    for (const auto& route_stop : stops) { 
        for (auto& stop_ : stops_) { 
            if (stop_.name == route_stop->name) { 
                stop_.buses_by_stop.insert(bus_name); 
            } 
        } 
    } 
} 
 
BusPtr TransportCatalogue::GetRoute(const std::string_view& bus_name) const { 
    if (busname_to_bus_.count(bus_name)) { 
        return busname_to_bus_.at(bus_name); 
    } 
    return nullptr; 
} 
 
StopPtr TransportCatalogue::GetStop(const std::string_view& stop_name) const { 
    if (stopname_to_stop_.count(stop_name)) { 
        return stopname_to_stop_.at(stop_name); 
    } 
    return nullptr; 
} 
 
void TransportCatalogue::SetStopDistance(StopPtr from, StopPtr to, const int distance) { 
    stops_distances_[{from, to}] = distance; 
} 
 
int TransportCatalogue::GetStopDistance(StopPtr from, StopPtr to) const { 
    if (stops_distances_.count({from, to})) return stops_distances_.at({from, to}); 
    else if (stops_distances_.count({to, from})) return stops_distances_.at({to, from}); 
    else return 0; 
}
    
std::optional<BusStat> TransportCatalogue::GetRouteStatistics(const std::string_view& bus_name) const {
    BusStat statistics{}; 
    BusPtr bus = GetRoute(bus_name); 
    if (!bus) { 
        throw std::invalid_argument("bus not found"); 
    } 
     
    if (bus->type == RouteType::Round) { 
        statistics.stops_count = bus->stops.size(); 
    } 
    else { 
        statistics.stops_count = bus->stops.size() * 2 - 1; 
    } 
     
    auto unique_stops = bus->stops; 
    std::sort(unique_stops.begin(), unique_stops.end()); 
    auto it = unique(unique_stops.begin(), unique_stops.end()); 
    unique_stops.erase(it, unique_stops.end()); 
    statistics.unique_stops_count = unique_stops.size(); 
 
    int route_length = 0; 
    double geo_distance = 0.0; 
 
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) { 
        auto from = bus->stops[i]; 
        auto to = bus->stops[i + 1]; 
        if (bus->type == RouteType::Round) { 
            geo_distance += geo::ComputeDistance(from->coordinates, to->coordinates); 
            route_length += GetStopDistance(from, to); 
        } 
        else { 
            geo_distance += geo::ComputeDistance(from->coordinates, to->coordinates) * 2; 
            route_length += GetStopDistance(from, to) + GetStopDistance(to, from); 
        } 
    } 
     
    statistics.route_length = route_length; 
    statistics.curvature = route_length / geo_distance; 
 
    return statistics;
}
 
const std::map<std::string_view, BusPtr> TransportCatalogue::SortBuses() const { 
    std::map<std::string_view, BusPtr> result; 
    for (const auto& bus : busname_to_bus_) { 
        result.emplace(bus); 
    } 
    return result; 
} 
 
} // namespace transport_catalogue