#pragma once
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

enum class NodeType{
    // Data types
    INT,
    FLOAT,
    STRING,
    VECTOR,
    INDEX,
    MAP_LITERAL,
    SET_LITERAL,
    // Operators
    ADD, SUB, MUL, DIV, MOD, EXP, //binary
    BIT_AND, BIT_OR, BIT_XOR,
    FACTORIAL, NEG, // unary
    IDENT, ASSIGN, // variables
    // boolean 
    BOOL,
    EQUALS, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, NOT_EQUAL,
    AND, OR, NOT,
    IF, ELSE, ELSE_IF, WHILE, FOR,
    FUNCTION_DEF, FUNCTION_CALL, RETURN,
    STRUCT_DEF, MEMBER, METHOD_CALL,
    IMPORT,
    BLOCK,
    POST_INCREMENT, POST_DECREMENT
};

struct ASTNode {
    NodeType type;
    std::int64_t value; // only for INT
    double fvalue; // only for FLOAT
    std::string name; // only for IDENT
    ASTNode* left = nullptr;
    ASTNode* right = nullptr;
    std::vector<ASTNode*> children;

    // Constructors

    // INT
    ASTNode(std::int64_t val) : type(NodeType::INT), value(val), fvalue(0.0) {}
    // FLOAT
    ASTNode(NodeType t, double val) : type(t), value(0), fvalue(val) {}
    // BOOL
    ASTNode(NodeType t, std::int64_t val) : type(t), value(val), fvalue(0.0) {}
    // STRING
    ASTNode(NodeType t, const std::string& n) : type(t), value(0), fvalue(0.0), name(n) {}
    // BINARY 
    ASTNode(NodeType t, ASTNode* l, ASTNode* r) : type(t), value(0), fvalue(0.0), left(l), right(r) {}
    // MEMBER (lhs.name)
    ASTNode(NodeType t, ASTNode* l, const std::string& n) : type(t), value(0), fvalue(0.0), name(n), left(l) {}
    // METHOD CALL (lhs.name(args))
    ASTNode(NodeType t, ASTNode* l, const std::string& n, const std::vector<ASTNode*>& nodes)
        : type(t), value(0), fvalue(0.0), name(n), left(l), children(nodes) {}
    // IDENT
    ASTNode(const std::string& n) : type(NodeType::IDENT), value(0), fvalue(0.0), name(n) {}
    // NODE WITH CHILDREN
    ASTNode(NodeType t, const std::vector<ASTNode*>& nodes) : type(t), value(0), fvalue(0.0), children(nodes) {}
    // NAMED NODE WITH CHILDREN (e.g. function defs/calls)
    ASTNode(NodeType t, const std::string& n, const std::vector<ASTNode*>& nodes)
        : type(t), value(0), fvalue(0.0), name(n), children(nodes) {}
};
