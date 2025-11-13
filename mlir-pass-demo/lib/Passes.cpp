#include "mlir/IR/PatternMatch.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "circt/Dialect/HW/HWOps.h"
#include "circt/Dialect/SV/SVOps.h"
#include "circt/Dialect/Seq/SeqOps.h"

// 只生成 per-pass 的声明/定义；不要手写 create/注册函数
#define GEN_PASS_DECL_DFFDEMO
#define GEN_PASS_DEF_DFFDEMO
#include "Passes.h.inc"

using namespace mlir;
using namespace circt;

namespace {

struct FlipI1ConstPattern : OpRewritePattern<hw::ConstantOp> {
  using OpRewritePattern<hw::ConstantOp>::OpRewritePattern;
  LogicalResult matchAndRewrite(hw::ConstantOp op, PatternRewriter &rewriter) const override {
    auto intTy = dyn_cast<IntegerType>(op.getType());
    if (!intTy) return failure();

    // 若此行 API 不匹配，改为：auto v = op.getValueAttr().getValue();
    APInt v = op.getValue();
    if (!v.isZero()) return failure();

    APInt one(v.getBitWidth(), 1);
    rewriter.replaceOpWithNewOp<hw::ConstantOp>(op, one);
    return success();
  }
};


struct DffDemoPass : public ::impl::DffDemoBase<DffDemoPass> {
  void runOnOperation() override {
    ModuleOp mod = getOperation();

    llvm::outs() << "[pass-ran] dff-demo\n";
    int _numConst = 0, _constUse = 0, _numToClk = 0, _clkUse = 0;

// 枚举所有 hw.constant 的使用者
    mod.walk([&](circt::hw::ConstantOp cst) {
      ++_numConst;
      Value v = cst.getResult();
      for (OpOperand &use : v.getUses()) {
        ++_constUse;
        Operation *user = use.getOwner();
        unsigned argNo  = use.getOperandNumber();
        llvm::outs() << "[const-use] " << user->getName() << " operand#" << argNo << "\n";
  }
});

// 枚举所有 seq.to_clock 的使用者
    mod.walk([&](circt::seq::ToClockOp tclk) {
      ++_numToClk;
      for (Operation *user : tclk.getResult().getUsers()) {
        ++_clkUse;
        llvm::outs() << "[clock-user] " << user->getName() << "\n";
  }
});

    llvm::outs() << "[summary] constOps=" << _numConst
                << " uses=" << _constUse
                << " to_clock=" << _numToClk
                << " clkUsers=" << _clkUse << "\n";
    llvm::outs().flush();

  }
};

} // namespace
