#include "json_builder.h"

using namespace std;
using namespace json;

void Builder::CheckDoneJson() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Calling member functions after done JSON");
    }
}

void Builder::CheckNotDict() {
    if (nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Calling member functions for Dict(without Key");
    }
}

Builder::Builder() {
    nodes_stack_.emplace_back(&root_);
}


DictItemContext Builder::StartDict() {
    CheckDoneJson();
    CheckNotDict();
    Dict empty;
    Node tmp(empty);
    if (nodes_stack_.back()->IsArray()) {
        Node& new_top = nodes_stack_.back()->AddElemArray(tmp);
        nodes_stack_.emplace_back(&new_top);
        return *this;
    }
    nodes_stack_.back()->ChangeNode(tmp);
    return *this;
}

Builder& Builder::Value(Node val) {
    CheckDoneJson();
    CheckNotDict();
    if (nodes_stack_.back()->IsArray()) {
        nodes_stack_.back()->AddElemArray(val);
        return *this;
    }
    nodes_stack_.back()->ChangeNode(val);
    nodes_stack_.pop_back();
    return *this;
}

KeyItemContext Builder::Key(std::string key) {
    CheckDoneJson();
    if (nodes_stack_.back()->IsDict()) {
        Node& value = nodes_stack_.back()->AddKeyDict(key);
        nodes_stack_.emplace_back(&value);
    }
    else {
        throw std::logic_error("Calling member functions Key() Not for Dict");
    }
    return *this;
}

Builder& Builder::EndDict() {
    CheckDoneJson();
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Calling member functions EndDict() not for Dict");
    }
    nodes_stack_.pop_back();
    return *this;
}

ArrayItemContext Builder::StartArray() {
    CheckDoneJson();
    CheckNotDict();
    Array empty;
    Node tmp(empty);
    if (nodes_stack_.back()->IsArray()) {
        Node& new_top = nodes_stack_.back()->AddElemArray(tmp);
        nodes_stack_.emplace_back(&new_top);
        ArrayItemContext* result = new ArrayItemContext(*this);
        return *result;
    }
    nodes_stack_.back()->ChangeNode(tmp);
    return *this;
}

Builder& Builder::EndArray() {
    CheckDoneJson();
    if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Calling member functions EndArray() not for Array");
    }
    nodes_stack_.pop_back();
    return *this;
}

Node& Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Not done json. Stack nodes not empty JSON.");
    }
    return root_;
}



//___________ItemContext è íàñëåäíèêè______________________________

ItemContext::ItemContext(Builder& builder)
    :builder_(builder)
{}

ArrayItemContext ItemContext::StartArray() {
    return builder_.StartArray();
}

DictItemContext ItemContext::StartDict() {
    return builder_.StartDict();
}

Builder& ItemContext::Value(Node val) {
    return builder_.Value(val);
}

KeyItemContext ItemContext::Key(std::string key) {
    return builder_.Key(key);
}

Builder& ItemContext::EndDict() {
    return builder_.EndDict();
}

Builder& ItemContext::EndArray() {
    return builder_.EndArray();
}

Node& ItemContext::Build() {
    return builder_.Build();
}

ValueDictItemContext KeyItemContext::Value(Node val) {
    return builder_.Value(val) ;
}

ValueArrayItemContext ArrayItemContext::Value(Node val) {
    return builder_.Value(val);
}

ValueArrayItemContext ValueArrayItemContext::Value(Node val) {
    return builder_.Value(val);
}

KeyItemContext::KeyItemContext(Builder& builder)
    : ItemContext(builder)
{}

DictItemContext::DictItemContext(Builder& builder)
    : ItemContext(builder)
{}

ArrayItemContext::ArrayItemContext(Builder& builder)
    : ItemContext(builder)
{}

ValueDictItemContext::ValueDictItemContext(Builder& builder)
    : ItemContext(builder)
{}

ValueArrayItemContext::ValueArrayItemContext(Builder& builder)
    : ItemContext(builder)
{}