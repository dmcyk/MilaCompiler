//
//  parser_extras.cpp
//  Mila
//
//  Created by Damian Malarczyk on 02.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "parser_utils.hpp"
#include "ast_ctx.hpp"

using namespace AST;

_Noreturn void op_errorMessage(const char* format, ...) {
    std::cout << "Internal error. Parser operation computing failed" << std::endl;
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    std::cout << std::endl;
    exit(1);
}

std::experimental::optional<int64_t>  computeOperation(const OperationExpr* op, const AST::ASTContext& ctx) {
    OperationType type = op->getType();
    
    
    if (auto unary = dynamic_cast<const UnaryOperationExpr*>(op)) {
        auto type = unary->getType();
        if (auto operand = computeNumberNode(unary->getOperand().get(), ctx)) {
            if (type == op_negate) {
                return !*operand;
            } else if (type == op_unary_minus) {
                return 0 - *operand;
            } else {
                op_errorMessage("Incorrect unary operation");
                return std::experimental::nullopt;
            }
        }
        return std::experimental::nullopt;
        
    }
    auto binary = dynamic_cast<const BinaryOperationExpr*>(op);
    if (!binary) {
        return std::experimental::nullopt;
    }
    
    auto leftOptional = computeNumberNode((binary->getLhs()).get(), ctx);
    auto rightOptional = computeNumberNode(binary->getRhs().get(), ctx);
    
    if (!leftOptional || !rightOptional) {
        return std::experimental::nullopt;
    }
    
    uint64_t result;
    uint64_t left = *leftOptional;
    uint64_t right = *rightOptional;
    switch (type) {
        case op_undefined:
            op_errorMessage("Undefined opeartor");
            break;
        case op_negate: // mute warning
        case op_unary_minus:
            op_errorMessage("Unary operation in binary expression");
            break;
        case op_minus:
            result = left - right;
            break;
        case op_plus:
            result = left + right;
            break;
        case op_modulo:
            if (right == 0) {
                op_errorMessage("Error: division by zero");
                return std::experimental::nullopt;
            }
            result = left % right;
            break;
        case op_multiplication:
            result = left * right;
            break;
        case op_division:
            if (right == 0) {
                op_errorMessage("Error: division by zero");
                return std::experimental::nullopt;
            }
            result = left / right;
            break;
        case op_less:
            result = left < right;
            break;
        case op_greater:
            result = left > right;
            break;
        case op_or:
            result = left || right;
            break;
        case op_and:
            result = left && right;
            break;
        case op_equal:
            result = left == right;
            break;
        case op_not_equal:
            result = left != right;
            break;
        case op_less_or_equal:
            result = left <= right;
            break;
        case op_greater_or_equal:
            result = left >= right;
            break;
            
    }
    
    return result;
}

std::experimental::optional<int64_t>  computeNumberNode(const NodeExpr* node, const AST::ASTContext& ctx) {
    
    if (const NumberNodeExpr* expr =  dynamic_cast<const NumberNodeExpr*>(node)) {
        return expr->val;
    } else if (const ConstExpr* expr =  dynamic_cast<const ConstExpr*>(node)) {
        return computeNumberNode(expr->value.get(), ctx);
    } else if (const ValueExpr* expr =  dynamic_cast<const ValueExpr*>(node)) {
        auto found = ctx.globalNamedConstants.find(expr->getName());
        if (found != ctx.globalNamedConstants.end()) {
            return found->second->getValue().getLimitedValue();
        }
    } else if (const OperationExpr* op = dynamic_cast<const OperationExpr*>(node)){
        return computeOperation(op, ctx);
    }
    return std::experimental::nullopt;
    
}
