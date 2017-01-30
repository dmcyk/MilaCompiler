//
//  parser_math.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "parser.hpp" 


void Parser::_cmp() {
    if (compareOptionalComparison()) {
        return;
    }
    error();
}

std::unique_ptr<NodeExpr> Parser::_extract() {
    if (!(compareOptional(SpecialSymbolType::opening_bracket) || compareTokenOptional(TokenType::number) || compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::negate) || compareOptional(SpecialSymbolType::minus))) {
        error();
    }
    
    auto found = _comparison();
    if (found) {
        found = _binary(std::move(found));
    }
    
    return found;
}

std::unique_ptr<NodeExpr> Parser::_binary(std::unique_ptr<NodeExpr> left) {
    
    if (compareKeywordOptional(Keyword::_and) || compareKeywordOptional(Keyword::_or)) {
        std::unique_ptr<BinaryOperationExpr> op = nullptr;
        OperationType type;
        if (current.data.specialSymbol.data.keyword == Keyword::_and) {
            type = op_and;
            
        } else {
            type = op_or;
        }
        
        next();
        std::unique_ptr<NodeExpr> right = _comparison();
        
        
        return _binary(std::make_unique<BinaryOperationExpr>(type, std::move(left), std::move(right)));
        
    } else if (compareOptionalExtractFollow()){
        return left;
    }
    error();
    
}


std::unique_ptr<NodeExpr> Parser::_comparison() {
    if (!(compareOptional(SpecialSymbolType::opening_bracket) || compareTokenOptional(TokenType::number) || compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::negate) || compareOptional(SpecialSymbolType::minus))) {
        error();
    }
    std::unique_ptr<NodeExpr> left = _operation();
    
    return _comparisonPrime(std::move(left));
}


std::unique_ptr<NodeExpr> Parser::_comparisonPrime(std::unique_ptr<NodeExpr> left) {
    if (compareOptionalComparison()) {
        
        OperationType type;
        switch (current.data.specialSymbol.type) {
            case SpecialSymbolType::greater:
                type = op_greater;
                break;
            case SpecialSymbolType::less:
                type = op_less;
                break;
            case SpecialSymbolType::equal:
                type = op_equal;
                break;
            case SpecialSymbolType::not_equal:
                type = op_not_equal;
                break;
            case SpecialSymbolType::greater_or_equal:
                type = op_greater_or_equal;
                break;
            case SpecialSymbolType::less_or_equal:
                type = op_less_or_equal;
                break;
            default:
                type = op_undefined;
                break;
        }
        _cmp();
        next();
        auto right = _operation();
        
        
        return _comparisonPrime(std::make_unique<BinaryOperationExpr>(type, std::move(left), std::move(right)));
        
    } else if (compareOptionalExtractFollow() || compareOptionalMath()) {
        return left;
    }
    
    return nullptr;
    
    
}

std::unique_ptr<NodeExpr> Parser::_operation() {
    if (!(compareOptional(SpecialSymbolType::opening_bracket) || compareTokenOptional(TokenType::number) || compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::negate) || compareOptional(SpecialSymbolType::minus))) {
        error();
    }
    auto left = _tExtract();
    
    return _operationPrime(std::move(left));
}

std::unique_ptr<NodeExpr> Parser::_operationPrime(std::unique_ptr<NodeExpr> left) {
    if (compareOptional(SpecialSymbolType::plus) || compareOptional(SpecialSymbolType::minus)) {
        
        OperationType type;
        if (compareOptional(SpecialSymbolType::plus)) {
            type = op_plus;
        } else if (compareOptional(SpecialSymbolType::minus)) {
            type = op_minus;
        }
        
        
        next();
        auto right = _tExtract();
        
        return _operationPrime(std::make_unique<BinaryOperationExpr>(type, std::move(left), std::move(right)));
        
    } else if (compareOptionalExtractFollow() || compareOptionalComparison() || compareOptionalMath()) {
        return left;
    }
    return nullptr;
    
}

std::unique_ptr<NodeExpr> Parser::_tExtract() {
    
    if (!(compareTokenOptional(TokenType::number) || compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::opening_bracket) || compareOptional(SpecialSymbolType::negate) || compareOptional(SpecialSymbolType::minus))) {
        error();
    }
    std::unique_ptr<NodeExpr> left = _fExtract();
    
    return _tExtractPrime(std::move(left));
}

std::unique_ptr<NodeExpr> Parser::_tExtractPrime(std::unique_ptr<NodeExpr> left) {
    
    if (compareOptional(SpecialSymbolType::multiplication)|| compareKeywordOptional(Keyword::_mod) || compareKeywordOptional(Keyword::_div)) {
        
        OperationType type;
        if (current.data.specialSymbol.type == SpecialSymbolType::multiplication) {
            type = op_multiplication;
        } else if (current.data.specialSymbol.data.keyword == Keyword::_div) {
            type = op_division;
        } else {
            type = op_modulo;
        }
        
        next();
        std::unique_ptr<NodeExpr> right = _fExtract();
        
        return _tExtractPrime(std::make_unique<BinaryOperationExpr>(type, std::move(left), std::move(right)));
        
    }  else if (compareOptionalExtractFollow() || compareOptionalMath() || compareOptionalComparison()) {
        return left;
    }
    return nullptr;
    
    
}


std::unique_ptr<NodeExpr> Parser::_fExtract() {
    if (compareOptional(SpecialSymbolType::opening_bracket)) {
        next();
        std::unique_ptr<NodeExpr> found = _extract();
        compare(SpecialSymbolType::closing_bracket);
        next();
        return found;
        
    } else {
        if (compareOptional(SpecialSymbolType::negate)) {
            next();
            std::unique_ptr<NodeExpr> operand = _fExtractPrime();
            
            return std::make_unique<UnaryOperationExpr>(op_negate, std::move(operand));
        } else if (compareOptional(SpecialSymbolType::minus)) {
            next();
            std::unique_ptr<NodeExpr> operand = _fExtractPrime();
            return std::make_unique<UnaryOperationExpr>(op_unary_minus, std::move(operand));
        } else {
            if (compareTokenOptional(TokenType::number) || compareTokenOptional(TokenType::expression))
                return _fExtractPrime();
            else {
                error();
            }
        }
        
    }
    
}

std::unique_ptr<NodeExpr> Parser::_fExtractPrime() {
    return _value();
    
}

