//
//  parser_call.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "parser.hpp"
 
std::unique_ptr<ValueExpr> Parser::_subscript(std::unique_ptr<ValueExpr> value) {
    if (compareOptional(SpecialSymbolType::opening_sq_bracket)) {
        next();
        auto subscriptIndex = _extract();
        compare(SpecialSymbolType::closing_sq_bracket);
        std::unique_ptr<SubscriptExpr> subscript = std::make_unique<SubscriptExpr>(std::move(value), std::move(subscriptIndex), value->isReference);
        next();
        return _subscript(std::move(subscript));
        
    } else if (compareTokenOptional(TokenType::specialsymbol) || compareOptional(SpecialSymbolType::keyword))  {
        return value;
    } else {
        error();
    }
    
}

std::vector<std::unique_ptr<NodeExpr>> Parser::_functionCall() {
    compare(SpecialSymbolType::opening_bracket);
    next();
    
    auto ret = _functionParams();
    
    compare(SpecialSymbolType::closing_bracket);
    next();
    return ret;
}

std::unique_ptr<NodeExpr> Parser::_command(const std::string& expr) {
    if (compareOptional(SpecialSymbolType::assignment) || compareOptional(SpecialSymbolType::opening_sq_bracket)) {
        std::unique_ptr<ValueReferenceExpr> reference = std::make_unique<ValueReferenceExpr>(expr, true);
        
        return _assignment(std::make_unique<ValueAssignmentReferenceExpr>(_subscript(std::move(reference))));
    } else if (compareOptional(SpecialSymbolType::opening_bracket)) {
        std::unique_ptr<ValueReferenceExpr> reference = std::make_unique<ValueReferenceExpr>(expr, false);
        
        auto args = _functionCall();
        return std::make_unique<CallExpr>(expr, std::move(args));
    } else {
        error();
    }
}


std::unique_ptr<NodeExpr> Parser::_call(const std::string& callee) {
    if (compareOptional(SpecialSymbolType::opening_bracket)) {
        auto args = _functionCall();
        return std::make_unique<CallExpr>(callee, std::move(args));
        
    } else if (compareTokenOptional(TokenType::specialsymbol) || compareOptional(SpecialSymbolType::keyword)) {
        std::unique_ptr<ValueReferenceExpr> extracted = nullptr;

        if (callee[callee.length() - 1] == '&') {
            std::string name = callee.substr(0, callee.length() - 1);
            extracted = std::make_unique<ValueReferenceExpr>(name, true);
            
        } else {
            extracted = std::make_unique<ValueReferenceExpr>(callee, false);
        }
        return _subscript(std::move(extracted));
    }
    error();
}
