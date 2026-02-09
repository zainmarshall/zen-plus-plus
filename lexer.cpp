#include "lexer.hpp"
#include <cctype>

Lexer::Lexer(const std::string &src) : source(src), pos(0) {}

char Lexer::currentChar() {
    return pos < source.size() ? source[pos] : '\0';
}

char Lexer::peekChar() {
    size_t next = pos + 1;
    return next < source.size() ? source[next] : '\0';
}

void Lexer::advance() { pos++; }


std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (currentChar() != '\0') {
        if (isspace(currentChar())) { advance(); continue; }
        if (isdigit(currentChar())) {
            int num = 0;
            while (isdigit(currentChar())) { num = num * 10 + (currentChar()-'0'); advance(); }
            tokens.push_back({TokenType::INT, num});
            continue;
        }
        if(isalpha(currentChar()) || currentChar() == '_') {
            std::string ident;
            while (isalnum(currentChar()) || currentChar() == '_') {
                ident += currentChar();
                advance();
            }
            if(ident == "true") {
                tokens.push_back({TokenType::TRUE, 0});
            } else if (ident == "false") {
                tokens.push_back({TokenType::FALSE, 0});
            } else if (ident == "if") {
                tokens.push_back({TokenType::IF, 0});
            } else if (ident == "else") {
                tokens.push_back({TokenType::ELSE, 0});
            } else {
                tokens.push_back({TokenType::IDENTIFIER, 0, ident});
            }
            continue;
        }
        char c = currentChar();
        char n = peekChar();
        if (c == '=' && n == '=') { tokens.push_back({TokenType::EQUAL, 0}); advance(); advance(); continue; }
        if (c == '!' && n == '=') { tokens.push_back({TokenType::NOT_EQUAL, 0}); advance(); advance(); continue; }
        if (c == '<' && n == '=') { tokens.push_back({TokenType::LESS_EQUAL, 0}); advance(); advance(); continue; }
        if (c == '>' && n == '=') { tokens.push_back({TokenType::GREATER_EQUAL, 0}); advance(); advance(); continue; }
        if (c == '&' && n == '&') { tokens.push_back({TokenType::AND, 0}); advance(); advance(); continue; }
        if (c == '|' && n == '|') { tokens.push_back({TokenType::OR, 0}); advance(); advance(); continue; }
        switch (currentChar()) {
            case '+': tokens.push_back({TokenType::PLUS, 0}); break;
            case '-': tokens.push_back({TokenType::MINUS, 0}); break;
            case '*': tokens.push_back({TokenType::STAR, 0}); break;
            case '/': tokens.push_back({TokenType::SLASH, 0}); break;
            case '%': tokens.push_back({TokenType::MOD, 0}); break;
            case '(': tokens.push_back({TokenType::LPAREN, 0}); break;
            case ')': tokens.push_back({TokenType::RPAREN, 0}); break;
            case '^': tokens.push_back({TokenType::EXP, 0}); break;
            case '!': tokens.push_back({TokenType::NOT, 0}); break;
            case '=': tokens.push_back({TokenType::ASSIGN, 0}); break;
            case '<': tokens.push_back({TokenType::LESS, 0}); break;
            case '>': tokens.push_back({TokenType::GREATER, 0}); break;
            case '{': tokens.push_back({TokenType::LBRACE, 0}); break;
            case '}': tokens.push_back({TokenType::RBRACE, 0}); break;
        }
        advance();
    }
    tokens.push_back({TokenType::END, 0});
    return tokens;
}