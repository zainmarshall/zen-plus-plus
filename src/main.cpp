#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <variant>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdint>
#include <cmath>
#include <iomanip>
#include <unordered_set>
#include <memory>
#include <algorithm>
#include "lexer.hpp"
#include "parser.hpp"
#include "ASTNode.hpp"

// Flow control signal enum (values declared after Value type)
enum class Signal { NONE, RETURN, BREAK, CONTINUE };

struct HashKey {
    std::variant<std::int64_t, std::string> data;

    HashKey(std::int64_t v) : data(v) {}
    HashKey(const std::string& s) : data(s) {}

    bool isInt() const { return std::holds_alternative<std::int64_t>(data); }
    bool isString() const { return std::holds_alternative<std::string>(data); }

    std::string toString() const {
        if (isInt()) return std::to_string(std::get<std::int64_t>(data));
        return std::get<std::string>(data);
    }
};

struct HashKeyHash {
    size_t operator()(const HashKey& key) const {
        if (key.isInt()) {
            return std::hash<std::int64_t>{}(std::get<std::int64_t>(key.data));
        }
        return std::hash<std::string>{}(std::get<std::string>(key.data));
    }
};

struct HashKeyEq {
    bool operator()(const HashKey& a, const HashKey& b) const {
        return a.data == b.data;
    }
};

struct ObjectData;
struct HashMapWrapper;
struct HashSetWrapper;

using ObjectPtr = std::shared_ptr<ObjectData>;
using HashMapPtr = std::shared_ptr<HashMapWrapper>;
using HashSetPtr = std::shared_ptr<HashSetWrapper>;

struct Value {
    std::variant<std::int64_t, double, std::string, std::vector<Value>, ObjectPtr, HashMapPtr, HashSetPtr> data;

    Value() : data(static_cast<std::int64_t>(0)) {}
    Value(std::int64_t v) : data(v) {}
    Value(double v) : data(v) {}
    Value(const std::string& s) : data(s) {}
    Value(const std::vector<Value>& v) : data(v) {}
    Value(const ObjectPtr& obj) : data(obj) {}
    Value(const HashMapPtr& map) : data(map) {}
    Value(const HashSetPtr& set) : data(set) {}

    bool isInt() const { return std::holds_alternative<std::int64_t>(data); }
    bool isFloat() const { return std::holds_alternative<double>(data); }
    bool isString() const { return std::holds_alternative<std::string>(data); }
    bool isVector() const { return std::holds_alternative<std::vector<Value>>(data); }
    bool isObject() const { return std::holds_alternative<ObjectPtr>(data); }
    bool isMap() const { return std::holds_alternative<HashMapPtr>(data); }
    bool isSet() const { return std::holds_alternative<HashSetPtr>(data); }
    bool isNumber() const { return isInt() || isFloat(); }

    std::int64_t asInt(const std::string& context) const {
        if (!isInt()) {
            throw std::runtime_error(context + " expects integer value");
        }
        return std::get<std::int64_t>(data);
    }

    double asDouble(const std::string& context) const {
        if (isFloat()) {
            return std::get<double>(data);
        }
        if (isInt()) {
            return static_cast<double>(std::get<std::int64_t>(data));
        }
        throw std::runtime_error(context + " expects numeric value");
    }

    const std::string& asString(const std::string& context) const {
        if (!isString()) {
            throw std::runtime_error(context + " expects string value");
        }
        return std::get<std::string>(data);
    }

    const std::vector<Value>& asVector(const std::string& context) const {
        if (!isVector()) {
            throw std::runtime_error(context + " expects vector value");
        }
        return std::get<std::vector<Value>>(data);
    }

    std::vector<Value>& asVectorRef(const std::string& context) {
        if (!isVector()) {
            throw std::runtime_error(context + " expects vector value");
        }
        return std::get<std::vector<Value>>(data);
    }

    ObjectPtr asObject(const std::string& context) const;
    HashMapPtr asMap(const std::string& context) const;
    HashSetPtr asSet(const std::string& context) const;
    bool truthy() const;
    static std::string formatDouble(double value);
    std::string toString() const;
};

struct ObjectData {
    std::string typeName;
    std::unordered_map<std::string, Value> fields;
};

struct HashMapWrapper {
    std::unordered_map<HashKey, Value, HashKeyHash, HashKeyEq> data;
};

struct HashSetWrapper {
    std::unordered_set<HashKey, HashKeyHash, HashKeyEq> data;
};

ObjectPtr Value::asObject(const std::string& context) const {
    if (!isObject()) {
        throw std::runtime_error(context + " expects struct object");
    }
    return std::get<ObjectPtr>(data);
}

HashMapPtr Value::asMap(const std::string& context) const {
    if (!isMap()) {
        throw std::runtime_error(context + " expects map");
    }
    return std::get<HashMapPtr>(data);
}

HashSetPtr Value::asSet(const std::string& context) const {
    if (!isSet()) {
        throw std::runtime_error(context + " expects set");
    }
    return std::get<HashSetPtr>(data);
}

bool Value::truthy() const {
    if (isInt()) return std::get<std::int64_t>(data) != 0;
    if (isFloat()) return std::get<double>(data) != 0.0;
    if (isString()) return !std::get<std::string>(data).empty();
    if (isVector()) return !std::get<std::vector<Value>>(data).empty();
    if (isMap()) return !std::get<HashMapPtr>(data)->data.empty();
    if (isSet()) return !std::get<HashSetPtr>(data)->data.empty();
    return true;
}

std::string Value::formatDouble(double value) {
    std::ostringstream oss;
    oss << std::setprecision(15) << value;
    std::string out = oss.str();
    if (out.find('.') != std::string::npos) {
        while (!out.empty() && out.back() == '0') {
            out.pop_back();
        }
        if (!out.empty() && out.back() == '.') {
            out.pop_back();
        }
    }
    return out.empty() ? "0" : out;
}

std::string Value::toString() const {
    if (isInt()) {
        return std::to_string(std::get<std::int64_t>(data));
    }
    if (isFloat()) {
        return formatDouble(std::get<double>(data));
    }
    if (isString()) {
        return std::get<std::string>(data);
    }
    if (isMap()) {
        const auto& map = std::get<HashMapPtr>(data)->data;
        std::string out = "{";
        bool first = true;
        for (const auto& it : map) {
            if (!first) out += ", ";
            first = false;
            out += it.first.toString();
            out += ": ";
            out += it.second.toString();
        }
        out += "}";
        return out;
    }
    if (isSet()) {
        const auto& set = std::get<HashSetPtr>(data)->data;
        std::string out = "{";
        bool first = true;
        for (const auto& it : set) {
            if (!first) out += ", ";
            first = false;
            out += it.toString();
        }
        out += "}";
        return out;
    }
    if (isObject()) {
        const auto& obj = *std::get<ObjectPtr>(data);
        std::string out = obj.typeName + "{";
        bool first = true;
        for (const auto& it : obj.fields) {
            if (!first) out += ", ";
            first = false;
            out += it.first + ": " + it.second.toString();
        }
        out += "}";
        return out;
    }
    const auto& vec = std::get<std::vector<Value>>(data);
    std::string out = "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) out += ", ";
        out += vec[i].toString();
    }
    out += "]";
    return out;
}

struct FunctionDef {
    std::vector<std::string> params;
    std::vector<const ASTNode*> defaults; // nullptr for required params
    const ASTNode* body;
};

struct StructDef {
    std::unordered_map<std::string, FunctionDef> methods;
    std::vector<std::pair<std::string, const ASTNode*>> fields;
};

std::string executableDir;

Signal currentSignal = Signal::NONE;
Value signalValue; // holds return value when Signal::RETURN

ASTArena globalArena; // arena for all AST nodes — lives for program duration
std::vector<std::unordered_map<std::string, Value>> scopes(1);
std::unordered_map<std::string, FunctionDef> functions;
std::unordered_map<std::string, StructDef> structDefs;
int functionCallDepth = 0;
std::unordered_set<std::string> importedModules;

struct InputBuffer {
    std::string data;
    size_t pos = 0;
    bool enabled = false;
} inputBuffer;

void setInputBuffer(const std::string& input) {
    inputBuffer.data = input;
    inputBuffer.pos = 0;
    inputBuffer.enabled = true;
}

void clearInputBuffer() {
    inputBuffer.data.clear();
    inputBuffer.pos = 0;
    inputBuffer.enabled = false;
}

std::int64_t checkedMul(std::int64_t a, std::int64_t b, const std::string& context) {
    long long result = static_cast<long long>(a) * static_cast<long long>(b);
    if (result > std::numeric_limits<std::int64_t>::max() || result < std::numeric_limits<std::int64_t>::min()) {
        throw std::runtime_error("Integer overflow in " + context);
    }
    return static_cast<std::int64_t>(result);
}

Value* findVariable(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

Value getVariable(const std::string& name) {
    Value* value = findVariable(name);
    if (value != nullptr) {
        return *value;
    }
    throw std::runtime_error("Undefined variable: " + name);
}

void setVariable(const std::string& name, const Value& value) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            found->second = value;
            return;
        }
    }
    scopes.back()[name] = value;
}

void resetRuntime() {
    scopes.clear();
    scopes.push_back({});
    functions.clear();
    structDefs.clear();
    functionCallDepth = 0;
    importedModules.clear();
    clearInputBuffer();
    currentSignal = Signal::NONE;
    signalValue = Value();
    globalArena.clear();
}

std::string loadStdlibSource();
Value runSource(const std::string& source, bool printResult);

std::int64_t readInt() {
    if (!inputBuffer.enabled) {
        std::int64_t value = 0;
        if (!(std::cin >> value)) {
            throw std::runtime_error("readInt() failed to read an integer from stdin");
        }
        // consume trailing newline so readLine() works immediately after
        int ch = std::cin.peek();
        if (ch == '\r') { std::cin.get(); ch = std::cin.peek(); }
        if (ch == '\n') std::cin.get();
        return value;
    }

    const std::string& s = inputBuffer.data;
    size_t n = s.size();
    while (inputBuffer.pos < n && std::isspace(static_cast<unsigned char>(s[inputBuffer.pos]))) {
        inputBuffer.pos++;
    }
    if (inputBuffer.pos >= n) {
        throw std::runtime_error("readInt() reached end of input");
    }
    std::int64_t sign = 1;
    if (s[inputBuffer.pos] == '-') {
        sign = -1;
        inputBuffer.pos++;
    }
    std::int64_t value = 0;
    bool sawDigit = false;
    while (inputBuffer.pos < n && std::isdigit(static_cast<unsigned char>(s[inputBuffer.pos]))) {
        sawDigit = true;
        value = value * 10 + (s[inputBuffer.pos] - '0');
        inputBuffer.pos++;
    }
    if (!sawDigit) {
        throw std::runtime_error("readInt() expects an integer");
    }
    // consume trailing newline so readLine() works immediately after
    if (inputBuffer.pos < n && s[inputBuffer.pos] == '\r') inputBuffer.pos++;
    if (inputBuffer.pos < n && s[inputBuffer.pos] == '\n') inputBuffer.pos++;
    return value * sign;
}

