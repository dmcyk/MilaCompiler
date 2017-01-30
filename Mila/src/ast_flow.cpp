//
//  ast_flow.cpp
//  Mila
//
//  Created by Damian Malarczyk on 25.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "ast_ctx.hpp"
#include "parser.hpp"
#include "ast_utils.hpp"
using namespace llvm;
using namespace AST;

Range<int64_t> RangeStore::extractConst(const ASTContext& ctx) const {
    std::vector<int64_t> res;
    
    auto startVal = computeNumberNode(start.get(), ctx);
    
    auto endVal = computeNumberNode(end.get(), ctx);
    
    if (!startVal || !endVal) {
        errorMessage("Error extracting range, value must be constant");
    }
    auto mode = std::get<0>(dscr);
    auto range = std::get<1>(dscr);
    
    int64_t startFinal;
    int64_t endFinal;
    if (mode == LoopMode::decreasing) {
        endFinal = *startVal;
        
        startFinal = *endVal;
        if (range == LoopRange::until) {
            startFinal += 1; 
        }
    } else {
        endFinal = *endVal;
        if (range == LoopRange::until) {
            endFinal -= 1;
        }
        startFinal = *startVal;
    }
    
    return Range<int64_t>(startFinal, endFinal);
    
}

Range<int64_t> CaseValue::extractConst(const ASTContext& ctx) const {
    if (storesRange) {
        return rng->extractConst(ctx);
    }
    auto val = computeNumberNode(singleValue.get(), ctx);
    if (!val) {
        errorMessage("Error extracting case value, it must be constant");
    }
    
    return Range<int64_t>(*val, *val);
}

Value* ExitExpr::generateCode(AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m, IRBuilder<> &builder) {
    auto cfunc = astCtx.currentFn;
    if (!cfunc) {
        auto mainRet = astCtx.getMutableValue("main", m, VariableMutation::userMutable);
        if (!mainRet) {
            errorMessage("Incorrect exit statement");
        }
        return builder.CreateRet(builder.CreateLoad(mainRet));
    }
    if (cfunc->getReturnType() == Type::getVoidTy(ctx)) {
        return builder.CreateRetVoid();
    }
    auto val = astCtx.getMutableValue(cfunc->getName(), m, VariableMutation::userMutable);
    if (!val) {
        errorMessage("Incorrect exit statement");
    }
    return builder.CreateRet(builder.CreateLoad(val, cfunc->getName()));
}


Value* IfExpr::generateCode(AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m, IRBuilder<> &builder){
    Value* condValue = cond->generateCode(astCtx, ctx, m, builder);
    if (!condValue) {
        return nullptr;
    }
    
    if (condValue->getType() != Type::getInt1Ty(ctx)) {
        condValue = builder.CreateICmpNE(condValue, builder.getInt64(0), "tmpcmpne");

    }

    auto fcn = builder.GetInsertBlock()->getParent();
    
    auto thenBlock = BasicBlock::Create(ctx, "then", fcn);
    auto elseBlock = BasicBlock::Create(ctx, "else", fcn);
    auto mergeBlock = BasicBlock::Create(ctx, "postifcnd", fcn);

    builder.CreateCondBr(condValue, thenBlock, elseBlock);
    
    
    builder.SetInsertPoint(thenBlock);
    
    then->generateCode(astCtx, ctx, m, builder);
    
    thenBlock = builder.GetInsertBlock();
    
    if (!thenBlock->getTerminator()) {
        builder.CreateBr(mergeBlock);
    }
    
    builder.SetInsertPoint(elseBlock);

    if (els) {
        els->generateCode(astCtx, ctx, m, builder);
        
        elseBlock = builder.GetInsertBlock();
        
        if (!elseBlock->getTerminator()) {
            builder.CreateBr(mergeBlock);
        }
    } else {
        builder.CreateBr(mergeBlock);
    }
    
    builder.SetInsertPoint(mergeBlock);
    
    return mergeBlock;
}

Value* LoopExpr::generateCode(AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m, IRBuilder<> &builder) {
    astCtx.incrementStackAccessingPrevious();
    if (preLoop) {
        preLoop->generateCode(astCtx, ctx, m, builder);
    }
    
    auto fcn = builder.GetInsertBlock()->getParent();

    auto loopBody = BasicBlock::Create(ctx, "loopbody", fcn);
    auto loopEnd = BasicBlock::Create(ctx, "loopend", fcn);
    
    builder.CreateBr(loopEnd);
    
    builder.SetInsertPoint(loopBody);
    
    loopBlock->generateCode(astCtx, ctx, m, builder);
    
    if (increment) {
        increment->generateCode(astCtx, ctx, m, builder);
    }
    
    builder.CreateBr(loopEnd);
    
    builder.SetInsertPoint(loopEnd);
    
    Value* cond = condition->generateCode(astCtx, ctx, m, builder);
    
    // could have changed during generation - dont it with loopBody as it will have to start at the same point
    loopEnd = builder.GetInsertBlock();
    
    builder.SetInsertPoint(loopEnd);
    
    auto postLoop = BasicBlock::Create(ctx, "postloop", fcn);

    builder.CreateCondBr(cond, loopBody, postLoop);
    builder.SetInsertPoint(postLoop);
    astCtx.popStack();
    return builder.getInt64(0);
}

