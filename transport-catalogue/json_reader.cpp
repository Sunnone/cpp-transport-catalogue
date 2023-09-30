#include "json_reader.h" 
 
const json::Node& JsonReader::GetBaseRequests() const { 
    if (!input_.GetRoot().AsMap().count("base_requests")) { 
        return dummy_; 
    } 
    return input_.GetRoot().AsMap().at("base_requests"); 
} 
 
const json::Node& JsonReader::GetStatRequests() const { 
    if (!input_.GetRoot().AsMap().count("stat_requests")) { 
        return dummy_; 
    } 
    return input_.GetRoot().AsMap().at("stat_requests"); 
} 
 
const json::Node& JsonReader::GetRenderSettings() const { 
    if (!input_.GetRoot().AsMap().count("render_settings")) { 
        return dummy_; 
    } 
    return input_.GetRoot().AsMap().at("render_settings"); 
} 
 
void JsonReader::FillCatalogue(TransportCatalogue& db) { 
    const json::Array& arr = GetBaseRequests().AsArray(); 
    for (auto& request_stops : arr) { 
        const auto& request_stops_map = request_stops.AsMap(); 
        const auto& type = request_stops_map.at("type").AsString(); 
        if (type == "Stop") { 
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map); 
            db.AddStop(stop_name, coordinates); 
        } 
    } 
    FillStopDistances(db); 
     
    for (auto& request_bus : arr) { 
        const auto& request_bus_map = request_bus.AsMap(); 
        const auto& type = request_bus_map.at("type").AsString(); 
        if (type == "Bus") { 
            auto [bus_number, stops, circular_route] = FillRoute(request_bus_map, db); 
            db.AddRoute(bus_number, stops, circular_route); 
        } 
    } 
} 
 
std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> JsonReader::FillStop(const json::Dict& request_map) const { 
    std::string_view stop_name = request_map.at("name").AsString(); 
    geo::Coordinates coordinates = { request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble() }; 
    std::map<std::string_view, int> stop_distances; 
    auto& distances = request_map.at("road_distances").AsMap(); 
    for (auto& [stop_name, dist] : distances) { 
        stop_distances.emplace(stop_name, dist.AsInt()); 
    } 
    return std::make_tuple(stop_name, coordinates, stop_distances); 
} 
 
std::tuple<std::string_view, std::vector<StopPtr>, bool> JsonReader::FillRoute(const json::Dict& request_map, TransportCatalogue& db) const { 
    std::string_view bus_number = request_map.at("name").AsString(); 
    std::vector<StopPtr> stops; 
    for (auto& stop : request_map.at("stops").AsArray()) { 
        stops.push_back(db.GetStop(stop.AsString())); 
    } 
    bool circular_route = request_map.at("is_roundtrip").AsBool(); 
 
    return std::make_tuple(bus_number, stops, circular_route); 
} 
 
void JsonReader::FillStopDistances(TransportCatalogue& db) const { 
    const json::Array& arr = GetBaseRequests().AsArray(); 
    for (auto& request_stops: arr) { 
        const auto& request_stops_map = request_stops.AsMap(); 
        const auto& type = request_stops_map.at("type").AsString(); 
        if (type == "Stop") { 
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map); 
            for (auto& [to_name, dist] : stop_distances) { 
                auto from = db.GetStop(stop_name); 
                auto to = db.GetStop(to_name); 
                db.SetStopDistance(from, to, dist); 
            } 
        } 
    } 
}

svg::Color JsonReader::FillColor(const json::Node& node) const {
    if (node.IsArray()) {
        if (node.AsArray().size() == 3) {
            svg::Rgb rgb;
            rgb.red = node.AsArray()[0].AsInt();
            rgb.green = node.AsArray()[1].AsInt();
            rgb.blue = node.AsArray()[2].AsInt();
            return rgb;
        }
        else if (node.AsArray().size() == 4) {
            svg::Rgba rgba;
            rgba.red = node.AsArray()[0].AsInt();
            rgba.green = node.AsArray()[1].AsInt();
            rgba.blue = node.AsArray()[2].AsInt();
            rgba.opacity = node.AsArray()[3].AsDouble();
            return rgba;
        }
        else { 
            throw std::logic_error("Wrong underlayer colortype"); 
        }
    }
    else if (node.IsString()) {
        return node.AsString();
    }
    else { 
        throw std::logic_error("Wrong underlayer color"); 
    }
}
 