double readFloat() {
    if (!inputBuffer.enabled) {
        double value = 0.0;
        if (!(std::cin >> value)) {
            throw std::runtime_error("readFloat() failed to read a float from stdin");
        }
        // consume trailing newline so readLine() works immediately after
        int ch = std::cin.peek();
        if (ch == '\r') { std::cin.get(); ch = std::cin.peek(); }
        if (ch == '\n') std::cin.get();
        return value;
    }

    const std::string& s = inputBuffer.data;
    size_t n = s.size();
    while (inputBuffer.pos < n && std::isspace(static_cast<unsigned char>(s[inputBuffer.pos]))) {
        inputBuffer.pos++;
    }
    if (inputBuffer.pos >= n) {
        throw std::runtime_error("readFloat() reached end of input");
    }

    size_t start = inputBuffer.pos;
    if (s[inputBuffer.pos] == '-') {
        inputBuffer.pos++;
    }
    bool sawDigit = false;
    bool sawDot = false;
    while (inputBuffer.pos < n) {
        char c = s[inputBuffer.pos];
        if (std::isdigit(static_cast<unsigned char>(c))) {
            sawDigit = true;
            inputBuffer.pos++;
            continue;
        }
        if (c == '.' && !sawDot) {
            sawDot = true;
            inputBuffer.pos++;
            continue;
        }
        break;
    }
    if (!sawDigit) {
        throw std::runtime_error("readFloat() expects a float");
    }
    double value = std::stod(s.substr(start, inputBuffer.pos - start));
    // consume trailing newline so readLine() works immediately after
    if (inputBuffer.pos < n && s[inputBuffer.pos] == '\r') inputBuffer.pos++;
    if (inputBuffer.pos < n && s[inputBuffer.pos] == '\n') inputBuffer.pos++;
    return value;
}
std::string readLine() {
    if (!inputBuffer.enabled) {
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("readLine() failed to read from stdin");
        }
        return line;
    }

    if (inputBuffer.pos >= inputBuffer.data.size()) {
        return "";
    }
    size_t start = inputBuffer.pos;
    size_t end = inputBuffer.data.find('\n', start);
    if (end == std::string::npos) {
        inputBuffer.pos = inputBuffer.data.size();
        return inputBuffer.data.substr(start);
    }
    inputBuffer.pos = end + 1;
    if (end > start && inputBuffer.data[end - 1] == '\r') {
        return inputBuffer.data.substr(start, end - start - 1);
    }
    return inputBuffer.data.substr(start, end - start);
}

HashKey toHashKey(const Value& v, const std::string& context) {
    if (v.isInt()) {
        return HashKey(v.asInt(context));
    }
    if (v.isString()) {
        return HashKey(v.asString(context));
    }
    throw std::runtime_error(context + " expects int or string key");
}

