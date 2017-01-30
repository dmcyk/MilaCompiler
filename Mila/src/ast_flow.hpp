//
//  ast_flow.h
//  Mila
//
//  Created by Damian Malarczyk on 27.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#ifndef ast_flow_h
#define ast_flow_h
#include "ast_op.hpp" 
#include "utils.hpp" 

namespace AST {
    class BlockExpr: public NodeExpr {
        
    public:
        std::vector<std::unique_ptr<NodeExpr>> exprs;
        
        BlockExpr(std::vector<std::unique_ptr<NodeExpr>> exprs): exprs(std::move(exprs)) {}
        BlockExpr(std::unique_ptr<NodeExpr> single) {
            exprs.push_back(std::move(single));

        }
        virtual void print(const ASTContext& astCtx) const override;

        
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
    };
    
    class RangeStore {
    public:
        std::unique_ptr<NodeExpr> start;
        std::unique_ptr<NodeExpr> end;
        std::tuple<LoopMode, LoopRange> dscr;
        
        RangeStore(std::unique_ptr<NodeExpr> start, std::unique_ptr<NodeExpr> end, std::tuple<LoopMode, LoopRange> dscr):
        start(std::move(start)), end(std::move(end)), dscr(dscr) {
            
        }
        
        Range<int64_t> extractConst(const ASTContext& ctx) const;
    };
    
    class CaseValue {
        
    public:
        std::unique_ptr<RangeStore> rng;
        std::unique_ptr<NodeExpr> singleValue;
        bool storesRange;
        
        CaseValue(std::unique_ptr<RangeStore> rng): rng(std::move(rng)) {
            storesRange = true;
            singleValue = nullptr;
        }
        CaseValue(std::unique_ptr<NodeExpr> single): singleValue(std::move(single)) {
            storesRange = false;
            rng = nullptr;
        }
        
        Range<int64_t> extractConst(const ASTContext& ctx) const;

    };
    
    class CaseExpr: public NodeExpr {
    public:
        std::unique_ptr<CaseValue> caseValue;
        std::unique_ptr<BlockExpr> block;
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
        CaseExpr(std::unique_ptr<CaseValue> caseValue, std::unique_ptr<BlockExpr> block): caseValue(std::move(caseValue)), block(std::move(block)) {
            
        }
    };
    
    class SwitchExpr: public NodeExpr {
    public:
        std::unique_ptr<NodeExpr> switchValue;
        std::vector<std::unique_ptr<CaseExpr>> cases;
        std::unique_ptr<BlockExpr> defaultCase;
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
        void verifyCases(const ASTContext& astCtx) const;
        
        SwitchExpr(std::unique_ptr<NodeExpr> switchValue): switchValue(std::move(switchValue)) {
            
        }
        
    };
    
    class IfExpr: public NodeExpr {
        std::unique_ptr<NodeExpr> cond;
        std::unique_ptr<BlockExpr> then, els;
    public:
        IfExpr(std::unique_ptr<NodeExpr> cond,std::unique_ptr<BlockExpr> then,std::unique_ptr<BlockExpr> els): cond(std::move(cond)), then(std::move(then)), els(std::move(els)) {}
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        virtual void print(const ASTContext& astCtx) const override;
        
    };
    
    class CallExpr: public NodeExpr {
        std::string callee;
        std::vector<std::unique_ptr<NodeExpr>> args;
        
    public:
        CallExpr(const std::string &callee,
                 std::vector<std::unique_ptr<NodeExpr>> args)
        : callee(callee), args(std::move(args)) {}
        
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
    };
    
    class ExitExpr: public NodeExpr {
    public:
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
    };
    
    class LoopExpr: public NodeExpr {
    public:
        std::unique_ptr<NodeExpr> preLoop;
        std::unique_ptr<NodeExpr> loopBlock;
        std::unique_ptr<NodeExpr> increment;
        std::unique_ptr<NodeExpr> condition;
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
    };
    
    class ForLoopExpr: public LoopExpr {
        
    public:
        LoopMode mode;
        LoopRange range;
        std::unique_ptr<BlockExpr> block;
        std::unique_ptr<VariableDeclarationExpr> decl;
        std::unique_ptr<AssignmentExpr> initialAsgn;
        std::unique_ptr<NodeExpr> target;
        
        ForLoopExpr(const LoopMode mode,
                    const LoopRange range,
                    std::unique_ptr<BlockExpr> block,
                    std::unique_ptr<VariableDeclarationExpr> decl,
                    std::unique_ptr<AssignmentExpr> initialAsgn,
                    std::unique_ptr<NodeExpr> target)
        :
        mode(mode),
        range(range), 
        block(std::move(block)),
        decl(std::move(decl)),
        initialAsgn(std::move(initialAsgn)),
        target(std::move(target)) {
            
        }
        
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
    };

    
    
}

#endif /* ast_flow_h */
