#pragma once
#include <memory>

enum class NodeType{
    INT,
    ADD, SUB, MUL, DIV, MOD, EXP,
    FACTORIAL,
};

struct ASTNode {
    NodeType type;
    int value; // only for INT
    ASTNode* left; 
    ASTNode* right;

    // Constructors

    // INT
    ASTNode(int val) : type(NodeType::INT), value(val) {}
    // BINARY 
    ASTNode(NodeType t, ASTNode* l, ASTNode* r) : type(t), left(l), right(r) {}
};