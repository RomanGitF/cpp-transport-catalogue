#pragma once

#include "json.h"

using namespace std;

namespace json {

    class Builder;
    class KeyItemContext;
    class DictItemContext;
    class ArrayItemContext;
    class ValueDictItemContext;
    class ValueArrayItemContext;

    class ItemContext {
    public:
        Builder& builder_;

        ItemContext(Builder& builder);
        Builder& Value(Node val);
        DictItemContext StartDict();
        Builder& EndDict();
        KeyItemContext Key(std::string key);
        ArrayItemContext StartArray();
        Builder& EndArray();
        Node& Build();
    };

    class KeyItemContext : public ItemContext {
    public:
        KeyItemContext(Builder& builder);
        ValueDictItemContext Value(Node val);
        KeyItemContext Key(std::string key) = delete;
        Builder& EndDict() = delete;
        ArrayItemContext EndArray() = delete;
        Node& Build() = delete;
    };

    class DictItemContext : public ItemContext {
    public:
        DictItemContext(Builder& builder);
        Node Build() = delete;
        Builder& Value(Node val) = delete;
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
        DictItemContext StartDict() = delete;
    };

    class ArrayItemContext : public ItemContext {
    public:
        ArrayItemContext(Builder& builder);
        ValueArrayItemContext Value(Node val);
        Node& Build() = delete;
        KeyItemContext Key(std::string key) = delete;
        Builder& EndDict() = delete;
    };

    class ValueDictItemContext : public ItemContext {
    public:
        ValueDictItemContext(Builder& builder);
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
        Node& Build() = delete;
        ItemContext& Value(Node val) = delete;
        DictItemContext StartDict() = delete;
    };

    class ValueArrayItemContext : public ItemContext {
    public:
        ValueArrayItemContext(Builder& builder);
        ValueArrayItemContext Value(Node val);
        KeyItemContext Key(std::string key) = delete;
        Node& Build() = delete;
        Builder& EndDict() = delete;
    };


    class Builder {
        void CheckDoneJson();
        void CheckNotDict();

    public:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Builder();

        Builder& Value(Node val);
        DictItemContext StartDict();
        KeyItemContext Key(std::string key);
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        Node& Build();
    };



} // namespace json

