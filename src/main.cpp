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
#include "lexer.hpp"
#include "parser.hpp"
#include "ASTNode.hpp"

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
    const ASTNode* body;
};

struct StructDef {
    std::unordered_map<std::string, FunctionDef> methods;
};

struct ReturnSignal {
    Value value;
};

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
}

std::string loadStdlibSource();
Value runSource(const std::string& source, bool printResult);

std::int64_t readInt() {
    if (!inputBuffer.enabled) {
        std::int64_t value = 0;
        if (!(std::cin >> value)) {
            throw std::runtime_error("readInt() failed to read an integer from stdin");
        }
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
    return value * sign;
}

double readFloat() {
    if (!inputBuffer.enabled) {
        double value = 0.0;
        if (!(std::cin >> value)) {
            throw std::runtime_error("readFloat() failed to read a float from stdin");
        }
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
                    throw std::runtime_error("Key not found in map");
                }
                return it->second;
            }
            std::int64_t index = indexVal.asInt("Index operator");
            if (index < 0) {
                throw std::runtime_error("Index cannot be negative");
            }
            if (container.isVector()) {
                const auto& vec = container.asVector("Index operator");
                if (static_cast<size_t>(index) >= vec.size()) {
                    throw std::runtime_error("Vector index out of bounds");
                }
                return vec[static_cast<size_t>(index)];
            }
            if (container.isString()) {
                const auto& str = container.asString("Index operator");
                if (static_cast<size_t>(index) >= str.size()) {
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
            throw std::runtime_error("Multiplication expects numeric values");
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
                // Check if target is a map — handle map key assignment directly
                if (node->left->left->type == NodeType::IDENT) {
                    Value* base = findVariable(node->left->left->name);
                    if (base == nullptr) {
                        throw std::runtime_error("Undefined variable: " + node->left->left->name);
                    }
                    if (base->isMap()) {
                        Value keyVal = evaluate(node->left->right);
                        HashKey key = toHashKey(keyVal, "map index assignment");
                        base->asMap("map index assignment")->data[key] = value;
                        return value;
                    }
                }

                std::function<Value&(const ASTNode*)> resolveIndexTarget = [&](const ASTNode* indexNode) -> Value& {
                    if (indexNode->type != NodeType::INDEX) {
                        throw std::runtime_error("Invalid index assignment target");
                    }

                    std::int64_t index = evaluate(indexNode->right).asInt("Index assignment");
                    if (index < 0) {
                        throw std::runtime_error("Index assignment cannot use negative index");
                    }

                    if (indexNode->left->type == NodeType::IDENT) {
                        Value* base = findVariable(indexNode->left->name);
                        if (base == nullptr) {
                            throw std::runtime_error("Undefined variable: " + indexNode->left->name);
                        }
                        std::vector<Value>& vec = base->asVectorRef("Index assignment");
                        if (static_cast<size_t>(index) >= vec.size()) {
                            throw std::runtime_error("Vector index out of bounds");
                        }
                        return vec[static_cast<size_t>(index)];
                    }

                    if (indexNode->left->type == NodeType::MEMBER) {
                        Value baseVal = evaluate(indexNode->left->left);
                        ObjectPtr obj = baseVal.asObject("Index assignment");
                        auto it = obj->fields.find(indexNode->left->name);
                        if (it == obj->fields.end()) {
                            throw std::runtime_error("Undefined member: " + indexNode->left->name);
                        }
                        std::vector<Value>& vec = it->second.asVectorRef("Index assignment");
                        if (static_cast<size_t>(index) >= vec.size()) {
                            throw std::runtime_error("Vector index out of bounds");
                        }
                        return vec[static_cast<size_t>(index)];
                    }

                    if (indexNode->left->type == NodeType::INDEX) {
                        Value& parent = resolveIndexTarget(indexNode->left);
                        std::vector<Value>& vec = parent.asVectorRef("Nested index assignment");
                        if (static_cast<size_t>(index) >= vec.size()) {
                            throw std::runtime_error("Vector index out of bounds");
                        }
                        return vec[static_cast<size_t>(index)];
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
            for (std::int64_t i = start; (step > 0) ? (i < end) : (i > end); i += step) {
                setVariable(varNode->name, Value(i));
                result = evaluate(body);
            }
            return result;
        }
        case NodeType::FUNCTION_DEF: {
            if (node->children.empty()) {
                throw std::runtime_error("Malformed function definition");
            }
            std::vector<std::string> params;
            for (size_t i = 0; i + 1 < node->children.size(); ++i) {
                if (node->children[i]->type != NodeType::IDENT) {
                    throw std::runtime_error("Function parameter must be an identifier");
                }
                params.push_back(node->children[i]->name);
            }
            functions[node->name] = FunctionDef{params, node->children.back()};
            return Value(static_cast<std::int64_t>(0));
        }
        case NodeType::STRUCT_DEF: {
            StructDef def;
            for (const auto* child : node->children) {
                if (child->type != NodeType::FUNCTION_DEF || child->children.empty()) {
                    throw std::runtime_error("Struct methods must be function definitions");
                }
                std::vector<std::string> params;
                for (size_t i = 0; i + 1 < child->children.size(); ++i) {
                    if (child->children[i]->type != NodeType::IDENT) {
                        throw std::runtime_error("Struct method parameter must be an identifier");
                    }
                    params.push_back(child->children[i]->name);
                }
                def.methods[child->name] = FunctionDef{params, child->children.back()};
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
            if (fn.params.size() != node->children.size()) {
                throw std::runtime_error("Method '" + node->name + "' expects " +
                    std::to_string(fn.params.size()) + " args, got " +
                    std::to_string(node->children.size()));
            }
            std::vector<Value> argValues;
            argValues.reserve(node->children.size());
            for (const auto* arg : node->children) {
                argValues.push_back(evaluate(arg));
            }
            scopes.push_back({});
            scopes.back()["self"] = base;
            for (size_t i = 0; i < fn.params.size(); ++i) {
                scopes.back()[fn.params[i]] = argValues[i];
            }
            functionCallDepth++;
            try {
                Value result = evaluate(fn.body);
                functionCallDepth--;
                scopes.pop_back();
                return result;
            } catch (const ReturnSignal& signal) {
                functionCallDepth--;
                scopes.pop_back();
                return signal.value;
            } catch (...) {
                functionCallDepth--;
                scopes.pop_back();
                throw;
            }
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
                Value objVal(obj);
                auto itInit = itStruct->second.methods.find("init");
                if (itInit != itStruct->second.methods.end()) {
                    const FunctionDef& initFn = itInit->second;
                    if (!initFn.params.empty()) {
                        return objVal;
                    }
                    scopes.push_back({});
                    scopes.back()["self"] = objVal;
                    functionCallDepth++;
                    try {
                        (void)evaluate(initFn.body);
                        functionCallDepth--;
                        scopes.pop_back();
                    } catch (const ReturnSignal&) {
                        functionCallDepth--;
                        scopes.pop_back();
                    } catch (...) {
                        functionCallDepth--;
                        scopes.pop_back();
                        throw;
                    }
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
                auto resolveVectorTarget = [&](const ASTNode* targetNode) -> std::vector<Value>& {
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
                    throw std::runtime_error("push() first argument must be a vector variable or member");
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

            if (node->name == "read") {
                if (!node->children.empty()) {
                    throw std::runtime_error("read() expects 0 arguments");
                }
                return Value(readInt());
            }

            if (node->name == "readInt") {
                if (!node->children.empty()) {
                    throw std::runtime_error("readInt() expects 0 arguments");
                }
                return Value(readInt());
            }

            if (node->name == "readFloat") {
                if (!node->children.empty()) {
                    throw std::runtime_error("readFloat() expects 0 arguments");
                }
                return Value(readFloat());
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
                std::cout << "\n";
                return Value(static_cast<std::int64_t>(0));
            }

            auto it = functions.find(node->name);
            if (it == functions.end()) {
                throw std::runtime_error("Undefined function: " + node->name);
            }
            const FunctionDef& fn = it->second;
            if (fn.params.size() != node->children.size()) {
                throw std::runtime_error("Function '" + node->name + "' expects " +
                    std::to_string(fn.params.size()) + " args, got " +
                    std::to_string(node->children.size()));
            }

            std::vector<Value> argValues;
            argValues.reserve(node->children.size());
            for (const auto* arg : node->children) {
                argValues.push_back(evaluate(arg));
            }

            scopes.push_back({});
            for (size_t i = 0; i < fn.params.size(); ++i) {
                scopes.back()[fn.params[i]] = argValues[i];
            }
            functionCallDepth++;
            try {
                Value result = evaluate(fn.body);
                functionCallDepth--;
                scopes.pop_back();
                return result;
            } catch (const ReturnSignal& signal) {
                functionCallDepth--;
                scopes.pop_back();
                return signal.value;
            } catch (...) {
                functionCallDepth--;
                scopes.pop_back();
                throw;
            }
        }
        case NodeType::RETURN: {
            if (functionCallDepth == 0) {
                throw std::runtime_error("'return' used outside function");
            }
            throw ReturnSignal{evaluate(node->left)};
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
    Parser parser(tokens);
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
    std::stringstream buffer;
    std::ifstream manifest("stdlib/manifest.txt");
    auto trim = [](std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) {
            s.clear();
            return;
        }
        s = s.substr(start, end - start + 1);
    };
    if (manifest) {
        std::string line;
        while (std::getline(manifest, line)) {
            trim(line);
            if (line.empty() || line[0] == '#') {
                continue;
            }
            std::ifstream part("stdlib/" + line);
            if (!part) {
                continue;
            }
            buffer << part.rdbuf() << "\n";
        }
        return buffer.str();
    }

    std::ifstream prelude("stdlib/prelude.zpp");
    if (!prelude) {
        return "";
    }
    buffer << prelude.rdbuf();
    return buffer.str();
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
    if (argc == 1) {
        return runRepl();
    }
    if (argc == 2) {
        return runFile(argv[1]);
    }

    std::cerr << "Usage: " << argv[0] << " [file]\n";
    return 1;
}
#endif
