#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/Pass.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Support/LogicalResult.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

// CIRCT 方言
#include "circt/Dialect/HW/HWDialect.h"
#include "circt/Dialect/SV/SVDialect.h"
#include "circt/Dialect/Comb/CombDialect.h"
#include "circt/Dialect/Seq/SeqDialect.h"

// TableGen 声明 createDffDemo()
#define GEN_PASS_DECL_DFFDEMO
#include "Passes.h.inc"

using namespace mlir;

int main(int argc, char **argv) {
  llvm::InitLLVM y(argc, argv);

  llvm::cl::opt<std::string> inputFilename(llvm::cl::Positional,
                                           llvm::cl::desc("<input .mlir>"),
                                           llvm::cl::Required);
  llvm::cl::opt<std::string> outputFilename("o", llvm::cl::desc("Output file"),
                                            llvm::cl::value_desc("filename"),
                                            llvm::cl::init("-"));
  llvm::cl::ParseCommandLineOptions(argc, argv, "dff-demo minimal driver\n");

  MLIRContext context;
  DialectRegistry registry;
  registry.insert<circt::hw::HWDialect, circt::sv::SVDialect, circt::comb::CombDialect, circt::seq::SeqDialect>();
  context.appendDialectRegistry(registry);
  context.loadAllAvailableDialects();

  // 直接用文件名解析
  OwningOpRef<ModuleOp> module = parseSourceFile<ModuleOp>(inputFilename, &context);
  if (!module) {
    llvm::WithColor::error() << "failed to parse input MLIR: " << inputFilename << "\n";
    return 1;
  }

  // 跑 pass
  PassManager pm(&context);
  pm.enableVerifier(true);
  pm.addPass(createDffDemo()); // 由 Passes.h.inc 提供
  if (failed(pm.run(*module)))
    return 1;

  // 写出
  std::error_code ec;
  llvm::raw_fd_ostream os(outputFilename, ec, llvm::sys::fs::OF_Text);
  if (ec) {
    llvm::WithColor::error() << "could not open output: " << ec.message() << "\n";
    return 1;
  }
  module->print(os);
  return 0;
}
