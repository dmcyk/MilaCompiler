//
//  parser.c
//  LexialAnalyzer
//
//  Created by Damian Malarczyk on 11.11.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//
#include <cstdlib>
#include <iostream>
#include <string.h>
#include "parser_utils.hpp"
#include "utils.hpp"
#include "lexan.hpp"
#include "lexan_printing.hpp"

using namespace llvm;

/// organization rules
/// comparing and there's only epsilon in current entry in the table - do not read next symbol
/// comparing and then willing to check next thing - call next
/// comparing one symbol and thats all - call next after comparing
/// do not call next right after calling _ functions!! input already should be in given state

_Noreturn void Parser::error(Keyword k) {
    std::cout << "Parser error" << std::endl;
    std::cout << "Incorrect input found\nExpected ";
    std::cout << *getKeywordLiteral(k) << std::endl;
    exit(1);
}

_Noreturn void Parser::error(SpecialSymbolType ss) {
    std::cout << "Parser error" << std::endl;
    std::cout << "Incorrect input found\nExpected ";
    printSpecialSymbolDescription(ss);
    std::cout << "\nGot:" << std::endl;
    printToken(current);
    exit(1);
}
_Noreturn void Parser::error(TokenType t) {
    std::cout << "Parser error" << std::endl;

    std::cout << "Incorrect input found\nExpected ";
    std::string msg;
    switch (t) {
        case TokenType::number:
            msg = "number"; break;
        case TokenType::expression:
            msg = "expression"; break;
        case TokenType::specialsymbol:
            msg = "symbol"; break;
        case TokenType::string_constant:
            msg = "string constant"; break;
        case TokenType::incorrect:
            msg = "\\compiler error, incorrect token requested\\"; break;
    }
    std::cout << msg << std::endl;
    std::cout << "\nGot:" << std::endl;
    printToken(current);
    exit(1);
}


_Noreturn void Parser::error() {
    std::cout << "Parser error" << std::endl;
    std::cout << "Incorrect input at: \n";
    printToken(current);
    exit(1);
}

_Noreturn void Parser::errorMessage(const char* format, ...) {
    std::cout << "Parser error" << std::endl;
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    std::cout << std::endl;
    exit(1);
}


using namespace AST;


void Parser::_entry() {
    compareKeyword(Keyword::_program);
    next();
    
    compareToken(TokenType::expression);
    program = std::make_unique<ProgramExpr>(nullptr, std::vector<std::unique_ptr<NodeExpr>>(), current.data.expression);
    next();
    compare(SpecialSymbolType::semicolon);
    next();
    std::vector<std::unique_ptr<NodeExpr>> collected;
    while (1) {
        if (compareKeywordOptional(Keyword::_begin)) { // main
            if (program->main != nullptr) {
                errorMessage("Only one main function can be defined");
            }
            
            program->main = std::make_unique<FunctionExpr>("main", std::vector<std::unique_ptr<ParameterDefinitionExpr>>(), _newScope(), astCtx.getType(vtype_int32));
            compare(SpecialSymbolType::point);
            next();
            continue;
        }
        std::unique_ptr<NodeExpr> found = _entryPrime();
        
        if (found == nullptr) {
            break;
        } else {
            collected.push_back(std::move(found));
        }
    }
    program->globals = std::move(collected);
}

std::experimental::optional<FunctionAttr> attributeForName(const std::string& name) {
    if (name == "NoReturn") {
        return FunctionAttr::noreturn;
    } else if (name == "Inline") {
        return FunctionAttr::alwaysInline;
    } else if (name == "NoInline") {
        return FunctionAttr::noInline;
    }
    return std::experimental::nullopt;
}

void Parser::_fcnAttributes(std::set<FunctionAttr>& store) {
    if (compareOptional(SpecialSymbolType::at)) {
        next();
        compareToken(TokenType::expression);
        auto attrName = std::string(current.data.expression);
        next();
        auto attr = attributeForName(attrName);
        if (!attr) {
            errorMessage("Uknown attribute name");
        }
        store.insert(*attr);
        _fcnAttributes(store);
    } else if (compareKeywordOptional(Keyword::_function) || compareKeywordOptional(Keyword::_extern) || compareKeywordOptional(Keyword::_procedure)) {
        return;
    } else {
        error();
    }
}

