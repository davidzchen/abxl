#include "abxl/status/status_matchers.h"

#include "absl/status/status.h"

namespace testing {
namespace status {
namespace internal_status {

void StatusIsMatcherCommonImpl::DescribeTo(std::ostream* os) const {
  *os << "has a status code that ";
  code_matcher_.DescribeTo(os);
  *os << ", and has an error message that ";
  message_matcher_.DescribeTo(os);
}

void StatusIsMatcherCommonImpl::DescribeNegationTo(std::ostream* os) const {
  *os << "has a status code that ";
  code_matcher_.DescribeNegationTo(os);
  *os << ", or has an error message that ";
  message_matcher_.DescribeNegationTo(os);
}

bool StatusIsMatcherCommonImpl::MatchAndExplain(
    const absl::Status& status,
    ::testing::MatchResultListener* result_listener) const {
  ::testing::StringMatchResultListener inner_listener;

  inner_listener.Clear();
  if (!code_matcher_.MatchAndExplain(
          static_cast<absl::StatusCode>(status.code()), &inner_listener)) {
    *result_listener << (inner_listener.str().empty()
                             ? "whose status code is wrong"
                             : "which has a status code " +
                                   inner_listener.str());
    return false;
  }

  if (!message_matcher_.Matches(std::string(status.message()))) {
    *result_listener << "whose error message is wrong";
    return false;
  }

  return true;
}

}  // namespace internal_status
}  // namespace status
}  // namespace testing
