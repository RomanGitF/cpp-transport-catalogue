#include "svg.h"

namespace svg {
    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        using namespace std::literals;
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    std::ostream& operator << (std::ostream& out, StrokeLineJoin join) {
        using namespace std::literals;
        switch (join) {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
        }
        return out;
    }

    std::ostream& operator << (std::ostream& out, const Color color) {
        std::visit(OstreamColor{ out }, color);
        return out;
    }

    std::ostream& operator << (std::ostream& out, const Rgb color) {
        out << "rgb(" << static_cast<int>(color.red) << "," << static_cast<int>(color.green)
            << "," << static_cast<int>(color.blue) << ")";

        //out << "rgba("s << std::to_string(color.red) << ","s << std::to_string(color.green)
          //  << ","s << std::to_string(color.blue) << ")"s;
        return out;
    }
    std::ostream& operator << (std::ostream& out, const Rgba color) {
        out << "rgba(" << static_cast<int>(color.red) << "," << static_cast<int>(color.green)
            << "," << static_cast<int>(color.blue) << "," << color.opacity << ")";

        // out << "rgba("s << std::to_string(color.red) << ","s << std::to_string(color.green)
        //     << ","s << std::to_string(color.blue) << ","s << color.opacity << ")"s;
        return out;

    }
    std::ostream& operator << (std::ostream& out, std::monostate) {
        out << "none";
        return out;
    }


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }



    void OstreamColor::operator() (std::monostate) {
        out << "none";
    }
    void OstreamColor::operator() (std::string color) {
        out << color;
    }
    void OstreamColor::operator() (Rgb color) {
        out << "rgb(" << static_cast<int>(color.red) << "," << static_cast<int>(color.green)
            << "," << static_cast<int>(color.blue) << ")";
    }
    void OstreamColor::operator() (Rgba color) {
        out << "rgba(" << static_cast<int>(color.red) << "," << static_cast<int>(color.green)
            << "," << static_cast<int>(color.blue) << "," << color.opacity << ")";
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        PathProps::RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        bool first = true;
        out << "<polyline points=\""sv;
        for (const Point& it : points_) {
            if (first) {
                first = false;
            }
            else { out << " "; }
            out << it.x << "," << it.y;
        }
        out << "\" "sv;
        PathProps::RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(std::move(point));
        return *this;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = (std::move(pos));
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = (std::move(offset));
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = (std::move(font_family));
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = (std::move(font_weight));
        return *this;
    }

    Text& Text::SetData(std::string data) {
        text_ = (std::move(data));

        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << " "sv;
        PathProps::RenderAttrs(out);
        out << ">";
        for (auto& it : text_) {
            if (it == '"') { out << "&quot;"sv; }
            else if (it == '\'') { out << "&apos;"sv; }
            else if (it == '<') { out << "&lt;"sv; }
            else if (it == '>') { out << "&gt;"sv; }
            else if (it == '&') { out << "&amp;"sv; }
            else out << it;
        }
        out << "</text>"sv;
    }



    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        docs_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (auto& it : docs_) {
            it->Render(out);
        }
        out << "</svg>"sv;
    }



}  // namespace svg