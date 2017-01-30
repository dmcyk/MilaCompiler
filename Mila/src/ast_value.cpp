//
//  ast_value.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "ast_utils.hpp"
#include "ast_ctx.hpp"
#include "parser_utils.hpp"
#include <string>
using namespace AST;


Value* NumberNodeExpr::generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) {
    return builder.getInt64(val);
}

Value* ConstExpr::generateCode(AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m, IRBuilder<> &builder) {
    if (astCtx.globalNamedConstants.find(name) != astCtx.globalNamedConstants.end()) {
        errorMessage("Constant %s already defined", name.c_str());
    }
    auto compute = computeNumberNode(value.get(), astCtx);
    if (!compute) {
        errorMessage("Couldn't compute constant named %s", name.c_str());
    }
    
    auto val = ConstantInt::get(ctx, APInt(64, *compute, true));
    astCtx.globalNamedConstants[name] = val;
    return val;
    
}

Value* ConstStringExpr::generateCode(ASTContext& astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder) {
    auto alreadyDefined = astCtx.globalStringStore.find(content);
    
    if (alreadyDefined != astCtx.globalStringStore.end()) {
        return alreadyDefined->second;
    }
    auto val = builder.CreateGlobalStringPtr(content, "_const_string");
    astCtx.globalStringStore[content] = val;
    return val; 
}

Value* CallExpr::generateCode(ASTContext& astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder) {
    Function* f = m->getFunction(callee);
    if (!f) {
        errorMessage("Function %s not declared", callee.c_str());
    }
    if (args.size() != f->arg_size()) {
        errorMessage("Incorrect number of argument for function `%s`\n, expected: %d, got: %d", callee.c_str(), f->arg_size(), args.size());

    }
    std::vector<Value *> argv;
    for (size_t i = 0, e = args.size(); i != e; ++i) {
        argv.push_back(args[i]->generateCode(astCtx, ctx, m, builder));
        if (!argv.back()) {
            errorMessage("Couldn't generate arguments for function `%s` call", callee.c_str());
        }
    }
    
    return builder.CreateCall(f, argv);
}

Value* VariableDeclarationExpr::generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) {
    
    auto llvmType = type->llvmType(ctx);
    if (!llvmType) {
        errorMessage("Incorrect variable type given: %s", type->getBuiltin());
    }
    
    if (isGlobal) {
        auto redefined = astCtx.getValue(name, m);
        if (redefined) {
            errorMessage("Value %s already defined in current scope", name.c_str());
        }
        
        Constant* initializer;
        if (type->getBuiltin() == vtype_array) {
            initializer = ConstantAggregateZero::get(llvmType);
            astCtx.globalArrayType[name] = dynamic_cast<const ArrayExpressionType*>(type);
        } else {
            initializer = builder.getInt64(0);
        }
        
        GlobalVariable* global = new GlobalVariable(
                                                    *m,
                                                    llvmType,
                                                    false,
                                                    GlobalVariable::LinkageTypes::ExternalLinkage,
                                                    initializer,
                                                    name);
        
        return global;
        
    } else {
        if (!astCtx.currentFn) {
            errorMessage("Internal error, no parent function defined for %s variable", name.c_str());
        }
        
        if (astCtx.getFromStack(name)) {
            errorMessage("Value %s already defined in current scope", name.c_str());
        }
        auto fn = astCtx.currentFn;
        IRBuilder<> tmpBuilder(&fn->getEntryBlock(),fn->getEntryBlock().begin());
        
        auto var = tmpBuilder.CreateAlloca(llvmType, nullptr, name);
        
        astCtx.insertToStack(name, var, stackMutable);
        if (type->getBuiltin() == vtype_array) {
            astCtx.insertArrayTypeStack(name, dynamic_cast<const ArrayExpressionType*>(type));
            
        }
        return var;
    }
};
Value* AssignmentExpr::generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) {

    Value* targetVar = to->generateCode(astCtx, ctx, m, builder);
    Value* toAssign = value->generateCode(astCtx, ctx, m, builder);

    return builder.CreateStore(toAssign, targetVar);
};

