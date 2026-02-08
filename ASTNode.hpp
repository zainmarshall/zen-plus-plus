#pragma once
#include <memory>

enum class NodeType{
    INT,
    ADD, SUB, MUL, DIV, MOD, EXP,
    FACTORIAL,
    IDENT, ASSIGN
};

struct ASTNode {
    NodeType type;
    int value; // only for INT
    std::string name; // only for IDENT
    ASTNode* left; 
    ASTNode* right;

    // Constructors

    // INT
    ASTNode(int val) : type(NodeType::INT), value(val) {}
    // BINARY 
    ASTNode(NodeType t, ASTNode* l, ASTNode* r) : type(t), left(l), right(r) {}
    // IDENT
    ASTNode(const std::string& n) : type(NodeType::IDENT), name(n) {}
};