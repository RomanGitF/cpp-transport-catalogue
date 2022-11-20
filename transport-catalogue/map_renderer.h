#pragma once

#include <list>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

#include <deque>

#include "svg.h"
#include "json.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "domain.h"

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:

    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

svg::Color ReadColor(const json::Node& node);

struct SetRender {
    double width__;
    double heigth__;
    double padding__;
    double line_width__;
    svg::Point bus_label_offset__;
    int bus_label_font_size__;
    double underlayer_width__;
    double stop_radius__;
    svg::Point stop_label_offset__;
    double stop_label_font_size__;
    std::list<svg::Color> color_palette__;
    svg::Color underlayer_color__;

    SetRender(const json::Dict& render_settings);
};

class MapRender {
    SetRender set_;
    svg::Document output_;
    SphereProjector CreatorProjector(const transport::Catalogue& catalogue);
    void DrawRoute(const std::vector<domain::Stop*> stops,
        SphereProjector& proj, svg::Color color, bool is_round);           
    void DrawName(std::string_view name, svg::Point point, svg::Color color);
    void DrawNameRoute(const domain::Bus& bus, SphereProjector& proj, svg::Color color);
    void DrawSymbolStop(const std::map<std::string_view, domain::Stop*>& stops, SphereProjector& proj);
    void DrawNameStop(const std::map<std::string_view, domain::Stop*>& stops, SphereProjector& proj);

public:
    MapRender(const json::Dict& render_settings);

    void Draw(const transport::Catalogue& catalogue, std::ostream& out);

};


// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
    double max_width, double max_height, double padding)
    : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}