Value evaluate(const ASTNode* node) {
    switch(node->type) {
        case NodeType::INT: return Value(node->value);
        case NodeType::FLOAT: return Value(node->fvalue);
        case NodeType::STRING: return Value(node->name);
        case NodeType::MAP_LITERAL: {
            return Value(std::make_shared<HashMapWrapper>());
        }
        case NodeType::SET_LITERAL: {
            return Value(std::make_shared<HashSetWrapper>());
        }
        case NodeType::VECTOR: {
            std::vector<Value> items;
            for (const auto* child : node->children) {
                items.push_back(evaluate(child));
            }
            return Value(items);
        }
        case NodeType::INDEX: {
            Value container = evaluate(node->left);
            Value indexVal = evaluate(node->right);
            if (container.isMap()) {
                HashKey key = toHashKey(indexVal, "map index");
                auto& data = container.asMap("map index")->data;
                auto it = data.find(key);
                if (it == data.end()) {
                    return Value(static_cast<std::int64_t>(0));
                }
                return it->second;
            }
            if (container.isSet()) {
                HashKey key = toHashKey(indexVal, "set index");
                auto& data = container.asSet("set index")->data;
                return Value(static_cast<std::int64_t>(data.find(key) != data.end()));
            }
            std::int64_t index = indexVal.asInt("Index operator");
            if (container.isVector()) {
                const auto& vec = container.asVector("Index operator");
                if (index < 0) index += static_cast<std::int64_t>(vec.size());
                if (index < 0 || static_cast<size_t>(index) >= vec.size()) {
                    throw std::runtime_error("Vector index out of bounds");
                }
                return vec[static_cast<size_t>(index)];
            }
            if (container.isString()) {
                const auto& str = container.asString("Index operator");
                if (index < 0) index += static_cast<std::int64_t>(str.size());
                if (index < 0 || static_cast<size_t>(index) >= str.size()) {
                    throw std::runtime_error("String index out of bounds");
                }
                return Value(std::string(1, str[static_cast<size_t>(index)]));
            }
            throw std::runtime_error("Index operator expects vector, string, or map");
        }
        case NodeType::ADD: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isInt() && right.isInt()) {
                return Value(left.asInt("Addition") + right.asInt("Addition"));
            }
            if (left.isNumber() && right.isNumber()) {
                return Value(left.asDouble("Addition") + right.asDouble("Addition"));
            }
            if (left.isVector() && right.isVector()) {
                std::vector<Value> combined = left.asVector("Vector concat");
                const auto& rhs = right.asVector("Vector concat");
                combined.insert(combined.end(), rhs.begin(), rhs.end());
                return Value(combined);
            }
            return Value(left.toString() + right.toString());
        }
        case NodeType::SUB: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                if (left.isInt() && right.isInt()) {
                    return Value(left.asInt("Subtraction") - right.asInt("Subtraction"));
                }
                return Value(left.asDouble("Subtraction") - right.asDouble("Subtraction"));
            }
            throw std::runtime_error("Subtraction expects numeric values");
        }
        case NodeType::MUL: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                if (left.isInt() && right.isInt()) {
                    return Value(checkedMul(left.asInt("Multiplication"), right.asInt("Multiplication"), "multiplication"));
                }
                return Value(left.asDouble("Multiplication") * right.asDouble("Multiplication"));
            }
            // String repeat: "abc" * 3 or 3 * "abc"
            if (left.isString() && right.isInt()) {
                std::int64_t n = right.asInt("String repeat");
                if (n < 0) throw std::runtime_error("String repeat count cannot be negative");
                std::string result;
                const std::string& s = left.asString("String repeat");
                for (std::int64_t i = 0; i < n; ++i) result += s;
                return Value(result);
            }
            if (left.isInt() && right.isString()) {
                std::int64_t n = left.asInt("String repeat");
                if (n < 0) throw std::runtime_error("String repeat count cannot be negative");
                std::string result;
                const std::string& s = right.asString("String repeat");
                for (std::int64_t i = 0; i < n; ++i) result += s;
                return Value(result);
            }
            throw std::runtime_error("Multiplication expects numeric values or string * int");
        }
        case NodeType::DIV: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                if (left.isInt() && right.isInt()) {
                    std::int64_t divisor = right.asInt("Division");
                    if (divisor == 0) throw std::runtime_error("Division by zero");
                    return Value(left.asInt("Division") / divisor);
                }
                return Value(left.asDouble("Division") / right.asDouble("Division"));
            }
            throw std::runtime_error("Division expects numeric values");
        }
        case NodeType::MOD: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            std::int64_t divisor = right.asInt("Modulo");
            if (divisor == 0) throw std::runtime_error("Modulo by zero");
            return Value(left.asInt("Modulo") % divisor);
        }
        case NodeType::EXP: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isInt() && right.isInt()) {
                std::int64_t base = left.asInt("Exponentiation");
                std::int64_t exp = right.asInt("Exponentiation");
                if (exp < 0) {
                    throw std::runtime_error("Negative exponents are not supported for integers");
                }
                std::int64_t result = 1;
                for(std::int64_t i = 0; i < exp; ++i) {
                    result = checkedMul(result, base, "exponentiation");
                }
                return Value(result);
            }
            if (left.isNumber() && right.isNumber()) {
                return Value(std::pow(left.asDouble("Exponentiation"), right.asDouble("Exponentiation")));
            }
            throw std::runtime_error("Exponentiation expects numeric values");
        };
        case NodeType::BIT_AND: return Value(evaluate(node->left).asInt("Bitwise AND") & evaluate(node->right).asInt("Bitwise AND"));
        case NodeType::BIT_OR: return Value(evaluate(node->left).asInt("Bitwise OR") | evaluate(node->right).asInt("Bitwise OR"));
        case NodeType::BIT_XOR: return Value(evaluate(node->left).asInt("Bitwise XOR") ^ evaluate(node->right).asInt("Bitwise XOR"));
        case NodeType::BIT_SHIFT_LEFT: return Value(evaluate(node->left).asInt("Left shift") << evaluate(node->right).asInt("Left shift"));
        case NodeType::BIT_SHIFT_RIGHT: return Value(evaluate(node->left).asInt("Right shift") >> evaluate(node->right).asInt("Right shift"));
        case NodeType::BIT_NOT: return Value(~evaluate(node->left).asInt("Bitwise NOT"));
        case NodeType::NEG: {
            Value v = evaluate(node->left);
            if (v.isInt()) return Value(-v.asInt("Unary minus"));
            if (v.isFloat()) return Value(-v.asDouble("Unary minus"));
            throw std::runtime_error("Unary minus expects numeric value");
        }
        case NodeType::FACTORIAL: {
            std::int64_t val = evaluate(node->left).asInt("Factorial");
            if (val < 0) {
                throw std::runtime_error("Factorial is undefined for negative integers");
            }
            std::int64_t result = 1;
            for(std::int64_t i = 1; i <= val; ++i) {
                result = checkedMul(result, i, "factorial");
            }
            return Value(result);
        }
        case NodeType::IDENT: {
            if (node->name == "INF") {
                return Value(static_cast<std::int64_t>(1000000000000000007LL));
            }
            return getVariable(node->name);
        }
        case NodeType::ASSIGN: {
            Value value = evaluate(node->right);
            if (node->left->type == NodeType::IDENT) {
                std::string varName = node->left->name;
                setVariable(varName, value);
                return value;
            }
            if (node->left->type == NodeType::MEMBER) {
                Value base = evaluate(node->left->left);
                ObjectPtr obj = base.asObject("Member assignment");
                obj->fields[node->left->name] = value;
                return value;
            }
            if (node->left->type == NodeType::INDEX) {
                // Check if target is a map or set (shared_ptr makes evaluate work)
                Value container = evaluate(node->left->left);
                if (container.isMap()) {
                    Value keyVal = evaluate(node->left->right);
                    HashKey key = toHashKey(keyVal, "map index assignment");
                    container.asMap("map index assignment")->data[key] = value;
                    return value;
                }
                if (container.isSet()) {
                    Value keyVal = evaluate(node->left->right);
                    HashKey key = toHashKey(keyVal, "set index assignment");
                    if (value.truthy()) {
                        container.asSet("set index assignment")->data.insert(key);
                    } else {
                        container.asSet("set index assignment")->data.erase(key);
                    }
                    return value;
                }

                std::function<Value&(const ASTNode*)> resolveIndexTarget = [&](const ASTNode* indexNode) -> Value& {
                    if (indexNode->type != NodeType::INDEX) {
                        throw std::runtime_error("Invalid index assignment target");
                    }

                    std::int64_t index = evaluate(indexNode->right).asInt("Index assignment");

                    auto resolveIdx = [](std::int64_t idx, size_t sz) -> size_t {
                        if (idx < 0) idx += static_cast<std::int64_t>(sz);
                        if (idx < 0 || static_cast<size_t>(idx) >= sz) {
                            throw std::runtime_error("Vector index out of bounds");
                        }
                        return static_cast<size_t>(idx);
                    };

                    if (indexNode->left->type == NodeType::IDENT) {
                        Value* base = findVariable(indexNode->left->name);
                        if (base == nullptr) {
                            throw std::runtime_error("Undefined variable: " + indexNode->left->name);
                        }
                        std::vector<Value>& vec = base->asVectorRef("Index assignment");
                        return vec[resolveIdx(index, vec.size())];
                    }

                    if (indexNode->left->type == NodeType::MEMBER) {
                        Value baseVal = evaluate(indexNode->left->left);
                        ObjectPtr obj = baseVal.asObject("Index assignment");
                        auto it = obj->fields.find(indexNode->left->name);
                        if (it == obj->fields.end()) {
                            throw std::runtime_error("Undefined member: " + indexNode->left->name);
                        }
                        std::vector<Value>& vec = it->second.asVectorRef("Index assignment");
                        return vec[resolveIdx(index, vec.size())];
                    }

                    if (indexNode->left->type == NodeType::INDEX) {
                        Value& parent = resolveIndexTarget(indexNode->left);
                        std::vector<Value>& vec = parent.asVectorRef("Nested index assignment");
                        return vec[resolveIdx(index, vec.size())];
                    }

                    throw std::runtime_error("Index assignment target must be a vector variable");
                };

                Value& target = resolveIndexTarget(node->left);
                target = value;
                return value;
            }
            throw std::runtime_error("Invalid assignment target");
        }
        case NodeType::POST_INCREMENT:
        case NodeType::POST_DECREMENT: {
            std::string varName = node->left->name;
            Value old = getVariable(varName);
            std::int64_t delta = (node->type == NodeType::POST_INCREMENT) ? 1 : -1;
            if (old.isInt()) {
                setVariable(varName, Value(old.asInt("post-increment/decrement") + delta));
            } else {
                setVariable(varName, Value(old.asDouble("post-increment/decrement") + static_cast<double>(delta)));
            }
            return old;
        }
        case NodeType::BOOL: return Value(node->value);
        case NodeType::EQUALS: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                return Value(static_cast<std::int64_t>(left.asDouble("Comparison") == right.asDouble("Comparison")));
            }
            return Value(static_cast<std::int64_t>(left.toString() == right.toString()));
        }
        case NodeType::NOT_EQUAL: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                return Value(static_cast<std::int64_t>(left.asDouble("Comparison") != right.asDouble("Comparison")));
            }
            return Value(static_cast<std::int64_t>(left.toString() != right.toString()));
        }
        case NodeType::LESS: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                return Value(static_cast<std::int64_t>(left.asDouble("Comparison") < right.asDouble("Comparison")));
            }
            throw std::runtime_error("Comparison expects numeric values");
        }
        case NodeType::GREATER: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                return Value(static_cast<std::int64_t>(left.asDouble("Comparison") > right.asDouble("Comparison")));
            }
            throw std::runtime_error("Comparison expects numeric values");
        }
        case NodeType::LESS_EQUAL: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                return Value(static_cast<std::int64_t>(left.asDouble("Comparison") <= right.asDouble("Comparison")));
            }
            throw std::runtime_error("Comparison expects numeric values");
        }
        case NodeType::GREATER_EQUAL: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isNumber() && right.isNumber()) {
                return Value(static_cast<std::int64_t>(left.asDouble("Comparison") >= right.asDouble("Comparison")));
            }
            throw std::runtime_error("Comparison expects numeric values");
        }
        case NodeType::AND: return Value(static_cast<std::int64_t>(evaluate(node->left).truthy() && evaluate(node->right).truthy()));
        case NodeType::OR: return Value(static_cast<std::int64_t>(evaluate(node->left).truthy() || evaluate(node->right).truthy()));
        case NodeType::NOT: return Value(static_cast<std::int64_t>(!evaluate(node->left).truthy()));
        case NodeType::BLOCK: {
            Value result = Value(static_cast<std::int64_t>(0));
            for (const auto* stmt : node->children) {
                result = evaluate(stmt);
                if (currentSignal != Signal::NONE) return result;
            }
            return result;
        }
        case NodeType::IF: {
            if (node->children.size() < 2) {
                throw std::runtime_error("Malformed if node");
            }
            if (evaluate(node->children[0]).truthy()) {
                return evaluate(node->children[1]);
            }
            size_t i = 2;
            while (i + 1 < node->children.size()) {
                if (evaluate(node->children[i]).truthy()) {
                    return evaluate(node->children[i + 1]);
                }
                i += 2;
            }
            if (i < node->children.size()) {
                return evaluate(node->children[i]);
            }
            return Value(static_cast<std::int64_t>(0));
        }
        case NodeType::WHILE: {
            if (node->children.size() != 2) {
                throw std::runtime_error("Malformed while node");
            }
            Value result = Value(static_cast<std::int64_t>(0));
            while (evaluate(node->children[0]).truthy()) {
                result = evaluate(node->children[1]);
                if (currentSignal == Signal::BREAK) { currentSignal = Signal::NONE; break; }
                if (currentSignal == Signal::CONTINUE) { currentSignal = Signal::NONE; continue; }
                if (currentSignal == Signal::RETURN) return result;
            }
            return result;
        }
        case NodeType::FOR: {
            if (node->children.size() < 4 || node->children.size() > 5) {
                throw std::runtime_error("Malformed for node");
            }

            const ASTNode* varNode = node->children[0];
            if (varNode->type != NodeType::IDENT) {
                throw std::runtime_error("For loop variable must be an identifier");
            }

            std::int64_t start = evaluate(node->children[1]).asInt("For start");
            std::int64_t end = evaluate(node->children[2]).asInt("For end");
            std::int64_t step = 0;
            const ASTNode* body = nullptr;

            if (node->children.size() == 5) {
                step = evaluate(node->children[3]).asInt("For step");
                body = node->children[4];
            } else {
                step = (start <= end) ? 1 : -1;
                body = node->children[3];
            }

            if (step == 0) {
                throw std::runtime_error("For loop step cannot be 0");
            }

            Value result = Value(static_cast<std::int64_t>(0));
            bool skipVar = (varNode->name == "_");
            for (std::int64_t i = start; (step > 0) ? (i < end) : (i > end); i += step) {
                if (!skipVar) setVariable(varNode->name, Value(i));
                result = evaluate(body);
                if (currentSignal == Signal::BREAK) { currentSignal = Signal::NONE; break; }
                if (currentSignal == Signal::CONTINUE) { currentSignal = Signal::NONE; continue; }
                if (currentSignal == Signal::RETURN) return result;
            }
            return result;
        }
        case NodeType::FOR_EACH: {
            size_t numVars = static_cast<size_t>(node->value);
            if (numVars == 0) numVars = 1; // backward compat
            size_t collIdx = numVars;
            size_t bodyIdx = numVars + 1;
            if (node->children.size() < bodyIdx + 1) {
                throw std::runtime_error("Malformed for-each node");
            }
            Value collection = evaluate(node->children[collIdx]);
            const ASTNode* body = node->children[bodyIdx];
            Value result = Value(static_cast<std::int64_t>(0));

            if (numVars == 1) {
                // Single variable (original behavior)
                const ASTNode* varNode = node->children[0];
                bool skipVar = (varNode->name == "_");

                if (collection.isVector()) {
                    const auto& vec = collection.asVector("for-each");
                    for (size_t i = 0; i < vec.size(); ++i) {
                        if (!skipVar) setVariable(varNode->name, vec[i]);
                        result = evaluate(body);
                        if (currentSignal == Signal::BREAK) { currentSignal = Signal::NONE; break; }
                        if (currentSignal == Signal::CONTINUE) { currentSignal = Signal::NONE; continue; }
                        if (currentSignal == Signal::RETURN) return result;
                    }
                } else if (collection.isMap()) {
                    HashMapPtr hm = collection.asMap("for-each");
                    for (auto& [key, val] : hm->data) {
                        if (std::holds_alternative<std::int64_t>(key.data)) {
                            setVariable(varNode->name, Value(std::get<std::int64_t>(key.data)));
                        } else {
                            setVariable(varNode->name, Value(std::get<std::string>(key.data)));
                        }
                        result = evaluate(body);
                        if (currentSignal == Signal::BREAK) { currentSignal = Signal::NONE; break; }
                        if (currentSignal == Signal::CONTINUE) { currentSignal = Signal::NONE; continue; }
                        if (currentSignal == Signal::RETURN) return result;
                    }
                } else if (collection.isSet()) {
                    HashSetPtr hs = collection.asSet("for-each");
                    for (auto& key : hs->data) {
                        if (std::holds_alternative<std::int64_t>(key.data)) {
                            setVariable(varNode->name, Value(std::get<std::int64_t>(key.data)));
                        } else {
                            setVariable(varNode->name, Value(std::get<std::string>(key.data)));
                        }
                        result = evaluate(body);
                        if (currentSignal == Signal::BREAK) { currentSignal = Signal::NONE; break; }
                        if (currentSignal == Signal::CONTINUE) { currentSignal = Signal::NONE; continue; }
                        if (currentSignal == Signal::RETURN) return result;
                    }
                } else {
                    throw std::runtime_error("for-each requires a vector, map, or set");
                }
            } else {
                // Tuple unpacking
                if (collection.isVector()) {
                    const auto& vec = collection.asVector("for-each");
                    for (size_t i = 0; i < vec.size(); ++i) {
                        const Value& elem = vec[i];
                        if (elem.isVector()) {
                            const auto& inner = elem.asVector("for-each tuple unpack");
                            for (size_t j = 0; j < numVars; ++j) {
                                const std::string& vname = node->children[j]->name;
                                if (vname != "_") {
                                    setVariable(vname, j < inner.size() ? inner[j] : Value(static_cast<std::int64_t>(0)));
                                }
                            }
                        } else {
                            throw std::runtime_error("Tuple unpacking requires vector elements");
                        }
                        result = evaluate(body);
                        if (currentSignal == Signal::BREAK) { currentSignal = Signal::NONE; break; }
                        if (currentSignal == Signal::CONTINUE) { currentSignal = Signal::NONE; continue; }
                        if (currentSignal == Signal::RETURN) return result;
                    }
                } else if (collection.isMap()) {
                    if (numVars != 2) {
                        throw std::runtime_error("Map unpacking requires exactly 2 variables");
                    }
                    HashMapPtr hm = collection.asMap("for-each");
                    for (auto& [key, val] : hm->data) {
                        Value keyVal;
                        if (std::holds_alternative<std::int64_t>(key.data)) {
                            keyVal = Value(std::get<std::int64_t>(key.data));
                        } else {
                            keyVal = Value(std::get<std::string>(key.data));
                        }
                        if (node->children[0]->name != "_") setVariable(node->children[0]->name, keyVal);
                        if (node->children[1]->name != "_") setVariable(node->children[1]->name, val);
                        result = evaluate(body);
                        if (currentSignal == Signal::BREAK) { currentSignal = Signal::NONE; break; }
                        if (currentSignal == Signal::CONTINUE) { currentSignal = Signal::NONE; continue; }
                        if (currentSignal == Signal::RETURN) return result;
                    }
                } else {
                    throw std::runtime_error("Tuple unpacking for-each requires vector or map");
                }
            }
            return result;
        }
        case NodeType::BREAK: {
            currentSignal = Signal::BREAK;
            return Value(static_cast<std::int64_t>(0));
        }
        case NodeType::CONTINUE: {
            currentSignal = Signal::CONTINUE;
            return Value(static_cast<std::int64_t>(0));
        }
        case NodeType::TERNARY: {
            Value cond = evaluate(node->children[0]);
            if (cond.truthy()) {
                return evaluate(node->children[1]);
            }
            return evaluate(node->children[2]);
        }
        case NodeType::MULTI_ASSIGN: {
            // children: [name1, name2, ..., nameN, val1, val2, ..., valN]
            // node->value stores the name count
            size_t numNames = static_cast<size_t>(node->value);
            size_t numValues = node->children.size() - numNames;
            if (numValues != numNames) {
                throw std::runtime_error("Multi-assignment: expected " + std::to_string(numNames) +
                    " values, got " + std::to_string(numValues));
            }
            // Evaluate all values first (enables a, b = b, a swap)
            std::vector<Value> values;
            for (size_t i = numNames; i < node->children.size(); ++i) {
                values.push_back(evaluate(node->children[i]));
            }
            for (size_t i = 0; i < numNames; ++i) {
                const std::string& name = node->children[i]->name;
                if (name != "_") {
                    setVariable(name, values[i]);
                }
            }
            return values.back();
        }
        case NodeType::DESTRUCT_ASSIGN: {
            size_t numNames = static_cast<size_t>(node->value);
            Value rhs = evaluate(node->children[numNames]);
            const auto& vec = rhs.asVector("destructuring assignment");
            for (size_t i = 0; i < numNames; ++i) {
                const std::string& name = node->children[i]->name;
                if (name != "_") {
                    setVariable(name, i < vec.size() ? vec[i] : Value(static_cast<std::int64_t>(0)));
                }
            }
            return rhs;
        }
        case NodeType::FUNCTION_DEF: {
            if (node->children.empty()) {
                throw std::runtime_error("Malformed function definition");
            }
            std::vector<std::string> params;
            std::vector<const ASTNode*> defaults;
            for (size_t i = 0; i + 1 < node->children.size(); ++i) {
                if (node->children[i]->type == NodeType::ASSIGN) {
                    params.push_back(node->children[i]->left->name);
                    defaults.push_back(node->children[i]->right);
                } else if (node->children[i]->type == NodeType::IDENT) {
                    params.push_back(node->children[i]->name);
                    defaults.push_back(nullptr);
                } else {
                    throw std::runtime_error("Function parameter must be an identifier");
                }
            }
            functions[node->name] = FunctionDef{params, defaults, node->children.back()};
            return Value(static_cast<std::int64_t>(0));
        }
        case NodeType::STRUCT_DEF: {
            StructDef def;
            for (const auto* child : node->children) {
                if (child->type == NodeType::ASSIGN && child->left && child->left->type == NodeType::IDENT) {
                    def.fields.push_back({child->left->name, child->right});
                    continue;
                }
                if (child->type != NodeType::FUNCTION_DEF || child->children.empty()) {
                    throw std::runtime_error("Struct body must contain field or function definitions");
                }
                std::vector<std::string> params;
                std::vector<const ASTNode*> mdefaults;
                for (size_t i = 0; i + 1 < child->children.size(); ++i) {
                    if (child->children[i]->type == NodeType::ASSIGN) {
                        params.push_back(child->children[i]->left->name);
                        mdefaults.push_back(child->children[i]->right);
                    } else if (child->children[i]->type == NodeType::IDENT) {
                        params.push_back(child->children[i]->name);
                        mdefaults.push_back(nullptr);
                    } else {
                        throw std::runtime_error("Struct method parameter must be an identifier");
                    }
                }
                def.methods[child->name] = FunctionDef{params, mdefaults, child->children.back()};
            }
            structDefs[node->name] = def;
            return Value(static_cast<std::int64_t>(0));
        }
        case NodeType::MEMBER: {
            Value base = evaluate(node->left);
            ObjectPtr obj = base.asObject("Member access");
            auto it = obj->fields.find(node->name);
            if (it == obj->fields.end()) {
                throw std::runtime_error("Undefined member: " + node->name);
            }
            return it->second;
        }
        case NodeType::METHOD_CALL: {
            Value base = evaluate(node->left);
            if (base.isMap()) {
                HashMapPtr map = base.asMap("Map method");
                const std::string& method = node->name;
                if (method == "set") {
                    if (node->children.size() != 2) {
                        throw std::runtime_error("map.set expects 2 arguments");
                    }
                    HashKey key = toHashKey(evaluate(node->children[0]), "map.set");
                    Value val = evaluate(node->children[1]);
                    map->data[key] = val;
                    return Value(static_cast<std::int64_t>(1));
                }
                if (method == "get") {
                    if (node->children.size() < 1 || node->children.size() > 2) {
                        throw std::runtime_error("map.get expects 1 or 2 arguments");
                    }
                    HashKey key = toHashKey(evaluate(node->children[0]), "map.get");
                    Value def = node->children.size() == 2 ? evaluate(node->children[1]) : Value(static_cast<std::int64_t>(0));
                    auto it = map->data.find(key);
                    if (it == map->data.end()) return def;
                    return it->second;
                }
                if (method == "has") {
                    if (node->children.size() != 1) {
                        throw std::runtime_error("map.has expects 1 argument");
                    }
                    HashKey key = toHashKey(evaluate(node->children[0]), "map.has");
                    return Value(static_cast<std::int64_t>(map->data.find(key) != map->data.end()));
                }
                if (method == "remove") {
                    if (node->children.size() != 1) {
                        throw std::runtime_error("map.remove expects 1 argument");
                    }
                    HashKey key = toHashKey(evaluate(node->children[0]), "map.remove");
                    return Value(static_cast<std::int64_t>(map->data.erase(key) > 0));
                }
                if (method == "size") {
                    if (!node->children.empty()) {
                        throw std::runtime_error("map.size expects 0 arguments");
                    }
                    return Value(static_cast<std::int64_t>(map->data.size()));
                }
                if (method == "clear") {
                    if (!node->children.empty()) {
                        throw std::runtime_error("map.clear expects 0 arguments");
                    }
                    map->data.clear();
                    return Value(static_cast<std::int64_t>(0));
                }
                throw std::runtime_error("Unknown map method: " + method);
            }
            if (base.isSet()) {
                HashSetPtr set = base.asSet("Set method");
                const std::string& method = node->name;
                if (method == "add") {
                    if (node->children.size() != 1) {
                        throw std::runtime_error("set.add expects 1 argument");
                    }
                    HashKey key = toHashKey(evaluate(node->children[0]), "set.add");
                    set->data.insert(key);
                    return Value(static_cast<std::int64_t>(1));
                }
                if (method == "has") {
                    if (node->children.size() != 1) {
                        throw std::runtime_error("set.has expects 1 argument");
                    }
                    HashKey key = toHashKey(evaluate(node->children[0]), "set.has");
                    return Value(static_cast<std::int64_t>(set->data.find(key) != set->data.end()));
                }
                if (method == "remove") {
                    if (node->children.size() != 1) {
                        throw std::runtime_error("set.remove expects 1 argument");
                    }
                    HashKey key = toHashKey(evaluate(node->children[0]), "set.remove");
                    return Value(static_cast<std::int64_t>(set->data.erase(key) > 0));
                }
                if (method == "size") {
                    if (!node->children.empty()) {
                        throw std::runtime_error("set.size expects 0 arguments");
                    }
                    return Value(static_cast<std::int64_t>(set->data.size()));
                }
                if (method == "clear") {
                    if (!node->children.empty()) {
                        throw std::runtime_error("set.clear expects 0 arguments");
                    }
                    set->data.clear();
                    return Value(static_cast<std::int64_t>(0));
                }
                throw std::runtime_error("Unknown set method: " + method);
            }
            ObjectPtr obj = base.asObject("Method call");
            auto itDef = structDefs.find(obj->typeName);
            if (itDef == structDefs.end()) {
                throw std::runtime_error("Undefined struct type: " + obj->typeName);
            }
            auto itMethod = itDef->second.methods.find(node->name);
            if (itMethod == itDef->second.methods.end()) {
                throw std::runtime_error("Undefined method: " + node->name);
            }
            const FunctionDef& fn = itMethod->second;
            size_t reqParams = 0;
            for (size_t i = 0; i < fn.defaults.size(); ++i) {
                if (fn.defaults[i] == nullptr) reqParams = i + 1;
            }
            if (node->children.size() < reqParams || node->children.size() > fn.params.size()) {
                throw std::runtime_error("Method '" + node->name + "' expects " +
                    std::to_string(reqParams) +
                    (reqParams != fn.params.size() ? "-" + std::to_string(fn.params.size()) : "") +
                    " args, got " + std::to_string(node->children.size()));
            }
            std::vector<Value> argValues;
            argValues.reserve(fn.params.size());
            for (const auto* arg : node->children) {
                argValues.push_back(evaluate(arg));
            }
            for (size_t i = argValues.size(); i < fn.params.size(); ++i) {
                argValues.push_back(evaluate(fn.defaults[i]));
            }
            scopes.push_back({});
            scopes.back()["self"] = base;
            for (size_t i = 0; i < fn.params.size(); ++i) {
                scopes.back()[fn.params[i]] = argValues[i];
            }
            functionCallDepth++;
            Value result = evaluate(fn.body);
            functionCallDepth--;
            scopes.pop_back();
            if (currentSignal == Signal::RETURN) {
                currentSignal = Signal::NONE;
                return signalValue;
            }
            return result;
        }
        case NodeType::LAMBDA: {
            // Register lambda as a function and return its name
            if (node->children.empty()) {
                throw std::runtime_error("Malformed lambda");
            }
            std::vector<std::string> params;
            std::vector<const ASTNode*> defaults;
            for (size_t i = 0; i + 1 < node->children.size(); ++i) {
                if (node->children[i]->type == NodeType::ASSIGN) {
                    params.push_back(node->children[i]->left->name);
                    defaults.push_back(node->children[i]->right);
                } else {
                    params.push_back(node->children[i]->name);
                    defaults.push_back(nullptr);
                }
            }
            functions[node->name] = FunctionDef{params, defaults, node->children.back()};
            return Value(node->name);
        }
        case NodeType::SLICE: {
            Value container = evaluate(node->left);
            // children[0] = start, children[1] = end, children[2] = step (can be nullptr)
            bool hasStart = (node->children.size() > 0 && node->children[0] != nullptr);
            bool hasEnd = (node->children.size() > 1 && node->children[1] != nullptr);
            bool hasStep = (node->children.size() > 2 && node->children[2] != nullptr);

            if (container.isVector()) {
                const auto& vec = container.asVector("slice");
                std::int64_t len = static_cast<std::int64_t>(vec.size());
                std::int64_t step = hasStep ? evaluate(node->children[2]).asInt("slice step") : 1;
                if (step == 0) throw std::runtime_error("Slice step cannot be 0");

                std::int64_t start, end;
                if (step > 0) {
                    start = hasStart ? evaluate(node->children[0]).asInt("slice start") : 0;
                    end = hasEnd ? evaluate(node->children[1]).asInt("slice end") : len;
                } else {
                    start = hasStart ? evaluate(node->children[0]).asInt("slice start") : len - 1;
                    end = hasEnd ? evaluate(node->children[1]).asInt("slice end") : -(len + 1);
                }
                // Normalize negatives
                if (start < 0) start += len;
                if (end < 0) end += len;
                // Clamp
                if (step > 0) {
                    if (start < 0) start = 0;
                    if (start > len) start = len;
                    if (end < 0) end = 0;
                    if (end > len) end = len;
                } else {
                    if (start < -1) start = -1;
                    if (start >= len) start = len - 1;
                    if (end < -1) end = -1;
                    if (end >= len) end = len - 1;
                }
                std::vector<Value> result;
                if (step > 0) {
                    for (std::int64_t i = start; i < end; i += step)
                        result.push_back(vec[static_cast<size_t>(i)]);
                } else {
                    for (std::int64_t i = start; i > end; i += step)
                        result.push_back(vec[static_cast<size_t>(i)]);
                }
                return Value(result);
            }
            if (container.isString()) {
                const auto& str = container.asString("slice");
                std::int64_t len = static_cast<std::int64_t>(str.size());
                std::int64_t step = hasStep ? evaluate(node->children[2]).asInt("slice step") : 1;
                if (step == 0) throw std::runtime_error("Slice step cannot be 0");

                std::int64_t start, end;
                if (step > 0) {
                    start = hasStart ? evaluate(node->children[0]).asInt("slice start") : 0;
                    end = hasEnd ? evaluate(node->children[1]).asInt("slice end") : len;
                } else {
                    start = hasStart ? evaluate(node->children[0]).asInt("slice start") : len - 1;
                    end = hasEnd ? evaluate(node->children[1]).asInt("slice end") : -(len + 1);
                }
                if (start < 0) start += len;
                if (end < 0) end += len;
                if (step > 0) {
                    if (start < 0) start = 0;
                    if (start > len) start = len;
                    if (end < 0) end = 0;
                    if (end > len) end = len;
                } else {
                    if (start < -1) start = -1;
                    if (start >= len) start = len - 1;
                    if (end < -1) end = -1;
                    if (end >= len) end = len - 1;
                }
                std::string result;
                if (step > 0) {
                    for (std::int64_t i = start; i < end; i += step)
                        result += str[static_cast<size_t>(i)];
                } else {
                    for (std::int64_t i = start; i > end; i += step)
                        result += str[static_cast<size_t>(i)];
                }
                return Value(result);
            }
            throw std::runtime_error("Slice requires vector or string");
        }
        case NodeType::FUNCTION_CALL: {
            if (node->name == "map") {
                if (!node->children.empty()) {
                    throw std::runtime_error("map() expects 0 arguments");
                }
                return Value(std::make_shared<HashMapWrapper>());
            }
            if (node->name == "set") {
                if (!node->children.empty()) {
                    throw std::runtime_error("set() expects 0 arguments");
                }
                return Value(std::make_shared<HashSetWrapper>());
            }
            auto itStruct = structDefs.find(node->name);
            if (itStruct != structDefs.end()) {
                if (!node->children.empty()) {
                    throw std::runtime_error("Struct constructors do not take arguments yet");
                }
                ObjectPtr obj = std::make_shared<ObjectData>();
                obj->typeName = node->name;
                for (const auto& field : itStruct->second.fields) {
                    obj->fields[field.first] = evaluate(field.second);
                }
                Value objVal(obj);
                auto itInit = itStruct->second.methods.find("init");
                if (itInit != itStruct->second.methods.end()) {
                    const FunctionDef& initFn = itInit->second;
                    size_t initRequired = 0;
                    for (size_t i = 0; i < initFn.defaults.size(); ++i) {
                        if (initFn.defaults[i] == nullptr) initRequired = i + 1;
                    }
                    if (initRequired > 0) {
                        return objVal;
                    }
                    scopes.push_back({});
                    scopes.back()["self"] = objVal;
                    functionCallDepth++;
                    (void)evaluate(initFn.body);
                    functionCallDepth--;
                    scopes.pop_back();
                    if (currentSignal == Signal::RETURN) currentSignal = Signal::NONE;
                }
                return objVal;
            }
            if (node->name == "len") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("len() expects 1 argument");
                }
                Value v = evaluate(node->children[0]);
                if (v.isString()) return Value(static_cast<std::int64_t>(v.asString("len").size()));
                if (v.isVector()) return Value(static_cast<std::int64_t>(v.asVector("len").size()));
                throw std::runtime_error("len() expects string or vector");
            }

            if (node->name == "push") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("push() expects 2 arguments");
                }
                std::function<std::vector<Value>&(const ASTNode*)> resolveVectorTarget = [&](const ASTNode* targetNode) -> std::vector<Value>& {
                    if (targetNode->type == NodeType::IDENT) {
                        Value* target = findVariable(targetNode->name);
                        if (target == nullptr) {
                            throw std::runtime_error("Undefined variable: " + targetNode->name);
                        }
                        return target->asVectorRef("push");
                    }
                    if (targetNode->type == NodeType::MEMBER) {
                        Value base = evaluate(targetNode->left);
                        ObjectPtr obj = base.asObject("push");
                        auto it = obj->fields.find(targetNode->name);
                        if (it == obj->fields.end()) {
                            throw std::runtime_error("Undefined member: " + targetNode->name);
                        }
                        return it->second.asVectorRef("push");
                    }
                    if (targetNode->type == NodeType::INDEX) {
                        // resolve the parent container and index into it
                        std::function<Value&(const ASTNode*)> resolveIndexRef = [&](const ASTNode* idxNode) -> Value& {
                            std::int64_t index = evaluate(idxNode->right).asInt("push index");
                            if (index < 0) throw std::runtime_error("push() negative index");
                            if (idxNode->left->type == NodeType::IDENT) {
                                Value* base = findVariable(idxNode->left->name);
                                if (base == nullptr) throw std::runtime_error("Undefined variable: " + idxNode->left->name);
                                std::vector<Value>& vec = base->asVectorRef("push");
                                if (static_cast<size_t>(index) >= vec.size()) throw std::runtime_error("push() index out of bounds");
                                return vec[static_cast<size_t>(index)];
                            }
                            if (idxNode->left->type == NodeType::INDEX) {
                                Value& parent = resolveIndexRef(idxNode->left);
                                std::vector<Value>& vec = parent.asVectorRef("push");
                                if (static_cast<size_t>(index) >= vec.size()) throw std::runtime_error("push() index out of bounds");
                                return vec[static_cast<size_t>(index)];
                            }
                            if (idxNode->left->type == NodeType::MEMBER) {
                                Value base = evaluate(idxNode->left->left);
                                ObjectPtr obj = base.asObject("push");
                                auto it = obj->fields.find(idxNode->left->name);
                                if (it == obj->fields.end()) throw std::runtime_error("Undefined member: " + idxNode->left->name);
                                std::vector<Value>& vec = it->second.asVectorRef("push");
                                if (static_cast<size_t>(index) >= vec.size()) throw std::runtime_error("push() index out of bounds");
                                return vec[static_cast<size_t>(index)];
                            }
                            throw std::runtime_error("push() invalid index target");
                        };
                        Value& element = resolveIndexRef(targetNode);
                        return element.asVectorRef("push");
                    }
                    throw std::runtime_error("push() first argument must be a vector variable, member, or indexed element");
                };
                std::vector<Value>& vec = resolveVectorTarget(node->children[0]);
                vec.push_back(evaluate(node->children[1]));
                return Value(static_cast<std::int64_t>(vec.size()));
            }

            if (node->name == "pop") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("pop() expects 1 argument");
                }
                auto resolveVectorTarget = [&](const ASTNode* targetNode) -> std::vector<Value>& {
                    if (targetNode->type == NodeType::IDENT) {
                        Value* target = findVariable(targetNode->name);
                        if (target == nullptr) {
                            throw std::runtime_error("Undefined variable: " + targetNode->name);
                        }
                        return target->asVectorRef("pop");
                    }
                    if (targetNode->type == NodeType::MEMBER) {
                        Value base = evaluate(targetNode->left);
                        ObjectPtr obj = base.asObject("pop");
                        auto it = obj->fields.find(targetNode->name);
                        if (it == obj->fields.end()) {
                            throw std::runtime_error("Undefined member: " + targetNode->name);
                        }
                        return it->second.asVectorRef("pop");
                    }
                    throw std::runtime_error("pop() argument must be a vector variable or member");
                };
                std::vector<Value>& vec = resolveVectorTarget(node->children[0]);
                if (vec.empty()) {
                    throw std::runtime_error("pop() on empty vector");
                }
                Value out = vec.back();
                vec.pop_back();
                return out;
            }

            if (node->name == "read" || node->name == "readInt") {
                if (node->children.empty()) {
                    return Value(readInt());
                }
                if (node->children.size() == 1) {
                    Value arg = evaluate(node->children[0]);
                    if (arg.isInt()) {
                        // read(n) -> read n integers into a new vector
                        std::int64_t n = arg.asInt("read()");
                        if (n < 0) throw std::runtime_error("read() count must be non-negative");
                        std::vector<Value> result;
                        result.reserve(n);
                        for (std::int64_t i = 0; i < n; ++i) {
                            result.push_back(Value(readInt()));
                        }
                        return Value(result);
                    }
                    if (arg.isVector()) {
                        // read(v) -> read len(v) integers into v, return v
                        auto resolveVecForRead = [&](const ASTNode* targetNode) -> std::vector<Value>& {
                            if (targetNode->type == NodeType::IDENT) {
                                Value* target = findVariable(targetNode->name);
                                if (target == nullptr) {
                                    throw std::runtime_error("Undefined variable: " + targetNode->name);
                                }
                                return target->asVectorRef("read");
                            }
                            if (targetNode->type == NodeType::MEMBER) {
                                Value base = evaluate(targetNode->left);
                                ObjectPtr obj = base.asObject("read");
                                auto it = obj->fields.find(targetNode->name);
                                if (it == obj->fields.end()) {
                                    throw std::runtime_error("Undefined member: " + targetNode->name);
                                }
                                return it->second.asVectorRef("read");
                            }
                            throw std::runtime_error("read() vector argument must be a variable or member");
                        };
                        std::vector<Value>& vec = resolveVecForRead(node->children[0]);
                        for (size_t i = 0; i < vec.size(); ++i) {
                            vec[i] = Value(readInt());
                        }
                        return Value(vec);
                    }
                    throw std::runtime_error("read() argument must be an integer count or a vector");
                }
                throw std::runtime_error("read() expects 0 or 1 arguments");
            }

            if (node->name == "readFloat") {
                if (node->children.empty()) {
                    return Value(readFloat());
                }
                if (node->children.size() == 1) {
                    Value arg = evaluate(node->children[0]);
                    if (arg.isInt()) {
                        std::int64_t n = arg.asInt("readFloat()");
                        if (n < 0) throw std::runtime_error("readFloat() count must be non-negative");
                        std::vector<Value> result;
                        result.reserve(n);
                        for (std::int64_t i = 0; i < n; ++i) {
                            result.push_back(Value(readFloat()));
                        }
                        return Value(result);
                    }
                    if (arg.isVector()) {
                        auto resolveVecForRead = [&](const ASTNode* targetNode) -> std::vector<Value>& {
                            if (targetNode->type == NodeType::IDENT) {
                                Value* target = findVariable(targetNode->name);
                                if (target == nullptr) {
                                    throw std::runtime_error("Undefined variable: " + targetNode->name);
                                }
                                return target->asVectorRef("readFloat");
                            }
                            if (targetNode->type == NodeType::MEMBER) {
                                Value base = evaluate(targetNode->left);
                                ObjectPtr obj = base.asObject("readFloat");
                                auto it = obj->fields.find(targetNode->name);
                                if (it == obj->fields.end()) {
                                    throw std::runtime_error("Undefined member: " + targetNode->name);
                                }
                                return it->second.asVectorRef("readFloat");
                            }
                            throw std::runtime_error("readFloat() vector argument must be a variable or member");
                        };
                        std::vector<Value>& vec = resolveVecForRead(node->children[0]);
                        for (size_t i = 0; i < vec.size(); ++i) {
                            vec[i] = Value(readFloat());
                        }
                        return Value(vec);
                    }
                    throw std::runtime_error("readFloat() argument must be an integer count or a vector");
                }
                throw std::runtime_error("readFloat() expects 0 or 1 arguments");
            }

            if (node->name == "readLine") {
                if (!node->children.empty()) {
                    throw std::runtime_error("readLine() expects 0 arguments");
                }
                return Value(readLine());
            }

            if (node->name == "print") {
                if (node->children.empty()) {
                    throw std::runtime_error("print() expects at least 1 argument");
                }
                for (size_t i = 0; i < node->children.size(); ++i) {
                    if (i > 0) {
                        std::cout << " ";
                    }
                    std::cout << evaluate(node->children[i]).toString();
                }
                return Value(static_cast<std::int64_t>(0));
            }

            if (node->name == "println") {
                for (size_t i = 0; i < node->children.size(); ++i) {
                    if (i > 0) {
                        std::cout << " ";
                    }
                    std::cout << evaluate(node->children[i]).toString();
                }
                std::cout << "\n";
                return Value(static_cast<std::int64_t>(0));
            }

            if (node->name == "ord") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("ord() expects 1 argument");
                }
                Value v = evaluate(node->children[0]);
                std::string s = v.asString("ord()");
                if (s.empty()) {
                    throw std::runtime_error("ord() called on empty string");
                }
                return Value(static_cast<std::int64_t>(static_cast<unsigned char>(s[0])));
            }

            if (node->name == "chr") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("chr() expects 1 argument");
                }
                Value v = evaluate(node->children[0]);
                std::int64_t code = v.asInt("chr()");
                return Value(std::string(1, static_cast<char>(code)));
            }

            if (node->name == "sort" || node->name == "sortdec") {
                if (node->children.size() < 1 || node->children.size() > 2) {
                    throw std::runtime_error(node->name + "() expects 1 or 2 arguments");
                }
                // resolve the vector target in-place (same pattern as push/pop)
                auto resolveVecTarget = [&](const ASTNode* targetNode) -> std::vector<Value>& {
                    if (targetNode->type == NodeType::IDENT) {
                        Value* target = findVariable(targetNode->name);
                        if (target == nullptr) {
                            throw std::runtime_error("Undefined variable: " + targetNode->name);
                        }
                        return target->asVectorRef(node->name);
                    }
                    if (targetNode->type == NodeType::MEMBER) {
                        Value base = evaluate(targetNode->left);
                        ObjectPtr obj = base.asObject(node->name);
                        auto it = obj->fields.find(targetNode->name);
                        if (it == obj->fields.end()) {
                            throw std::runtime_error("Undefined member: " + targetNode->name);
                        }
                        return it->second.asVectorRef(node->name);
                    }
                    throw std::runtime_error(node->name + "() first argument must be a vector variable or member");
                };
                std::vector<Value>& vec = resolveVecTarget(node->children[0]);

                // determine comparison function
                const FunctionDef* userCmp = nullptr;
                if (node->children.size() == 2) {
                    const ASTNode* cmpNode = node->children[1];
                    std::string cmpName;
                    // Try direct function name first (backward compat)
                    if (cmpNode->type == NodeType::IDENT && functions.find(cmpNode->name) != functions.end()) {
                        cmpName = cmpNode->name;
                    } else {
                        // Evaluate expression (supports lambdas, variables)
                        Value cmpVal = evaluate(cmpNode);
                        cmpName = cmpVal.asString(node->name + "() comparator");
                    }
                    auto fit = functions.find(cmpName);
                    if (fit == functions.end()) {
                        throw std::runtime_error("Undefined comparator function: " + cmpName);
                    }
                    if (fit->second.params.size() != 2) {
                        throw std::runtime_error("Comparator function must take exactly 2 parameters");
                    }
                    userCmp = &fit->second;
                }

                // default comparison: supports int, float, string, vectors (lexicographic)
                // returns negative if a < b, 0 if equal, positive if a > b
                std::function<std::int64_t(const Value&, const Value&)> defaultCmp =
                    [&](const Value& a, const Value& b) -> std::int64_t {
                    if (a.isNumber() && b.isNumber()) {
                        double da = a.isFloat() ? std::get<double>(a.data)
                                                : static_cast<double>(std::get<std::int64_t>(a.data));
                        double db = b.isFloat() ? std::get<double>(b.data)
                                                : static_cast<double>(std::get<std::int64_t>(b.data));
                        if (da < db) return -1;
                        if (da > db) return 1;
                        return 0;
                    }
                    if (a.isString() && b.isString()) {
                        int c = std::get<std::string>(a.data).compare(std::get<std::string>(b.data));
                        if (c < 0) return -1;
                        if (c > 0) return 1;
                        return 0;
                    }
                    if (a.isVector() && b.isVector()) {
                        const auto& va = std::get<std::vector<Value>>(a.data);
                        const auto& vb = std::get<std::vector<Value>>(b.data);
                        size_t minLen = std::min(va.size(), vb.size());
                        for (size_t i = 0; i < minLen; i++) {
                            std::int64_t r = defaultCmp(va[i], vb[i]);
                            if (r != 0) return r;
                        }
                        if (va.size() < vb.size()) return -1;
                        if (va.size() > vb.size()) return 1;
                        return 0;
                    }
                    throw std::runtime_error("sort(): cannot compare values of different or unsortable types");
                };

                // call user comparator: returns the int result
                auto callUserCmp = [&](const Value& a, const Value& b) -> std::int64_t {
                    scopes.push_back({});
                    scopes.back()[userCmp->params[0]] = a;
                    scopes.back()[userCmp->params[1]] = b;
                    functionCallDepth++;
                    Value result = evaluate(userCmp->body);
                    functionCallDepth--;
                    scopes.pop_back();
                    if (currentSignal == Signal::RETURN) {
                        currentSignal = Signal::NONE;
                        result = signalValue;
                    }
                    if (!result.isNumber()) {
                        throw std::runtime_error("Comparator must return a number");
                    }
                    return result.isInt() ? std::get<std::int64_t>(result.data)
                                          : static_cast<std::int64_t>(std::get<double>(result.data));
                };

                bool descending = (node->name == "sortdec");

                // comparison wrapper
                auto compare = [&](const Value& a, const Value& b) -> bool {
                    std::int64_t r;
                    if (userCmp) {
                        r = callUserCmp(a, b);
                    } else {
                        r = defaultCmp(a, b);
                    }
                    return descending ? (r > 0) : (r < 0);
                };

                // quicksort implementation
                std::function<void(std::vector<Value>&, int, int)> quicksort =
                    [&](std::vector<Value>& arr, int lo, int hi) {
                    if (lo >= hi) return;
                    if (hi - lo == 1) {
                        if (compare(arr[hi], arr[lo])) std::swap(arr[lo], arr[hi]);
                        return;
                    }
                    // median-of-three pivot selection
                    int mid = lo + (hi - lo) / 2;
                    if (compare(arr[hi], arr[lo])) std::swap(arr[lo], arr[hi]);
                    if (compare(arr[mid], arr[lo])) std::swap(arr[lo], arr[mid]);
                    if (compare(arr[hi], arr[mid])) std::swap(arr[mid], arr[hi]);
                    Value pivot = arr[mid];
                    std::swap(arr[mid], arr[hi - 1]);
                    int i = lo, j = hi - 1;
                    while (true) {
                        while (compare(arr[++i], pivot)) {}
                        while (compare(pivot, arr[--j])) {}
                        if (i >= j) break;
                        std::swap(arr[i], arr[j]);
                    }
                    std::swap(arr[i], arr[hi - 1]);
                    quicksort(arr, lo, i - 1);
                    quicksort(arr, i + 1, hi);
                };

                if (vec.size() > 1) {
                    quicksort(vec, 0, static_cast<int>(vec.size()) - 1);
                }

                return Value(static_cast<std::int64_t>(0));
            }

            if (node->name == "reverse") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("reverse() expects 1 argument");
                }
                auto resolveVecTarget = [&](const ASTNode* targetNode) -> std::vector<Value>& {
                    if (targetNode->type == NodeType::IDENT) {
                        Value* target = findVariable(targetNode->name);
                        if (target == nullptr) {
                            throw std::runtime_error("Undefined variable: " + targetNode->name);
                        }
                        return target->asVectorRef("reverse");
                    }
                    if (targetNode->type == NodeType::MEMBER) {
                        Value base = evaluate(targetNode->left);
                        ObjectPtr obj = base.asObject("reverse");
                        auto it = obj->fields.find(targetNode->name);
                        if (it == obj->fields.end()) {
                            throw std::runtime_error("Undefined member: " + targetNode->name);
                        }
                        return it->second.asVectorRef("reverse");
                    }
                    throw std::runtime_error("reverse() argument must be a vector variable or member");
                };
                std::vector<Value>& vec = resolveVecTarget(node->children[0]);
                std::reverse(vec.begin(), vec.end());
                return Value(static_cast<std::int64_t>(0));
            }

            if (node->name == "parseInt") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("parseInt() expects 1 argument");
                }
                Value v = evaluate(node->children[0]);
                std::string s = v.asString("parseInt()");
                try {
                    return Value(static_cast<std::int64_t>(std::stoll(s)));
                } catch (...) {
                    throw std::runtime_error("parseInt() failed to parse: " + s);
                }
            }

            if (node->name == "str") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("str() expects 1 argument");
                }
                Value v = evaluate(node->children[0]);
                return Value(v.toString());
            }

            if (node->name == "int") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("int() expects 1 argument");
                }
                Value v = evaluate(node->children[0]);
                if (v.isInt()) return v;
                if (v.isFloat()) return Value(static_cast<std::int64_t>(std::get<double>(v.data)));
                if (v.isString()) {
                    const std::string& s = std::get<std::string>(v.data);
                    try {
                        return Value(static_cast<std::int64_t>(std::stoll(s)));
                    } catch (...) {
                        throw std::runtime_error("int() failed to parse: " + s);
                    }
                }
                throw std::runtime_error("int() cannot convert this type to integer");
            }

            if (node->name == "float") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("float() expects 1 argument");
                }
                Value v = evaluate(node->children[0]);
                if (v.isFloat()) return v;
                if (v.isInt()) return Value(static_cast<double>(std::get<std::int64_t>(v.data)));
                if (v.isString()) {
                    const std::string& s = std::get<std::string>(v.data);
                    try {
                        return Value(std::stod(s));
                    } catch (...) {
                        throw std::runtime_error("float() failed to parse: " + s);
                    }
                }
                throw std::runtime_error("float() cannot convert this type to float");
            }

            if (node->name == "split") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("split() expects 2 arguments");
                }
                Value sv = evaluate(node->children[0]);
                Value dv = evaluate(node->children[1]);
                const std::string& s = sv.asString("split()");
                const std::string& delim = dv.asString("split()");
                std::vector<Value> parts;
                if (delim.empty()) {
                    // split each character
                    for (char c : s) parts.push_back(Value(std::string(1, c)));
                } else {
                    size_t start = 0, end;
                    while ((end = s.find(delim, start)) != std::string::npos) {
                        parts.push_back(Value(s.substr(start, end - start)));
                        start = end + delim.size();
                    }
                    parts.push_back(Value(s.substr(start)));
                }
                return Value(parts);
            }

            if (node->name == "join") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("join() expects 2 arguments");
                }
                Value vv = evaluate(node->children[0]);
                Value dv = evaluate(node->children[1]);
                const auto& vec = vv.asVector("join()");
                const std::string& delim = dv.asString("join()");
                std::string result;
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (i > 0) result += delim;
                    result += vec[i].toString();
                }
                return Value(result);
            }

            if (node->name == "find") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("find() expects 2 arguments");
                }
                Value container = evaluate(node->children[0]);
                Value target = evaluate(node->children[1]);
                if (container.isVector()) {
                    const auto& vec = container.asVector("find()");
                    for (size_t i = 0; i < vec.size(); ++i) {
                        if (vec[i].toString() == target.toString()) {
                            return Value(static_cast<std::int64_t>(i));
                        }
                    }
                    return Value(static_cast<std::int64_t>(-1));
                }
                if (container.isString()) {
                    const std::string& s = container.asString("find()");
                    const std::string& t = target.asString("find()");
                    size_t pos = s.find(t);
                    if (pos == std::string::npos) return Value(static_cast<std::int64_t>(-1));
                    return Value(static_cast<std::int64_t>(pos));
                }
                throw std::runtime_error("find() expects vector or string as first argument");
            }

            if (node->name == "count") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("count() expects 2 arguments");
                }
                Value container = evaluate(node->children[0]);
                Value target = evaluate(node->children[1]);
                if (container.isVector()) {
                    const auto& vec = container.asVector("count()");
                    std::int64_t cnt = 0;
                    std::string ts = target.toString();
                    for (const auto& v : vec) {
                        if (v.toString() == ts) cnt++;
                    }
                    return Value(cnt);
                }
                if (container.isString()) {
                    const std::string& s = container.asString("count()");
                    const std::string& t = target.asString("count()");
                    if (t.empty()) return Value(static_cast<std::int64_t>(0));
                    std::int64_t cnt = 0;
                    size_t pos = 0;
                    while ((pos = s.find(t, pos)) != std::string::npos) {
                        cnt++;
                        pos += t.size();
                    }
                    return Value(cnt);
                }
                throw std::runtime_error("count() expects vector or string as first argument");
            }

            if (node->name == "replace") {
                if (node->children.size() != 3) {
                    throw std::runtime_error("replace() expects 3 arguments (string, old, new)");
                }
                std::string s = evaluate(node->children[0]).asString("replace()");
                std::string oldStr = evaluate(node->children[1]).asString("replace()");
                std::string newStr = evaluate(node->children[2]).asString("replace()");
                if (oldStr.empty()) return Value(s);
                size_t pos = 0;
                while ((pos = s.find(oldStr, pos)) != std::string::npos) {
                    s.replace(pos, oldStr.size(), newStr);
                    pos += newStr.size();
                }
                return Value(s);
            }

            if (node->name == "upper") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("upper() expects 1 argument");
                }
                std::string s = evaluate(node->children[0]).asString("upper()");
                for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                return Value(s);
            }

            if (node->name == "lower") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("lower() expects 1 argument");
                }
                std::string s = evaluate(node->children[0]).asString("lower()");
                for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                return Value(s);
            }

            if (node->name == "startswith") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("startswith() expects 2 arguments");
                }
                std::string s = evaluate(node->children[0]).asString("startswith()");
                std::string prefix = evaluate(node->children[1]).asString("startswith()");
                return Value(static_cast<std::int64_t>(s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0));
            }

            if (node->name == "endswith") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("endswith() expects 2 arguments");
                }
                std::string s = evaluate(node->children[0]).asString("endswith()");
                std::string suffix = evaluate(node->children[1]).asString("endswith()");
                return Value(static_cast<std::int64_t>(s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0));
            }

            if (node->name == "trim") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("trim() expects 1 argument");
                }
                std::string s = evaluate(node->children[0]).asString("trim()");
                size_t start = s.find_first_not_of(" \t\r\n");
                size_t end = s.find_last_not_of(" \t\r\n");
                if (start == std::string::npos) return Value(std::string(""));
                return Value(s.substr(start, end - start + 1));
            }

            if (node->name == "substr") {
                if (node->children.size() < 2 || node->children.size() > 3) {
                    throw std::runtime_error("substr() expects 2-3 arguments (string, start[, length])");
                }
                std::string s = evaluate(node->children[0]).asString("substr()");
                std::int64_t start = evaluate(node->children[1]).asInt("substr()");
                if (start < 0) start += static_cast<std::int64_t>(s.size());
                if (start < 0) start = 0;
                if (static_cast<size_t>(start) >= s.size()) return Value(std::string(""));
                if (node->children.size() == 3) {
                    std::int64_t len = evaluate(node->children[2]).asInt("substr()");
                    if (len < 0) return Value(std::string(""));
                    return Value(s.substr(static_cast<size_t>(start), static_cast<size_t>(len)));
                }
                return Value(s.substr(static_cast<size_t>(start)));
            }

            if (node->name == "contains") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("contains() expects 2 arguments");
                }
                std::string s = evaluate(node->children[0]).asString("contains()");
                std::string sub = evaluate(node->children[1]).asString("contains()");
                return Value(static_cast<std::int64_t>(s.find(sub) != std::string::npos));
            }

            if (node->name == "min" || node->name == "max") {
                bool isMin = (node->name == "min");
                if (node->children.size() == 1) {
                    Value v = evaluate(node->children[0]);
                    const auto& vec = v.asVector(node->name + "()");
                    if (vec.empty()) throw std::runtime_error(node->name + "() on empty vector");
                    Value best = vec[0];
                    for (size_t i = 1; i < vec.size(); ++i) {
                        if (best.isNumber() && vec[i].isNumber()) {
                            double a = best.asDouble(node->name);
                            double b = vec[i].asDouble(node->name);
                            if (isMin ? b < a : b > a) best = vec[i];
                        } else if (best.isString() && vec[i].isString()) {
                            const std::string& a = std::get<std::string>(best.data);
                            const std::string& b = std::get<std::string>(vec[i].data);
                            if (isMin ? b < a : b > a) best = vec[i];
                        } else {
                            throw std::runtime_error(node->name + "() cannot compare mixed types");
                        }
                    }
                    return best;
                }
                if (node->children.size() == 2) {
                    Value a = evaluate(node->children[0]);
                    Value b = evaluate(node->children[1]);
                    if (a.isNumber() && b.isNumber()) {
                        double da = a.asDouble(node->name);
                        double db = b.asDouble(node->name);
                        if (isMin ? da <= db : da >= db) return a;
                        return b;
                    }
                    if (a.isString() && b.isString()) {
                        const std::string& sa = std::get<std::string>(a.data);
                        const std::string& sb = std::get<std::string>(b.data);
                        if (isMin ? sa <= sb : sa >= sb) return a;
                        return b;
                    }
                    throw std::runtime_error(node->name + "() expects comparable values");
                }
                throw std::runtime_error(node->name + "() expects 1 or 2 arguments");
            }

            if (node->name == "all" || node->name == "any") {
                if (node->children.size() != 2) {
                    throw std::runtime_error(node->name + "() expects 2 arguments (vector, predicate)");
                }
                Value vecVal = evaluate(node->children[0]);
                const auto& vec = vecVal.asVector(node->name + "()");
                // Resolve predicate function
                const ASTNode* predNode = node->children[1];
                std::string fnName;
                if (predNode->type == NodeType::IDENT && functions.find(predNode->name) != functions.end()) {
                    fnName = predNode->name;
                } else {
                    Value predVal = evaluate(predNode);
                    fnName = predVal.asString(node->name + "() predicate");
                }
                auto fit = functions.find(fnName);
                if (fit == functions.end()) {
                    throw std::runtime_error("Undefined predicate function: " + fnName);
                }
                if (fit->second.params.size() != 1) {
                    throw std::runtime_error(node->name + "() predicate must take exactly 1 parameter");
                }
                bool isAll = (node->name == "all");
                const FunctionDef& pred = fit->second;
                for (const auto& elem : vec) {
                    scopes.push_back({});
                    scopes.back()[pred.params[0]] = elem;
                    functionCallDepth++;
                    Value result = evaluate(pred.body);
                    functionCallDepth--;
                    scopes.pop_back();
                    if (currentSignal == Signal::RETURN) {
                        currentSignal = Signal::NONE;
                        result = signalValue;
                    }
                    if (isAll && !result.truthy()) return Value(static_cast<std::int64_t>(0));
                    if (!isAll && result.truthy()) return Value(static_cast<std::int64_t>(1));
                }
                return Value(static_cast<std::int64_t>(isAll ? 1 : 0));
            }

            if (node->name == "swap") {
                if (node->children.size() != 3) {
                    throw std::runtime_error("swap() expects 3 arguments (vector, i, j)");
                }
                // resolve vector target in-place
                auto resolveVecForSwap = [&](const ASTNode* targetNode) -> std::vector<Value>& {
                    if (targetNode->type == NodeType::IDENT) {
                        Value* target = findVariable(targetNode->name);
                        if (target == nullptr) throw std::runtime_error("Undefined variable: " + targetNode->name);
                        return target->asVectorRef("swap");
                    }
                    if (targetNode->type == NodeType::MEMBER) {
                        Value base = evaluate(targetNode->left);
                        ObjectPtr obj = base.asObject("swap");
                        auto it = obj->fields.find(targetNode->name);
                        if (it == obj->fields.end()) throw std::runtime_error("Undefined member: " + targetNode->name);
                        return it->second.asVectorRef("swap");
                    }
                    throw std::runtime_error("swap() first argument must be a vector variable or member");
                };
                std::vector<Value>& vec = resolveVecForSwap(node->children[0]);
                std::int64_t i = evaluate(node->children[1]).asInt("swap()");
                std::int64_t j = evaluate(node->children[2]).asInt("swap()");
                if (i < 0) i += static_cast<std::int64_t>(vec.size());
                if (j < 0) j += static_cast<std::int64_t>(vec.size());
                if (i < 0 || static_cast<size_t>(i) >= vec.size() || j < 0 || static_cast<size_t>(j) >= vec.size()) {
                    throw std::runtime_error("swap() index out of bounds");
                }
                std::swap(vec[static_cast<size_t>(i)], vec[static_cast<size_t>(j)]);
                return Value(static_cast<std::int64_t>(0));
            }

            if (node->name == "fill") {
                if (node->children.size() < 2) {
                    throw std::runtime_error("fill() expects at least 2 arguments");
                }
                // Last arg is the fill value, all preceding are dimensions
                Value val = evaluate(node->children.back());
                std::vector<std::int64_t> dims;
                for (size_t i = 0; i + 1 < node->children.size(); ++i) {
                    dims.push_back(evaluate(node->children[i]).asInt("fill()"));
                    if (dims.back() < 0) throw std::runtime_error("fill() dimension cannot be negative");
                }
                // Build from innermost to outermost
                std::function<Value(size_t)> buildFill = [&](size_t dimIdx) -> Value {
                    std::int64_t count = dims[dimIdx];
                    std::vector<Value> result;
                    result.reserve(static_cast<size_t>(count));
                    if (dimIdx == dims.size() - 1) {
                        for (std::int64_t i = 0; i < count; ++i) {
                            if (val.isVector()) {
                                result.push_back(Value(val.asVector("fill()")));
                            } else {
                                result.push_back(val);
                            }
                        }
                    } else {
                        for (std::int64_t i = 0; i < count; ++i) {
                            result.push_back(buildFill(dimIdx + 1));
                        }
                    }
                    return Value(result);
                };
                return buildFill(0);
            }

            // graph(n) — create adjacency list with n empty vectors
            // graph(n, m) — read m undirected edges (u v) from stdin
            // dgraph(n, m) — read m directed edges
            // wgraph(n, m) — read m undirected weighted edges (u v w)
            // dwgraph(n, m) — read m directed weighted edges
            if (node->name == "graph" || node->name == "dgraph" ||
                node->name == "wgraph" || node->name == "dwgraph") {
                if (node->children.size() < 1 || node->children.size() > 2) {
                    throw std::runtime_error(node->name + "() expects 1 or 2 arguments");
                }
                std::int64_t n = evaluate(node->children[0]).asInt(node->name + "()");
                if (n < 0) throw std::runtime_error(node->name + "() size cannot be negative");

                // Create n empty vectors
                std::vector<Value> adj;
                adj.reserve(static_cast<size_t>(n));
                for (std::int64_t i = 0; i < n; ++i) {
                    adj.push_back(Value(std::vector<Value>{}));
                }

                if (node->children.size() == 2) {
                    std::int64_t m = evaluate(node->children[1]).asInt(node->name + "()");
                    bool directed = (node->name == "dgraph" || node->name == "dwgraph");
                    bool weighted = (node->name == "wgraph" || node->name == "dwgraph");

                    for (std::int64_t e = 0; e < m; ++e) {
                        std::int64_t u = readInt();
                        std::int64_t v = readInt();
                        if (weighted) {
                            std::int64_t w = readInt();
                            std::vector<Value> edge = {Value(v), Value(w)};
                            adj[static_cast<size_t>(u)].asVectorRef(node->name + "()").push_back(Value(edge));
                            if (!directed) {
                                std::vector<Value> redge = {Value(u), Value(w)};
                                adj[static_cast<size_t>(v)].asVectorRef(node->name + "()").push_back(Value(redge));
                            }
                        } else {
                            adj[static_cast<size_t>(u)].asVectorRef(node->name + "()").push_back(Value(v));
                            if (!directed) {
                                adj[static_cast<size_t>(v)].asVectorRef(node->name + "()").push_back(Value(u));
                            }
                        }
                    }
                }
                return Value(adj);
            }

            auto it = functions.find(node->name);
            if (it == functions.end()) {
                throw std::runtime_error("Undefined function: " + node->name);
            }
            const FunctionDef& fn = it->second;
            // Calculate required params (last non-default + 1)
            size_t requiredParams = 0;
            for (size_t i = 0; i < fn.defaults.size(); ++i) {
                if (fn.defaults[i] == nullptr) requiredParams = i + 1;
            }
            if (node->children.size() < requiredParams || node->children.size() > fn.params.size()) {
                throw std::runtime_error("Function '" + node->name + "' expects " +
                    std::to_string(requiredParams) +
                    (requiredParams != fn.params.size() ? "-" + std::to_string(fn.params.size()) : "") +
                    " args, got " + std::to_string(node->children.size()));
            }

            std::vector<Value> argValues;
            argValues.reserve(fn.params.size());
            for (const auto* arg : node->children) {
                argValues.push_back(evaluate(arg));
            }
            // Fill in defaults for missing args
            for (size_t i = argValues.size(); i < fn.params.size(); ++i) {
                argValues.push_back(evaluate(fn.defaults[i]));
            }

            scopes.push_back({});
            for (size_t i = 0; i < fn.params.size(); ++i) {
                scopes.back()[fn.params[i]] = argValues[i];
            }
            functionCallDepth++;
            Value result = evaluate(fn.body);
            functionCallDepth--;
            scopes.pop_back();
            if (currentSignal == Signal::RETURN) {
                currentSignal = Signal::NONE;
                return signalValue;
            }
            return result;
        }
        case NodeType::RETURN: {
            if (functionCallDepth == 0) {
                throw std::runtime_error("'return' used outside function");
            }
            signalValue = evaluate(node->left);
            currentSignal = Signal::RETURN;
            return signalValue;
        }
        case NodeType::IMPORT: {
            const std::string& name = node->name;
            if (importedModules.find(name) != importedModules.end()) {
                return Value(static_cast<std::int64_t>(0));
            }
            importedModules.insert(name);

            std::string source;
            if (name == "std") {
                source = loadStdlibSource();
                if (source.empty()) {
                    throw std::runtime_error("import std failed: stdlib not found");
                }
            } else {
                std::string path = name;
                std::ifstream file(path);
                if (!file && path.find('.') == std::string::npos) {
                    path = name + ".zpp";
                    file.open(path);
                }
                if (!file) {
                    throw std::runtime_error("import failed: cannot open '" + name + "'");
                }
                std::stringstream buffer;
                buffer << file.rdbuf();
                source = buffer.str();
            }

            runSource(source, false);
            return Value(static_cast<std::int64_t>(0));
        }
        case NodeType::ELSE:
        case NodeType::ELSE_IF:
            throw std::runtime_error("Unexpected standalone else node");
        
      
    }
    return Value(static_cast<std::int64_t>(0)); 
}

