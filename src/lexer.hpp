#pragma once
#include <vector>
#include <string>

enum class TokenType { INT, PLUS, MINUS, STAR, SLASH, MOD, LPAREN, RPAREN, END, EXP, FACTORIAL, IDENTIFIER, ASSIGN, 
    PLUS_PLUS, MINUS_MINUS,
    PLUS_ASSIGN, MINUS_ASSIGN, STAR_ASSIGN, SLASH_ASSIGN, MOD_ASSIGN, EXP_ASSIGN,
    STRING,
    TRUE, FALSE, EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND, OR, NOT,
    IF, ELSE, WHILE, FOR, FN, RETURN,
    COMMA,
    LBRACE, RBRACE, LBRACKET, RBRACKET};

struct Token {
    TokenType type;
    int value; 
    std::string name;
};

class Lexer {
public:
    Lexer(const std::string &src);
    std::vector<Token> tokenize();
private:
    std::string source;
    size_t pos = 0;
    char currentChar();
    char peekChar();
    void advance();
};
