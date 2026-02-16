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
#include "lexer.hpp"
#include "parser.hpp"
#include "ASTNode.hpp"

struct Value {
    std::variant<int, std::string, std::vector<Value>> data;

    Value() : data(0) {}
    Value(int v) : data(v) {}
    Value(const std::string& s) : data(s) {}
    Value(const std::vector<Value>& v) : data(v) {}

    bool isInt() const { return std::holds_alternative<int>(data); }
    bool isString() const { return std::holds_alternative<std::string>(data); }
    bool isVector() const { return std::holds_alternative<std::vector<Value>>(data); }

    int asInt(const std::string& context) const {
        if (!isInt()) {
            throw std::runtime_error(context + " expects integer value");
        }
        return std::get<int>(data);
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

    bool truthy() const {
        if (isInt()) return std::get<int>(data) != 0;
        if (isString()) return !std::get<std::string>(data).empty();
        return !std::get<std::vector<Value>>(data).empty();
    }

    std::string toString() const {
        if (isInt()) {
            return std::to_string(std::get<int>(data));
        }
        if (isString()) {
            return std::get<std::string>(data);
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
};

struct FunctionDef {
    std::vector<std::string> params;
    const ASTNode* body;
};

struct ReturnSignal {
    Value value;
};

std::vector<std::unordered_map<std::string, Value>> scopes(1);
std::unordered_map<std::string, FunctionDef> functions;
int functionCallDepth = 0;

int checkedMul(int a, int b, const std::string& context) {
    long long result = static_cast<long long>(a) * static_cast<long long>(b);
    if (result > std::numeric_limits<int>::max() || result < std::numeric_limits<int>::min()) {
        throw std::runtime_error("Integer overflow in " + context);
    }
    return static_cast<int>(result);
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

Value evaluate(const ASTNode* node) {
    switch(node->type) {
        case NodeType::INT: return Value(node->value);
        case NodeType::STRING: return Value(node->name);
        case NodeType::VECTOR: {
            std::vector<Value> items;
            for (const auto* child : node->children) {
                items.push_back(evaluate(child));
            }
            return Value(items);
        }
        case NodeType::INDEX: {
            Value container = evaluate(node->left);
            int index = evaluate(node->right).asInt("Index operator");
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
            throw std::runtime_error("Index operator expects vector or string");
        }
        case NodeType::ADD: {
            Value left = evaluate(node->left);
            Value right = evaluate(node->right);
            if (left.isInt() && right.isInt()) {
                return Value(left.asInt("Addition") + right.asInt("Addition"));
            }
            if (left.isVector() && right.isVector()) {
                std::vector<Value> combined = left.asVector("Vector concat");
                const auto& rhs = right.asVector("Vector concat");
                combined.insert(combined.end(), rhs.begin(), rhs.end());
                return Value(combined);
            }
            return Value(left.toString() + right.toString());
        }
        case NodeType::SUB: return Value(evaluate(node->left).asInt("Subtraction") - evaluate(node->right).asInt("Subtraction"));
        case NodeType::MUL: return Value(checkedMul(evaluate(node->left).asInt("Multiplication"), evaluate(node->right).asInt("Multiplication"), "multiplication"));
        case NodeType::DIV: return Value(evaluate(node->left).asInt("Division") / evaluate(node->right).asInt("Division"));
        case NodeType::MOD: return Value(evaluate(node->left).asInt("Modulo") % evaluate(node->right).asInt("Modulo"));
        case NodeType::EXP: {
            int base = evaluate(node->left).asInt("Exponentiation");
            int exp = evaluate(node->right).asInt("Exponentiation");
            if (exp < 0) {
                throw std::runtime_error("Negative exponents are not supported for integers");
            }
            int result = 1;
            for(int i = 0; i < exp; ++i) {
                result = checkedMul(result, base, "exponentiation");
            }
            return Value(result);
        };
        case NodeType::NEG: return Value(-evaluate(node->left).asInt("Unary minus"));
        case NodeType::FACTORIAL: {
            int val = evaluate(node->left).asInt("Factorial");
            if (val < 0) {
                throw std::runtime_error("Factorial is undefined for negative integers");
            }
            int result = 1;
            for(int i = 1; i <= val; ++i) {
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
            if (node->left->type == NodeType::INDEX) {
                std::function<Value&(const ASTNode*)> resolveIndexTarget = [&](const ASTNode* indexNode) -> Value& {
                    if (indexNode->type != NodeType::INDEX) {
                        throw std::runtime_error("Invalid index assignment target");
                    }

                    int index = evaluate(indexNode->right).asInt("Index assignment");
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
        case NodeType::BOOL: return Value(node->value);
        case NodeType::EQUALS: return Value(evaluate(node->left).toString() == evaluate(node->right).toString());
        case NodeType::NOT_EQUAL: return Value(evaluate(node->left).toString() != evaluate(node->right).toString());
        case NodeType::LESS: return Value(evaluate(node->left).asInt("Comparison") < evaluate(node->right).asInt("Comparison"));
        case NodeType::GREATER: return Value(evaluate(node->left).asInt("Comparison") > evaluate(node->right).asInt("Comparison"));
        case NodeType::LESS_EQUAL: return Value(evaluate(node->left).asInt("Comparison") <= evaluate(node->right).asInt("Comparison"));
        case NodeType::GREATER_EQUAL: return Value(evaluate(node->left).asInt("Comparison") >= evaluate(node->right).asInt("Comparison"));
        case NodeType::AND: return Value(evaluate(node->left).truthy() && evaluate(node->right).truthy());
        case NodeType::OR: return Value(evaluate(node->left).truthy() || evaluate(node->right).truthy());
        case NodeType::NOT: return Value(!evaluate(node->left).truthy());
        case NodeType::BLOCK: {
            Value result = Value(0);
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
            return Value(0);
        }
        case NodeType::WHILE: {
            if (node->children.size() != 2) {
                throw std::runtime_error("Malformed while node");
            }
            Value result = Value(0);
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

            int start = evaluate(node->children[1]).asInt("For start");
            int end = evaluate(node->children[2]).asInt("For end");
            int step = 0;
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

            Value result = Value(0);
            for (int i = start; (step > 0) ? (i < end) : (i > end); i += step) {
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
            return Value(0);
        }
        case NodeType::FUNCTION_CALL: {
            if (node->name == "len") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("len() expects 1 argument");
                }
                Value v = evaluate(node->children[0]);
                if (v.isString()) return Value(static_cast<int>(v.asString("len").size()));
                if (v.isVector()) return Value(static_cast<int>(v.asVector("len").size()));
                throw std::runtime_error("len() expects string or vector");
            }

            if (node->name == "push") {
                if (node->children.size() != 2) {
                    throw std::runtime_error("push() expects 2 arguments");
                }
                if (node->children[0]->type != NodeType::IDENT) {
                    throw std::runtime_error("push() first argument must be a vector variable");
                }
                Value* target = findVariable(node->children[0]->name);
                if (target == nullptr) {
                    throw std::runtime_error("Undefined variable: " + node->children[0]->name);
                }
                std::vector<Value>& vec = target->asVectorRef("push");
                vec.push_back(evaluate(node->children[1]));
                return Value(static_cast<int>(vec.size()));
            }

            if (node->name == "pop") {
                if (node->children.size() != 1) {
                    throw std::runtime_error("pop() expects 1 argument");
                }
                if (node->children[0]->type != NodeType::IDENT) {
                    throw std::runtime_error("pop() argument must be a vector variable");
                }
                Value* target = findVariable(node->children[0]->name);
                if (target == nullptr) {
                    throw std::runtime_error("Undefined variable: " + node->children[0]->name);
                }
                std::vector<Value>& vec = target->asVectorRef("pop");
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
                int value = 0;
                if (!(std::cin >> value)) {
                    throw std::runtime_error("read() failed to read an integer from stdin");
                }
                return Value(value);
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
                return Value(0);
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
        case NodeType::ELSE:
        case NodeType::ELSE_IF:
            throw std::runtime_error("Unexpected standalone else node");
        
      
    }
    return Value(0); 
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
