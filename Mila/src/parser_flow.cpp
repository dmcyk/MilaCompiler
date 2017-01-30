//
//  parser_flow.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "parser.hpp"

std::unique_ptr<NodeExpr> Parser::_if() {
    if (!(compareOptional(SpecialSymbolType::opening_bracket) || compareTokenOptional(TokenType::number) || compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::negate))) {
        error();
    }
    std::unique_ptr<NodeExpr> cond = _extract();
    
    compareKeyword(Keyword::_then);
    next();
    auto blocks = _ifPrime();
    
    auto thenBlock = std::move(std::get<0>(blocks));
    if (!thenBlock) {
        error();
    }
    return std::make_unique<IfExpr>(std::move(cond), std::move(thenBlock), std::move(std::get<1>(blocks)));
    
}
std::tuple<std::unique_ptr<BlockExpr>, std::unique_ptr<BlockExpr>> Parser::_ifPrime() {
    std::unique_ptr<BlockExpr> thenBlock = nullptr;
    std::unique_ptr<BlockExpr> elseBlock = nullptr;
    if (compareKeywordOptional(Keyword::_begin)) {
        thenBlock = _newScope();
        compare(SpecialSymbolType::semicolon); // semicolon of _end
        next();
        if (compareKeywordOptional(Keyword::_else)) {
            elseBlock = std::make_unique<BlockExpr>(_else());
        }
        
    } else if (compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::keyword)) {
        thenBlock = _block(false);
        elseBlock = _ifPrimePrime();
        
    } else {
        error();
    }
    return std::make_tuple(std::move(thenBlock), std::move(elseBlock));
    
}

std::unique_ptr<BlockExpr> Parser::_ifPrimePrime() {
    if (compareOptional(SpecialSymbolType::keyword) || compareTokenOptional(TokenType::expression)) {
        return _else();
    } else if (compareOptional(SpecialSymbolType::semicolon)) {
        next();
        return nullptr;
    } else {
        error();
    }
    
}

std::unique_ptr<BlockExpr> Parser::_else() {

    
    if (compareKeywordOptional(Keyword::_else)) {
        next();
        auto block = _block(true);
        return block;
    } else if (compareOptional(SpecialSymbolType::semicolon)) {
        next();
        return nullptr; // no else block, if finished and had it `_end` consumed in _newScope
    } else {
        error();
    }
    
}


std::unique_ptr<BlockExpr> Parser::_newScope() {
    compareKeyword(Keyword::_begin);
    next();
    return _scope();
}

std::unique_ptr<BlockExpr> Parser::_scope() {
    if (compareTokenOptional(TokenType::expression) || compareOptionalStartingKeywords()) {
        
        auto exprs = _kPrime({});
        std::unique_ptr<BlockExpr> block = std::make_unique<BlockExpr>(std::move(exprs));
        compareKeyword(Keyword::_end);
        next();
        return block;
    }
    compareKeyword(Keyword::_end);
    next();
    std::vector<std::unique_ptr<NodeExpr>> expr;
    return std::make_unique<BlockExpr>(std::move(expr));
    
}

void Parser::_scopePrime() {
    if (compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::keyword)) {
        _entry();
        _scopePrimePrime();
    } else {
        error();
    }
}

void Parser::_scopePrimePrime() {
    if (compareKeywordOptional(Keyword::_end)) {
    } else if (compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::keyword)) {
        _entry();
    } else {
        error();
    }
}

void Parser::_scopeTerm() {
    if (compareOptional(SpecialSymbolType::semicolon) || compareOptional(SpecialSymbolType::point)) {
        next();
    } else {
        error();
    }
}

std::unique_ptr<BlockExpr> Parser::_block(bool singleConsumeSemicolon) {
    
    std::unique_ptr<NodeExpr> single = nullptr;
    if (compareKeywordOptional(Keyword::_begin)) {
        return _newScope();
    } else if (compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::keyword)) {
        single = _k(singleConsumeSemicolon);
    } else {
        error();
    }
    if (single) {
        return  std::make_unique<BlockExpr>(std::move(single));
    }
    return nullptr;
}