Value runSource(const std::string& source, bool printResult) {
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens, &globalArena);
    ASTNode* ast = parser.parseExpression();
    Value result = evaluate(ast);
    if (printResult) {
        std::cout << result.toString() << "\n";
    }
    return result;
}

std::string runSourceCaptured(const std::string& source, const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;
    auto* cinBuf = std::cin.rdbuf(in.rdbuf());
    auto* coutBuf = std::cout.rdbuf(out.rdbuf());
    try {
        setInputBuffer(input);
        runSource(source, false);
    } catch (const std::exception& e) {
        out << "Runtime error: " << e.what() << "\n";
    }
    clearInputBuffer();
    std::cin.rdbuf(cinBuf);
    std::cout.rdbuf(coutBuf);
    return out.str();
}

std::string loadStdlibSource() {
    if (std::getenv("ZENPP_NO_STDLIB") != nullptr) {
        return "";
    }
#ifdef __EMSCRIPTEN__
    // Embedded stdlib for WebAssembly builds (no filesystem access)
    return
#include "stdlib_embed.inc"
    ;
#endif
    auto trim = [](std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) {
            s.clear();
            return;
        }
        s = s.substr(start, end - start + 1);
    };

    std::vector<std::string> searchDirs;
    if (std::getenv("ZENPP_STDLIB") != nullptr) {
        searchDirs.push_back(std::getenv("ZENPP_STDLIB"));
    }
    searchDirs.push_back("stdlib");
    if (!executableDir.empty()) {
        searchDirs.push_back(executableDir + "/stdlib");
        searchDirs.push_back(executableDir + "/../lib/zenpp/stdlib");
    }
    searchDirs.push_back("/usr/local/lib/zenpp/stdlib");

    for (const auto& dir : searchDirs) {
        std::ifstream manifest(dir + "/manifest.txt");
        if (!manifest) continue;
        std::stringstream buffer;
        std::string line;
        while (std::getline(manifest, line)) {
            trim(line);
            if (line.empty() || line[0] == '#') continue;
            std::ifstream part(dir + "/" + line);
            if (!part) continue;
            buffer << part.rdbuf() << "\n";
        }
        std::string result = buffer.str();
        if (!result.empty()) return result;
    }

    return "";
}

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
extern "C" {
EMSCRIPTEN_KEEPALIVE
char* zenpp_eval(const char* source, const char* input) {
    resetRuntime();
    std::string src = source ? source : "";
    std::string in = input ? input : "";
    std::string output = runSourceCaptured(src, in);
    char* result = static_cast<char*>(std::malloc(output.size() + 1));
    if (!result) return nullptr;
    std::memcpy(result, output.c_str(), output.size() + 1);
    return result;
}
}
#endif

