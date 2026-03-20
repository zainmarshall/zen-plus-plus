#include "parser.hpp"
#include "ASTNode.hpp"
#include <stdexcept>
#include <cstdint>

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

ASTNode* Parser::parseExpression() { return parseProgram(); }

ASTNode* Parser::parseProgram() {
    std::vector<ASTNode*> statements;
    while (currentToken().type != TokenType::END) {
        statements.push_back(parseStatment());
    }
    return new ASTNode(NodeType::BLOCK, statements);
}

//The order is: parseStatment -> parseExpr -> parseTerm -> parseFactor
ASTNode* Parser::parseStatment() {
    Token tok = currentToken();
    if (tok.type == TokenType::RETURN) {
        return parseReturnStatement();
    }
    if (tok.type == TokenType::IMPORT) {
        return parseImportStatement();
    }
    if (tok.type == TokenType::STRUCT) {
        return parseStructDefinition();
    }
    if (tok.type == TokenType::FN) {
        return parseFunctionDefinition();
    }
    if(tok.type == TokenType::IF) {
       return parseIfStatement();
    }
    if(tok.type == TokenType::WHILE) {
        return parseWhileStatement();
    }
    if(tok.type == TokenType::FOR) {
        return parseForStatement();
    }
    if (tok.type == TokenType::BREAK) {
        advance();
        return new ASTNode(NodeType::BREAK, nullptr, nullptr);
    }
    if (tok.type == TokenType::CONTINUE) {
        advance();
        return new ASTNode(NodeType::CONTINUE, nullptr, nullptr);
    }
    if(tok.type == TokenType::IDENTIFIER) {
        if (tok.name == "map" && peekToken().type == TokenType::IDENTIFIER) {
            advance(); // consume 'map'
            std::string name = currentToken().name;
            advance();
            return new ASTNode(NodeType::ASSIGN, new ASTNode(name),
                new ASTNode(NodeType::FUNCTION_CALL, "map", std::vector<ASTNode*>{}));
        }
        if (tok.name == "set" && peekToken().type == TokenType::IDENTIFIER) {
            advance(); // consume 'set'
            std::string name = currentToken().name;
            advance();
            return new ASTNode(NodeType::ASSIGN, new ASTNode(name),
                new ASTNode(NodeType::FUNCTION_CALL, "set", std::vector<ASTNode*>{}));
        }
        // Multi-assignment: a, b = 1, 2
        if (peekToken().type == TokenType::COMMA) {
            size_t savedPos = pos;
            std::vector<std::string> names;
            names.push_back(tok.name);
            advance(); // consume first ident
            bool isMultiAssign = true;
            while (currentToken().type == TokenType::COMMA) {
                advance(); // consume comma
                if (currentToken().type != TokenType::IDENTIFIER) {
                    isMultiAssign = false;
                    break;
                }
                names.push_back(currentToken().name);
                advance();
            }
            if (isMultiAssign && currentToken().type == TokenType::ASSIGN) {
                advance(); // consume =
                std::vector<ASTNode*> children;
                for (const auto& n : names) {
                    children.push_back(new ASTNode(n));
                }
                // parse comma-separated values
                children.push_back(parseTernary());
                while (currentToken().type == TokenType::COMMA) {
                    advance();
                    children.push_back(parseTernary());
                }
                // Store name count in node's value field
                ASTNode* result = new ASTNode(NodeType::MULTI_ASSIGN, children);
                result->value = static_cast<std::int64_t>(names.size());
                return result;
            }
            pos = savedPos;
        }

        size_t startPos = pos;
        ASTNode* lhs = parsePostfix();
        if (currentToken().type == TokenType::ASSIGN &&
            (lhs->type == NodeType::IDENT || lhs->type == NodeType::INDEX || lhs->type == NodeType::MEMBER)) {
            advance();
            ASTNode* exprNode = parseTernary();
            return new ASTNode(NodeType::ASSIGN, lhs, exprNode);
        }
        pos = startPos;

        Token next = peekToken();
        if (next.type == TokenType::ASSIGN) {
            std::string varName = tok.name;
            advance();
            advance();
            ASTNode* exprNode = parseExpr();
            return new ASTNode(NodeType::ASSIGN, new ASTNode(varName), exprNode);
        }
        if (next.type == TokenType::PLUS_PLUS || next.type == TokenType::MINUS_MINUS) {
            std::string varName = tok.name;
            advance();
            advance();
            NodeType opType = (next.type == TokenType::PLUS_PLUS) ? NodeType::ADD : NodeType::SUB;
            ASTNode* rhs = new ASTNode(opType, new ASTNode(varName), new ASTNode(1));
            return new ASTNode(NodeType::ASSIGN, new ASTNode(varName), rhs);
        }
        NodeType opType;
        bool isCompound = true;
        switch (next.type) {
            case TokenType::PLUS_ASSIGN: opType = NodeType::ADD; break;
            case TokenType::MINUS_ASSIGN: opType = NodeType::SUB; break;
            case TokenType::STAR_ASSIGN: opType = NodeType::MUL; break;
            case TokenType::SLASH_ASSIGN: opType = NodeType::DIV; break;
            case TokenType::MOD_ASSIGN: opType = NodeType::MOD; break;
            case TokenType::EXP_ASSIGN: opType = NodeType::EXP; break;
            case TokenType::BIT_AND_ASSIGN: opType = NodeType::BIT_AND; break;
            case TokenType::BIT_OR_ASSIGN: opType = NodeType::BIT_OR; break;
            case TokenType::BIT_XOR_ASSIGN: opType = NodeType::BIT_XOR; break;
            case TokenType::SHIFT_LEFT_ASSIGN: opType = NodeType::BIT_SHIFT_LEFT; break;
            case TokenType::SHIFT_RIGHT_ASSIGN: opType = NodeType::BIT_SHIFT_RIGHT; break;
            default: isCompound = false; break;
        }
        if (isCompound) {
            std::string varName = tok.name;
            advance();
            advance();
            ASTNode* exprNode = parseTernary();
            ASTNode* rhs = new ASTNode(opType, new ASTNode(varName), exprNode);
            return new ASTNode(NodeType::ASSIGN, new ASTNode(varName), rhs);
        }
    }
    // Destructuring: [a, b, c] = expr
    if (tok.type == TokenType::LBRACKET) {
        size_t savedPos = pos;
        advance(); // consume [
        std::vector<std::string> names;
        bool isDestructuring = true;
        while (true) {
            if (currentToken().type == TokenType::IDENTIFIER) {
                names.push_back(currentToken().name);
                advance();
            } else {
                isDestructuring = false;
                break;
            }
            if (currentToken().type == TokenType::COMMA) {
                advance();
                continue;
            }
            break;
        }
        if (isDestructuring && !names.empty() && currentToken().type == TokenType::RBRACKET) {
            advance(); // consume ]
            if (currentToken().type == TokenType::ASSIGN) {
                advance(); // consume =
                ASTNode* rhs = parseTernary();
                std::vector<ASTNode*> children;
                for (const auto& n : names) {
                    children.push_back(new ASTNode(n));
                }
                children.push_back(rhs);
                ASTNode* result = new ASTNode(NodeType::DESTRUCT_ASSIGN, children);
                result->value = static_cast<std::int64_t>(names.size());
                return result;
            }
        }
        pos = savedPos;
    }
    return parseTernary();
}

