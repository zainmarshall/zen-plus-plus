#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string &src) : source(src), pos(0) {}

char Lexer::currentChar() {
    return pos < source.size() ? source[pos] : '\0';
}

char Lexer::peekChar() {
    size_t next = pos + 1;
    return next < source.size() ? source[next] : '\0';
}

char Lexer::peekChar2() {
    size_t next = pos + 2;
    return next < source.size() ? source[next] : '\0';
}

void Lexer::advance() { pos++; }

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (currentChar() != '\0') {
        if (isspace(currentChar())) { advance(); continue; }

        if (currentChar() == '/' && peekChar() == '/') {
            while (currentChar() != '\0' && currentChar() != '\n') {
                advance();
            }
            continue;
        }

        if (currentChar() == '/' && peekChar() == '*') {
            advance();
            advance();
            bool terminated = false;
            while (currentChar() != '\0') {
                if (currentChar() == '*' && peekChar() == '/') {
                    advance();
                    advance();
                    terminated = true;
                    break;
                }
                advance();
            }
            if (!terminated) {
                throw std::runtime_error("Unterminated block comment");
            }
            continue;
        }

        if (isdigit(currentChar())) {
            std::string numStr;
            while (isdigit(currentChar())) {
                numStr += currentChar();
                advance();
            }
            if (currentChar() == '.') {
                numStr += '.';
                advance();
                while (isdigit(currentChar())) {
                    numStr += currentChar();
                    advance();
                }
                double num = std::stod(numStr);
                tokens.push_back({TokenType::FLOAT, 0, num, ""});
            } else {
                std::int64_t num = std::stoll(numStr);
                tokens.push_back({TokenType::INT, num, 0.0, ""});
            }
            continue;
        }

        if (currentChar() == '"') {
            advance();
            std::string value;
            while (currentChar() != '\0' && currentChar() != '"') {
                if (currentChar() == '\\') {
                    advance();
                    switch (currentChar()) {
                        case 'n': value += '\n'; break;
                        case 't': value += '\t'; break;
                        case 'r': value += '\r'; break;
                        case '\\': value += '\\'; break;
                        case '"': value += '"'; break;
                        case '\0': throw std::runtime_error("Unterminated string escape");
                        default: throw std::runtime_error("Unsupported string escape sequence");
                    }
                } else {
                    value += currentChar();
                }
                advance();
            }
            if (currentChar() != '"') {
                throw std::runtime_error("Unterminated string literal");
            }
            advance();
            tokens.push_back({TokenType::STRING, 0, 0.0, value});
            continue;
        }

        if (currentChar() == '\'') {
            advance();
            if (currentChar() == '\0') {
                throw std::runtime_error("Unterminated char literal");
            }

            char value = '\0';
            if (currentChar() == '\\') {
                advance();
                switch (currentChar()) {
                    case 'n': value = '\n'; break;
                    case 't': value = '\t'; break;
                    case 'r': value = '\r'; break;
                    case '\\': value = '\\'; break;
                    case '\'': value = '\''; break;
                    case '0': value = '\0'; break;
                    case '\0': throw std::runtime_error("Unterminated char literal escape");
                    default: throw std::runtime_error("Unsupported char escape sequence");
                }
            } else {
                value = currentChar();
            }
            advance();

            if (currentChar() != '\'') {
                throw std::runtime_error("Char literal must contain exactly one character");
            }
            advance();
            tokens.push_back({TokenType::INT, static_cast<unsigned char>(value), 0.0, ""});
            continue;
        }

        if(isalpha(currentChar()) || currentChar() == '_') {
            std::string ident;
            while (isalnum(currentChar()) || currentChar() == '_') {
                ident += currentChar();
                advance();
            }
            if(ident == "true") {
                tokens.push_back({TokenType::TRUE, 0, 0.0, ""});
            } else if (ident == "false") {
                tokens.push_back({TokenType::FALSE, 0, 0.0, ""});
            } else if (ident == "if") {
                tokens.push_back({TokenType::IF, 0, 0.0, ""});
            } else if (ident == "else") {
                tokens.push_back({TokenType::ELSE, 0, 0.0, ""});
            } else if (ident == "while") {
                tokens.push_back({TokenType::WHILE, 0, 0.0, ""});
            } else if (ident == "for") {
                tokens.push_back({TokenType::FOR, 0, 0.0, ""});
            } else if (ident == "fn") {
                tokens.push_back({TokenType::FN, 0, 0.0, ""});
            } else if (ident == "return") {
                tokens.push_back({TokenType::RETURN, 0, 0.0, ""});
            } else if (ident == "import") {
                tokens.push_back({TokenType::IMPORT, 0, 0.0, ""});
            } else if (ident == "struct") {
                tokens.push_back({TokenType::STRUCT, 0, 0.0, ""});
            } else if (ident == "break") {
                tokens.push_back({TokenType::BREAK, 0, 0.0, ""});
            } else if (ident == "continue") {
                tokens.push_back({TokenType::CONTINUE, 0, 0.0, ""});
            } else if (ident == "in") {
                tokens.push_back({TokenType::IN, 0, 0.0, ""});
            } else {
                tokens.push_back({TokenType::IDENTIFIER, 0, 0.0, ident});
            }
            continue;
        }

        char c = currentChar();
        char n = peekChar();
        char nn = peekChar2();
        if (c == '=' && n == '=') { tokens.push_back({TokenType::EQUAL, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '!' && n == '=') { tokens.push_back({TokenType::NOT_EQUAL, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '<' && n == '<' && nn == '=') { tokens.push_back({TokenType::SHIFT_LEFT_ASSIGN, 0, 0.0, ""}); advance(); advance(); advance(); continue; }
        if (c == '>' && n == '>' && nn == '=') { tokens.push_back({TokenType::SHIFT_RIGHT_ASSIGN, 0, 0.0, ""}); advance(); advance(); advance(); continue; }
        if (c == '<' && n == '<') { tokens.push_back({TokenType::SHIFT_LEFT, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '>' && n == '>') { tokens.push_back({TokenType::SHIFT_RIGHT, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '<' && n == '=') { tokens.push_back({TokenType::LESS_EQUAL, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '>' && n == '=') { tokens.push_back({TokenType::GREATER_EQUAL, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '*' && n == '*' && nn == '=') { tokens.push_back({TokenType::EXP_ASSIGN, 0, 0.0, ""}); advance(); advance(); advance(); continue; }
        if (c == '*' && n == '*') { tokens.push_back({TokenType::EXP, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '&' && n == '&') { tokens.push_back({TokenType::AND, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '|' && n == '|') { tokens.push_back({TokenType::OR, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '&' && n == '=') { tokens.push_back({TokenType::BIT_AND_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '|' && n == '=') { tokens.push_back({TokenType::BIT_OR_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '^' && n == '=') { tokens.push_back({TokenType::BIT_XOR_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '+' && n == '+') { tokens.push_back({TokenType::PLUS_PLUS, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '-' && n == '-') { tokens.push_back({TokenType::MINUS_MINUS, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '+' && n == '=') { tokens.push_back({TokenType::PLUS_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '-' && n == '=') { tokens.push_back({TokenType::MINUS_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '*' && n == '=') { tokens.push_back({TokenType::STAR_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '/' && n == '=') { tokens.push_back({TokenType::SLASH_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '%' && n == '=') { tokens.push_back({TokenType::MOD_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }

        switch (currentChar()) {
            case '+': tokens.push_back({TokenType::PLUS, 0, 0.0, ""}); break;
            case '-': tokens.push_back({TokenType::MINUS, 0, 0.0, ""}); break;
            case '*': tokens.push_back({TokenType::STAR, 0, 0.0, ""}); break;
            case '/': tokens.push_back({TokenType::SLASH, 0, 0.0, ""}); break;
            case '%': tokens.push_back({TokenType::MOD, 0, 0.0, ""}); break;
            case '(': tokens.push_back({TokenType::LPAREN, 0, 0.0, ""}); break;
            case ')': tokens.push_back({TokenType::RPAREN, 0, 0.0, ""}); break;
            case '^': tokens.push_back({TokenType::BIT_XOR, 0, 0.0, ""}); break;
            case '&': tokens.push_back({TokenType::BIT_AND, 0, 0.0, ""}); break;
            case '|': tokens.push_back({TokenType::BIT_OR, 0, 0.0, ""}); break;
            case '!': tokens.push_back({TokenType::NOT, 0, 0.0, ""}); break;
            case '=': tokens.push_back({TokenType::ASSIGN, 0, 0.0, ""}); break;
            case '<': tokens.push_back({TokenType::LESS, 0, 0.0, ""}); break;
            case '>': tokens.push_back({TokenType::GREATER, 0, 0.0, ""}); break;
            case '{': tokens.push_back({TokenType::LBRACE, 0, 0.0, ""}); break;
            case '}': tokens.push_back({TokenType::RBRACE, 0, 0.0, ""}); break;
            case '[': tokens.push_back({TokenType::LBRACKET, 0, 0.0, ""}); break;
            case ']': tokens.push_back({TokenType::RBRACKET, 0, 0.0, ""}); break;
            case ',': tokens.push_back({TokenType::COMMA, 0, 0.0, ""}); break;
            case '.': tokens.push_back({TokenType::DOT, 0, 0.0, ""}); break;
            case '?': tokens.push_back({TokenType::QUESTION, 0, 0.0, ""}); break;
            case ':': tokens.push_back({TokenType::COLON, 0, 0.0, ""}); break;
            default:
                throw std::runtime_error(std::string("Unexpected character: '") + currentChar() + "'");
        }
        advance();
    }
    tokens.push_back({TokenType::END, 0, 0.0, ""});
    return tokens;
}
