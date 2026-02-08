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
    if(tok.type == TokenType::IF) {
       // return parseIfStatement();
    }
    if(tok.type == TokenType::IDENTIFIER && peekToken().type == TokenType::ASSIGN){
        std::string varName = tok.name;
        advance();
        advance();
        ASTNode* exprNode = parseExpr();
        return new ASTNode(NodeType::ASSIGN, new ASTNode(varName), exprNode);
    }
    return parseComparison();
}

ASTNode* Parser::parseIfStatement() {
    advance(); // consume if
    ASTNode* condition = parseComparison();
    
}

// ASTNode* Parser::parseBlock() {
//     advance(); // consume {
//     std::vector<ASTNode*> statements;
//     while (currentToken().type != TokenType::RBRACE) {
//         statements.push_back(parseStatment());
//     }
//     advance(); // consume }
//     // For simplicity, we return the first statement in the block
// }

ASTNode* Parser::parseComparison() {
    ASTNode* left = parseExpr();
    Token next = currentToken();
    if(next.type == TokenType::EQUAL || next.type == TokenType::NOT_EQUAL ||
       next.type == TokenType::LESS || next.type == TokenType::GREATER ||
       next.type == TokenType::LESS_EQUAL || next.type == TokenType::GREATER_EQUAL) {
        Token tok = currentToken();
        advance();
        ASTNode* right = parseExpr();
        NodeType nodeType;
        switch(tok.type) {
            case TokenType::EQUAL: nodeType = NodeType::EQUALS; break;
            case TokenType::NOT_EQUAL: nodeType = NodeType::NOT_EQUAL; break;
            case TokenType::LESS: nodeType = NodeType::LESS; break;
            case TokenType::GREATER: nodeType = NodeType::GREATER; break;
            case TokenType::LESS_EQUAL: nodeType = NodeType::LESS_EQUAL; break;
            case TokenType::GREATER_EQUAL: nodeType = NodeType::GREATER_EQUAL; break;
            default: throw std::runtime_error("Invalid comparison operator");
        }
        return new ASTNode(nodeType, left, right);
    }
    return left;
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
    if (tok.type == TokenType::NOT) {
        advance();
        ASTNode* rhs = parseFactor();
        return new ASTNode(NodeType::NOT, rhs, nullptr);
    }
    if (tok.type == TokenType::INT) { advance(); node = new ASTNode(tok.value); }
    else if (tok.type == TokenType::TRUE) { advance(); node = new ASTNode(NodeType::BOOL, 1); }
    else if (tok.type == TokenType::FALSE) { advance(); node = new ASTNode(NodeType::BOOL, 0); }
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
    while (currentToken().type == TokenType::NOT || currentToken().type == TokenType::FACTORIAL) {
        advance();
        node = new ASTNode(NodeType::FACTORIAL, node, nullptr);
    }
    return node;
}

