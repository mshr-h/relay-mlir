#pragma once

#include "llvm/Support/Debug.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorHandling.h"
#include <string>
#include <cstdio>
#include <vector>
namespace mlir {

template <typename ... Args>
std::string format(const std::string& fmt, Args ... args )
{
    size_t len = std::snprintf( nullptr, 0, fmt.c_str(), args ... );
    std::vector<char> buf(len + 1);
    std::snprintf(&buf[0], len + 1, fmt.c_str(), args ... );
    return std::string(&buf[0], &buf[0] + len);
}

/// Fatal error
template <class S, class... Args>
[[noreturn]] inline void Fatal(const S &fmt, Args &&...args) {
    auto msg = mlir::format(fmt, std::forward<Args>(args)...);
    llvm::report_fatal_error(llvm::StringRef(msg));
}

/// Error
template <class S, class... Args>
inline void Error(const S &fmt, Args &&...args) {
    llvm::errs() << mlir::format(fmt, std::forward<Args>(args)...) << '\n';
}

/// Error
template <class S, class... Args>
inline void Debug(const S &fmt, Args &&...args) {
    llvm::dbgs() << mlir::format(fmt, std::forward<Args>(args)...) << '\n';
}

}  // namespace mlir
