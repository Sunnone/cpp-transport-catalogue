#pragma once 
 
#include "json.h" 
#include "map_renderer.h" 
#include "request_handler.h" 
#include "transport_catalogue.h" 
 
#include <iostream> 
 
using namespace transport_catalogue; 
using namespace domain; 
 
class JsonReader { 
public: 
    JsonReader(std::istream& input) 
        : input_(json::Load(input)) { 
    } 
 
    const json::Node& GetBaseRequests() const; 
    const json::Node& GetStatRequests() const; 
    const json::Node& GetRenderSettings() const; 
 
    void FillCatalogue(TransportCatalogue& db); 
    std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> FillStop(const json::Dict& request_map) const; 
    std::tuple<std::string_view, std::vector<StopPtr>, bool> FillRoute(const json::Dict& request_map, TransportCatalogue& db) const; 
    void FillStopDistances(TransportCatalogue& db) const;
    svg::Color FillColor(const json::Node& node) const;
    renderer::MapRenderer FillRenderSettings(const json::Dict& request_map) const;
    
    void ProcessRequests(const json::Node& stat_requests, RequestHandler& rh, std::ostream& output) const;

    const json::Node MakeRoute(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node MakeStop(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node MakeMap(const json::Dict& request_map, RequestHandler& rh) const;
 
private: 
    json::Document input_; 
    json::Node dummy_ = nullptr; 
};