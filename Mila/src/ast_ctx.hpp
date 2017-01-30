//
//  ast_ctx.h
//  Mila
//
//  Created by Damian Malarczyk on 27.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#ifndef ast_ctx_h
#define ast_ctx_h
#include "ast_func.hpp" 
#include <experimental/optional>

namespace AST {
    
    enum class RuntimeErrorMessageKey {
        arrayIndexOverflow
    };
    class ASTContext {
        ExpressionType intType;
        ExpressionType int32Type;
        ExpressionType int8Type;
        ExpressionType voidType;
        PointerExpressionType pIntType;
        PointerExpressionType pInt32Type;
        PointerExpressionType pInt8Type;
        PointerExpressionType pVoidType;
        std::vector<std::map<std::string, AllocaInst*>> stack;
        std::vector<std::map<std::string, std::tuple<VariableMutation, AllocaInst*>>> mutableStack;

        std::vector<std::map<std::string, const ArrayExpressionType*>> arrayTypeStack;
        Value* getMutableValueOpt(const std::string& name, Module* m, std::experimental::optional<VariableMutation>) const;

        
    public:
        std::map<std::string, const ConstantInt*> globalNamedConstants;
        std::vector<std::unique_ptr<ArrayExpressionType>> arrayTypeStore;
        std::map<std::string, const ArrayExpressionType*> globalArrayType;
        int externs = 0;
        Function* currentFn;
        FunctionExpr* currentFnExpr;
        std::vector<std::unique_ptr<AST::PrototypeExpr>> declaredPrototypes;
        std::unique_ptr<legacy::FunctionPassManager> fpm;
        std::map<std::string, Value*> globalStringStore;
        std::map<std::string, Value*> runtimeErrorStringMessages;
        
        bool optimization;
        
        ASTContext():
        intType(ExpressionType(vtype_int)), int32Type(ExpressionType(vtype_int32)), voidType(ExpressionType(vtype_void)),
        int8Type(ExpressionType(vtype_int8)), pInt8Type(PointerExpressionType(vtype_int8)),
        pIntType(PointerExpressionType(vtype_int)), pInt32Type(PointerExpressionType(vtype_int32)), pVoidType(PointerExpressionType(vtype_void)) {
            fpm = nullptr;
            optimization = false;
        }
        
        void initializateForCodeGeneration(LLVMContext &ctx, Module* m);
        
        Value* runtimeErrorMessageForKey(RuntimeErrorMessageKey);
        const ExpressionType* getType(ExpressionBuiltinType type) const;
        const PointerExpressionType* getPointerTypeTo(ExpressionBuiltinType type) const;
        std::vector<std::unique_ptr<AST::NodeExpr>> milaStdLib() const;
        
        const AllocaInst* getFromStack(const std::string& name) const;
        const ArrayExpressionType* getFromArrayTypeStack(const std::string& name) const; 
        inline void insertToStack(const std::string& name, AllocaInst* inst, VariableMutation isMutable) {
            if (isMutable != VariableMutation::notMutable) {
                mutableStack[mutableStack.size() - 1][name] = std::make_tuple(isMutable, inst);
            } else {
                stack[stack.size() - 1][name] = inst;
            }
        }
        
        inline void insertArrayTypeStack(const std::string& name, const ArrayExpressionType* arr) {
            arrayTypeStack[arrayTypeStack.size() - 1][name] = arr;
        }
        
        inline void incrementStack() {
            std::map<std::string, AllocaInst*> inc;
            std::map<std::string, std::tuple<VariableMutation, AllocaInst*>> incM;

            stack.push_back(inc);
            mutableStack.push_back(incM);
            std::map<std::string, const ArrayExpressionType*> arrayInc;
            arrayTypeStack.push_back(arrayInc);
        }
        
        inline void incrementStackAccessingPrevious() {
            auto inc = stack[stack.size() - 1];
            stack.push_back(inc);
            auto incM = mutableStack[mutableStack.size() - 1];
            
            mutableStack.push_back(incM);
            std::map<std::string, const ArrayExpressionType*> arrayInc = arrayTypeStack[arrayTypeStack.size() - 1];
            arrayTypeStack.push_back(arrayInc);

        }
        
        inline void popStack() {
            stack.pop_back();
            mutableStack.pop_back(); 
            arrayTypeStack.pop_back();
        }
        
        const ArrayExpressionType* getArrayType(ExpressionBuiltinType to, int64_t start, int64_t end);
        const ArrayExpressionType* getVariableArrayType(const std::string& name); 
        
        Value* getValue(const std::string& name, Module* m) const;
        Value* getMutableValue(const std::string& name, Module* m, VariableMutation mutation) const;
        Value* getUserMutableValue(const std::string& name, Module* m) const;
        Value* getPrivateMutableValue(const std::string& name, Module* m) const;

    };
}

#endif /* ast_ctx_h */
