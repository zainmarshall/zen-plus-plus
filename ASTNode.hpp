#pragma once
#include <memory>
#include <string>
#include <vector>

enum class NodeType{
    // Data types
    INT,
    // Operators
    ADD, SUB, MUL, DIV, MOD, EXP, //binary
    FACTORIAL, // unary
    IDENT, ASSIGN, // variables
    // boolean 
    BOOL,
    EQUALS, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, NOT_EQUAL,
    NOT,
    IF, ELSE, ELSE_IF, WHILE, FOR,
    BLOCK
};

struct ASTNode {
    NodeType type;
    int value; // only for INT 
    std::string name; // only for IDENT
    ASTNode* left; 
    ASTNode* right;
    std::vector<ASTNode*> children;

    // Constructors

    // INT
    ASTNode(int val) : type(NodeType::INT), value(val) {}
    // BOOL
    ASTNode(NodeType t, int val) : type(t), value(val) {}
    // BINARY 
    ASTNode(NodeType t, ASTNode* l, ASTNode* r) : type(t), left(l), right(r) {}
    // IDENT
    ASTNode(const std::string& n) : type(NodeType::IDENT), name(n) {}
    // NODE WITH CHILDREN
    ASTNode(NodeType t, const std::vector<ASTNode*>& nodes) : type(t), children(nodes) {}
};