int runFile(const std::string& filePath) {
    std::ifstream input(filePath);
    if (!input) {
        std::cerr << "Failed to open file: " << filePath << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();

    try {
        runSource(buffer.str(), false);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
        return 1;
    }
}

int runRepl() {
    std::string line;
    std::string buffer;
    int braceDepth = 0;
    while (true) {
        std::cout << "zen++> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line == "exit") {
            break;
        }
        if (line.empty()) {
            if (buffer.empty() || braceDepth > 0) {
                continue;
            }
        }
        if (!line.empty()) {
            if (!buffer.empty()) {
                buffer += "\n";
            }
            buffer += line;
            for (char c : line) {
                if (c == '{') braceDepth++;
                else if (c == '}') braceDepth--;
            }
        }
        if (!buffer.empty() && braceDepth == 0) {
            try {
                runSource(buffer, true);
            } catch (const std::exception& e) {
                std::cerr << "Runtime error: " << e.what() << "\n";
            }
            buffer.clear();
        }

    }
    return 0;
}

// CLI: `./zenpp` => REPL, `./zenpp <file>` => run file
#ifndef __EMSCRIPTEN__
int main(int argc, char* argv[]){
    std::string argv0 = argv[0];
    size_t lastSlash = argv0.find_last_of("/\\");
    executableDir = (lastSlash != std::string::npos) ? argv0.substr(0, lastSlash) : ".";

    if (argc == 1) {
        return runRepl();
    }
    if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "--version" || arg == "-v") {
            std::cout << "zenpp 0.1.0\n";
            return 0;
        }
        return runFile(argv[1]);
    }

    std::cerr << "Usage: " << argv[0] << " [file]\n";
    return 1;
}
#endif