std::unique_ptr<NodeExpr> Parser::_entryPrime() {
    if (compareKeywordOptional(Keyword::_var)) {
        std::vector<std::unique_ptr<NodeExpr>> first = _newVariable(true);
        
        _newVariableRepeat(true, first);
        return std::make_unique<MultipleExpr>(std::move(first));
    } else if (compareKeywordOptional(Keyword::_function) || compareKeywordOptional(Keyword::_extern) || compareKeywordOptional(Keyword::_procedure) || compareOptional(SpecialSymbolType::at)) {
        std::set<FunctionAttr> attributes;
        _fcnAttributes(attributes);
        
        bool isExtern = compareKeywordOptional(Keyword::_extern);
        if (isExtern) {
            next();
        }
        
        bool isProcedure = true;

        if (compareKeywordOptional(Keyword::_function)) {
            isProcedure = false;
        } else {
            compareKeyword(Keyword::_procedure);
        }
        next();
        
        compareToken(TokenType::expression);
        std::string name(current.data.expression);
        next();
        compare(SpecialSymbolType::opening_bracket);
        next();
        
        std::vector<std::unique_ptr<ParameterDefinitionExpr>> args;
        _parameters(args);
        compare(SpecialSymbolType::closing_bracket);
        next();
        auto type = astCtx.getType(vtype_void);

        if (isProcedure) {
            compare(SpecialSymbolType::semicolon);
            next();

        } else {
            compare(SpecialSymbolType::colon);
            next();
            compareToken(TokenType::expression);
            auto returnType = _getType(current.data.expression);
            if (!returnType) {
                errorMessage("Unknown function type: %s", current.data.expression);

            }
            type = *returnType;
                
            next();
            compare(SpecialSymbolType::semicolon);
            next();
        }
        
        std::unique_ptr<FunctionExpr> fExpr;
        if (isExtern) {
            fExpr = std::make_unique<FunctionExpr>(std::make_unique<PrototypeExpr>(name, std::move(args), PrototypeType::externd, type), nullptr);
        } else {
            fExpr = _functionDeclaration(std::make_unique<PrototypeExpr>(name, std::move(args), PrototypeType::local, type));
        }
        fExpr->attributes = attributes;
        
        return std::move(fExpr); 
    } else if (compareKeywordOptional(Keyword::_const)) {
        next();
        std::vector<std::unique_ptr<NodeExpr>> store;
        _let(store);
        _letRepeat(store);
        return std::make_unique<MultipleExpr>(std::move(store));
        
    }
    return nullptr;
}

std::unique_ptr<NodeExpr> Parser::_exprSemicolon() {
    auto res = _expr();
    compare(SpecialSymbolType::semicolon);
    next();
    return res;
}

std::unique_ptr<NodeExpr> Parser::_expr() {
    compareToken(TokenType::expression);
    std::string expr(current.data.expression);
    
    next();
    return _command(expr);
    
}

std::unique_ptr<AssignmentExpr> Parser::_assignment(std::unique_ptr<ValueAssignmentReferenceExpr> lhs) {
    compare(SpecialSymbolType::assignment);
    next();
    
    std::unique_ptr<NodeExpr> f = _extract();
    
    return std::make_unique<AssignmentExpr>(std::move(lhs), std::move(f));
}

void Parser::_callParameters(std::vector<std::unique_ptr<NodeExpr>>& store) {
    if (compareOptional(SpecialSymbolType::closing_bracket)) {

    } else if (compareOptional(SpecialSymbolType::coma)) {
        next();
        store.push_back(_extract());
        _callParameters(store);
    } else {
        error();
    }
    
}

std::vector<std::unique_ptr<NodeExpr>> Parser::_kPrime(std::vector<std::unique_ptr<NodeExpr>> exprs) {
    if (compareKeywordOptional(Keyword::_end)) {
        return exprs;
    }
    auto current = _k(true);
    if (!current) {
        errorMessage("");
    }
    exprs.push_back(std::move(current));
    auto next = _kPrime(std::move(exprs));
    return next;
}

void Parser::_kSemicolon() {
    compare(SpecialSymbolType::semicolon);
    next();
}

