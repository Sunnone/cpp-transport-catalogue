#pragma once 
 
#include <iostream> 
#include <map> 
#include <string> 
#include <variant> 
#include <vector> 
 
namespace json { 
 
class Node; 
using Array = std::vector<Node>; 
using Dict = std::map<std::string, Node>; 
 
// Эта ошибка должна выбрасываться при ошибках парсинга JSON 
class ParsingError : public std::runtime_error { 
public: 
    using runtime_error::runtime_error; 
}; 
 
namespace loader { 
     
std::string Load(std::istream& input);  
Node LoadNull(std::istream& input); 
Node LoadBool(std::istream& input); 
Node LoadNumber(std::istream& input); 
std::string LoadString(std::istream& input); 
Node LoadArray(std::istream& input);     
Node LoadDict(std::istream& input); 
Node LoadNode(std::istream& input); 
 
}  // namespace loader 
 
class Node final
    : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = variant;
     
    bool IsNull() const; 
    bool IsBool() const; 
    bool IsInt() const; 
    bool IsDouble() const; 
    bool IsPureDouble() const; 
    bool IsString() const; 
    bool IsArray() const; 
    bool IsMap() const; 
     
    const Array& AsArray() const; 
    const Dict& AsMap() const; 
    bool AsBool() const; 
    int AsInt() const; 
    double AsDouble() const; 
    const std::string& AsString() const; 
     
    const Value& GetValue() const; 
 
    bool operator==(const Node& rhs) const; 
    bool operator!=(const Node& rhs) const;
}; 
 
class Document { 
public: 
    explicit Document(Node root); 
 
    const Node& GetRoot() const; 
     
    bool operator==(const Document& rhs) const; 
    bool operator!=(const Document& rhs) const; 
 
private: 
    Node root_; 
}; 
 
Document Load(std::istream& input); 
     
namespace valueprinter { 
     
// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп 
struct PrintContext { 
    std::ostream& out; 
    int indent_step = 4; 
    int indent = 0; 
 
    void PrintIndent() const; 
 
    // Возвращает новый контекст вывода с увеличенным смещением 
    PrintContext Indented() const; 
}; 
     
void PrintString(const std::string& value, std::ostream& out); 
     
template <typename Value> 
void PrintValue(const Value& value, const PrintContext& context) { 
    context.out << value; 
} 
void PrintValue(const std::nullptr_t&, const PrintContext& context); 
void PrintValue(bool value, const PrintContext& context); 
void PrintValue(const std::string& value, const PrintContext& context); 
void PrintValue(Array nodes, const PrintContext& context);  
void PrintValue(Dict nodes, const PrintContext& context); 
     
} // namespace valueprinter 
 
void PrintNode(const Node& node, const valueprinter::PrintContext& context); 
void Print(const Document& doc, std::ostream& output); 
 
}  // namespace json