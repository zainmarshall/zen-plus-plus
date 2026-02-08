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
    }
    return 0; 
}

// REPL
int main(){
    std::string line;
    while(true){
        std::cout << "zen++> ";
        if(!std::getline(std::cin, line) || line == "exit"){
            break;
        }

        Lexer lexer(line);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        ASTNode* ast = parser.parseExpression();
        int result = evaluate(ast);
        std::cout << result << "\n";
    }
}