ASTNode* Parser::parseReturnStatement() {
    advance(); // consume return
    ASTNode* value = parseTernary();
    return new ASTNode(NodeType::RETURN, value, nullptr);
}

ASTNode* Parser::parseImportStatement() {
    advance(); // consume import
    if (currentToken().type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected identifier after 'import'");
    }
    std::string name = currentToken().name;
    advance();
    return new ASTNode(NodeType::IMPORT, name, std::vector<ASTNode*>{});
}

ASTNode* Parser::parseStructDefinition() {
    advance(); // consume struct
    if (currentToken().type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected struct name after 'struct'");
    }
    std::string name = currentToken().name;
    advance();
    if (currentToken().type != TokenType::LBRACE) {
        throw std::runtime_error("Expected '{' to start struct body");
    }
    advance(); // consume {
    std::vector<ASTNode*> methods;
    while (currentToken().type != TokenType::RBRACE) {
        if (currentToken().type == TokenType::END) {
            throw std::runtime_error("Unexpected end of input in struct body");
        }
        if (currentToken().type == TokenType::FN) {
            methods.push_back(parseFunctionDefinition());
        } else if (currentToken().type == TokenType::IDENTIFIER) {
            std::string fieldName = currentToken().name;
            advance();
            if (currentToken().type != TokenType::ASSIGN) {
                throw std::runtime_error("Expected '=' after field name in struct body");
            }
            advance();
            ASTNode* defaultValue = parseTernary();
            methods.push_back(new ASTNode(NodeType::ASSIGN, new ASTNode(fieldName), defaultValue));
        } else {
            throw std::runtime_error("Struct body must contain field or function definitions");
        }
    }
    advance(); // consume }
    return new ASTNode(NodeType::STRUCT_DEF, name, methods);
}

