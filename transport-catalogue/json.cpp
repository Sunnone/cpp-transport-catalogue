#include "json.h" 
 
using namespace std::string_literals; 
 
namespace json { 
     
using Number = std::variant<int, double>; 
 
namespace loader { 
 
std::string Load(std::istream& input) { 
    std::string str; 
    while (std::isalpha(input.peek())) { 
        str.push_back(static_cast<char>(input.get())); 
    } 
    return str; 
} 
     
Node LoadNull(std::istream& input) { 
    auto str = Load(input);  
    if (str != "null"s) { 
        throw ParsingError("Null parsing error"s); 
    } 
    return Node(nullptr); 
} 
     
Node LoadBool(std::istream& input) { 
    const auto str = Load(input); 
     if (str == "true"s) { 
        return Node(true); 
    } 
    else if (str == "false"s) { 
        return Node(false); 
    } 
    else { 
        throw ParsingError("Bool parsing error"s); 
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
                // В случае неудачи, например, при переполнении, 
                // код ниже попробует преобразовать строку в double 
            } 
        } 
        return std::stod(parsed_num); 
    } catch (...) { 
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s); 
    } 
} 
 
// Считывает содержимое строкового литерала JSON-документа 
std::string LoadString(std::istream& input) { 
    auto it = std::istreambuf_iterator<char>(input); 
    auto end = std::istreambuf_iterator<char>(); 
    std::string s; 
    while (true) { 
        if (it == end) { 
            // Поток закончился до того, как встретили закрывающую кавычку? 
            throw ParsingError("String parsing error"s); 
        } 
        const char ch = *it; 
        if (ch == '"') { 
            // Встретили закрывающую кавычку 
            ++it; 
            break; 
        } else if (ch == '\\') { 
            // Встретили начало escape-последовательности 
            ++it; 
            if (it == end) { 
                // Поток завершился сразу после символа обратной косой черты 
                throw ParsingError("String parsing error"s); 
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
        } else if (ch == '\n' || ch == '\r') { 
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n 
            throw ParsingError("Unexpected end of line"s); 
        } else { 
            // Просто считываем очередной символ и помещаем его в результирующую строку 
            s.push_back(ch); 
        } 
        ++it; 
    } 
 
    return s; 
} 
 
Node LoadArray(std::istream& input) { 
    Array result; 
    if (input.peek() == -1) { 
        throw ParsingError("Array parsing error"s); 
    } 
    for (char c; input >> c && c != ']';) { 
        if (c != ',') { 
            input.putback(c); 
        } 
        result.push_back(LoadNode(input)); 
    } 
    return Node(std::move(result)); 
} 
     
Node LoadDict(std::istream& input) { 
    Dict result; 
    if (input.peek() == -1) { 
        throw ParsingError("Dict parsing error"s); 
    } 
    for (char c; input >> c && c != '}';) { 
        if (c == ',') { 
            input >> c; 
        } 
        std::string key = LoadString(input); 
        input >> c; 
        result.insert({std::move(key), LoadNode(input)}); 
    } 
    return Node(std::move(result)); 
} 
 
Node LoadNode(std::istream& input) { 
    char c; 
    input >> c; 
    if (c == 'n') { 
        input.putback(c); 
        return LoadNull(input); 
    } 
    else if (c == 't' || c == 'f') { 
        input.putback(c); 
        return LoadBool(input); 
    } 
    else if (c == '[') { 
        return LoadArray(input); 
    } 
    else if (c == '{') { 
        return LoadDict(input); 
    } 
    else if (c == '"') { 
        return LoadString(input); 
    } 
    else { 
        input.putback(c); 
        return LoadNumber(input); 
    } 
} 
 
}  // namespace loader 
 
bool Node::IsNull() const { 
    return std::holds_alternative<std::nullptr_t>(*this); 
} 
     
bool Node::IsBool() const { 
    return std::holds_alternative<bool>(*this); 
} 
     
bool Node::IsInt() const { 
    return std::holds_alternative<int>(*this); 
} 
 
bool Node::IsDouble() const { 
    return std::holds_alternative<double>(*this) || std::holds_alternative<int>(*this); 
} 
 
bool Node::IsPureDouble() const { 
    return std::holds_alternative<double>(*this); 
} 
 
bool Node::IsString() const { 
    return std::holds_alternative<std::string>(*this); 
} 
 
bool Node::IsArray() const { 
    return std::holds_alternative<Array>(*this); 
} 
 
bool Node::IsMap() const { 
    return std::holds_alternative<Dict>(*this); 
} 
 
