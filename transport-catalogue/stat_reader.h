#pragma once

#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <set>
#include <string_view>
#include <unordered_map>


namespace transport_catalogue {

    namespace query {

        enum class QueryType {
            StopY,
            BusY
        };

        struct Query {
            QueryType type;
            std::string name;

            void ParseQueryString(std::string& input);
        };

        std::string GetType(std::string_view& line);
        std::string GetName(std::string_view& line);

        class QueryReader {
        public:
            void LoadQuery(TransportCatalogue& tc, Query query, std::ostream& out);

            void ParseQuery(TransportCatalogue& tc, std::ostream& out);

        };

    } //namespace query

    namespace output {

        void OutputRouteAbout(TransportCatalogue& tc, std::string_view route, std::ostream& out);

        void OutputStopAbout(TransportCatalogue& tc, std::string_view name, std::ostream& out);

    }//namespace output
}//transport_catalogue