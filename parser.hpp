#pragma once
#include <vector>
#include "lexer.hpp"
#include "ASTNode.hpp"

class Parser {
public:
    Parser(const std::vector<Token>& toks);
    ASTNode* parseExpression(); 
private:
    std::vector<Token> tokens;
    size_t pos = 0;

    Token currentToken();
    Token peekToken(size_t offset = 1);
    void advance();

    ASTNode* parseStatment();
    ASTNode* parseIfStatement();
    ASTNode* parseBlock();
    ASTNode* parseComparison();
    ASTNode* parseExpr();
    ASTNode* parseTerm();
    ASTNode* parseFactor();
};
