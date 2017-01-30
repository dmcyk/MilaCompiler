//
//  main.cpp
//  Mila
//
//  Created by Damian Malarczyk on 03.10.2016.
//  Copyright © 2016 Damian Malarczyk. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "lexan.hpp"
#include "lexan_printing.hpp"
#include "parser.hpp"


int parserRead(FILE* fp, const std::string& outputname, const std::string& fileName, bool dumpIR, bool dumpAST, bool optimization) {
    
    auto p = Parser(fp);
    return p.generateCode(outputname, fileName, dumpIR, dumpAST, optimization);
    
}

int controlFile(const char* srcFile, bool dumpIR, bool dumpAST = false, bool optimization = false) {
    
    FILE* input = fopen(srcFile, "rb");
    if (!input) {
        std::cout << "Couldn't open file at path: " << srcFile << std::endl;
        return 1;
    }
    std::string fileName(srcFile);
    

    auto slash = fileName.find_last_of("/");

    if (slash != std::string::npos) {
        fileName = fileName.substr(slash + 1, fileName.size());
    }
    
    std::cout << "Compiling " << fileName << std::endl;
    std::string outputFileName;
    auto point = fileName.find(".");
    if (point != std::string::npos) {
        outputFileName = fileName.substr(0, point);
    }
    

    outputFileName.append(".o");
    
    int res = parserRead(input, outputFileName, fileName, dumpIR, dumpAST, optimization);
    
    fclose(input);
    printf("\n");
    return res;
}


void testFilesInDir(const char* path, bool optimization) {
    struct dirent* dp;
    DIR* dfd;
    
    dfd = opendir(path);
    if (!dfd) {
        std::cout << "Incorrect directory path given" << std::endl;
        exit(1); 
    }
    char nameBuff[2048];
    
    while ((dp = readdir(dfd)) != NULL) {
        struct stat stbuf;
        snprintf(nameBuff, 2048, "%s/%s", path,dp->d_name) ;
        if(stat(nameBuff, &stbuf ) == -1) {
            printf("Unable to stat file: %s\n", nameBuff);
            continue;
        }
        
        if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
            continue;
            // Skip directories
        } else if (strstr(nameBuff, ".p") != NULL) {
            controlFile(nameBuff, false, false, optimization);
        }
    }
}


int main(int argc, const char * argv[]) {
    if (argc < 2) {
        printf("Error: no source file\n");
        return 1;
    }
    
    bool dumpIR = false;
    bool dumpAST = false;
    bool optimization = false;
    bool programsInDirectory = false;
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-dumpIR")) {
            dumpIR = true;
        } else if (!strcmp(argv[i], "-dumpAST")) {
            dumpAST = true;
        } else if (!strcmp(argv[i], "-O"))  {
            optimization = true;
        } else if (!strcmp(argv[i], "-dir")) {
            programsInDirectory = true;
        }
    }
    if (programsInDirectory) {
        testFilesInDir(argv[1], optimization);
    } else {
        controlFile(argv[1], dumpIR, dumpAST, optimization);
    }
    return 0;
}
