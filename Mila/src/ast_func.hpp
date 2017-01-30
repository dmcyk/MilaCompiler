//
//  ast_func.h
//  Mila
//
//  Created by Damian Malarczyk on 27.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#ifndef ast_func_h
#define ast_func_h
#include "ast_flow.hpp"
#include <set>
namespace AST {
    
    class ParameterDefinitionExpr: public NodeExpr {
    public:
        std::string name;
        const ExpressionType* type;
        ParameterDefinitionExpr(const std::string& name, const ExpressionType* type): name(name), type(type) {}
        
        virtual void print(const ASTContext& astCtx) const override;
        inline bool operator==(const ParameterDefinitionExpr& rhs);
        inline bool operator!=(const ParameterDefinitionExpr& rhs);

    };
    
  
    class PrototypeExpr: public NodeExpr {
    public:
        
        std::string name;
        std::vector<std::unique_ptr<ParameterDefinitionExpr>> args;
        PrototypeType type;
        const ExpressionType* returnType;

        PrototypeExpr(const std::string& name, std::vector<std::unique_ptr<ParameterDefinitionExpr>> args, PrototypeType type, const ExpressionType* returnType): name(name), args(std::move(args)), type(type), returnType(returnType) {}
        
        virtual void print(const ASTContext& astCtx) const override;
        
        bool operator==(const PrototypeExpr& rhs);
        inline bool operator!=(const PrototypeExpr& rhs);

    };
    
    enum class FunctionAttr {
        noreturn,
        alwaysInline,
        noInline
    };
    
    
    class FunctionExpr: public NodeExpr {
        std::unique_ptr<BlockExpr> block;
        void constraint();
        
        void applyAttributes(Function* f, ASTContext& astCtx, LLVMContext& ctx, Module* m) const;
    public:
        BasicBlock* arrayCheckFailBlock = nullptr;
        std::set<FunctionAttr> attributes;
        std::unique_ptr<PrototypeExpr> prototype;
        
        FunctionExpr(const std::string& name, std::vector<std::unique_ptr<ParameterDefinitionExpr>> args, std::unique_ptr<BlockExpr> block, const ExpressionType* returnType): prototype(std::make_unique<PrototypeExpr>(name, std::move(args), PrototypeType::local, returnType)), block(std::move(block)) {
            constraint();
        }
        FunctionExpr(std::unique_ptr<PrototypeExpr> proto, std::unique_ptr<BlockExpr> block): prototype(std::move(proto)), block(std::move(block)) {
            constraint();
        }
        
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
                
        virtual void print(const ASTContext& astCtx) const override;
        
    };
    
    class ProgramExpr: public NodeExpr {
    public:
        std::unique_ptr<FunctionExpr> main;
        std::vector<std::unique_ptr<NodeExpr>> globals;
        std::string name;
        ProgramExpr(std::unique_ptr<FunctionExpr> main, std::vector<std::unique_ptr<NodeExpr>> globals, const std::string& name): main(std::move(main)), globals(std::move(globals)), name(name) {}
        
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
    };
}

#endif /* ast_func_h */
