#include "request_handler.h"

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.GetRouteStatistics(bus_name);
}

const std::set<std::string_view> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const { 
    return db_.GetStop(stop_name)->buses_by_stop; 
}

bool RequestHandler::IsBusNumber(const std::string_view& bus_name) const { 
    return db_.GetRoute(bus_name); 
} 
 
bool RequestHandler::IsStopName(const std::string_view& stop_name) const { 
    return db_.GetStop(stop_name); 
}

svg::Document RequestHandler::RenderMap() const { 
    return renderer_.RenderSVG(db_.SortBuses()); 
}