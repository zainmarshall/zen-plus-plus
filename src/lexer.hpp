#pragma once
#include <vector>
#include <string>
#include <cstdint>

enum class TokenType { INT, FLOAT, PLUS, MINUS, STAR, SLASH, MOD, LPAREN, RPAREN, END, EXP, FACTORIAL, IDENTIFIER, ASSIGN, 
    PLUS_PLUS, MINUS_MINUS,
    PLUS_ASSIGN, MINUS_ASSIGN, STAR_ASSIGN, SLASH_ASSIGN, MOD_ASSIGN, EXP_ASSIGN,
    BIT_AND, BIT_OR, BIT_XOR,
    BIT_AND_ASSIGN, BIT_OR_ASSIGN, BIT_XOR_ASSIGN,
    STRING,
    TRUE, FALSE, EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, AND, OR, NOT,
    IF, ELSE, WHILE, FOR, FN, RETURN, IMPORT, STRUCT,
    COMMA, DOT,
    LBRACE, RBRACE, LBRACKET, RBRACKET};

struct Token {
    TokenType type;
    std::int64_t value;
    double fvalue;
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
    char peekChar2();
    void advance();
};