renderer::MapRenderer JsonReader::FillRenderSettings(const json::Dict& request_map) const { 
    renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width").AsDouble();
    render_settings.height = request_map.at("height").AsDouble();
    render_settings.padding = request_map.at("padding").AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius").AsDouble();
    render_settings.line_width = request_map.at("line_width").AsDouble();
    
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size").AsInt();
    const json::Array& bus_label_offset = request_map.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset = {bus_label_offset[0].AsDouble(),
                                        bus_label_offset[1].AsDouble()};
    
    render_settings.stop_label_font_size = request_map.at("stop_label_font_size").AsInt();
    const json::Array& stop_label_offset = request_map.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset = {stop_label_offset[0].AsDouble(),
                                         stop_label_offset[1].AsDouble() };
    
    render_settings.underlayer_color = FillColor(request_map.at("underlayer_color"));
    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    
    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    render_settings.color_palette.reserve(color_palette.size());
    for (const auto& color_element : color_palette) {
        render_settings.color_palette.emplace_back(FillColor(color_element));
    }
    
    return render_settings;
}

void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh, std::ostream& output) const { 
    json::Array result; 
    for (auto& request : stat_requests.AsArray()) { 
        const auto& request_map = request.AsMap(); 
        const auto& type = request_map.at("type").AsString(); 
        if (type == "Stop") { 
            result.push_back(MakeStop(request_map, rh).AsMap()); 
        } 
        else if (type == "Bus") { 
            result.push_back(MakeRoute(request_map, rh).AsMap()); 
        } 
        else if (type == "Map") { 
            result.push_back(MakeMap(request_map, rh).AsMap()); 
        } 
    } 
    json::Print(json::Document{result}, output); 
} 
 
const json::Node JsonReader::MakeRoute(const json::Dict& request_map, RequestHandler& rh) const { 
    json::Dict result; 
    const std::string& bus_name = request_map.at("name").AsString(); 
    result["request_id"] = request_map.at("id").AsInt(); 
    if (!rh.IsBusNumber(bus_name)) { 
        result["error_message"] = json::Node{ static_cast<std::string>("not found") }; 
    } 
    else { 
        result["curvature"] = rh.GetBusStat(bus_name)->curvature; 
        result["route_length"] = rh.GetBusStat(bus_name)->route_length; 
        result["stop_count"] = static_cast<int>(rh.GetBusStat(bus_name)->stops_count); 
        result["unique_stop_count"] = static_cast<int>(rh.GetBusStat(bus_name)->unique_stops_count); 
    } 
 
    return json::Node{result}; 
} 
 
const json::Node JsonReader::MakeStop(const json::Dict& request_map, RequestHandler& rh) const { 
    json::Dict result; 
    const std::string& stop_name = request_map.at("name").AsString(); 
    result["request_id"] = request_map.at("id").AsInt(); 
    if (!rh.IsStopName(stop_name)) { 
        result["error_message"] = json::Node{ static_cast<std::string>("not found") }; 
    } 
    else { 
        json::Array buses; 
        for (const auto& bus : rh.GetBusesByStop(stop_name)) { 
            buses.push_back(std::string(bus)); 
        } 
        result["buses"] = buses; 
    } 
 
    return json::Node{result}; 
} 
 
const json::Node JsonReader::MakeMap(const json::Dict& request_map, RequestHandler& rh) const { 
    json::Dict result; 
    result["request_id"] = request_map.at("id").AsInt(); 
    std::ostringstream strm; 
    svg::Document map = rh.RenderMap(); 
    map.Render(strm); 
    result["map"] = strm.str(); 
 
    return json::Node{result}; 
}