//
//  ast_values.h
//  Mila
//
//  Created by Damian Malarczyk on 27.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#ifndef ast_values_h
#define ast_values_h

#include "ast.hpp"

namespace AST {

    class NumberNodeExpr: public NodeExpr {
    public:
        uint64_t val;
        NumberNodeExpr(int val): val(val) {
            
        };
        
        virtual void print(const ASTContext& astCtx) const override;
        
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
    };
    
    class ConstExpr: public NodeExpr {
        
    public:
        std::string name;
        std::unique_ptr<NodeExpr> value;
        ConstExpr(const std::string& name, std::unique_ptr<NodeExpr> constValue): name(name), value(std::move(constValue)) {}
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
    };

    class ConstStringExpr: public NodeExpr {
        std::string content;
    public:
        ConstStringExpr(const std::string& content): content(content) {}
        
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
    };
    
    enum class VariableMutation {
        userMutable, innerMutable, notMutable
    };
    class VariableDeclarationExpr: public NodeExpr {
        std::string name;
        const ExpressionType* type;
        bool isGlobal;
    public:
        VariableMutation stackMutable = VariableMutation::userMutable;
        VariableDeclarationExpr(const std::string& name, const ExpressionType* type, bool isGlobal): name(name), type(type), isGlobal(isGlobal) {}
        
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
        const std::string& getName() const {
            return name;
        }
        
    };
    
    class ValueExpr: public NodeExpr {
    public:
        bool isReference;
        VariableMutation mutation = VariableMutation::userMutable;

        ValueExpr(bool isReference): isReference(isReference) {}
        
        virtual const std::string& getName() const = 0;
    };

    class ValueReferenceExpr: public ValueExpr {
    public:
        const std::string name;
        ValueReferenceExpr(const std::string& name, bool isReference): ValueExpr(isReference),name(name) {}
        ValueReferenceExpr(const std::string& name, bool isReference, const VariableMutation& mutation): ValueExpr(isReference),name(name) {
            ValueExpr::mutation = mutation;
        }

        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        virtual const std::string& getName() const override {
            return name;
        }
    };

    class SubscriptExpr: public ValueExpr {
        std::unique_ptr<ValueExpr> value;
        std::unique_ptr<NodeExpr> subscriptIndex;
    public:
        SubscriptExpr(std::unique_ptr<ValueExpr> value, std::unique_ptr<NodeExpr> subscript, bool isReference): ValueExpr(isReference), value(std::move(value)), subscriptIndex(std::move(subscript)) {};
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;

        virtual const std::string& getName() const override {
            return value->getName();
        }
    };
    
    class ValueAssignmentReferenceExpr: public NodeExpr {
    public:
        std::unique_ptr<ValueExpr> target;
        
        ValueAssignmentReferenceExpr(std::unique_ptr<ValueExpr> val): target(std::move(val)) {}
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
    };
}

#endif /* ast_values_h */
