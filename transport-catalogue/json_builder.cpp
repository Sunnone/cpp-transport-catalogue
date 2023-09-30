#include "json_builder.h"

namespace json {
    
    
// --------------- Builder ---------------
    
DictKeyContext Builder::Key(const std::string& key) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Wrong map key: " + key);
    }

    auto key_ptr = std::make_unique<Node>(key);
    
    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.emplace_back(std::move(key_ptr));
    }
 
    return DictKeyContext(*this);
}

Builder& Builder::Value(const Node::Value& value) {
    AddNode(GetNode(value));
    return *this;
}

DictItemContext Builder::StartDict() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));
    return DictItemContext(*this);
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Unable to close as without opening");
    }
    
    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Prev node is not a Dict");
    }
 
    Node node = *nodes_stack_.back();
    nodes_stack_.pop_back();
    AddNode(node);
 
    return *this;
}

ArrayItemContext Builder::StartArray() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));
    return ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Unable to close without opening");
    }
    
    if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Prev node is not an Array");
    }
    
    Node node = *nodes_stack_.back();
    nodes_stack_.pop_back();
    AddNode(node);
    return *this;
}

Node Builder::Build() {
    if (root_.IsNull() || nodes_stack_.size() > 1) {
        throw std::logic_error("Wrong Build()");
    }
    return root_;
}
    
void Builder::AddNode(const Node& node) {
    if (nodes_stack_.empty()) { 
        if (!root_.IsNull()) {
            throw std::logic_error("Unable to create node");
        }
        root_ = node;
        return;
    }
    
    if (nodes_stack_.back()->IsArray()) {
        auto array = nodes_stack_.back()->AsArray();
        array.emplace_back(node);
        nodes_stack_.pop_back();
        auto arr_ptr = std::make_unique<Node>(array);
        nodes_stack_.emplace_back(std::move(arr_ptr));
        return;
    }
    
    if (nodes_stack_.back()->IsString()) {
        auto str = nodes_stack_.back()->AsString();
        nodes_stack_.pop_back();
        
        if (nodes_stack_.back()->IsDict()) {
            auto dict = nodes_stack_.back()->AsDict();
            dict.emplace(std::move(str), node);
            nodes_stack_.pop_back();
            auto dict_ptr = std::make_unique<Node>(dict);
            nodes_stack_.emplace_back(std::move(dict_ptr));
        }
        
        return;
    }
    
    throw std::logic_error("Value() called in unknow container");
}

Node Builder::GetNode(const Node::Value& value) {
    if (std::holds_alternative<int>(value)) {
        return Node(std::get<int>(value));
    }
    if (std::holds_alternative<double>(value)) {
        return Node(std::get<double>(value));
    }
    if (std::holds_alternative<std::string>(value)) {
        return Node(std::get<std::string>(value));
    }
    if (std::holds_alternative<std::nullptr_t>(value)) {
        return Node(std::get<std::nullptr_t>(value));
    }
    if (std::holds_alternative<bool>(value)) {
        return Node(std::get<bool>(value));
    }
    if (std::holds_alternative<Dict>(value)) {
        return Node(std::get<Dict>(value));
    }
    if (std::holds_alternative<Array>(value)) {
        return Node(std::get<Array>(value));
    }
    return {};
}
    
// --------------- BaseContext ---------------

BaseContext::BaseContext(Builder& builder)
    : builder_(builder) {
}
 
DictKeyContext BaseContext::Key(const std::string& key) {
    return builder_.Key(key);
}
    
Builder& BaseContext::Value(const Node::Value& value) {
    return builder_.Value(value);
}
    
ArrayItemContext BaseContext::StartArray() {
    return ArrayItemContext(builder_.StartArray());
}
    
Builder& BaseContext::EndArray() {
    return builder_.EndArray();
}
 
DictItemContext BaseContext::StartDict() {
    return DictItemContext(builder_.StartDict());
}
    
Builder& BaseContext::EndDict() {
    return builder_.EndDict();
}
    
// --------------- DictItemContext ---------------

DictItemContext::DictItemContext(Builder& builder)
     : BaseContext(builder) {
}

// --------------- DictKeyContext ---------------
    
DictKeyContext::DictKeyContext(Builder& builder)
    : BaseContext(builder) {
}

// --------------- ArrayItemContext ---------------
    
ArrayItemContext::ArrayItemContext(Builder& builder)
    : BaseContext(builder) {
}

} // namespace json