#include "json.h"

using namespace std;

namespace json {

    namespace {

        /////////////// Load Nodes Service ////////////

        Node LoadNode(istream& input);

        Array LoadArray(istream& input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c == ']') {
                return result;
            }
            else {
                throw ParsingError("Load array error"s);
            }
        }

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        bool CheckNull(istream& input) {
            std::string check;
            char ch;
            while (check.size() < 4) {
                ch = EOF;
                input >> ch;
                check += ch;
            }


            if (check != "null") {
                throw ParsingError("Failed to read null"s);
            }
            return true;
        }

        bool LoadBool(istream& input) {
            std::string check;
            char ch;
            input >> ch;

            if (ch == 't') {
                input.putback(ch);
                while (check.size() < 4) {
                    ch = EOF;
                    input >> ch;
                    check += ch;
                }
                if (check == "true") {
                    return true;
                }
            }
            else if (ch == 'f') {
                input.putback(ch);
                while (check.size() < 5) {
                    ch = EOF;
                    input >> ch;
                    check += ch;
                }
                if (check == "false") {
                    return false;
                }
            }
            throw ParsingError("Failed to read bool"s);
        }

        Dict LoadDict(istream& input) {

            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (c == '}') {
                return result;
            }
            else {
                throw ParsingError("Load dict error"s);
            }

        }

        Node LoadNode(istream& input) {
            char c = ' ';
            while (c == ' ') {
                input >> c;
            }
            if (c == 'n') {
                input.putback(c);
                CheckNull(input);
                return Node();
            }
            if (c == 't' || c == 'f') {
                input.putback(c);
                return Node(LoadBool(input));
            }
            if (c == '[') {
                return Node(LoadArray(input));
            }
            else if (c == '{') {
                return Node(LoadDict(input));
            }
            else if (c == '"') {
                return Node(LoadString(input));
            }
            /* else if (input.eof()) {
                 throw ParsingError("Load node error");
             }*/
            else {
                input.putback(c);
                return Node(LoadNumber(input));
            }
        }


    }  // namespace

    ////////////-----------    Node  -------------
    Node::Node(std::nullptr_t) {}

    Node::Node(int value)
        : node_(value) {}

    Node::Node(double value)
        : node_(move(value)) {}

    Node::Node(Number value) {
        holds_alternative<double>(value) ? node_ = get<double>(value) : node_ = get<int>(value) ;
    }

    Node::Node(bool value)
        : node_(move(value)) {}

    Node::Node(Array array)
        : node_(move(array)) {}

    Node::Node(Dict map)
        : node_(move(map)) {}

    Node::Node(string value)
        : node_(move(value)) {}

    bool Node::IsInt() const { return holds_alternative<int>(node_); }
    bool Node::IsDouble() const { return holds_alternative<double>(node_) || std::holds_alternative<int>(node_); }
    bool Node::IsPureDouble() const { return std::holds_alternative<double>(node_); }
    bool Node::IsBool() const { return std::holds_alternative<bool>(node_); }
    bool Node::IsString() const { return std::holds_alternative<std::string>(node_); }
    bool Node::IsNull() const { return std::holds_alternative<std::nullptr_t>(node_); }
    bool Node::IsArray() const { return std::holds_alternative<Array>(node_); }
    bool Node::IsMap() const { return std::holds_alternative<Dict>(node_); }

    int Node::AsInt() const {
        if (!IsInt()) throw std::logic_error("logic_error");
        return get<int>(node_);
    }

    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return get<double>(node_);
        }
        else if (IsInt()) {
            return static_cast<double>(get<int>(node_));
        }
        else { throw std::logic_error("logic_error"); }
    }

    bool Node::AsBool() const {
        if (!IsBool()) throw std::logic_error("logic_error");
        return get<bool>(node_);
    }

    const std::string& Node::AsString() const {
        if (!IsString()) throw std::logic_error("logic_error");
        return get<string>(node_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) throw std::logic_error("logic_error");
        return get<Array>(node_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) throw std::logic_error("logic_error");
        return get<Dict>(node_);
    }

    const Node::Value& Node::GetValue() const {
        return node_;
    }

    bool Node::operator== (const Node& left) const {
        return node_ == left.GetValue();
    }

    bool Node::operator!= (const Node& left) const {
        return node_ != left.GetValue();
    }

    //////////// ----------  OstreamNode___________________

    void OstreamNode::operator() (std::nullptr_t) const {
        out << "null";
    }
    void OstreamNode::operator() (int value) const {
        out << value;
    }
    void OstreamNode::operator() (double value) const {
        out << value;
    }
    void OstreamNode::operator() (bool value) const {

        value ? out << "true" : out << "false";

    }
    void OstreamNode::operator() (Array array) const {
        out << "[";
        bool first = true;
        for (const auto& it : array) {

            if (first) {
                out << "\n";
                first = false;
            }
            else {
                out << ", \n";
            }
            Print(it, out);

        }
        out << "\n]";
    }
    void OstreamNode::operator() (Dict map) const {
        bool IsFirst = true;
        out << "{";
        for (const auto& [key, value] : map) {
            if (IsFirst) {
                IsFirst = false;
            }
            else {
                out << ", ";
            }
            out << "\"" << key << "\"";
            out << " : ";
            std::visit(OstreamNode{ out }, value.GetValue());
        }

        out << "}";
    }
    void OstreamNode::operator() (std::string value) const {
        out << '\"';
        for (const auto& it : value) {
            switch (it) {
            case '\\':
                out << "\\" << it;
                break;
            case '\"':
                out << "\\\"";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\t':
                out << "\t";
                break;
            default:
                out << it;
            }
        }
        out << '\"';
    }

    //////////////// _________Document________________

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    bool Document::operator== (const Document& right) const {
        return (root_ == right.root_);
    }

    bool Document::operator!= (const Document& right) const {
        return (root_ != right.root_);
    }

    /// //////////////////////_______________Print___________________

    void Print(const Document& doc, std::ostream& out) {
        std::visit(OstreamNode{ out }, doc.GetRoot().GetValue());
    }

}  // namespace json