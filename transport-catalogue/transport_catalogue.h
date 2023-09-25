#pragma once

#include "geo.h"

#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace transport_catalogue {

    enum RouteType {
        Straight,
        Round
    };

    namespace detail {

        template<typename Type>
        class StopHasher {
        public:
            size_t operator()(std::pair<const Type*, const Type*> stop) const {
                return hasher_(stop.first) + 47 * hasher_(stop.second);
            }

        private:
            std::hash<const Type*> hasher_;
        };

    }//namespace detail

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
    };

    using StopPtr = const Stop*;

    struct Bus {
        std::string name;
        std::vector<StopPtr> stops;
    };

    using BusPtr = const Bus*;

    struct BusStat {
        size_t stops_on_route = 0;
        size_t unique_stops = 0;
        double geo_distance = 0.0;
        uint64_t route_length = 0;
        double curvature = 0.0;
    };

    class TransportCatalogue {
    public:
        void AddStop(std::string_view name, geo::Coordinates coord);

        void AddRoute(std::string_view number, RouteType type, const std::vector<std::string_view>& stops);

        BusPtr GetRoute(std::string_view name);
        StopPtr GetStop(std::string_view name);

        std::set<std::string_view> GetBuses(std::string_view stop);

        void SetStopDistance(std::string_view stop_from, uint64_t dist, std::string_view stop_to);

        uint64_t GetStopDistance(StopPtr stop_from, StopPtr stop_to);

        BusStat GetStatistics(BusPtr bus);

    private:
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, StopPtr> stopname_to_stop_;
        std::unordered_map<std::string_view, BusPtr> busname_to_bus_;

        std::unordered_map<StopPtr, std::set<std::string_view>> buses_to_stop_;
        std::unordered_map<std::pair<StopPtr, StopPtr>, uint64_t, detail::StopHasher<Stop>> dist_btw_stops;
    };

}//namespace transport_catalogue