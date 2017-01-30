//
//  parser_dump_ast.cpp
//  Mila
//
//  Created by Damian Malarczyk on 02.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include "parser_utils.hpp"

using namespace AST;

void ValueReferenceExpr::print(const ASTContext& ctx) const {
    astDump << "(value_reference. name: " << name << ")\n";
}

void VariableDeclarationExpr::print(const ASTContext& ctx) const {
    astDump << "(declaration: " << name << ", type: " << expressionBuiltinTypeDescription(type->getBuiltin())  << ")\n";
}

void MultipleExpr::print(const ASTContext& ctx) const {
    for(auto& i: contained) {
        i->print(ctx);
    }
}

void NumberNodeExpr::print(const ASTContext& ctx) const {
    astDump << "(number_constant " << val << ")\n";
}

void ConstExpr::print(const ASTContext& ctx) const {
    astDump << "(global_constant " << *computeNumberNode(value.get(), ctx) << ")\n";
}

void ConstStringExpr::print(const ASTContext& ctx) const {
    astDump << "(const_string_expr\n";
    ++astDump;
    astDump << content << std::endl;
    --astDump;
    astDump << ")\n";
}
void UnaryOperationExpr::print(const ASTContext& ctx) const {
    astDump << "(unary_operation. type: " << operationTypeString(type) << std::endl;
    ++astDump;
    operand->print(ctx);
    --astDump;
}


void BinaryOperationExpr::print(const ASTContext& ctx) const {
    astDump << "(binary_operation, type: " << operationTypeString(type) << "\n";
    ++astDump;
    
    astDump.dump("lhs", *lhs, ctx);
    astDump.dump("rhs", *rhs, ctx);

    --astDump;
    astDump << ")\n";
}

void AssignmentExpr::print(const ASTContext& ctx) const {
    astDump << "(assignemnt\n";
    ++astDump;
    astDump.dump("to", *to, ctx);
    astDump.dump("value", *value, ctx);
    --astDump;
    astDump << ")\n";
}

void ParameterDefinitionExpr::print(const ASTContext& ctx) const  {
    astDump << "(parameter, name: " << name << ", type: ";
    type->printDescription();
    std::cout << ")" << std::endl;
}

void PrototypeExpr::print(const ASTContext& ctx) const {
    std::cout << "name: " << name << ", type: " << prototypetypeName(type) << std::endl;
    if (args.size() > 0) {
        ++astDump;
        astDump << "(arguments: " << std::endl;
        ++astDump;
        for (auto i = args.begin(); i != args.end(); i++) {
            (*i)->print(ctx);
            
        }
        --astDump;
        astDump << ")\n";
        --astDump;
    }
}

void FunctionExpr::print(const ASTContext& ctx) const  {
    astDump << "(function.";
    if (prototype->returnType)
        std::cout << " return type: " << expressionBuiltinTypeDescription(prototype->returnType->getBuiltin()) << ", ";
    prototype->print(ctx);
    if (block) {
        ++astDump;
        astDump.dump("func_block", *block, ctx);
        --astDump;
    }
    astDump << ")\n";
    
}

void ProgramExpr::print(const ASTContext& ctx) const  {
    astDump << "(program, name: " << name << std::endl;
    main->print(ctx);
    
    for (auto i = globals.begin(); i != globals.end(); i++) {
        (*i)->print(ctx);
    }
    astDump << ")\n";
    
}



void CallExpr::print(const ASTContext& ctx) const {
    astDump << "(function_call " << callee;
    if (args.size() > 0) {
        std::cout << std::endl;
        ++astDump;
        astDump << "(args: " << std::endl;
        ++astDump;
        for (auto i = args.begin(); i != args.end(); i++) {
            (*i)->print(ctx);
        }
        --astDump;
        astDump << ")\n";
        --astDump;
        astDump << ")\n";
        
    } else {
        std::cout << ")\n";
    }
    
}

void SubscriptExpr::print(const ASTContext& ctx) const {
    astDump << "(subscript" << std::endl;
    ++astDump;
    astDump.dump("value", *value, ctx);
    astDump.dump("index", *subscriptIndex, ctx);

    --astDump;
    astDump << ")\n";
}

void BlockExpr::print(const ASTContext& ctx) const {
    astDump << "(block";
    
    if (exprs.size() > 0) {
        std::cout << std::endl;
        ++astDump;
        for (auto i = exprs.begin(); i != exprs.end(); i++) {
            (*i)->print(ctx);
        }
        --astDump;
        astDump << ")\n";
    } else {
        std::cout << ")\n";
    }
    
}

void IfExpr::print(const ASTContext& ctx) const {
    astDump << "(if\n";
    ++astDump;
    
    astDump.dump("condition", *cond, ctx);
    astDump.dump("then", *then, ctx);

    if (els) {
        astDump.dump("else", *els, ctx);
    }
    --astDump;
    astDump << ")\n";
}

void ValueAssignmentReferenceExpr::print(const ASTContext& ctx) const {
    astDump.dump("value_asgn", *target, ctx);
}

void ForLoopExpr::print(const ASTContext& ctx) const {
    astDump << "(for_loop\n";
    ++astDump;
    astDump.dump("loop_declaration", *decl, ctx);
    astDump.dump("initial_asgn", *initialAsgn, ctx);
    astDump.dump("target", *target, ctx);
    astDump.dump("loop_block", *block, ctx);
    --astDump;
    astDump << ")\n";
}

void LoopExpr::print(const ASTContext& ctx) const {
    astDump << "(loop\n";
    ++astDump;
    if (preLoop) {
        astDump.dump("pre_loop", *preLoop, ctx);
    }
    
    astDump.dump("condition", *condition, ctx);
    
    astDump.dump("body", *loopBlock, ctx);
    
    if (increment) {
        astDump.dump("increment", *increment, ctx);
    }
    --astDump;
    astDump << ")\n";
}

void ExitExpr::print(const ASTContext& ctx) const {
    astDump << "(exit) \n";
}

void CaseExpr::print(const ASTContext& ctx) const {
    astDump << "(case\n";
    ++astDump;
    if (caseValue->storesRange) {
        astDump << "(value\n";
        ++astDump;
        astDump << "(range\n";
        ++astDump;
        astDump.dump("start_val", *caseValue->rng->start, ctx);
        astDump.dump("end_val", *caseValue->rng->end, ctx);
        --astDump;
        astDump << ")\n";
        --astDump;
    } else {
        astDump.dump("value", *caseValue->singleValue, ctx);
    }
    astDump.dump("case_block", *block, ctx);
    --astDump;
    astDump << ")\n";

    
}

void SwitchExpr::print(const ASTContext& ctx) const {
    astDump << "(switch\n";
    ++astDump;
    astDump.dump("cmp_target", *switchValue, ctx);
    for (auto& caseRef: cases) {
        caseRef->print(ctx);
    }
    astDump.dump("default", *defaultCase, ctx);
    --astDump;
    astDump << ")\n";
}
