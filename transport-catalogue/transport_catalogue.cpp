#include "transport_catalogue.h"

#include <algorithm>

using namespace std::string_literals;

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string_view name, geo::Coordinates coordinates) {
        stops_.push_back({ { name.begin(), name.end() }, coordinates });
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddRoute(std::string_view number, RouteType type, const std::vector<std::string_view>& stops) {
        Bus result;
        result.name = { number.begin(), number.end() };
        for (auto& stop : stops) {
            auto found_stop = GetStop(stop);
            if (found_stop) {
                result.stops.push_back(found_stop);
            }
        }
        if (type == RouteType::Straight) {
            auto tmp = result.stops;
            tmp.pop_back();
            std::reverse(tmp.begin(), tmp.end());
            std::move(tmp.begin(), tmp.end(), std::back_inserter(result.stops));
        }
        buses_.push_back(std::move(result));
        busname_to_bus_[buses_.back().name] = &buses_.back();

        for (auto& stop : stops) {
            auto found_stop = GetStop(stop);
            if (found_stop != nullptr) {
                buses_to_stop_[found_stop].insert(buses_.back().name);
            }
        }
    }

    BusPtr TransportCatalogue::GetRoute(std::string_view name) {
        if (busname_to_bus_.count(name)) {
            return busname_to_bus_.at(name);
        }
        else {
            return nullptr;
        }
    }

    StopPtr TransportCatalogue::GetStop(std::string_view name) {
        if (stopname_to_stop_.count(name)) {
            return stopname_to_stop_.at(name);
        }
        else {
            return nullptr;
        }
    }

    std::set<std::string_view> TransportCatalogue::GetBuses(std::string_view stop) {
        auto found_stop = GetStop(stop);
        if (found_stop) {
            if (buses_to_stop_.count(found_stop)) {
                return buses_to_stop_.at(found_stop);
            }
        }
        return {};
    }

    void TransportCatalogue::SetStopDistance(std::string_view stop_from, uint64_t dist, std::string_view stop_to) {
        auto p_stop_from = GetStop(stop_from);
        auto p_stop_to = GetStop(stop_to);
        if (p_stop_from && p_stop_to) {
            dist_btw_stops[{p_stop_from, p_stop_to}] = dist;
        }
        return;
    }

    uint64_t TransportCatalogue::GetStopDistance(StopPtr p_stop_from, StopPtr p_stop_to) {
        if (p_stop_from && p_stop_to) {
            if (dist_btw_stops.count({ p_stop_from, p_stop_to })) {
                return dist_btw_stops.at({ p_stop_from, p_stop_to });
            }
            if (dist_btw_stops.count({ p_stop_to, p_stop_from })) {
                return dist_btw_stops.at({ p_stop_to, p_stop_from });
            }
        }
        return 0;
    }

    BusStat TransportCatalogue::GetStatistics(BusPtr bus) {
        BusStat statistics;
        statistics.stops_on_route = bus->stops.size();

        auto tmp = bus->stops;
        sort(tmp.begin(), tmp.end());
        auto it = unique(tmp.begin(), tmp.end());
        tmp.erase(it, tmp.end());
        statistics.unique_stops = tmp.size();

        for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
            statistics.geo_distance += ComputeDistance(bus->stops[i]->coordinates, bus->stops[i + 1]->coordinates);
            statistics.route_length += GetStopDistance(bus->stops[i], bus->stops[i + 1]);
        }
        statistics.curvature = statistics.route_length / statistics.geo_distance;

        return statistics;
    }

}//namespace transport_catalogue