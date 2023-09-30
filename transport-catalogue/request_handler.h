#pragma once 
 
#include "json.h" 
#include "map_renderer.h" 
#include "transport_catalogue.h"

#include <sstream>

using namespace transport_catalogue; 
using namespace domain;

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
        : db_(db)
        , renderer_(renderer) {
    }

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;
    
    // Возвращает маршруты, проходящие через
    const std::set<std::string_view> GetBusesByStop(const std::string_view& stop_name) const;
    
    bool IsBusNumber(const std::string_view& bus_number) const;
    bool IsStopName(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты" 
    const TransportCatalogue& db_; 
    const renderer::MapRenderer& renderer_;
};