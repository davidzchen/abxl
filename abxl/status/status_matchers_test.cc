#include "abxl/status/status_matchers.h"

#include <sstream>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace testing {
namespace status {
namespace {

using ::testing::_;
using ::testing::ElementsAre;
using ::testing::HasSubstr;
using ::testing::Matcher;
using ::testing::MatchesRegex;
using ::testing::Ne;
using ::testing::Not;
using ::testing::PrintToString;

// Matches a value less than the given upper bound. This matcher is chatty (it
// always explains the match result with some detail), and thus is useful for
// testing that an outer matcher correctly incorporates an inner matcher's
// explanation.
MATCHER_P(LessThan, upper, "") {
  if (arg < upper) {
    *result_listener << "which is " << (upper - arg) << " less than " << upper;
    return true;
  }
  *result_listener << "which is " << (arg - upper) << " more than " << upper;
  return false;
}

// Returns the description of the given matcher.
template <typename T>
std::string Describe(const Matcher<T>& matcher) {
  std::stringstream ss;
  matcher.DescribeTo(&ss);
  return ss.str();
}

// Returns the description of the negation of the given matcher.
template <typename T>
std::string DescribeNegation(const Matcher<T>& matcher) {
  std::stringstream ss;
  matcher.DescribeNegationTo(&ss);
  return ss.str();
}

// Returns the explanation on the result of using the given matcher to
// match the given value.
template <typename T, typename V>
std::string ExplainMatch(const Matcher<T>& matcher, const V& value) {
  ::testing::StringMatchResultListener listener;
  matcher.MatchAndExplain(value, &listener);
  return listener.str();
}

TEST(IsOkAndHoldsTest, MatchesValue) {
  absl::StatusOr<std::string> status_or_message("Hello, world");
  EXPECT_THAT(status_or_message, IsOkAndHolds("Hello, world"));
  EXPECT_THAT(status_or_message, IsOkAndHolds(HasSubstr("Hello,")));
}

TEST(IsOkAndHoldsTest, MatchesContainer) {
  absl::StatusOr<std::vector<std::string>> status_or_messages =
      std::vector<std::string>{"Hello, world", "Hello, tf"};
  EXPECT_THAT(status_or_messages,
              IsOkAndHolds(ElementsAre("Hello, world", "Hello, tf")));
  EXPECT_THAT(status_or_messages,
              IsOkAndHolds(ElementsAre(HasSubstr("world"), HasSubstr("tf"))));
}

TEST(IsOkAndHoldsTest, DoesNotMatchStatus) {
  absl::StatusOr<std::string> status_or_message =
      absl::InvalidArgumentError("Invalid argument");
  EXPECT_THAT(status_or_message, Not(IsOkAndHolds("Hello, world")));
}

TEST(IsOkAndHoldsTest, DoesNotMatchValue) {
  absl::StatusOr<std::string> status_or_message("Hello, tf");
  EXPECT_THAT(status_or_message, Not(IsOkAndHolds("Hello, world")));
}

TEST(IsOkAndHoldsTest, DoesNotMatchContainer) {
  absl::StatusOr<std::vector<int>> status_or_container({1, 2, 3});
  EXPECT_THAT(status_or_container, Not(IsOkAndHolds(ElementsAre(4, 5, 6))));
}

TEST(IsOkAndHoldsTest, DescribeExpectedValue) {
  Matcher<absl::StatusOr<std::string>> is_ok_and_has_substr =
      IsOkAndHolds(HasSubstr("Hello"));
  EXPECT_EQ(Describe(is_ok_and_has_substr),
            "is OK and has a value that has substring \"Hello\"");
  EXPECT_EQ(DescribeNegation(is_ok_and_has_substr),
            "isn't OK or has a value that has no substring \"Hello\"");
}

TEST(IsOkAndHoldsTest, ExplainNotMatchingStatus) {
  Matcher<absl::StatusOr<int>> is_ok_and_less_than =
      IsOkAndHolds(LessThan(100));
  absl::StatusOr<int> status = absl::UnknownError("Unknown");
  EXPECT_THAT(ExplainMatch(is_ok_and_less_than, status),
              HasSubstr("which has status UNKNOWN: Unknown"));
}

TEST(IsOkAndHoldsTest, ExplainNotMatchingValue) {
  Matcher<absl::StatusOr<int>> is_ok_and_less_than =
      IsOkAndHolds(LessThan(100));
  EXPECT_EQ(ExplainMatch(is_ok_and_less_than, 120),
            "which contains value 120, which is 20 more than 100");
}

TEST(IsOkAndHoldsTest, ExplainNotMatchingContainer) {
  Matcher<absl::StatusOr<std::vector<int>>> is_ok_and_less_than =
      IsOkAndHolds(ElementsAre(1, 2, 3));
  std::vector<int> actual{4, 5, 6};
  EXPECT_THAT(ExplainMatch(is_ok_and_less_than, actual),
              HasSubstr("which contains value " + PrintToString(actual)));
}

TEST(StatusIsTest, MatchesOK) {
  EXPECT_THAT(absl::OkStatus(), StatusIs(absl::StatusCode::kOk));
  absl::StatusOr<std::string> message("Hello, world");
  EXPECT_THAT(message, StatusIs(absl::StatusCode::kOk));
}

TEST(StatusIsTest, DoesNotMatchOk) {
  EXPECT_THAT(absl::DeadlineExceededError("Deadline exceeded"),
              Not(StatusIs(absl::StatusCode::kOk)));
  absl::StatusOr<std::string> status = absl::NotFoundError("Not found");
  EXPECT_THAT(status, Not(StatusIs(absl::StatusCode::kOk)));
}

TEST(StatusIsTest, MatchesStatus) {
  absl::Status s = absl::CancelledError("Cancelled");
  EXPECT_THAT(s, StatusIs(absl::StatusCode::kCancelled));
  EXPECT_THAT(s, StatusIs(absl::StatusCode::kCancelled, "Cancelled"));
  EXPECT_THAT(s, StatusIs(_, "Cancelled"));
  EXPECT_THAT(s, StatusIs(absl::StatusCode::kCancelled, _));
  EXPECT_THAT(s, StatusIs(Ne(absl::StatusCode::kInvalidArgument), _));
  EXPECT_THAT(s, StatusIs(absl::StatusCode::kCancelled, HasSubstr("Can")));
  EXPECT_THAT(s, StatusIs(absl::StatusCode::kCancelled, MatchesRegex("Can.*")));
}

TEST(StatusIsTest, StatusOrMatchesStatus) {
  absl::StatusOr<int> s = absl::InvalidArgumentError("Invalid Argument");
  EXPECT_THAT(s, StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(s,
              StatusIs(absl::StatusCode::kInvalidArgument, "Invalid Argument"));
  EXPECT_THAT(s, StatusIs(_, "Invalid Argument"));
  EXPECT_THAT(s, StatusIs(absl::StatusCode::kInvalidArgument, _));
  EXPECT_THAT(s, StatusIs(Ne(absl::StatusCode::kCancelled), _));
  EXPECT_THAT(
      s, StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr("Argument")));
  EXPECT_THAT(s, StatusIs(absl::StatusCode::kInvalidArgument,
                          MatchesRegex(".*Argument")));
}

TEST(StatusIsTest, DoesNotMatchStatus) {
  absl::Status s = absl::InternalError("Internal");
  EXPECT_THAT(s, Not(StatusIs(absl::StatusCode::kFailedPrecondition)));
  EXPECT_THAT(
      s, Not(StatusIs(absl::StatusCode::kInternal, "Failed Precondition")));
  EXPECT_THAT(s, Not(StatusIs(_, "Failed Precondition")));
  EXPECT_THAT(s, Not(StatusIs(absl::StatusCode::kFailedPrecondition, _)));
}

TEST(StatusIsTest, StatusOrDoesNotMatchStatus) {
  absl::StatusOr<int> s = absl::FailedPreconditionError("Failed Precondition");
  EXPECT_THAT(s, Not(StatusIs(absl::StatusCode::kInternal)));
  EXPECT_THAT(s,
              Not(StatusIs(absl::StatusCode::kFailedPrecondition, "Internal")));
  EXPECT_THAT(s, Not(StatusIs(_, "Internal")));
  EXPECT_THAT(s, Not(StatusIs(absl::StatusCode::kInternal, _)));
}

TEST(StatusIsTest, DescribeExpectedValue) {
  Matcher<absl::Status> status_is =
      StatusIs(absl::StatusCode::kUnavailable, std::string("Unavailable"));
  EXPECT_EQ(Describe(status_is),
            "has a status code that is equal to UNAVAILABLE, "
            "and has an error message that is equal to \"Unavailable\"");
}

TEST(StatusIsTest, DescribeNegatedExpectedValue) {
  Matcher<absl::StatusOr<std::string>> status_is =
      StatusIs(absl::StatusCode::kAborted, std::string("Aborted"));
  EXPECT_EQ(DescribeNegation(status_is),
            "has a status code that isn't equal to ABORTED, "
            "or has an error message that isn't equal to \"Aborted\"");
}

TEST(StatusIsTest, ExplainNotMatchingErrorCode) {
  Matcher<absl::Status> status_is = StatusIs(absl::StatusCode::kNotFound, _);
  const absl::Status status = absl::AlreadyExistsError("Already exists");
  EXPECT_EQ(ExplainMatch(status_is, status), "whose status code is wrong");
}

TEST(StatusIsTest, ExplainNotMatchingErrorMessage) {
  Matcher<absl::Status> status_is =
      StatusIs(absl::StatusCode::kNotFound, "Not found");
  const absl::Status status = absl::NotFoundError("Already exists");
  EXPECT_EQ(ExplainMatch(status_is, status), "whose error message is wrong");
}

TEST(StatusIsTest, ExplainStatusOrNotMatchingErrorCode) {
  Matcher<absl::StatusOr<int>> status_is =
      StatusIs(absl::StatusCode::kAlreadyExists, _);
  const absl::StatusOr<int> status_or = absl::NotFoundError("Not found");
  EXPECT_EQ(ExplainMatch(status_is, status_or), "whose status code is wrong");
}

TEST(StatusIsTest, ExplainStatusOrNotMatchingErrorMessage) {
  Matcher<absl::StatusOr<int>> status_is =
      StatusIs(absl::StatusCode::kAlreadyExists, "Already exists");
  const absl::StatusOr<int> status_or = absl::AlreadyExistsError("Not found");
  EXPECT_EQ(ExplainMatch(status_is, status_or), "whose error message is wrong");
}

TEST(StatusIsTest, ExplainStatusOrHasValue) {
  Matcher<absl::StatusOr<int>> status_is =
      StatusIs(absl::StatusCode::kResourceExhausted, "Resource exhausted");
  const absl::StatusOr<int> value = -1;
  EXPECT_EQ(ExplainMatch(status_is, value), "whose status code is wrong");
}

TEST(IsOkTest, MatchesOK) {
  EXPECT_THAT(absl::OkStatus(), IsOk());
  absl::StatusOr<std::string> message = std::string("Hello, world");
  EXPECT_THAT(message, IsOk());
}

TEST(IsOkTest, DoesNotMatchOK) {
  EXPECT_THAT(absl::PermissionDeniedError("Permission denied"), Not(IsOk()));
  absl::StatusOr<std::string> status =
      absl::UnauthenticatedError("Unauthenticated");
  EXPECT_THAT(status, Not(IsOk()));
}

TEST(IsOkTest, DescribeExpectedValue) {
  Matcher<absl::Status> status_is_ok = IsOk();
  EXPECT_EQ(Describe(status_is_ok), "is OK");
  Matcher<absl::StatusOr<std::string>> status_or_is_ok = IsOk();
  EXPECT_EQ(Describe(status_or_is_ok), "is OK");
}

TEST(IsOkTest, DescribeNegatedExpectedValue) {
  Matcher<absl::Status> status_is_ok = IsOk();
  EXPECT_EQ(DescribeNegation(status_is_ok), "is not OK");
  Matcher<absl::StatusOr<std::string>> status_or_is_ok = IsOk();
  EXPECT_EQ(DescribeNegation(status_or_is_ok), "is not OK");
}

}  // namespace
}  // namespace status
}  // namespace testing
