#pragma once
#include <vector>
#include <string>

enum class TokenType { INT, PLUS, MINUS, STAR, SLASH, MOD, LPAREN, RPAREN, END, EXP, FACTORIAL, IDENTIFIER };

struct Token {
    TokenType type;
    int value; 
};

class Lexer {
public:
    Lexer(const std::string &src);
    std::vector<Token> tokenize();
private:
    std::string source;
    size_t pos = 0;
    char currentChar();
    void advance();
};
