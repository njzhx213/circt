include "mlir/Pass/PassBase.td"

def DffDemo : Pass<"dff-demo", "mlir::ModuleOp"> {
  let summary = "Demo pass: walk and rewrite CIRCT HW/SV ops";
  let description = [{
    A minimal pass that walks CIRCT HW/SV dialect ops and applies a tiny rewrite.
  }];
}
