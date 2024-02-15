#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <stack>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include <llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace std;

std::set<std::string> source_sets = {};

void cfgTraversal(std::map<std::string, std::set<Instruction *>> *analysisMap, Function *F );
void printAnalysisMap(std::map<std::string, std::set<Instruction *>> analysisMap);

void readSourceFile(std::string sourcePath){
  std::ifstream sourceFile;
    sourceFile.open(sourcePath);

    std::string tmpLine;

    // read in an entire line at a time 
    while(std::getline(sourceFile, tmpLine))
    {
        // Place the input line into a stream that reads from
        // a string instead of a file.
        std::stringstream inputLine(tmpLine);
        std::string s;
        if(!(inputLine >> s)){
          continue;
        }
        source_sets.insert(s);
        errs()<<"insert: "<<s<<"\n";
    }
    return;
}

std::string getSimpleNodeLabel(const BasicBlock *Node)
{
  if (!Node->getName().empty())
    return Node->getName().str();
  std::string Str;
  raw_string_ostream OS(Str);
  Node->printAsOperand(OS, false);
  return OS.str();
}

std::string getVarName(const Instruction *ins)
{
  if (!ins->getName().empty())
    return ins->getName().str();
  std::string Str;
  raw_string_ostream OS(Str);
  ins->printAsOperand(OS, false);
  return OS.str();
}

int main(int argc, char **argv)
{
  // Read the IR file.
  // readSourceFile(argv[2]);
  static LLVMContext Context;
  SMDiagnostic Err;

  // Extract Module M from IR (assuming only one Module exists)
  auto M = parseIRFile(argv[1], Err, Context);
  if (M == nullptr)
  {
    fprintf(stderr, "error: failed to load LLVM IR file \"%s\"", argv[1]);
    return EXIT_FAILURE;
  }

  // 1.Extract Function main from Module M
  Function *F = M->getFunction("main");

  // 2.Define analysisMap as a mapping of basic block labels to empty set (of instructions/variables):
  // Note: All variables are of type "alloca" instructions. Ex.
  // Variable a: %a = alloca i32, align 4
  std::map<std::string, std::set<Instruction *>> analysisMap;

  for (auto &BB : *F)
  {
    std::set<Instruction *> emptySet;
    analysisMap[getSimpleNodeLabel(&BB)] = emptySet;
  }

  // 3. Traverse the CFG and update analysisMap
  // * You need to finish it
  cfgTraversal(&analysisMap,F);

  // 4. Print out the result
  // * You need to finish it
  printAnalysisMap(analysisMap);

  return 0;
}

void cfgTraversal(std::map<std::string, std::set<Instruction *>> *analysisMap, Function *F ){
  // Traversing the CFG in Depth First Order.
  // In order to do so, we use a stack: traversalStack.
  std::stack<std::pair<BasicBlock *, std::set<Instruction *>>> traversalStack;
  // First, we create the initial analysisNode consisting of the following two items to the stack:
  // 	    - entry basic block
  //	    - an empty set (of instructions) as the initial list of variables
  BasicBlock *entryBB = &F->getEntryBlock();
  std::set<Instruction *> emptySet;
  std::pair<BasicBlock *, std::set<Instruction *>> analysisNode = std::make_pair(entryBB, emptySet);
  traversalStack.push(analysisNode);
  std::set<BasicBlock *> visited;
  visited.insert(entryBB);

  while (!traversalStack.empty()){

    // Please write your code here
    auto [bb, instrs] = std::move(traversalStack.top());
    traversalStack.pop();
    
    auto label = getSimpleNodeLabel(bb);
    
    for (auto it = bb->begin(); it != bb->end(); it++) {
      for (auto& use: it->operands()) {
        if (!isa<Instruction>(use)) { continue; }
        analysisMap->at(label).insert(cast<Instruction>(use.get()));
      }
    }

    auto last = bb->getTerminator();
    for (int i = 0; i < last->getNumSuccessors(); i++) {
      auto succ = last->getSuccessor(i);
      if (visited.find(succ) != visited.end()) { continue; }
      visited.insert(succ);
      traversalStack.push(make_pair(succ, emptySet));
    }
  }
  
}

// Printing Analysis Map
void printAnalysisMap(std::map<std::string, std::set<Instruction *>> analysisMap)
{
  errs() << "PRINTING ANALYSIS MAP:\n";

  // Please write your code here
  for (auto [label, instrs] : analysisMap) {
    errs() << label << ": {";
    auto first = true;
    for (auto ins: instrs) {
      // auto name = getVarName(ins);
      auto name = ins->getName();
      //ins->dump();
      //auto alloca = dyn_cast<AllocaInst>(ins);
      //if (alloca) alloca->dump();
      if (name.empty()) { continue; }
      if (first) {
        first = false;
      } else {
        errs() << ",";
      }
      errs() << name;
    }
    errs() << "}\n";
  }
}
