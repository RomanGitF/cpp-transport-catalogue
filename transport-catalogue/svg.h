#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <list>
#include <optional>
#include <variant>

#include "svg.pb.h"


namespace svg {
    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);
    std::ostream& operator << (std::ostream& out, StrokeLineJoin join);

    struct Rgb {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;

        Rgb(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
            :red(red),
            green(green),
            blue(blue) {}
    };

    struct Rgba {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;

        Rgba(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, double opacity = 1.0)
            :red(red),
            green(green),
            blue(blue),
            opacity(opacity) {}
    };

    std::ostream& operator << (std::ostream& out, const Rgb color);
    std::ostream& operator << (std::ostream& out, const Rgba color);
    std::ostream& operator << (std::ostream& out, std::monostate);

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline Color NoneColor{ "none" };

    std::ostream& operator << (std::ostream& out, const Color color);

    struct OstreamColor {
        std::ostream& out;

        void operator() (std::monostate);
        void operator() (std::string);
        void operator() (Rgb);
        void operator() (Rgba);
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    void SerializeColor(const Color& color, SvgProto::Color& proto);
    Color DeserializeColor(const SvgProto::Color& proto);

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;
    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    template <typename Owner>
    class PathProps {
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;

        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

    public:

        virtual Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        virtual Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        virtual Owner& SetStrokeWidth(double width) {
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        virtual Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        virtual Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = std::move(line_join);
            return AsOwner();
        }

        virtual void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv;
                std::visit(OstreamColor{ out }, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(OstreamColor{ out }, *stroke_color_);
                out << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }
        virtual ~PathProps() = default;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);
    private:
        void RenderObject(const RenderContext& context) const override;
        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
        std::list<Point> points_;
        void RenderObject(const RenderContext& context) const override;
    public:
        Polyline& AddPoint(Point point);
    };

    class Text final : public Object, public PathProps<Text> {
        Point pos_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string text_;
        void RenderObject(const RenderContext& context) const override;
    public:
        Text& SetPosition(Point pos = { 0, 0 });
        Text& SetOffset(Point offset = { 0, 0 });
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);
    };

    class ObjectContainer {

    public:
        template <typename Obj>
        void Add(Obj obj) {              //возможно надо сделать не шаблонным ?????
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        virtual ~ObjectContainer() = default;

    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& objs) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
        std::list<std::unique_ptr<Object>> docs_;
    public:
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;
    };
}  // namespace svg
