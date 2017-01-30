//
//  ast.c
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//


#include "ast_utils.hpp"
#include "ast_ctx.hpp"
using namespace llvm;
using namespace AST;

namespace AST {
    bool isAllowedReturnType(ExpressionBuiltinType type) {
        return type == vtype_int || type == vtype_int32;
    }
}


Value* MultipleExpr::generateCode(ASTContext& astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder) {
    for(auto& i: contained) {
        i->generateCode(astCtx, ctx, m, builder);
    }
    return builder.getInt64(0);
}


Value* ProgramExpr::generateCode(ASTContext& astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder) {
    m->setModuleIdentifier(name);
    
    std::vector<std::unique_ptr<NodeExpr>> stdlib = astCtx.milaStdLib();
    
    for (auto& i: stdlib) {
        i->generateCode(astCtx, ctx, m, builder);
    }
    astCtx.externs = 0; // important - stdlib will surely have implementation 

    astCtx.initializateForCodeGeneration(ctx, m);
    for (auto& i: globals) {
        auto res = i->generateCode(astCtx, ctx, m, builder);
        if (auto func = dynamic_cast<FunctionExpr*>(i.get())) {
            if (!res) { // missing function - forward
                astCtx.declaredPrototypes.push_back(std::move(func->prototype));
            }
        }
    }
    
    main->generateCode(astCtx, ctx, m, builder);
    
    return nullptr;
}