Value* ForLoopExpr::generateCode(AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m, IRBuilder<> &builder) {
    auto valueName = decl->getName();
    
    std::vector<std::unique_ptr<NodeExpr>> declAssign;
    declAssign.push_back(std::move(decl));
    declAssign.push_back(std::move(initialAsgn));
    this->LoopExpr::preLoop = std::make_unique<MultipleExpr>(std::move(declAssign));
    
    this->LoopExpr::loopBlock = std::move(block);
    
    OperationType op;

    if (mode == LoopMode::increasing) {
        op = OperationType::op_plus;
    } else {
        op = OperationType::op_minus;
    }
    std::unique_ptr<NodeExpr> res = std::make_unique<BinaryOperationExpr>(op, std::make_unique<ValueReferenceExpr>(valueName, false), std::make_unique<NumberNodeExpr>(1));
   

    auto asgnRef = std::make_unique<ValueAssignmentReferenceExpr>(std::make_unique<ValueReferenceExpr>(valueName, true, VariableMutation::innerMutable));

    this->increment = std::make_unique<AssignmentExpr>(std::move(asgnRef), std::move(res));
    
    auto ref = std::make_unique<ValueAssignmentReferenceExpr>(std::make_unique<ValueReferenceExpr>(valueName, false));

    std::unique_ptr<NodeExpr> rangedTarget;
    if (range == LoopRange::until) {
        rangedTarget = std::move(target);
    } else {
        rangedTarget = std::make_unique<BinaryOperationExpr>(op, std::move(target), std::make_unique<NumberNodeExpr>(1));
    }
    
    this->LoopExpr::condition = std::make_unique<UnaryOperationExpr>(op_negate, std::make_unique<BinaryOperationExpr>(op_equal, std::move(ref), std::move(rangedTarget)));
    return this->LoopExpr::generateCode(astCtx, ctx, m, builder);
}

Value* CaseExpr::generateCode(AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m, IRBuilder<> &builder) {
    errorMessage("Internal error trying to generate switch case code");
}

void SwitchExpr::verifyCases(const ASTContext& ctx) const {
    std::vector<Range<int64_t>> ranges;
    for (auto& caseInst: cases) {
        ranges.push_back(caseInst->caseValue->extractConst(ctx));
    }
    for (int i = 0; i < ranges.size(); i++) {
        auto current = ranges[i];
        
        if (!current.verify()) {
            errorMessage("Incorrect range given - (%lld,%lld). Starting value may not be bigger than ending.", *current.begin(), *current.end()); 
        }
        for (int j = 0; j < ranges.size(); j++) {
            if (i != j) {
                auto& toCheck = ranges[j];
                if (current.overlapsWith(toCheck)) {
                    errorMessage("Incorrect case values with ranges (%lld,%lld) - (%lld,%lld)", *current.begin(), *current.end(), *toCheck.begin(), *toCheck.end());
                }
            }
        }
    }
    
}

Value* SwitchExpr::generateCode(AST::ASTContext &astCtx, llvm::LLVMContext &ctx, llvm::Module *m, IRBuilder<> &builder) {
    auto val = switchValue->generateCode(astCtx, ctx, m, builder);
    auto fcn = astCtx.currentFn;
    auto defBlock = BasicBlock::Create(ctx, "switchdef", fcn);
    auto postSwitch = BasicBlock::Create(ctx, "postswitch", fcn);
    
    auto mainBlock = builder.GetInsertBlock();
    
    builder.SetInsertPoint(defBlock);
    
    defaultCase->generateCode(astCtx, ctx, m, builder);
    
    defBlock = builder.GetInsertBlock();
    
    if (!defBlock->getTerminator()) {
        builder.CreateBr(postSwitch);
    }

    auto switchInst = SwitchInst::Create(val, defBlock, cases.size());
    
    verifyCases(astCtx);
    
    for (auto& caseInst: cases) {
        auto caseValue = caseInst->caseValue->extractConst(astCtx);
        
        auto caseBlock = BasicBlock::Create(ctx, "switch_case", fcn);
        builder.SetInsertPoint(caseBlock);
        caseInst->block->generateCode(astCtx, ctx, m, builder);
        caseBlock = builder.GetInsertBlock();
        
        if (!caseBlock->getTerminator()) {
            builder.CreateBr(postSwitch);
        }
        
        for (auto& val: caseValue) {
            switchInst->addCase(ConstantInt::get(ctx, APInt(64, val, true)), caseBlock);
        }
    }
    builder.SetInsertPoint(mainBlock);

    builder.Insert(switchInst);
    builder.SetInsertPoint(postSwitch);
    return builder.getInt64(0);
    
}

