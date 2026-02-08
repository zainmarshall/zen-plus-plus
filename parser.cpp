#include "parser.hpp"
#include "ASTNode.hpp"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& toks) : tokens(toks), pos(0) {}

Token Parser::currentToken() { return tokens[pos]; }
Token Parser::peekToken(size_t offset) {
    size_t index = pos + offset;
    if (index >= tokens.size()) {
        return tokens.back();
    }
    return tokens[index];
}
void Parser::advance() { if (pos < tokens.size()) pos++; }

ASTNode* Parser::parseExpression() { return parseStatment(); }

//The order is: parseStatment -> parseExpr -> parseTerm -> parseFactor
ASTNode* Parser::parseStatment() {
    Token tok = currentToken();
    if(tok.type == TokenType::IDENTIFIER && peekToken().type == TokenType::ASSIGN){
        std::string varName = tok.name;
        advance();
        advance();
        ASTNode* exprNode = parseExpr();
        return new ASTNode(NodeType::ASSIGN, new ASTNode(varName), exprNode);
    }
    return parseExpr();
}

ASTNode* Parser::parseExpr() {
    ASTNode* node = parseTerm();
    while (true) {
        Token tok = currentToken();
        if (tok.type == TokenType::PLUS) {
             advance();
             node = new ASTNode(NodeType::ADD, node, parseTerm());
            }
        else if (tok.type == TokenType::MINUS) {
            advance();
            node = new ASTNode(NodeType::SUB, node, parseTerm());
        }
        else break;
    }
    return node;
}

ASTNode* Parser::parseTerm() {
    ASTNode* node = parseFactor();
    while (true) {
        Token tok = currentToken();
        if(tok.type == TokenType::EXP) {
            advance();
            node = new ASTNode(NodeType::EXP, node, parseFactor());
        }
        else if (tok.type == TokenType::STAR) { advance(); node = new ASTNode(NodeType::MUL, node, parseFactor()); }
        else if (tok.type == TokenType::SLASH) { advance(); node = new ASTNode(NodeType::DIV, node, parseFactor()); }
        else if (tok.type == TokenType::MOD) { advance(); node = new ASTNode(NodeType::MOD, node, parseFactor()); }
        else break;
    }
    return node;
}

ASTNode* Parser::parseFactor() {
    Token tok = currentToken();
    ASTNode* node = nullptr;
    if (tok.type == TokenType::INT) { advance(); node = new ASTNode(tok.value); }
    else if (tok.type == TokenType::IDENTIFIER) { advance(); node = new ASTNode(tok.name); }
    else if (tok.type == TokenType::LPAREN) {
        advance();
        node = parseExpr();
        if (currentToken().type != TokenType::RPAREN) throw std::runtime_error("Expected ')'");
        advance();
    }else{
        throw std::runtime_error("Unexpected token");
    }

    // Unary Operators
    tok= currentToken();
    if(tok.type == TokenType::FACTORIAL){
        advance();
        node = new ASTNode(NodeType::FACTORIAL, node, nullptr);
    }
    return node;
}

