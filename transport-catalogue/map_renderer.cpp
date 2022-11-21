#include "map_renderer.h"

using namespace domain;

bool IsZero(double value) {
	return std::abs(value) < EPSILON;
}

svg::Color ReadColor(const json::Node& node) {
	svg::Color result;
	if (node.IsString()) {
		result = node.AsString();
	}
	else if (node.IsArray()) {
		const auto& col = node.AsArray();
		if (col.size() == 3) {
			svg::Rgb rgb(col[0].AsInt(), col[1].AsInt(), col[2].AsInt());
			result = rgb;
		}
		else if (col.size() == 4) {
			svg::Rgba rgba(col[0].AsInt(), col[1].AsInt(), col[2].AsInt(), col[3].AsDouble());
			result = rgba;
		}
	}
	return result;
}

//-----------------------MapRender

svg::Point MapRenderer::SphereProjector::operator()(geo::Coordinates coords) const {
	return {
		(coords.lng - min_lon_) * zoom_coeff_ + padding_,
		(max_lat_ - coords.lat) * zoom_coeff_ + padding_
	};
}

MapRenderer::SetRender::SetRender(const json::Dict& render_settings)
	:width__(render_settings.at("width").AsDouble()),
	heigth__(render_settings.at("height").AsDouble()),
	padding__(render_settings.at("padding").AsDouble()),
	line_width__(render_settings.at("line_width").AsDouble()),
	bus_label_offset__(render_settings.at("bus_label_offset").AsArray()[0].AsDouble(),
		render_settings.at("bus_label_offset").AsArray()[1].AsDouble()),
	bus_label_font_size__(render_settings.at("bus_label_font_size").AsInt()),
	underlayer_width__(render_settings.at("underlayer_width").AsDouble()),
	stop_radius__(render_settings.at("stop_radius").AsDouble()),
	stop_label_offset__(render_settings.at("stop_label_offset").AsArray()[0].AsDouble(),
		render_settings.at("stop_label_offset").AsArray()[1].AsDouble()),
	stop_label_font_size__(render_settings.at("stop_label_font_size").AsDouble())
{
	for (const json::Node& it : render_settings.at("color_palette").AsArray()) {
		color_palette__.emplace_back(ReadColor(it));
	}
	underlayer_color__ = ReadColor(render_settings.at("underlayer_color"));
}

MapRenderer::MapRenderer(const json::Dict& render_settings)
	:set_(render_settings) {}

MapRenderer::SphereProjector MapRenderer::CreatorProjector(const transport::Catalogue& catalogue) {
	std::list<geo::Coordinates> coordinates;
	for (const auto& [name, bus] : catalogue.GetAllBuses()) {
		for (const auto& stop : bus->stops__) {
			coordinates.emplace_back(stop->coordinates__);
		}
	}
	SphereProjector proj{
		coordinates.begin(), coordinates.end(), set_.width__, set_.heigth__, set_.padding__ };
	return proj;
}

void MapRenderer::DrawRoute(const std::vector<domain::Stop*> stops, SphereProjector& proj, svg::Color color, bool is_round) {
	svg::Polyline route;
	for (const auto& stop : stops) {
		route.AddPoint(proj(stop->coordinates__));
	}
	if (!is_round) {
		std::vector<Stop*> rstops(stops.rbegin(), stops.rend());
		rstops.erase(rstops.begin());
		for (const auto& stop : rstops) {
			route.AddPoint(proj(stop->coordinates__));
		}
	}
	route.SetStrokeWidth(set_.line_width__)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
		.SetStrokeColor(color)
		.SetFillColor("none");
	output_.Add(route);
}

void MapRenderer::DrawName(std::string_view name, svg::Point point, svg::Color color) {
	svg::Text back_text;
	back_text.SetPosition(point)
		.SetOffset(set_.bus_label_offset__)
		.SetFontSize(set_.bus_label_font_size__)
		.SetFontFamily("Verdana")
		.SetFontWeight("bold")
		.SetData(std::string(name))
		.SetFillColor(set_.underlayer_color__)
		.SetStrokeColor(set_.underlayer_color__)
		.SetStrokeWidth(set_.underlayer_width__)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	output_.Add(back_text);

	svg::Text text;
	text.SetPosition(point)
		.SetOffset(set_.bus_label_offset__)
		.SetFontSize(set_.bus_label_font_size__)
		.SetFontFamily("Verdana")
		.SetFontWeight("bold")
		.SetData(std::string(name))
		.SetFillColor(color);
	output_.Add(text);
}

void MapRenderer::DrawNameRoute(const domain::Bus& bus, SphereProjector& proj, svg::Color color) {
	if (bus.stops__.empty()) {
		return;
	}
	DrawName(bus.name__, proj(bus.stops__.front()->coordinates__), color);
	bool equal = bus.stops__.front()->name__ == bus.stops__.back()->name__;
	if (!bus.is_roundtrip__ && !equal) {
		DrawName(bus.name__, proj(bus.stops__.back()->coordinates__), color);
	}
}

void MapRenderer::DrawSymbolStop(const std::map<std::string_view, domain::Stop*>& stops, SphereProjector& proj) {
	for (const auto& [name, stop] : stops) {
		if (!stop->cross_buses__.empty()) {
			svg::Circle circle;
			circle.SetCenter(proj(stop->coordinates__))
				.SetRadius(set_.stop_radius__)
				.SetFillColor("white");
			output_.Add(circle);
		}
	}
}

void MapRenderer::DrawNameStop(const std::map<std::string_view, domain::Stop*>& stops, SphereProjector& proj) {
	for (const auto& [name, stop] : stops) {
		if (!stop->cross_buses__.empty()) {
			svg::Text back_text;
			back_text.SetPosition(proj(stop->coordinates__))
				.SetOffset(set_.stop_label_offset__)
				.SetFontSize(set_.stop_label_font_size__)
				.SetFontFamily("Verdana")
				.SetData(std::string(name))
				.SetFillColor(set_.underlayer_color__)
				.SetStrokeColor(set_.underlayer_color__)
				.SetStrokeWidth(set_.underlayer_width__)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			output_.Add(back_text);

			svg::Text text;
			text.SetPosition(proj(stop->coordinates__))
				.SetOffset(set_.stop_label_offset__)
				.SetFontSize(set_.stop_label_font_size__)
				.SetFontFamily("Verdana")
				.SetData(std::string(name))
				.SetFillColor("black");
			output_.Add(text);
		}
	}
}

void MapRenderer::Draw(const transport::Catalogue& catalogue, std::ostream& out) {

	auto color = set_.color_palette__.begin();
	auto last_color = set_.color_palette__.end();

	SphereProjector proj = CreatorProjector(catalogue);
	for (const auto& [name, bus] : catalogue.GetAllBuses()) {
		DrawRoute(bus->stops__, proj, *color, bus->is_roundtrip__);
		++color;
		if (color == last_color) {
			color = set_.color_palette__.begin();
		}
	}

	color = set_.color_palette__.begin();
	for (const auto& [name, bus] : catalogue.GetAllBuses()) {
		DrawNameRoute(*bus, proj, *color);
		++color;
		if (color == last_color) {
			color = set_.color_palette__.begin();
		}
	}

	DrawSymbolStop(catalogue.GetAllStops(), proj);
	DrawNameStop(catalogue.GetAllStops(), proj);
	output_.Render(out);
}

