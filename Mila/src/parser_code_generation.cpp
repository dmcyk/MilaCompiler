//
//  parser_code_generation.cpp
//  Mila
//
//  Created by Damian Malarczyk on 24.01.2017.
//  Copyright © 2017 Damian Malarczyk. All rights reserved.
//

#include <stdio.h>
#include "parser.hpp"
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

using namespace llvm;

std::string hexStdLibString = "23696e636c756465203c696f73747265616d3e0d0a23696e636c756465203c637374646c69623e0d0a0d0a65787465726e20224322207b0d0a20202020766f69642077726974656c6e28696e7436345f74206929207b0d0a20202020202020207072696e74662822256c6c645c6e222c2069293b0d0a202020207d0d0a20202020766f696420777269746528696e7436345f74206929207b0d0a20202020202020207072696e74662822256c6c64222c2069293b0d0a202020207d0d0a202020200d0a20202020766f696420726561646c6e28696e7436345f742a2070747229207b0d0a20202020202020207374643a3a737472696e6720627566663b0d0a20202020202020207374643a3a63696e203e3e20627566663b0d0a20202020202020202a707472203d207374643a3a73746f692862756666293b0d0a202020207d0d0a202020200d0a20202020766f696420696e6328696e7436345f742a2070747229207b0d0a2020202020202020282a707472292b2b3b0d0a202020207d0d0a202020200d0a20202020766f69642064656328696e7436345f742a2070747229207b0d0a2020202020202020282a707472292d2d3b0d0a202020207d0d0a0d0a20202020766f6964207072696e7428636861722a2070747229207b0d0a20202020202020207374643a3a636f7574203c3c207074723b0d0a202020207d0d0a0d0a20202020766f6964206563686f28636861722a2070747229207b0d0a20202020202020207374643a3a636f7574203c3c20707472203c3c207374643a3a656e646c3b0d0a202020207d0d0a20202020766f6964206e65774c696e652829207b0d0a20202020202020207374643a3a636f7574203c3c207374643a3a656e646c3b0d0a202020207d0d0a0d0a202020205f4e6f72657475726e200d0a20202020766f6964205f6578697428696e7436345f74206929207b0d0a2020202020202020657869742869293b0d0a202020207d0d0a0d0a202020205f4e6f72657475726e200d0a20202020766f6964205f657869744d65737361676528696e7436345f7420692c20636861722a2070747229207b0d0a20202020202020206563686f28707472293b0d0a2020202020202020657869742869293b0d0a202020207d0d0a0d0a7d";

int Parser::dumpCode(const std::string& outputName, const std::string& srcFile, bool dumpIR, bool optimization) {
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
    
    LLVMContext context;
    IRBuilder<> builder(context);
    std::unique_ptr<Module> module = llvm::make_unique<Module>("MilaProgram", context);

    auto targetTriple = sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);
    
    std::string error;
    auto target = TargetRegistry::lookupTarget(targetTriple, error);
    
    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!target) {
        errs() << error;
        return 1;
    }
    
    auto cpu = "generic";
    auto features = "";
    
    TargetOptions opt;
    auto rm = Optional<Reloc::Model>();
    auto targetMachine = target->createTargetMachine(module->getTargetTriple(), cpu, features, opt, rm);
    
    module->setDataLayout(targetMachine->createDataLayout());
    
    std::error_code ec;
    raw_fd_ostream dest(outputName, ec, sys::fs::F_None);
    
    if (ec) {
        errs() << "Could not open file: " << ec.message();
        return 1;
    }
    
    legacy::PassManager pass;
    auto fileType = TargetMachine::CGFT_ObjectFile;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, fileType)) {
        errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }
    
    auto fpm = llvm::make_unique<legacy::FunctionPassManager>(module.get());
    fpm->add(createInstructionCombiningPass());
    fpm->add(createSROAPass());
    fpm->add(createEarlyCSEPass());
    fpm->add(createLCSSAPass());
    fpm->add(createReassociatePass());
    fpm->add(createGVNPass());
    fpm->add(createCFGSimplificationPass());
    fpm->doInitialization();
    astCtx.fpm = std::move(fpm);
    astCtx.optimization = optimization;
    module->setSourceFileName(srcFile);
    
    program->generateCode(astCtx, context, module.get(), builder);
    
    if (verifyModule(*module, &outs())) {
        return 1;
    }
    
    if (astCtx.declaredPrototypes.size() > 0) {
        std::cerr << "Fatal Error: Definitions of some declared functions has not been found.\nDeclare unimplemented functions with `extern` keyword to generate obj file." << std::endl;
        exit(1);
    }
    
    pass.run(*module);
    dest.flush();
    
    raw_fd_ostream stdDummy("stdlib.cpp", ec, sys::fs::F_None);
    stdDummy << hexToString(hexStdLibString);
    stdDummy.close();
    std::stringstream command;
    
    if (!astCtx.externs) {
        command << "clang++ stdlib.cpp " << outputName << " -o " << program->name;
        
        int ret = system(command.str().c_str());
        
        if (WEXITSTATUS(ret) == 0) {
            remove(outputName.c_str());
            remove("stdlib.cpp");
            std::cout << "Written to " << program->name << std::endl;
        } else {
            std::cout << "Error creating binary, leaving STDLIB source and generated obj file" << std::endl;
        }
        
    } else {
        std::cout << "Extern functions found, use `clang++ stdlib.cpp your_source.cpp " << outputName << " -o " << program->name << "` to compile program\n";
    }
    
    if (dumpIR)
        module->dump();
    
    return 0;
}

int Parser::generateCode(const std::string &outputname, const std::string& sourceFile, bool dumpIR, bool dumpAST, bool optimization) {
    astCtx.declaredPrototypes.clear();
    
    next();
    _entry();
    
    compare(SpecialSymbolType::eof);
    
    Token empty;
    input = NULL;
    current = empty;
    
    if (dumpAST) {
        program->print(astCtx);
        return 0;
    }
    
    int res = dumpCode(outputname, sourceFile, dumpIR, optimization);
    
    
    return res;
}
