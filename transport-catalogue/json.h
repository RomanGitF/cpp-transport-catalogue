#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Number = std::variant<int, double>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };
    //inline const std::string NoneData{ "null" };
    class Node {

        using Value = std::variant <std::nullptr_t, bool, int, double, std::string, Array, Dict>;
        Value node_;

    public:

        Node() = default;
        Node(std::nullptr_t);
        Node(double value);
        Node(bool value);
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(Number);
        Node(std::string value);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Value& GetValue() const;

        bool operator== (const Node& left) const;
        bool operator!= (const Node& left) const;

    };

    struct OstreamNode {
        std::ostream& out;

        void operator() (std::nullptr_t) const;
        void operator() (double value) const;
        void operator() (bool value) const;
        void operator() (Array array) const;
        void operator() (Dict map) const;
        void operator() (int value) const;
        void operator() (std::string value) const;
    };

    class Document {
        Node root_;
    public:
        Document() = default;
        Document(Node root);
        const Node& GetRoot() const;

        bool operator== (const Document& right) const;
        bool operator!= (const Document& right) const;

    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json