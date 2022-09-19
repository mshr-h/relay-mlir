#include "PassDetail.hpp"
#include "llvm/Support/Debug.h"
#include "tvm-mlir/Dialect/Relay/Passes.hpp"
#include "tvm-mlir/Support/Common.hpp"

namespace mlir {
namespace relay {

class ShapeInference : public ShapeInferenceBase<ShapeInference> {
    void runOnOperation() override;
};

void ShapeInference::runOnOperation() {
    // Infer types of Relay operators
    auto func = getOperation();
    ValueRange retValues;
    func.walk([&](Operation *op) {
        // Get return value of the function
        if (func::ReturnOp::classof(op)) {
            retValues = llvm::cast<func::ReturnOp>(op).getOperands();
            return;
        }

        // Skip operators not defined in Relay dialect
        if (op->getDialect()->getNamespace() !=
            RelayDialect::getDialectNamespace())
            return;

        // Infer type with operator interface
        auto opInterface = dyn_cast<InferShapedTypeOpInterface>(op);
        if (!opInterface) return;
        ValueShapeRange operands(op->getOperands());
        SmallVector<ShapedTypeComponents> inferredShapes;
        auto result = opInterface.inferReturnTypeComponents(
            op->getContext(), op->getLoc(), op->getOperands(),
            op->getAttrDictionary(), op->getRegions(), inferredShapes);
        if (result.failed())
            FatalError("Cannot infer type for operator `{}`.",
                       op->getName().getStringRef());

        // Assign inferred types to output tensors
        for (auto [value, type] : zip(op->getResults(), inferredShapes))
            value.setType(RankedTensorType::get(
                type.getDims(),
                value.getType().cast<TensorType>().getElementType()));
    });

    // Update return type of function
    auto funcType = FunctionType::get(&getContext(), func.getArgumentTypes(),
                                      TypeRange(retValues));
    func.setType(funcType);
}

std::unique_ptr<Pass> createShapeInferencePass() {
    return std::make_unique<ShapeInference>();
}

}  // namespace relay

}  // namespace mlir