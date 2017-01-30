//
//  ast_math.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "ast_utils.hpp"
#include "ast_op.hpp"
using namespace AST;

Value* UnaryOperationExpr::generateCode(ASTContext& astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder) {
    Value* arg = operand->generateCode(astCtx, ctx, m, builder);
    if (!arg) {
        errorMessage("Missing unary operation value");
    }
    switch (type) {
        case op_negate:
            return builder.CreateNot(arg, "nottmp");
        case op_unary_minus:
            return builder.CreateSub(builder.getInt64(0), arg, "umintmp");
        case op_undefined:
            errorMessage("Internal error, requesting undefined binary operation");
        default:
            errorMessage("Internal error, unary operation with binary operator");

    }
}

Value* BinaryOperationExpr::generateCode(ASTContext& astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder) {
    Value* lhs = getLhs()->generateCode(astCtx, ctx, m, builder);
    Value* rhs = getRhs()->generateCode(astCtx, ctx, m, builder);
    if (!lhs || !rhs) {
        errorMessage("Missing binary operation values");
    }
    
    switch (type) {
        case op_plus: {
            return builder.CreateAdd(lhs, rhs, "addtmp");
        }
        case op_multiplication: {
            return builder.CreateMul(lhs, rhs, "multmp");
        }
        case op_minus: {
            return builder.CreateSub(lhs, rhs, "subtmp");
        }
        case op_division: {
            return builder.CreateSDiv(lhs, rhs, "divtmp");
        }
        case op_modulo: {
            return builder.CreateSub(lhs, builder.CreateMul(rhs, builder.CreateSDiv(lhs, rhs, "divtmp"), "subtmp"), "modtmp");
        }
        case op_greater: {
            return builder.CreateICmpSGT(lhs, rhs, "gcmptmp");
        }
        case op_greater_or_equal: {
            return builder.CreateICmpSGE(lhs, rhs, "gecmptmp");
        }
        case op_less: {
            return builder.CreateICmpSLT(lhs, rhs, "lcmptmp");
        }
        case op_less_or_equal: {
            return builder.CreateICmpSLE(lhs, rhs, "lecmptmp");
        }
        case op_equal: {
            return builder.CreateICmpEQ(lhs, rhs, "ecmptmp");
        }
        case op_not_equal: {
            return builder.CreateICmpNE(lhs, rhs, "necmptmp");
        }
        case op_and: {
            return builder.CreateAnd(lhs, rhs, "andtmp");
        }
        case op_or: {
            return builder.CreateOr(lhs, rhs, "ortmp");
        }
        case op_unary_minus:
        case op_negate:
            errorMessage("Internal error, binary operation with unary operator");
        case op_undefined:
            errorMessage("Internal error, requesting undefined binary operation");
    }
}

