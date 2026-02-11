#include <iostream>
#include <string>
#include <unordered_map>
#include "lexer.hpp"
#include "parser.hpp"
#include "ASTNode.hpp"

std::unordered_map<std::string, int> variables;
int evaluate(const ASTNode* node) {
    switch(node->type) {
        case NodeType::INT: return node->value;
        case NodeType::ADD: return evaluate(node->left) + evaluate(node->right);
        case NodeType::SUB: return evaluate(node->left) - evaluate(node->right);
        case NodeType::MUL: return evaluate(node->left) * evaluate(node->right);
        case NodeType::DIV: return evaluate(node->left) / evaluate(node->right);
        case NodeType::MOD: return evaluate(node->left) % evaluate(node->right);
        case NodeType::EXP: {
            int base = evaluate(node->left);
            int exp = evaluate(node->right);
            int result = 1;
            for(int i = 0; i < exp; ++i) {
                result *= base;
            }
            return result;
        };
        case NodeType::FACTORIAL: {
            int val = evaluate(node->left);
            int result = 1;
            for(int i = 1; i <= val; ++i) {
                result *= i;
            }
            return result;
        }
        case NodeType::IDENT: {
            auto it = variables.find(node->name);
            if(it == variables.end()) throw std::runtime_error("Undefined variable: " + node->name);
            return it->second;
        }
        case NodeType::ASSIGN: {
            std::string varName = node->left->name;
            int value = evaluate(node->right);
            variables[varName] = value;
            return value;
        }
        case NodeType::BOOL: return node->value;
        case NodeType::EQUALS: return evaluate(node->left) == evaluate(node->right);
        case NodeType::NOT_EQUAL: return evaluate(node->left) != evaluate(node->right);
        case NodeType::LESS: return evaluate(node->left) < evaluate(node->right);
        case NodeType::GREATER: return evaluate(node->left) > evaluate(node->right);
        case NodeType::LESS_EQUAL: return evaluate(node->left) <= evaluate(node->right);
        case NodeType::GREATER_EQUAL: return evaluate(node->left) >= evaluate(node->right);
        case NodeType::NOT: return !evaluate(node->left);
        case NodeType::BLOCK: {
            int result = 0;
            for (const auto* stmt : node->children) {
                result = evaluate(stmt);
            }
            return result;
        }
        case NodeType::IF: {
            if (node->children.size() < 2) {
                throw std::runtime_error("Malformed if node");
            }
            if (evaluate(node->children[0])) {
                return evaluate(node->children[1]);
            }
            size_t i = 2;
            while (i + 1 < node->children.size()) {
                if (evaluate(node->children[i])) {
                    return evaluate(node->children[i + 1]);
                }
                i += 2;
            }
            if (i < node->children.size()) {
                return evaluate(node->children[i]);
            }
            return 0;
        }
        case NodeType::WHILE: {
            if (node->children.size() != 2) {
                throw std::runtime_error("Malformed while node");
            }
            int result = 0;
            while (evaluate(node->children[0])) {
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

            int start = evaluate(node->children[1]);
            int end = evaluate(node->children[2]);
            int step = 0;
            const ASTNode* body = nullptr;

            if (node->children.size() == 5) {
                step = evaluate(node->children[3]);
                body = node->children[4];
            } else {
                step = (start <= end) ? 1 : -1;
                body = node->children[3];
            }

            if (step == 0) {
                throw std::runtime_error("For loop step cannot be 0");
            }

            int result = 0;
            for (int i = start; (step > 0) ? (i < end) : (i > end); i += step) {
                variables[varNode->name] = i;
                result = evaluate(body);
            }
            return result;
        }
        case NodeType::ELSE:
        case NodeType::ELSE_IF:
            throw std::runtime_error("Unexpected standalone else node");
        
      
    }
    return 0; 
}

// REPL
int main(){
    std::string line;
    std::string buffer;
    while(true){
        std::cout << "zen++> ";
        if(!std::getline(std::cin, line)){
            break;
        }
        if(line == "exit"){
            break;
        }
        if(line.empty()){
            if(buffer.empty()){
                continue;
            }
            Lexer lexer(buffer);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            ASTNode* ast = parser.parseExpression();
            int result = evaluate(ast);
            std::cout << result << "\n";
            buffer.clear();
            continue;
        }
        if(buffer.empty()){
            Lexer lexer(line);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            ASTNode* ast = parser.parseExpression();
            int result = evaluate(ast);
            std::cout << result << "\n";
            continue;
        }
        buffer += "\n";
        buffer += line;

    }
}