#include "json.h"

#include <iterator>


//////////////Методы Node:
void json::Node::ChangeNode(Node& node) {
    *this = node;
}

json::Node& json::Node::AddKeyDict(std::string& key) {
    if (!IsDict()) {
        throw std::logic_error("Calling member functions AddKeyDict(sting key) not for Dict");
    }
    return std::get<Dict>(*this)[key];
}

json::Node& json::Node::AddElemArray(Node& elem) {
    if (!IsArray()) {
        throw std::logic_error("Calling member functions AddElemArray(Node& elem) not for Array");

}
std::get<Array>(*this).emplace_back(elem);
    return  std::get<Array>(*this).back();
}

bool json::Node::RemoveLastElem() {
    if (IsArray() && !std::get<Array>(*this).empty()) {
        std::get<Array>(*this).pop_back();
        return true;
    }
    return false;
}

bool json::Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}

int json::Node::AsInt() const {
    using namespace std::literals;
    if (!IsInt()) {
        throw std::logic_error("Not an int"s);
    }
    return std::get<int>(*this);
}

bool json::Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
    }

bool json::Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}

double json::Node::AsDouble() const {
    using namespace std::literals;
    if (!IsDouble()) {
        throw std::logic_error("Not a double"s);
    }
    return IsPureDouble() ? std::get<double>(*this) : AsInt();
}

bool json::Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool json::Node::AsBool() const {
    using namespace std::literals;
    if (!IsBool()) {
        throw std::logic_error("Not a bool"s);
    }
    return std::get<bool>(*this);
}

bool json::Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool json::Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

const json::Array& json::Node::AsArray() const {
    using namespace std::literals;
    if (!IsArray()) {
        throw std::logic_error("Not an array"s);
    }
    return std::get<Array>(*this);
}

bool json::Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

const std::string& json::Node::AsString() const {
    using namespace std::literals;
    if (!IsString()) {
        throw std::logic_error("Not a string"s);
    }
    return std::get<std::string>(*this);
}

bool json::Node::IsDict() const {
    return std::holds_alternative<Dict>(*this);
}

const json::Dict& json::Node::AsDict() const {
    using namespace std::literals;
    if (!IsDict()) {
        throw std::logic_error("Not a dict"s);
    }
    return std::get<Dict>(*this);
}

bool json::Node::operator==(const json::Node& rhs) const {
    return GetValue() == rhs.GetValue();
}

const json::Node::Value& json::Node::GetValue() const {
    return *this;
}

//////////////////////// ^^^^^^^^^^^^  end Node ^^^^^^^^^^^^

inline bool json::operator!=(const json::Node& lhs, const json::Node& rhs) {
    return !(lhs == rhs);
}

//////////////////////// методы Document


json::Document::Document(json::Node root)
    : root_(std::move(root)) {
}

const json::Node& json::Document::GetRoot() const {
    return root_;
}
/////////////////////////////// end Document

inline bool json::operator==(const json::Document& lhs, const json::Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool json::operator!=(const json::Document& lhs, const json::Document& rhs) {
    return !(lhs == rhs);
}
namespace json {

namespace {
using namespace std::literals;

Node LoadNode(std::istream& input);
Node LoadString(std::istream& input);

std::string LoadLiteral(std::istream& input) {
    std::string s;
    while (std::isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}

Node LoadArray(std::istream& input) {
    std::vector<Node> result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Array parsing error"s);
    }
    return Node(std::move(result));
}

Node LoadDict(std::istream& input) {
    Dict dict;

    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':') {
                if (dict.find(key) != dict.end()) {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                dict.emplace(std::move(key), LoadNode(input));
            } else {
                throw ParsingError(": is expected but '"s + c + "' has been found"s);
            }
        } else if (c != ',') {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
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
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}

Node LoadBool(std::istream& input) {
    const auto s = LoadLiteral(input);
    if (s == "true"sv) {
        return Node{true};
    } else if (s == "false"sv) {
        return Node{false};
    } else {
        throw ParsingError("Failed to parse '"s + s + "' as bool"s);
    }
}

Node LoadNull(std::istream& input) {
    if (auto literal = LoadLiteral(input); literal == "null"sv) {
        return Node{nullptr};
    } else {
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
    }
}

Node LoadNumber(std::istream& input) {
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
    } else {
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
            } catch (...) {
                // В случае неудачи, например, при переполнении
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(std::istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Unexpected EOF"s);
    }
    switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 't':
            // Атрибут [[fallthrough]] (провалиться) ничего не делает, и является
            // подсказкой компилятору и человеку, что здесь программист явно задумывал
            // разрешить переход к инструкции следующей ветки case, а не случайно забыл
            // написать break, return или throw.
            // В данном случае, встретив t или f, переходим к попытке парсинга
            // литералов true либо false
            [[fallthrough]];
        case 'f':
            input.putback(c);
            return LoadBool(input);
        case 'n':
            input.putback(c);
            return LoadNull(input);
        default:
            input.putback(c);
            return LoadNumber(input);
    }
}

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

void PrintNode(const Node& value, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintString(const std::string& value, std::ostream& out) {
    out.put('"');
    for (const char c : value) {
        switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
        }
    }
    out.put('"');
}

template <>
void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}

template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

// В специализаци шаблона PrintValue для типа bool параметр value передаётся
// по константной ссылке, как и в основном шаблоне.
// В качестве альтернативы можно использовать перегрузку:
// void PrintValue(bool value, const PrintContext& ctx);
template <>
void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
    ctx.out << (value ? "true"sv : "false"sv);
}

template <>
void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

template <>
void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value) {
            PrintValue(value, ctx);
        },
        node.GetValue());
}

}  // namespace

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json