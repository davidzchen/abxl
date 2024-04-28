#ifndef ABXL_STATUS_STATUS_MACROS_H_
#define ABXL_STATUS_STATUS_MACROS_H_

#include <utility>

#include "absl/base/optimization.h"
#include "absl/status/status.h"

namespace absl_extras {

#define STATUS_MACROS_CONCAT_NAME(x, y) STATUS_MACROS_CONCAT_IMPL(x, y)
#define STATUS_MACROS_CONCAT_IMPL(x, y) x##y

#define ASSIGN_OR_RETURN(lhs, rexpr) \
  ASSIGN_OR_RETURN_IMPL(             \
      STATUS_MACROS_CONCAT_NAME(_status_or_value, __COUNTER__), lhs, rexpr)

#define ASSIGN_OR_RETURN_IMPL(statusor, lhs, rexpr) \
  do {                                              \
    const auto statusor = (rexpr);                  \
    if (ABSL_PREDICT_FALSE(!statusor.ok())) {       \
      return statusor.status();                     \
    }                                               \
    lhs = std::move(statusor.value());              \
  } while (false)

#define RETURN_IF_ERROR_IMPL(__local_status, __status) \
  do {                                                 \
    const auto __local_status = __status;              \
    if (ABSL_PREDICT_FALSE(!__local_status.ok())) {    \
      return __local_status;                           \
    }                                                  \
  } while (false)

#define RETURN_IF_ERROR(status)                                               \
  RETURN_IF_ERROR_IMPL(STATUS_MACROS_CONCAT_NAME(_local_status, __COUNTER__), \
                       status)

#define RETURN_IF_NULL(ptr)                                              \
  do {                                                                   \
    if (!(ptr)) {                                                        \
      return absl::InvalidArgumentError(absl::StrCat(#ptr, " is null")); \
    }                                                                    \
  } while (false)

}  // namespace absl_extras

#endif  // ABXL_STATUS_STATUS_MACROS_H_