ASTNode* Parser::parseFunctionDefinition() {
    advance(); // consume fn
    if (currentToken().type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected function name after 'fn'");
    }
    std::string functionName = currentToken().name;
    advance();

    if (currentToken().type != TokenType::LPAREN) {
        throw std::runtime_error("Expected '(' after function name");
    }
    advance(); // consume (

    std::vector<ASTNode*> children;
    if (currentToken().type != TokenType::RPAREN) {
        while (true) {
            if (currentToken().type != TokenType::IDENTIFIER) {
                throw std::runtime_error("Expected parameter name");
            }
            std::string paramName = currentToken().name;
            advance();
            if (currentToken().type == TokenType::ASSIGN) {
                advance(); // consume =
                ASTNode* defaultVal = parseTernary();
                children.push_back(new ASTNode(NodeType::ASSIGN, new ASTNode(paramName), defaultVal));
            } else {
                children.push_back(new ASTNode(paramName));
            }

            if (currentToken().type == TokenType::COMMA) {
                advance();
                continue;
            }
            break;
        }
    }

    if (currentToken().type != TokenType::RPAREN) {
        throw std::runtime_error("Expected ')' after function parameters");
    }
    advance(); // consume )

    ASTNode* body = parseBlock();
    children.push_back(body);
    return new ASTNode(NodeType::FUNCTION_DEF, functionName, children);
}

ASTNode* Parser::parseIfStatement() {
    advance(); // consume if
    ASTNode* condition = parseTernary();
    ASTNode* thenBlock = parseBlock();

    std::vector<ASTNode*> children;
    children.push_back(condition);
    children.push_back(thenBlock);

    while (currentToken().type == TokenType::ELSE) {
        advance(); // consume else
        if (currentToken().type == TokenType::IF) {
            advance(); // consume if
            ASTNode* elseIfCondition = parseTernary();
            ASTNode* elseIfBlock = parseBlock();
            children.push_back(elseIfCondition);
            children.push_back(elseIfBlock);
        } else {
            ASTNode* elseBlock = parseBlock();
            children.push_back(elseBlock);
            break;
        }
    }

    return new ASTNode(NodeType::IF, children);
}

ASTNode* Parser::parseWhileStatement() {
    advance(); // consume while
    ASTNode* condition = parseTernary();
    ASTNode* body = parseBlock();
    std::vector<ASTNode*> children;
    children.push_back(condition);
    children.push_back(body);
    return new ASTNode(NodeType::WHILE, children);
}

