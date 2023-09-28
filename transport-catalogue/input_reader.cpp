#include "input_reader.h"

#include <algorithm>

using namespace std::string_literals;

namespace transport_catalogue {

    namespace input {

        std::string GetType(std::string_view& line) {
            auto space = line.find(' ');
            std::string_view type = line.substr(0, space);
            line = line.substr(space + 1);
            return std::string(type);
        }

        std::string GetName(std::string_view& line) {
            auto colon = line.find(':');
            std::string_view name = line.substr(0, colon);
            line = line.substr(colon + 1);
            return std::string(name);
        }

        inline std::vector<std::string_view> Split(std::string_view line, char separator) {
            std::vector<std::string_view> result;
            const int64_t pos_end = line.npos;
            line.remove_prefix(std::min(line.find_first_not_of(' '), line.size()));
            int64_t separ_pos = line.find(separator);

            while (!line.empty()) {
                separ_pos = line.find(separator);
                auto sline = line.substr(0, separ_pos);

                sline.remove_suffix(std::min(sline.length() - sline.find_last_not_of(' ') - 1, sline.length()));
                result.push_back(sline);
                if (separ_pos != pos_end) {
                    line.remove_prefix(std::min(separ_pos - line.find_first_not_of(' ') + 1, line.size()));
                }
                else {
                    break;
                }
                line.remove_prefix(std::min(line.find_first_not_of(' '), line.size()));
            }
            return result;
        }

        geo::Coordinates Command::ParseCoordinates(std::string_view lat, std::string_view lng) {
            lat.remove_prefix(std::min(lat.find_first_not_of(' '), lat.length()));
            lat.remove_suffix(std::min(lat.length() - lat.find_last_not_of(' ') - 1, lat.length()));

            lng.remove_prefix(std::min(lng.find_first_not_of(' '), lng.length()));
            lng.remove_suffix(std::min(lng.length() - lng.find_last_not_of(' ') - 1, lng.length()));

            return { std::stod(std::string(lat)), std::stod(std::string(lng)) };
        }

        std::vector<std::pair<std::string_view, std::string_view>> Command::ParseDistances(std::vector<std::string_view> vec_dist) {
            std::vector<std::pair<std::string_view, std::string_view>> result;

            for (size_t i = 2; i < vec_dist.size(); i++) {
                vec_dist[i].remove_prefix(std::min(vec_dist[i].find_first_not_of(' '), vec_dist[i].length()));
                std::string_view dist = vec_dist[i].substr(0, vec_dist[i].find('m'));

                std::string_view stop = vec_dist[i].substr(vec_dist[i].find('t') + 2, vec_dist[i].length());
                stop.remove_prefix(std::min(stop.find_first_not_of(' '), stop.length()));
                stop.remove_suffix(std::min(stop.length() - stop.find_last_not_of(' ') - 1, stop.length()));

                result.push_back({ dist, stop });
            }
            return result;
        }

        std::vector<std::string_view> Command::ParseStops(std::string_view& line) {
            std::vector<std::string_view> result;
            if (line.find('-') != line.npos) {
                route_type = RouteType::Straight;
                result = Split(description, '-');
            }
            if (line.find('>') != line.npos) {
                route_type = RouteType::Round;
                result = Split(description, '>');
            }
            return result;
        }

        void Command::ParseCommandString(std::string& input) {
            static std::unordered_map<std::string, InputType> const table = {
                {"Stop", InputType::StopX}, {"Bus", InputType::BusX}
            };
            std::string_view sv = input;
            sv.remove_prefix(std::min(sv.find_first_not_of(" "), sv.size()));
            auto type_code = GetType(sv);
            name = GetName(sv);

            switch (table.at(type_code)) {
            case InputType::StopX: {
                type = InputType::StopX;
                description = input.substr(input.find(':') + 2, input.length() - input.find(':') - 1);
                auto tmp = Split(description, ',');
                coordinates = ParseCoordinates(tmp[0], tmp[1]);
                if (tmp.size() > 2) {
                    distances = ParseDistances(tmp);
                }
                break;
            }
            case InputType::BusX: {
                type = InputType::BusX;
                description = input.substr(input.find(':') + 2, input.length() - input.find(':') - 1);
                route = ParseStops(sv);
                break;
            }
            }
        }

       

        void InputReader::ParseInput(std::istream& in, TransportCatalogue& tc) {
            {
                int query_count;
                in >> query_count;
                in.ignore();
                std::string command;

                for (int i = 0; i < query_count; ++i) {
                    getline(in, command);
                    Command tmp_command;
                    tmp_command.ParseCommandString(command);
                    commands_.push_back(std::move(tmp_command));
                }
            }
            InputReader::Load(tc);
        }
        
        
        void InputReader::Load(TransportCatalogue& tc) {
            auto it_stops = std::partition(commands_.begin(), commands_.end(), [](Command com) {
                return (com.type == InputType::StopX);
                });
            for (auto com_it = commands_.begin(); com_it != it_stops; ++com_it) {
                InputReader::LoadCommand(tc, *com_it, 0);
            }
            for (auto cur_it = commands_.begin(); cur_it != it_stops; ++cur_it) {
                InputReader::LoadCommand(tc, *cur_it, 1);
            }
            for (auto com_it = it_stops; com_it != commands_.end(); ++com_it) {
                InputReader::LoadCommand(tc, *com_it, 0);
            }
        }


        void InputReader::LoadCommand(TransportCatalogue& tc, Command com, bool dist) {
            switch (com.type) {
            case InputType::StopX:
                if (!dist) {
                    tc.AddStop(com.name, com.coordinates);
                }
                else {
                    if (!com.distances.empty()) {
                        for (auto& [dist, stop] : com.distances) {
                            std::string dist_str = { dist.begin(), dist.end() };
                            tc.SetStopDistance(com.name, stoull(dist_str), stop);
                        }
                    }
                }
                break;

            case InputType::BusX:
                tc.AddRoute(com.name, com.route_type, com.route);
                break;
            }
        }

    }//namespace input
}//namespace transport_catalogue