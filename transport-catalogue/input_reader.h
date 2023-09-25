#pragma once

#include "transport_catalogue.h"

#include <charconv>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace transport_catalogue {

    namespace input {

        enum class InputType {
            StopX,
            BusX
        };

        struct Command {
            InputType type;
            //bool is_descript;
            std::string name;

            std::string description;
            geo::Coordinates coordinates;
            std::vector<std::pair<std::string_view, std::string_view>> distances;
            RouteType route_type;
            std::vector<std::string_view> route;

            geo::Coordinates ParseCoordinates(std::string_view lat, std::string_view lng);

            std::vector<std::string_view> ParseStops(std::string_view& line);

            std::vector<std::pair<std::string_view, std::string_view>> ParseDistances(std::vector<std::string_view> vec_input);

            void ParseCommandString(std::string& input);
        };

        std::string GetType(std::string_view& line);
        std::string GetName(std::string_view& line);

        inline std::vector<std::string_view> Split(std::string_view& string, char delim);

        class InputReader {
        public:
            void Load(TransportCatalogue& tc);
            void LoadCommand(TransportCatalogue& tc, Command com, bool);

            void ParseInput(std::istream& in);

        private:
            std::vector<Command> commands_;
        };

    }//namespace input
}//namespace transport_catalogue