ASTNode* Parser::parseForStatement() {
    advance(); // consume for
    if (currentToken().type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected identifier after 'for'");
    }
    std::string varName = currentToken().name;
    advance();

    // Check for tuple unpacking: for x, y in collection { }
    std::vector<std::string> varNames = {varName};
    if (currentToken().type == TokenType::COMMA) {
        size_t savedPos = pos;
        bool isTupleUnpack = true;
        while (currentToken().type == TokenType::COMMA) {
            advance(); // consume comma
            if (currentToken().type != TokenType::IDENTIFIER) {
                isTupleUnpack = false;
                break;
            }
            varNames.push_back(currentToken().name);
            advance();
        }
        if (!isTupleUnpack || currentToken().type != TokenType::IN) {
            pos = savedPos;
            varNames = {varName};
        }
    }

    // for x in collection { ... } OR for x, y in collection { ... }
    if (currentToken().type == TokenType::IN) {
        advance(); // consume in
        ASTNode* collection = parseTernary();
        ASTNode* body = parseBlock();
        std::vector<ASTNode*> children;
        for (const auto& n : varNames) {
            children.push_back(new ASTNode(n));
        }
        children.push_back(collection);
        children.push_back(body);
        ASTNode* result = new ASTNode(NodeType::FOR_EACH, children);
        result->value = static_cast<std::int64_t>(varNames.size());
        return result;
    }

    std::vector<ASTNode*> parts;
    while (currentToken().type != TokenType::LBRACE) {
        if (currentToken().type == TokenType::END) {
            throw std::runtime_error("Unexpected end of input in for statement");
        }
        if (parts.size() >= 3) {
            throw std::runtime_error("For loop accepts at most 3 expressions before block");
        }
        parts.push_back(parseTernary());
    }

    if (parts.empty()) {
        throw std::runtime_error("For loop requires at least an end expression");
    }

    ASTNode* startExpr = nullptr;
    ASTNode* endExpr = nullptr;
    ASTNode* stepExpr = nullptr;

    if (parts.size() == 1) {
        startExpr = new ASTNode(0);
        endExpr = parts[0];
    } else if (parts.size() == 2) {
        startExpr = parts[0];
        endExpr = parts[1];
    } else {
        startExpr = parts[0];
        endExpr = parts[1];
        stepExpr = parts[2];
    }

    ASTNode* body = parseBlock();
    std::vector<ASTNode*> children;
    children.push_back(new ASTNode(varName));
    children.push_back(startExpr);
    children.push_back(endExpr);
    if (stepExpr != nullptr) {
        children.push_back(stepExpr);
    }
    children.push_back(body);
    return new ASTNode(NodeType::FOR, children);
}

ASTNode* Parser::parseBlock() {
    if (currentToken().type != TokenType::LBRACE) {
        throw std::runtime_error("Expected '{' to start block");
    }
    advance(); // consume {
    std::vector<ASTNode*> statements;
    while (currentToken().type != TokenType::RBRACE) {
        if (currentToken().type == TokenType::END) {
            throw std::runtime_error("Unexpected end of input in block");
        }
        statements.push_back(parseStatment());
    }
    advance(); // consume }
    return new ASTNode(NodeType::BLOCK, statements);
}

ASTNode* Parser::parseShift() {
    ASTNode* node = parseExpr();
    while (currentToken().type == TokenType::SHIFT_LEFT || currentToken().type == TokenType::SHIFT_RIGHT) {
        NodeType nt = (currentToken().type == TokenType::SHIFT_LEFT) ? NodeType::BIT_SHIFT_LEFT : NodeType::BIT_SHIFT_RIGHT;
        advance();
        node = new ASTNode(nt, node, parseExpr());
    }
    return node;
}

ASTNode* Parser::parseComparison() {
    ASTNode* left = parseShift();
    auto isCompOp = [](TokenType t) {
        return t == TokenType::EQUAL || t == TokenType::NOT_EQUAL ||
               t == TokenType::LESS || t == TokenType::GREATER ||
               t == TokenType::LESS_EQUAL || t == TokenType::GREATER_EQUAL;
    };
    if (!isCompOp(currentToken().type)) return left;

    ASTNode* result = nullptr;
    while (isCompOp(currentToken().type)) {
        Token tok = currentToken();
        advance();
        ASTNode* right = parseShift();
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
        ASTNode* cmp = new ASTNode(nodeType, left, right);
        if (result == nullptr) { result = cmp; }
        else { result = new ASTNode(NodeType::AND, result, cmp); }
        left = right;
    }
    return result;
}