std::unique_ptr<NodeExpr> Parser::_k(bool consumeExpressionSemicolon) {
    if (compareTokenOptional(TokenType::expression)) {
        if (consumeExpressionSemicolon) {
            return _exprSemicolon();
        } else {
            return _expr();
        }
        
    }
    
    compare(SpecialSymbolType::keyword);

    if (compareKeywordOptional(Keyword::_for)) {
        next();
        compareToken(TokenType::expression);
        std::string forVarName(current.data.expression);
        std::unique_ptr<VariableDeclarationExpr> forVariable = std::make_unique<VariableDeclarationExpr>(forVarName, astCtx.getType(vtype_int), false);
        forVariable->stackMutable = VariableMutation::innerMutable;
        next();
        
        auto forVarStart = _assignment(std::make_unique<ValueAssignmentReferenceExpr>(std::make_unique<ValueReferenceExpr>(forVarName, true, VariableMutation::innerMutable)));
        auto range = _range();
        
        auto maxValue = _extract();
        compareKeyword(Keyword::_do);
        next();
        
        auto block = _block(false);
        
        _kSemicolon();
        
        return std::make_unique<ForLoopExpr>(
                                             std::get<0>(range),
                                             std::get<1>(range),
                                             std::move(block),
                                             std::move(forVariable),
                                             std::move(forVarStart),
                                             std::move(maxValue)
        );
    } else if (compareKeywordOptional(Keyword::_if)) {
        next();
        return _if();
    } else if (compareKeywordOptional(Keyword::_while)) {
        std::unique_ptr<LoopExpr> loop = std::make_unique<LoopExpr>();

        next();
        loop->condition = _extract();
        compareKeyword(Keyword::_do);
        next();
        loop->loopBlock = _newScope();
        _kSemicolon();
        
        return std::move(loop);
    } else if (compareKeywordOptional(Keyword::_exit)) {
        next();
        
        if (consumeExpressionSemicolon) {
            _kSemicolon();
        }

        return std::make_unique<ExitExpr>();
    } else if (compareKeywordOptional(Keyword::_switch)) {
        next();
        auto variable = _extract();
        compare(SpecialSymbolType::colon);
        next();
        auto switchExpr = std::make_unique<SwitchExpr>(std::move(variable));
        _case(*switchExpr);
        
        return std::move(switchExpr);
    } else {
        error();
    }
    
}

void Parser::_case(SwitchExpr& switchExpr) {
    if (compareKeywordOptional(Keyword::_case)) {
        next();
        auto first = _extract();
        
        std::unique_ptr<CaseValue> val;
        if (compareOptionalRangeKeyword()) {
            auto rng = _range();

            val = std::make_unique<CaseValue>(std::make_unique<RangeStore>(std::move(first), _extract(), rng));
        } else {
            val = std::make_unique<CaseValue>(std::move(first));
        }
        
        compareKeyword(Keyword::_then);
        next();
        
        auto scope = _block(false);
        compare(SpecialSymbolType::semicolon);
        next();
        
        switchExpr.cases.push_back(std::make_unique<CaseExpr>(std::move(val), std::move(scope)));
        _casePrime(switchExpr);
    } else if (compareKeywordOptional(Keyword::_default)) {
        next();
        if (switchExpr.defaultCase) {
            errorMessage("Switch statement can't have more than one default cases");
        }
        
        switchExpr.defaultCase = _block(false);
        compare(SpecialSymbolType::semicolon);
        next();
        _casePrime(switchExpr);
    } else {
        error();
    }
}

void Parser::_casePrime(SwitchExpr& switchExpr) {
    if (compareKeywordOptional(Keyword::_case) || compareKeywordOptional(Keyword::_default)) {
        _case(switchExpr);
    } else if (compareOptionalStartingKeywords() || compareTokenOptional(TokenType::expression) || compareOptional(SpecialSymbolType::semicolon)) {
        return;
    } else {
        error();
    }
}

std::tuple<LoopMode, LoopRange> Parser::_range() {
    if (compareKeywordOptional(Keyword::_to)) {
        next();
        return std::make_tuple(LoopMode::increasing, LoopRange::to);
    } else if (compareKeywordOptional(Keyword::_downto)) {
        next();
        return std::make_tuple(LoopMode::decreasing, LoopRange::to);
    } else if (compareKeywordOptional(Keyword::_until)) {
        next();
        return std::make_tuple(LoopMode::increasing, LoopRange::until);
    } else if (compareKeywordOptional(Keyword::_downuntil)) {
        next();
        return std::make_tuple(LoopMode::decreasing, LoopRange::until);
    } else {
        error();
    }
}

void Parser::_n() {
    compareToken(TokenType::number);
    next();
    
}

std::unique_ptr<NodeExpr> Parser::_value() {
    
    
    if (current.type == TokenType::number) {
        uint64_t val = current.data.number;
        _n();
        return std::make_unique<NumberNodeExpr>(val);
    } else if (current.type == TokenType::expression) {
        std::string name = current.data.expression;
        
        next();
        return _call(name);
        
    }
    error();
    
}