bool Node::AsBool() const { 
    if (!IsBool()) { 
        throw std::logic_error("Wrong type"s); 
    } 
    return std::get<bool>(*this); 
} 
     
int Node::AsInt() const { 
    if (!IsInt()) { 
        throw std::logic_error("Wrong type"s); 
    } 
    return std::get<int>(*this); 
} 
 
double Node::AsDouble() const { 
    if (!IsDouble()) { 
        throw std::logic_error("Wrong type"s); 
    } 
    if (IsInt()) { 
        return static_cast<double>(std::get<int>(*this)); 
    } 
    return std::get<double>(*this); 
} 
 
const std::string& Node::AsString() const { 
    if (!IsString()) { 
        throw std::logic_error("Wrong type"s); 
    } 
    return std::get<std::string>(*this); 
} 
 
const Array& Node::AsArray() const { 
    if (!IsArray()) { 
        throw std::logic_error("Wrong type"s); 
    } 
    return std::get<Array>(*this); 
} 
 
const Dict& Node::AsMap() const { 
    if (!IsMap()) { 
        throw std::logic_error("Wrong type"s); 
    } 
    return std::get<Dict>(*this); 
} 
 
const Node::Value& Node::GetValue() const { 
    return *this; 
} 
 
bool Node::operator==(const Node& rhs) const { 
    return GetValue() == rhs.GetValue();
} 
 
bool Node::operator!=(const Node& rhs) const { 
    return !(GetValue() == rhs.GetValue()); 
} 
 
Document::Document(Node root) 
    : root_(std::move(root)) { 
} 
 
const Node& Document::GetRoot() const { 
    return root_; 
} 
     
bool Document::operator==(const Document& rhs) const { 
    return root_ == rhs.root_; 
} 
 
bool Document::operator!=(const Document& rhs) const { 
    return !(root_ == rhs.root_); 
} 
 
Document Load(std::istream& input) { 
    return Document{loader::LoadNode(input)}; 
} 
     
namespace valueprinter { 
     
void PrintContext::PrintIndent() const { 
    for (int i = 0; i < indent; ++i) { 
            out.put(' '); 
    } 
} 
  
PrintContext PrintContext::Indented() const { 
    return {out, indent_step, indent_step + indent}; 
} 
     
void PrintString(const std::string& value, std::ostream& out) { 
    out.put('"'); 
     
    for (const char ch : value) { 
        switch (ch) { 
            case '\r': 
                out << R"(\r)"; 
                break; 
            case '\n': 
                out << R"(\n)"; 
                break; 
            case '"': 
                out << R"(\")"; 
                break; 
            case '\\': 
                out << R"(\\)"; 
                break; 
            default: 
                out.put(ch); 
                break; 
        } 
    } 
     
    out.put('"'); 
} 
     
void PrintValue(const std::nullptr_t&, const PrintContext& context) { 
    context.out << "null"s; 
} 
  
void PrintValue(bool value, const PrintContext& context) { 
    context.out << std::boolalpha << value; 
} 
     
void PrintValue(const std::string& value, const PrintContext& context) { 
    PrintString(value, context.out); 
} 
     
void PrintValue(Array nodes, const PrintContext& context) { 
    std::ostream& out = context.out; 
    out << "[\n"s; 
    bool first = true; 
    auto inner_context = context.Indented(); 
    for (const Node& node : nodes) { 
        if (first) { 
            first = false; 
        } else { 
            out << ",\n"s; 
        } 
        inner_context.PrintIndent(); 
        PrintNode(node, inner_context); 
    } 
    out.put('\n'); 
    context.PrintIndent(); 
    out.put(']'); 
} 
  
void PrintValue(Dict nodes, const PrintContext& context) { 
    std::ostream& out = context.out; 
    out << "{\n"s; 
    bool first = true; 
    auto inner_context = context.Indented(); 
    for (const auto& [key, node] : nodes) { 
        if (first) { 
            first = false; 
        } else { 
            out << ",\n"s; 
        } 
        inner_context.PrintIndent(); 
        PrintString(key, context.out); 
        out << ": "s; 
        PrintNode(node, inner_context); 
    } 
    out.put('\n'); 
    context.PrintIndent(); 
    out.put('}'); 
} 
     
} // namespace valueprinter 
     
void PrintNode(const Node& node, const valueprinter::PrintContext& context) { 
    std::visit( 
        [&context](const auto& value){ valueprinter::PrintValue(value, context); }, 
        node.GetValue()); 
} 
 
void Print(const Document& doc, std::ostream& output) { 
    PrintNode(doc.GetRoot(), valueprinter::PrintContext{output}); 
} 
 
}  // namespace json 