ASTNode* Parser::parseLogicalAnd() {
    ASTNode* node = parseBitwiseOr();
    while (currentToken().type == TokenType::AND) {
        advance();
        node = new ASTNode(NodeType::AND, node, parseBitwiseOr());
    }
    return node;
}

ASTNode* Parser::parseLogicalOr() {
    ASTNode* node = parseLogicalAnd();
    while (currentToken().type == TokenType::OR) {
        advance();
        node = new ASTNode(NodeType::OR, node, parseLogicalAnd());
    }
    return node;
}

ASTNode* Parser::parseTernary() {
    ASTNode* cond = parseLogicalOr();
    if (currentToken().type == TokenType::QUESTION) {
        advance(); // consume ?
        ASTNode* thenExpr = parseTernary();
        if (currentToken().type != TokenType::COLON) {
            throw std::runtime_error("Expected ':' in ternary expression");
        }
        advance(); // consume :
        ASTNode* elseExpr = parseTernary();
        std::vector<ASTNode*> children = {cond, thenExpr, elseExpr};
        return new ASTNode(NodeType::TERNARY, children);
    }
    return cond;
}

ASTNode* Parser::parseBitwiseOr() {
    ASTNode* node = parseBitwiseXor();
    while (currentToken().type == TokenType::BIT_OR) {
        advance();
        node = new ASTNode(NodeType::BIT_OR, node, parseBitwiseXor());
    }
    return node;
}

ASTNode* Parser::parseBitwiseXor() {
    ASTNode* node = parseBitwiseAnd();
    while (currentToken().type == TokenType::BIT_XOR) {
        advance();
        node = new ASTNode(NodeType::BIT_XOR, node, parseBitwiseAnd());
    }
    return node;
}

