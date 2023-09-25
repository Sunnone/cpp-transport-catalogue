#include "stat_reader.h"

using namespace std::string_literals;

namespace transport_catalogue {

    namespace query {

        std::string GetType(std::string_view& line) {
            auto space = line.find(' ');
            std::string_view type = line.substr(0, space);
            line = line.substr(space + 1);
            return std::string(type);
        }

        std::string GetName(std::string_view& line) {
            return std::string(line);
        }

        void Query::ParseQueryString(std::string& input) {
            static std::unordered_map<std::string, QueryType> const table = {
                {"Stop", QueryType::StopY}, {"Bus", QueryType::BusY}
            };
            std::string_view sv = input;
            sv.remove_prefix(std::min(sv.find_first_not_of(" "), sv.size()));
            auto type_code = GetType(sv);
            name = GetName(sv);

            switch (table.at(type_code)) {
            case QueryType::StopY:
                type = QueryType::StopY;
                break;

            case QueryType::BusY:
                type = QueryType::BusY;
                break;
            }
        }

        void QueryReader::ParseQuery() {
            int query_count;
            std::cin >> query_count;
            std::cin.ignore();
            std::string query;

            for (int i = 0; i < query_count; ++i) {
                getline(std::cin, query);
                Query tmp_query;
                tmp_query.ParseQueryString(query);
                queries_.push_back(std::move(tmp_query));
            }
        }

        void QueryReader::Load(TransportCatalogue& tc, std::ostream& out) {
            for (auto q_it = queries_.begin(); q_it != queries_.end(); ++q_it) {
                QueryReader::LoadQuery(tc, *q_it, out);
            }
        }

        void QueryReader::LoadQuery(TransportCatalogue& tc, Query query, std::ostream& out) {
            switch (query.type) {
            case QueryType::StopY:
                output::OutputStopAbout(tc, query.name, out);
                break;
            case QueryType::BusY:
                output::OutputRouteAbout(tc, query.name, out);
                break;
            }
        }

    } //namespace query

    namespace output {

        void OutputRouteAbout(TransportCatalogue& tc, std::string_view route, std::ostream& out) {
            if (!tc.GetRoute(route)) {
                out << "Bus "s << route << ": not found"s << std::endl;
                return;
            }
            BusStat stat = tc.GetStatistics(tc.GetRoute(route));
            out << "Bus "s << route << ": "s
                << stat.stops_on_route << " stops on route, "s
                << stat.unique_stops << " unique stops, "s
                << stat.route_length << " route length, "s
                << std::setprecision(6) << stat.curvature << " curvature"s << std::endl;
            return;
        }

        void OutputStopAbout(TransportCatalogue& tc, std::string_view name, std::ostream& out) {
            std::set<std::string_view> buses = tc.GetBuses(name);
            if (!tc.GetStop(name)) {
                out << "Stop "s << name << ": not found"s << std::endl;
                return;
            }
            if (buses.size() == 0) {
                out << "Stop "s << name << ": no buses"s << std::endl;
                return;
            }
            std::cout << "Stop "s << name << ": buses "s;
            for (auto it = buses.begin(); it != buses.end(); ++it) {
                if (next(it) != buses.end()) {
                    out << (*it) << " "s;
                }
                else {
                    out << (*it);
                }
            }
            out << std::endl;
            return;
        }

    }//namespace output
}//namespace transport_catalogue