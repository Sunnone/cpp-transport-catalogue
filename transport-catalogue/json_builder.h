#pragma once

#include "json.h"

#include <memory>
#include <optional>

namespace json {
    
class DictItemContext;
class DictKeyContext;
class ArrayItemContext;

class Builder {
public:    
    DictKeyContext Key(const std::string& key);
    Builder& Value(const Node::Value& value);
    
    DictItemContext StartDict();
    Builder& EndDict();
    
    ArrayItemContext StartArray();
    Builder& EndArray();
    
    Node Build();
    void AddNode(const Node& node);
    Node GetNode(const Node::Value& value);

private:
    Node root_{nullptr};
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};
    
class BaseContext {
public:
    BaseContext(Builder& builder);
 
    DictKeyContext Key(const std::string& key);
    Builder& Value(const Node::Value& value);
    
    ArrayItemContext StartArray();
    Builder& EndArray();
    
    DictItemContext StartDict();
    Builder& EndDict();
 
protected:
    Builder& builder_; 
};

class DictItemContext : public BaseContext {
public:
    DictItemContext(Builder& builder);
    
    Builder& Value(const Node::Value& value) = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    DictItemContext StartDict() = delete;
};
    
class DictKeyContext : public BaseContext {
public:
    DictKeyContext(Builder& builder);
    
    DictKeyContext Key(const std::string& key) = delete;
    Builder& EndArray() = delete;
    Builder& EndDict() = delete;
};

class ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(Builder& builder);
    
    DictKeyContext Key(const std::string& key) = delete;
    Builder& EndDict() = delete;
};

} // namespace json