ASTNode* Parser::parseBitwiseAnd() {
    ASTNode* node = parseComparison();
    while (currentToken().type == TokenType::BIT_AND) {
        advance();
        node = new ASTNode(NodeType::BIT_AND, node, parseComparison());
    }
    return node;
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

ASTNode* Parser::parseExp() {
    ASTNode* node = parsePostfix();
    if (currentToken().type == TokenType::EXP) {
        advance();
        node = new ASTNode(NodeType::EXP, node, parseExp()); // right-associative
    }
    return node;
}

ASTNode* Parser::parseTerm() {
    ASTNode* node = parseExp();
    while (true) {
        Token tok = currentToken();
        if (tok.type == TokenType::STAR) { advance(); node = new ASTNode(NodeType::MUL, node, parseExp()); }
        else if (tok.type == TokenType::SLASH) { advance(); node = new ASTNode(NodeType::DIV, node, parseExp()); }
        else if (tok.type == TokenType::MOD) { advance(); node = new ASTNode(NodeType::MOD, node, parseExp()); }
        else break;
    }
    return node;
}

ASTNode* Parser::parsePostfix() {
    ASTNode* node = parseFactor();
    while (true) {
        if (currentToken().type == TokenType::LBRACKET && !currentToken().preceded_by_newline) {
            advance(); // consume [
            // Parse first expression (or detect empty start for slice)
            ASTNode* startExpr = nullptr;
            if (currentToken().type != TokenType::COLON && currentToken().type != TokenType::RBRACKET) {
                startExpr = parseLogicalOr();
            }
            if (currentToken().type == TokenType::COLON) {
                // Slice syntax: [start:end] or [start:end:step]
                advance(); // consume first :
                ASTNode* endExpr = nullptr;
                if (currentToken().type != TokenType::RBRACKET && currentToken().type != TokenType::COLON) {
                    endExpr = parseLogicalOr();
                }
                ASTNode* stepExpr = nullptr;
                if (currentToken().type == TokenType::COLON) {
                    advance(); // consume second :
                    if (currentToken().type != TokenType::RBRACKET) {
                        stepExpr = parseLogicalOr();
                    }
                }
                if (currentToken().type != TokenType::RBRACKET) {
                    throw std::runtime_error("Expected ']' after slice");
                }
                advance();
                ASTNode* sliceNode = new ASTNode(NodeType::SLICE, node, nullptr);
                sliceNode->children = {startExpr, endExpr, stepExpr};
                node = sliceNode;
            } else {
                // Regular index
                if (currentToken().type != TokenType::RBRACKET) {
                    throw std::runtime_error("Expected ']' after index expression");
                }
                advance();
                node = new ASTNode(NodeType::INDEX, node, startExpr);
            }
            continue;
        }
        if (currentToken().type == TokenType::DOT) {
            advance();
            if (currentToken().type != TokenType::IDENTIFIER) {
                throw std::runtime_error("Expected identifier after '.'");
            }
            std::string memberName = currentToken().name;
            advance();
            if (currentToken().type == TokenType::LPAREN) {
                advance();
                std::vector<ASTNode*> args;
                if (currentToken().type != TokenType::RPAREN) {
                    while (true) {
                        args.push_back(parseTernary());
                        if (currentToken().type == TokenType::COMMA) {
                            advance();
                            continue;
                        }
                        break;
                    }
                }
                if (currentToken().type != TokenType::RPAREN) {
                    throw std::runtime_error("Expected ')' after method arguments");
                }
                advance();
                node = new ASTNode(NodeType::METHOD_CALL, node, memberName, args);
            } else {
                node = new ASTNode(NodeType::MEMBER, node, memberName);
            }
            continue;
        }
        break;
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
    if (tok.type == TokenType::TILDE) {
        advance();
        ASTNode* rhs = parseFactor();
        return new ASTNode(NodeType::BIT_NOT, rhs, nullptr);
    }
    if (tok.type == TokenType::PLUS_PLUS || tok.type == TokenType::MINUS_MINUS) {
        bool isInc = (tok.type == TokenType::PLUS_PLUS);
        advance();
        if (currentToken().type != TokenType::IDENTIFIER) {
            throw std::runtime_error("Expected identifier after prefix operator");
        }
        std::string varName = currentToken().name;
        advance();
        NodeType opType = isInc ? NodeType::ADD : NodeType::SUB;
        ASTNode* rhs = new ASTNode(opType, new ASTNode(varName), new ASTNode(1));
        return new ASTNode(NodeType::ASSIGN, new ASTNode(varName), rhs);
    }
    if (tok.type == TokenType::MINUS) {
        advance();
        ASTNode* rhs = parseFactor();
        return new ASTNode(NodeType::NEG, rhs, nullptr);
    }
    if (tok.type == TokenType::PLUS) {
        advance();
        return parseFactor();
    }
    if (tok.type == TokenType::INT) { advance(); node = new ASTNode(tok.value); }
    else if (tok.type == TokenType::FLOAT) { advance(); node = new ASTNode(NodeType::FLOAT, tok.fvalue); }
    else if (tok.type == TokenType::STRING) { advance(); node = new ASTNode(NodeType::STRING, tok.name); }
    else if (tok.type == TokenType::FSTRING) {
        advance();
        const std::string& content = tok.name;
        ASTNode* result = nullptr;
        size_t i = 0;
        while (i < content.size()) {
            if (content[i] == '{') {
                size_t depth = 1;
                size_t j = i + 1;
                while (j < content.size() && depth > 0) {
                    if (content[j] == '{') depth++;
                    else if (content[j] == '}') depth--;
                    j++;
                }
                std::string exprStr = content.substr(i + 1, j - i - 2);
                Lexer exprLexer(exprStr);
                auto exprTokens = exprLexer.tokenize();
                Parser exprParser(exprTokens);
                ASTNode* exprAst = exprParser.parseExpression();
                ASTNode* strNode = new ASTNode(NodeType::FUNCTION_CALL, "str", std::vector<ASTNode*>{exprAst});
                if (result == nullptr) { result = strNode; }
                else { result = new ASTNode(NodeType::ADD, result, strNode); }
                i = j;
            } else {
                size_t j = i;
                while (j < content.size() && content[j] != '{') j++;
                std::string literal = content.substr(i, j - i);
                ASTNode* litNode = new ASTNode(NodeType::STRING, literal);
                if (result == nullptr) { result = litNode; }
                else { result = new ASTNode(NodeType::ADD, result, litNode); }
                i = j;
            }
        }
        if (result == nullptr) { result = new ASTNode(NodeType::STRING, std::string("")); }
        node = result;
    }
    else if (tok.type == TokenType::TRUE) { advance(); node = new ASTNode(NodeType::BOOL, static_cast<std::int64_t>(1)); }
    else if (tok.type == TokenType::FALSE) { advance(); node = new ASTNode(NodeType::BOOL, static_cast<std::int64_t>(0)); }
    else if (tok.type == TokenType::LBRACKET) {
        advance();
        std::vector<ASTNode*> elements;
        if (currentToken().type != TokenType::RBRACKET) {
            while (true) {
                elements.push_back(parseTernary());
                if (currentToken().type == TokenType::COMMA) {
                    advance();
                    continue;
                }
                break;
            }
        }
        if (currentToken().type != TokenType::RBRACKET) {
            throw std::runtime_error("Expected ']' after vector literal");
        }
        advance();
        node = new ASTNode(NodeType::VECTOR, elements);
    }
    else if (tok.type == TokenType::IDENTIFIER) {
        advance();
        if (currentToken().type == TokenType::LPAREN) {
            advance(); // consume (
            std::vector<ASTNode*> args;
            if (currentToken().type != TokenType::RPAREN) {
                while (true) {
                    args.push_back(parseTernary());
                    if (currentToken().type == TokenType::COMMA) {
                        advance();
                        continue;
                    }
                    break;
                }
            }
            if (currentToken().type != TokenType::RPAREN) {
                throw std::runtime_error("Expected ')' after function arguments");
            }
            advance(); // consume )
            node = new ASTNode(NodeType::FUNCTION_CALL, tok.name, args);
        } else if (currentToken().type == TokenType::PLUS_PLUS) {
            advance();
            node = new ASTNode(NodeType::POST_INCREMENT, new ASTNode(tok.name), nullptr);
        } else if (currentToken().type == TokenType::MINUS_MINUS) {
            advance();
            node = new ASTNode(NodeType::POST_DECREMENT, new ASTNode(tok.name), nullptr);
        } else {
            node = new ASTNode(tok.name);
        }
    }
    else if (tok.type == TokenType::FN) {
        // Lambda expression: fn(args) { body }
        advance(); // consume fn
        if (currentToken().type != TokenType::LPAREN) {
            throw std::runtime_error("Expected '(' for lambda expression");
        }
        advance(); // consume (
        std::vector<ASTNode*> children;
        if (currentToken().type != TokenType::RPAREN) {
            while (true) {
                if (currentToken().type != TokenType::IDENTIFIER) {
                    throw std::runtime_error("Expected parameter name in lambda");
                }
                std::string paramName = currentToken().name;
                advance();
                if (currentToken().type == TokenType::ASSIGN) {
                    advance();
                    ASTNode* defaultVal = parseTernary();
                    children.push_back(new ASTNode(NodeType::ASSIGN, new ASTNode(paramName), defaultVal));
                } else {
                    children.push_back(new ASTNode(paramName));
                }
                if (currentToken().type == TokenType::COMMA) {
                    advance();
                    continue;
                }
                break;
            }
        }
        if (currentToken().type != TokenType::RPAREN) {
            throw std::runtime_error("Expected ')' after lambda parameters");
        }
        advance(); // consume )
        ASTNode* body = parseBlock();
        children.push_back(body);
        static int lambdaCounter = 0;
        std::string lname = "__lambda_" + std::to_string(lambdaCounter++);
        node = new ASTNode(NodeType::LAMBDA, lname, children);
    }
    else if (tok.type == TokenType::LPAREN) {
        advance();
        node = parseTernary();
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
