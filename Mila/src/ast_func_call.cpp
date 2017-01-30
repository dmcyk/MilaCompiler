//
//  ast_func_call.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "ast_utils.hpp"
#include "ast_ctx.hpp" 
#include "parser_utils.hpp"
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Attributes.h>
using namespace AST;

inline bool ParameterDefinitionExpr::operator==(const ParameterDefinitionExpr& rhs) {
    if (name == rhs.name) {
        return (type == rhs.type);
    }
    return false;
}

inline bool ParameterDefinitionExpr::operator!=(const ParameterDefinitionExpr& rhs) {
    return !(*this == rhs);
}

bool PrototypeExpr::operator==(const PrototypeExpr& rhs) {
    if (name == rhs.name) {
        if (type == rhs.type) {
            if (args.size() == rhs.args.size()) {
                for (int i = 0; i < args.size(); i++) {
                    if (*(args[i]) != *(rhs.args[i])) {
                        return false;
                    }
                }
                return true;
            }
        }
    }
    return false;
}

inline bool PrototypeExpr::operator!=(const PrototypeExpr& rhs) {
    return !(*this == rhs);
}

Value* BlockExpr::generateCode(ASTContext& astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder) {
    for (auto i = exprs.begin(); i != exprs.end(); i++) {
        if (!(*i)->generateCode(astCtx, ctx, m, builder)) {
            if (dynamic_cast<ExitExpr*>((*i).get())) {
                break;
            } else {
                errorMessage("Erorr generating block, missing value");
            }
        }
    }
    return builder.getInt64(0);

}


void FunctionExpr::constraint() {
    auto builtin = prototype->returnType->getBuiltin();
    if (builtin != vtype_int && builtin != vtype_void) {
        if (prototype->name == "main" && builtin == vtype_int32) {
            return;
        }
        errorMessage("Non integer functions are unimplemented");
    }
}

void FunctionExpr::applyAttributes(llvm::Function *f, AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m) const {
    for (auto& attr: attributes) {
        switch (attr) {
            case FunctionAttr::noreturn: {
                f->addFnAttr(Attribute::AttrKind::NoReturn);
            } break;
            case FunctionAttr::alwaysInline: {
                f->addFnAttr(Attribute::AttrKind::AlwaysInline);
            } break;
            case FunctionAttr::noInline: {
                f->addFnAttr(Attribute::AttrKind::NoInline);
            } break; 
        }
    }
}


Value* FunctionExpr::generateCode(ASTContext& astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder)  {

    auto llReturnType = prototype->returnType->llvmType(ctx);
    if (!llReturnType) {
        errorMessage("Incorrect function type");
    }
    
    std::vector<Type*> args;
    for (auto& i: prototype->args) {
        args.push_back(i->type->llvmType(ctx));
    }
    auto builtin = prototype->returnType->getBuiltin();
    if (builtin != vtype_void) {
        if (!isAllowedReturnType(builtin)) {
            errorMessage("Type not implemented to be returned %s", expressionBuiltinTypeDescription(builtin));
        }
    }
    
    if (prototype->type != PrototypeType::externd) {
        
        Function* f;
        
        if (auto _f = m->getFunction(prototype->name)) {
            int foundPrototype = -1;
            for (int k = 0; k < astCtx.declaredPrototypes.size(); k++) {
                auto& currentPrototype = astCtx.declaredPrototypes[k];
                if (*(currentPrototype) == *(prototype)) {
                    foundPrototype = k;
                    break; 
                }
            }
            if (foundPrototype < 0)
                errorMessage("Function %s already defined", prototype->name.c_str());
            else {
                astCtx.declaredPrototypes.erase(astCtx.declaredPrototypes.begin() + foundPrototype);
                f = _f;
            }
            
        } else {
            FunctionType* ft = FunctionType::get(llReturnType, args, false);
            f = Function::Create(ft, Function::ExternalLinkage, prototype->name, m);
            unsigned inx = 0;
            for(auto &arg: f->args())
                arg.setName((prototype->args[inx++])->name);
            
            if (!block) { // forward declaration
                return nullptr;
            }
        }
        
        if (!block) { // forward declaration handled higher and return earlier if no body found
            // function can't be twice declared forward
            errorMessage("Missing function body for: %s", prototype->name.c_str());
        }
        
        AllocaInst* toReturn = nullptr;

        auto mainFnBlock = BasicBlock::Create(ctx, "entryFcnBlock", f);
        builder.SetInsertPoint(mainFnBlock);
        
        astCtx.incrementStack();
        
        unsigned inx = 0;
        astCtx.currentFn = f;
        astCtx.currentFnExpr = this;
        for(auto &arg: f->args()) {
            auto& prototypeArg = prototype->args[inx++];
            auto val = builder.CreateAlloca(prototypeArg->type->llvmType(ctx), 0, prototypeArg->name);
            builder.CreateStore(&arg, val);
            astCtx.insertToStack(prototypeArg->name, val, VariableMutation::userMutable);
            
        }
        
        if (builtin != vtype_void) {
            toReturn = builder.CreateAlloca(llReturnType, 0, prototype->name);
            builder.CreateStore(ConstantInt::get(ctx, APInt(builtin == vtype_int ? 64 : 32, 0, true)), toReturn);
            astCtx.insertToStack(prototype->name, toReturn, VariableMutation::userMutable);
        }
        
        block->generateCode(astCtx, ctx, m, builder);
        
        astCtx.currentFn = nullptr;
        astCtx.currentFnExpr = nullptr;
        mainFnBlock = builder.GetInsertBlock();

        builder.SetInsertPoint(mainFnBlock);
        
        if (!mainFnBlock->getTerminator()) {
            if (builtin != vtype_void) {
                builder.CreateRet(builder.CreateLoad(toReturn, prototype->name));
            } else {
                builder.CreateRetVoid();
            }
        }
        
        astCtx.popStack();
        if (verifyFunction(*f, &outs())) {
            printf("Function error\n");
            m->dump();
            exit(1);
        }
        
        if (astCtx.optimization) {
            astCtx.fpm->run(*f);
        }
        applyAttributes(f, astCtx, ctx, m);
        return f;
    } else {
        if (builtin != vtype_void && builtin != vtype_int) {
            errorMessage("Extern functions of type different than 64 bit Integer are not implemented");
        }
        FunctionType* ft = FunctionType::get(llReturnType, args, false);
        Function* f = Function::Create(ft, Function::ExternalLinkage, prototype->name, m);
        unsigned inx = 0;
        for(auto &arg: f->args())
            arg.setName((prototype->args[inx++])->name);
        astCtx.externs += 1;
        applyAttributes(f, astCtx, ctx, m);

        return f;
    }
}
