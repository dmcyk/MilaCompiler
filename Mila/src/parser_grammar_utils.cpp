//
//  parser_grammar_utils.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "parser.hpp"

void Parser::next() {
    current = nextToken(input);
}

int64_t Parser::extractArrayIndex(const NodeExpr* expr) {
    auto extracted = computeNumberNode(expr, astCtx);
    if (!extracted) {
        errorMessage("only static arrays are allowed");
    }
    return *extracted;
}
const ExpressionType* Parser::_type() {
    if (compareTokenOptional(TokenType::expression)) {
        auto type  = _getType(current.data.expression);
        if (type) {
            return *type;
        }
        errorMessage("Incorrect type %s given", current.data.expression);
        
    }

    compareKeyword(Keyword::_array);
    
    next();
    compare(SpecialSymbolType::opening_sq_bracket);
    next();
    auto start = _extract();
    auto startIndex = extractArrayIndex(start.get());
    
    compare(SpecialSymbolType::point);
    next();
    compare(SpecialSymbolType::point);
    next();
    auto to = _extract();
    auto endIndex = extractArrayIndex(to.get());
    
    if (startIndex > endIndex) {
        errorMessage("Incorrect array index, start index can't be bigger than end index. (%lld, %lld)", startIndex, endIndex);
    }
    
    compare(SpecialSymbolType::closing_sq_bracket);
    next();
    compareKeyword(Keyword::_of);
    next();
    
    auto innerType = _type();
    if (innerType->getBuiltin() != vtype_int) {
        errorMessage("Incorrect array type, only integer allowed");
    }
    return astCtx.getArrayType(innerType->getBuiltin(), startIndex, endIndex);

    
}

std::experimental::optional<const ExpressionType*> Parser::_getType(const std::string& str) {
    bool isPointer = str[str.length() - 1] == '*';
    std::string toCmp = isPointer ? str.substr(0, str.length() - 1) : str;
    std::experimental::optional<ExpressionBuiltinType> type;
    if (toCmp == "integer") {
        type = vtype_int;
    } else if (toCmp == "char") {
        type = vtype_int8;
    }
    
    if (type) {
        if (isPointer) {
            return astCtx.getPointerTypeTo(*type);
        } else {
            return astCtx.getType(*type);
        }
    }
    
    return std::experimental::nullopt;
}

bool Parser::compareOptional(SpecialSymbolType ssType) {
    if (current.type == TokenType::specialsymbol) {
        SpecialSymbol ss = current.data.specialSymbol;
        return ss.type == ssType;
    } else {
        return false;
    }
}

bool Parser::compareKeywordOptional(Keyword k) {
    if (current.type == TokenType::specialsymbol) {
        SpecialSymbol ss = current.data.specialSymbol;
        if (ss.type == SpecialSymbolType::keyword) {
            if (ss.data.keyword == k) {
                return true;
            }
        }
    }
    return false;
}

bool Parser::compareOptionalRangeKeyword() {
    return (compareKeywordOptional(Keyword::_to) || compareKeywordOptional(Keyword::_downto) || compareKeywordOptional(Keyword::_until) || compareKeywordOptional(Keyword::_downuntil));
}
bool Parser::compareOptionalExtractFollow() {
    return (compareKeywordOptional(Keyword::_do) || compareKeywordOptional(Keyword::_downto) || compareKeywordOptional(Keyword::_to) || compareKeywordOptional(Keyword::_downuntil) || compareKeywordOptional(Keyword::_until) || compareKeywordOptional(Keyword::_then) || compareOptional(SpecialSymbolType::semicolon) || compareOptional(SpecialSymbolType::colon) || compareOptional(SpecialSymbolType::closing_bracket) || compareOptional(SpecialSymbolType::closing_sq_bracket) || compareOptional(SpecialSymbolType::coma)
        || compareOptional(SpecialSymbolType::point) || compareKeywordOptional(Keyword::_else));
}

bool Parser::compareOptionalStartingKeywords() {
    return (compareKeywordOptional(Keyword::_for) || compareKeywordOptional(Keyword::_while) || compareKeywordOptional(Keyword::_if) || compareKeywordOptional(Keyword::_end) || compareKeywordOptional(Keyword::_switch));
}


bool Parser::compareOptionalMath() {
    return (compareOptional(SpecialSymbolType::plus) || compareOptional(SpecialSymbolType::minus) || compareOptional(SpecialSymbolType::multiplication) || compareKeywordOptional(Keyword::_and) || compareKeywordOptional(Keyword::_or) || compareKeywordOptional(Keyword::_mod));
}


bool Parser::compareOptionalComparison() {
    return (compareOptional(SpecialSymbolType::equal) || compareOptional(SpecialSymbolType::less) || compareOptional(SpecialSymbolType::greater) || compareOptional(SpecialSymbolType::greater_or_equal) || compareOptional(SpecialSymbolType::less_or_equal) || compareOptional(SpecialSymbolType::not_equal));
}

bool Parser::compareOptionalOperation() {
    return (compareOptionalMath() || compareOptionalComparison());
}

void Parser::compare(SpecialSymbolType ssType) {
    if (compareOptional(ssType) == false) {
        error(ssType);
    }
}

bool Parser::compareTokenOptional(TokenType tt) {
    return current.type == tt;
}

bool Parser::compareOptionalGlobalExpressionStart() {
    return (compareKeywordOptional(Keyword::_var) || compareKeywordOptional(Keyword::_begin) || compareKeywordOptional(Keyword::_const) || compareKeywordOptional(Keyword::_function) || compareKeywordOptional(Keyword::_procedure) || compareOptional(SpecialSymbolType::at) || compareKeywordOptional(Keyword::_extern));
}

void Parser::compareToken(TokenType tt) {
    if (compareTokenOptional(tt) == false) {
        error(tt);
    }
}


void Parser::compareKeyword(Keyword k) {
    if (compareKeywordOptional(k) == false) {
        error(k);
    }
}