Value* ValueReferenceExpr::generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) {

    if (!isReference) {
        Value* val = astCtx.getValue(name, m);
        if (val) {
            if (isa<ConstantInt>(val)) {
                return val; 
            }
            return builder.CreateLoad(val, name);
        } else {
            errorMessage("No such value ", name.c_str());
        }
    }
    
    Value* value = astCtx.getMutableValue(name, m, ValueExpr::mutation);
    
    if (!value) {
        errorMessage("Referenced value %s does not exist or is immutable", name.c_str());
        
    }
    return value;
};

Value* SubscriptExpr::generateCode(AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m, IRBuilder<> &builder) {
    auto& name = getName();
    
    Value* val;
    
    if (isReference) {
        val = astCtx.getMutableValue(name, m, ValueExpr::mutation);
    } else {
        val = astCtx.getValue(name, m);
    }
    

    if (!val) {
        errorMessage("Value %s not defined", name.c_str());
        
    }
    
    Value* usrIndx = subscriptIndex->generateCode(astCtx, ctx, m, builder);
    auto arrType = astCtx.getVariableArrayType(name);
    
    Value* indx[2];
    indx[0] = builder.getInt64(0);
    indx[1] = builder.getInt64(0);
    
    if (!astCtx.optimization) {
        auto _f =  builder.GetInsertBlock()->getParent();
        
        auto isSmaller = builder.CreateICmpSLT(usrIndx, builder.getInt64(arrType->startIndex));
        auto isBigger = builder.CreateICmpSGT(usrIndx, builder.getInt64(arrType->endIndex));
        auto check = builder.CreateOr(isSmaller, isBigger);
        
        BasicBlock* arrCheckFail;
        

        bool canReuseFail = false;
        
        if (astCtx.currentFnExpr) {
            if (astCtx.currentFnExpr->arrayCheckFailBlock) {
                canReuseFail = true;
                arrCheckFail = astCtx.currentFnExpr->arrayCheckFailBlock;
            } else {
                arrCheckFail = BasicBlock::Create(ctx, "indexcheckfail", _f);
                astCtx.currentFnExpr->arrayCheckFailBlock = arrCheckFail;
                
            }
        } else {
            arrCheckFail = BasicBlock::Create(ctx, "indexcheckfail", _f);
        }
        
        BasicBlock* arrPostCheckBlock = arrPostCheckBlock = BasicBlock::Create(ctx, "indexchecksuccess", _f);
        builder.CreateCondBr(check, arrCheckFail, arrPostCheckBlock);
        
        if (!canReuseFail) {
            builder.SetInsertPoint(arrCheckFail);
            auto exitFunc = m->getFunction("_exitMessage");
            auto refIndx = ArrayRef<Value*>(indx);
            auto message = astCtx.runtimeErrorMessageForKey(RuntimeErrorMessageKey::arrayIndexOverflow);
            auto ptr = builder.CreateInBoundsGEP(message, refIndx, "_arrfailmsg");
            builder.CreateCall(exitFunc, {builder.getInt64(1), ptr});
            builder.CreateUnreachable(); 
        }
       
        builder.SetInsertPoint(arrPostCheckBlock);
        
    }
   
    indx[1] = builder.CreateSub(usrIndx, builder.getInt64(arrType->startIndex));
    
    auto refIndx = ArrayRef<Value*>(indx);
    
    
    auto atIndexVal = builder.CreateGEP(val, refIndx, "arridx");
    if (isReference) {
        return atIndexVal;
    } else {
        return builder.CreateLoad(atIndexVal, "arridxval");
    }
}

Value* ValueAssignmentReferenceExpr::generateCode(ASTContext &astCtx, LLVMContext &ctx, Module *m, IRBuilder<> &builder) {

    auto value = target->generateCode(astCtx, ctx, m, builder);

    return value;
    
}


