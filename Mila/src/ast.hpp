//
//  ast.h
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#ifndef ast_h
#define ast_h

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include "llvm/IR/LegacyPassManager.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "parser_structure.hpp"
#include <experimental/optional>
using namespace llvm;

namespace AST {
    bool isAllowedReturnType(ExpressionBuiltinType type);
    
    
    class ASTContext;
    
    class NodeExpr {
    public:
        virtual ~NodeExpr() {}
        virtual void print(const ASTContext& astCtx) const = 0;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) {
            return nullptr;
        };
        
        
    };
    
    class MultipleExpr: public NodeExpr {
        std::vector<std::unique_ptr<NodeExpr>> contained;
    public:
        MultipleExpr(std::vector<std::unique_ptr<NodeExpr>> contained): contained(std::move(contained)) {};
        
        virtual void print(const ASTContext& astCtx) const override;
        
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
    };

    
};





#endif /* ast_h */
