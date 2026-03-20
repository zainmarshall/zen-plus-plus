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
    bool sawNewline = false;
    auto pushTok = [&](Token t) {
        t.preceded_by_newline = sawNewline;
        sawNewline = false;
        tokens.push_back(t);
    };
    while (currentChar() != '\0') {
        if (isspace(currentChar())) {
            if (currentChar() == '\n') sawNewline = true;
            advance();
            continue;
        }

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
                pushTok({TokenType::FLOAT, 0, num, ""});
            } else {
                std::int64_t num = std::stoll(numStr);
                pushTok({TokenType::INT, num, 0.0, ""});
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
            pushTok({TokenType::STRING, 0, 0.0, value});
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
            pushTok({TokenType::INT, static_cast<unsigned char>(value), 0.0, ""});
            continue;
        }

        if(isalpha(currentChar()) || currentChar() == '_') {
            std::string ident;
            while (isalnum(currentChar()) || currentChar() == '_') {
                ident += currentChar();
                advance();
            }
            // f-string: f"..."
            if (ident == "f" && currentChar() == '"') {
                advance(); // consume "
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
                            case '\0': throw std::runtime_error("Unterminated f-string escape");
                            default: throw std::runtime_error("Unsupported f-string escape sequence");
                        }
                    } else {
                        value += currentChar();
                    }
                    advance();
                }
                if (currentChar() != '"') {
                    throw std::runtime_error("Unterminated f-string");
                }
                advance();
                pushTok({TokenType::FSTRING, 0, 0.0, value});
                continue;
            }
            if(ident == "true") {
                pushTok({TokenType::TRUE, 0, 0.0, ""});
            } else if (ident == "false") {
                pushTok({TokenType::FALSE, 0, 0.0, ""});
            } else if (ident == "if") {
                pushTok({TokenType::IF, 0, 0.0, ""});
            } else if (ident == "else") {
                pushTok({TokenType::ELSE, 0, 0.0, ""});
            } else if (ident == "while") {
                pushTok({TokenType::WHILE, 0, 0.0, ""});
            } else if (ident == "for") {
                pushTok({TokenType::FOR, 0, 0.0, ""});
            } else if (ident == "fn") {
                pushTok({TokenType::FN, 0, 0.0, ""});
            } else if (ident == "return") {
                pushTok({TokenType::RETURN, 0, 0.0, ""});
            } else if (ident == "import") {
                pushTok({TokenType::IMPORT, 0, 0.0, ""});
            } else if (ident == "struct") {
                pushTok({TokenType::STRUCT, 0, 0.0, ""});
            } else if (ident == "break") {
                pushTok({TokenType::BREAK, 0, 0.0, ""});
            } else if (ident == "continue") {
                pushTok({TokenType::CONTINUE, 0, 0.0, ""});
            } else if (ident == "in") {
                pushTok({TokenType::IN, 0, 0.0, ""});
            } else {
                pushTok({TokenType::IDENTIFIER, 0, 0.0, ident});
            }
            continue;
        }

        char c = currentChar();
        char n = peekChar();
        char nn = peekChar2();
        if (c == '=' && n == '=') { pushTok({TokenType::EQUAL, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '!' && n == '=') { pushTok({TokenType::NOT_EQUAL, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '<' && n == '<' && nn == '=') { pushTok({TokenType::SHIFT_LEFT_ASSIGN, 0, 0.0, ""}); advance(); advance(); advance(); continue; }
        if (c == '>' && n == '>' && nn == '=') { pushTok({TokenType::SHIFT_RIGHT_ASSIGN, 0, 0.0, ""}); advance(); advance(); advance(); continue; }
        if (c == '<' && n == '<') { pushTok({TokenType::SHIFT_LEFT, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '>' && n == '>') { pushTok({TokenType::SHIFT_RIGHT, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '<' && n == '=') { pushTok({TokenType::LESS_EQUAL, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '>' && n == '=') { pushTok({TokenType::GREATER_EQUAL, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '*' && n == '*' && nn == '=') { pushTok({TokenType::EXP_ASSIGN, 0, 0.0, ""}); advance(); advance(); advance(); continue; }
        if (c == '*' && n == '*') { pushTok({TokenType::EXP, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '&' && n == '&') { pushTok({TokenType::AND, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '|' && n == '|') { pushTok({TokenType::OR, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '&' && n == '=') { pushTok({TokenType::BIT_AND_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '|' && n == '=') { pushTok({TokenType::BIT_OR_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '^' && n == '=') { pushTok({TokenType::BIT_XOR_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '+' && n == '+') { pushTok({TokenType::PLUS_PLUS, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '-' && n == '-') { pushTok({TokenType::MINUS_MINUS, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '+' && n == '=') { pushTok({TokenType::PLUS_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '-' && n == '=') { pushTok({TokenType::MINUS_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '*' && n == '=') { pushTok({TokenType::STAR_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '/' && n == '=') { pushTok({TokenType::SLASH_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }
        if (c == '%' && n == '=') { pushTok({TokenType::MOD_ASSIGN, 0, 0.0, ""}); advance(); advance(); continue; }

        switch (currentChar()) {
            case '+': pushTok({TokenType::PLUS, 0, 0.0, ""}); break;
            case '-': pushTok({TokenType::MINUS, 0, 0.0, ""}); break;
            case '*': pushTok({TokenType::STAR, 0, 0.0, ""}); break;
            case '/': pushTok({TokenType::SLASH, 0, 0.0, ""}); break;
            case '%': pushTok({TokenType::MOD, 0, 0.0, ""}); break;
            case '(': pushTok({TokenType::LPAREN, 0, 0.0, ""}); break;
            case ')': pushTok({TokenType::RPAREN, 0, 0.0, ""}); break;
            case '^': pushTok({TokenType::BIT_XOR, 0, 0.0, ""}); break;
            case '&': pushTok({TokenType::BIT_AND, 0, 0.0, ""}); break;
            case '|': pushTok({TokenType::BIT_OR, 0, 0.0, ""}); break;
            case '!': pushTok({TokenType::NOT, 0, 0.0, ""}); break;
            case '=': pushTok({TokenType::ASSIGN, 0, 0.0, ""}); break;
            case '<': pushTok({TokenType::LESS, 0, 0.0, ""}); break;
            case '>': pushTok({TokenType::GREATER, 0, 0.0, ""}); break;
            case '{': pushTok({TokenType::LBRACE, 0, 0.0, ""}); break;
            case '}': pushTok({TokenType::RBRACE, 0, 0.0, ""}); break;
            case '[': pushTok({TokenType::LBRACKET, 0, 0.0, ""}); break;
            case ']': pushTok({TokenType::RBRACKET, 0, 0.0, ""}); break;
            case ',': pushTok({TokenType::COMMA, 0, 0.0, ""}); break;
            case '.': pushTok({TokenType::DOT, 0, 0.0, ""}); break;
            case '~': pushTok({TokenType::TILDE, 0, 0.0, ""}); break;
            case '?': pushTok({TokenType::QUESTION, 0, 0.0, ""}); break;
            case ':': pushTok({TokenType::COLON, 0, 0.0, ""}); break;
            default:
                throw std::runtime_error(std::string("Unexpected character: '") + currentChar() + "'");
        }
        advance();
    }
    pushTok({TokenType::END, 0, 0.0, ""});
    return tokens;
}
