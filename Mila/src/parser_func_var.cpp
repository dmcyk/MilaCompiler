//
//  parser_func_var.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "parser.hpp"

std::unique_ptr<FunctionExpr> Parser::_functionDeclaration(std::unique_ptr<PrototypeExpr> prototype) {
    if (compareKeywordOptional(Keyword::_var) || compareKeywordOptional(Keyword::_begin)) {
        if (prototype->type == PrototypeType::externd) {
            errorMessage("extern function (%s) must not have a body", prototype->name.c_str());
        }
        std::vector<std::unique_ptr<NodeExpr>> expr;
        _localVars(expr);
        auto block = _newScope();
        
        expr.reserve(expr.size() + block->exprs.size());
        expr.insert(expr.end(), std::make_move_iterator(block->exprs.begin()), std::make_move_iterator(block->exprs.end()));
        
        block->exprs = std::move(expr);
        
        compare(SpecialSymbolType::semicolon);
        next();
        
        return std::make_unique<FunctionExpr>(std::move(prototype), std::move(block));
    } else if (compareKeywordOptional(Keyword::_forward)) {
        next();
        compare(SpecialSymbolType::semicolon);
        next();
        
        return std::make_unique<FunctionExpr>(std::move(prototype), nullptr);
        
    } else {
        error();
    }
}
void Parser::_parameters(std::vector<std::unique_ptr<ParameterDefinitionExpr>>&  collected) {
    if (compareOptional(SpecialSymbolType::closing_bracket)) {
        return;
    } else if (compareTokenOptional(TokenType::expression)) {
        std::string name(current.data.expression);
        next();
        compare(SpecialSymbolType::colon);
        next();
        compareToken(TokenType::expression);
        auto type = _getType(current.data.expression);
        if (!type) {
            errorMessage("Unknown parameter type %s", current.data.expression);
        }
        next();
        collected.push_back(std::make_unique<ParameterDefinitionExpr>(name, *type));
        _parametersPrime(collected);
        
    } else {
        error();
    }
    
}

std::vector<std::unique_ptr<NodeExpr>>  Parser::_functionParams() {
    std::vector<std::unique_ptr<NodeExpr>>  res;
    if (compareOptional(SpecialSymbolType::closing_bracket)) {
        
    } else if (compareOptional(SpecialSymbolType::opening_bracket) || compareTokenOptional(TokenType::expression) || compareTokenOptional(TokenType::number) || compareOptional(SpecialSymbolType::negate) || compareOptional(SpecialSymbolType::minus)) {
        res.push_back(_extract());
        _callParameters(res);
        
    } else if (compareTokenOptional(TokenType::string_constant)) {

        res.push_back(std::make_unique<ConstStringExpr>(std::string(current.data.expression)));
        next();
        _callParameters(res);
        
    } else {
        error();
    }
    return res;
}

void Parser::_parametersPrime(std::vector<std::unique_ptr<ParameterDefinitionExpr>>&  collected) {
    if (compareOptional(SpecialSymbolType::closing_bracket)) {
        return;
    } else if (compareOptional(SpecialSymbolType::semicolon)) {
        next();
        _parameters(collected);
        return;
    }
    error();
}

void Parser::_localVars(std::vector<std::unique_ptr<NodeExpr>>& store) {
    if (compareKeywordOptional(Keyword::_var)) {
        auto found = _newVariable(false);
        for (auto& a: found) {
            store.push_back(std::move(a));
        }
        
        _newVariableRepeat(false, store);
        _localVars(store);
    } else if (compareKeywordOptional(Keyword::_begin)) {
        return;
    } else {
        error();
    }
}

void Parser::_let(std::vector<std::unique_ptr<NodeExpr>>& store) {
    compareToken(TokenType::expression);
    auto name = std::string(current.data.expression);
    next();
    compare(SpecialSymbolType::equal);
    next();
    
    auto val = _extract();
    compare(SpecialSymbolType::semicolon);
    next();
    store.push_back(std::make_unique<ConstExpr>(name, std::move(val)));
}

void Parser::_letRepeat(std::vector<std::unique_ptr<NodeExpr>>& store) {
    if (compareOptionalGlobalExpressionStart()) {
        return;
    }
    compareToken(TokenType::expression);
    _let(store);
    _letRepeat(store);
}

std::vector<std::unique_ptr<NodeExpr>> Parser::_newVariable(bool isGlobal) {
    compareKeyword(Keyword::_var);
    next();
    std::vector<std::unique_ptr<NodeExpr>> store;
    _newVariablePrime(isGlobal, store);
    return store;
}

void Parser::_newVariablePrime(bool isGlobal, std::vector<std::unique_ptr<NodeExpr>>& store) {
    compareToken(TokenType::expression);
    std::string first(current.data.expression);
    
    
    next();
    auto found = _d();
    auto type = std::get<1>(found);
    store.push_back(std::make_unique<VariableDeclarationExpr>(first, type, isGlobal));
    for(auto& i: std::get<0>(found)) {
        store.push_back(std::make_unique<VariableDeclarationExpr>(i, type, isGlobal));
    }
    
    
}

void Parser::_newVariableRepeat(bool isGlobal, std::vector<std::unique_ptr<NodeExpr>>& store) {
    if (compareOptionalGlobalExpressionStart()) {
        return;
    }
    compareToken(TokenType::expression);
    _newVariablePrime(isGlobal, store);
    _newVariableRepeat(isGlobal, store);
}

std::tuple<std::experimental::optional<std::string>, std::experimental::optional<const ExpressionType*>> Parser::_dPrime() {
    std::experimental::optional<const ExpressionType*> type = std::experimental::nullopt;
    std::experimental::optional<std::string> name = std::experimental::nullopt;
    if (compareOptional(SpecialSymbolType::colon)) {
        next();
        
        type = _type();
        next();
        compare(SpecialSymbolType::semicolon);
        next();
    } else if (compareOptional(SpecialSymbolType::coma)) {
        next();
        compareToken(TokenType::expression);
        name = std::string(current.data.expression);
        next();
    } else {
        error();
    }
    return std::make_tuple(name, type);
}

std::tuple<std::vector<std::string>, const ExpressionType*> Parser::_d() {
    std::vector<std::string> names;
    std::experimental::optional<const ExpressionType*> type = std::experimental::nullopt;
    
    do {
        std::tuple<std::experimental::optional<std::string>, std::experimental::optional<const ExpressionType*>> res = _dPrime();
        auto name = std::get<0>(res);
        if (name) {
            names.push_back(*name);
        }
        type = std::get<1>(res);
    } while(type == std::experimental::nullopt);
    
    return std::make_tuple(names, *type);
    
}


