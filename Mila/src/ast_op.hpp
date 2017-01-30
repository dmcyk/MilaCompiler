//
//  ast_operation.h
//  Mila
//
//  Created by Damian Malarczyk on 27.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#ifndef ast_operation_h
#define ast_operation_h
#include "ast_val.hpp"

namespace AST {
    
    
    class OperationExpr: public NodeExpr {
    protected:
        OperationType type;
    public:
        OperationExpr(OperationType type): type(type) {}
        OperationType getType() const {
            return type;
        }
        
    };
    
    class UnaryOperationExpr: public OperationExpr {
        std::unique_ptr<NodeExpr> operand;
    public:
        UnaryOperationExpr(OperationType type, std::unique_ptr<NodeExpr> op): OperationExpr(type), operand(std::move(op)) {}
        
        const std::unique_ptr<NodeExpr>& getOperand() const {
            return operand;
        }
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
    };
    
    class BinaryOperationExpr: public OperationExpr {
        std::unique_ptr<NodeExpr> lhs, rhs;
    public:
        BinaryOperationExpr(OperationType type, std::unique_ptr<NodeExpr> lhs, std::unique_ptr<NodeExpr> rhs): OperationExpr(type), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        
        const std::unique_ptr<NodeExpr>& getLhs() const {
            return lhs;
        }
        const std::unique_ptr<NodeExpr>& getRhs() const {
            return rhs;
        }
        
        virtual void print(const ASTContext& astCtx) const override;
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
        
    };
    
    
    
    class AssignmentExpr: public NodeExpr {
        std::unique_ptr<ValueAssignmentReferenceExpr> to;
        std::unique_ptr<NodeExpr> value;
    public:
        AssignmentExpr(std::unique_ptr<ValueAssignmentReferenceExpr> to, std::unique_ptr<NodeExpr> value): to(std::move(to)), value(std::move(value)) { }
        
        virtual void print(const ASTContext& astCtx) const override;
        
        virtual Value* generateCode(ASTContext& astCtx, LLVMContext& ctx, Module* m, IRBuilder<>& builder) override;
    };
    
    
}

#endif /* ast_operation_h */
