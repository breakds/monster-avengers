#include "base/status.h"
#include "gtest/gtest.h"

namespace base {

TEST(Status, OK) {
  EXPECT_EQ(0, Status::OK().code());
  EXPECT_EQ("", Status::OK().error_message());
}

TEST(Status, Constructor) {
  Status status(-1, "The number ", 4, " is not 3!");
  EXPECT_EQ(-1, status.code());
  EXPECT_EQ("The number 4 is not 3!", status.error_message());
}

TEST(Status, SimpleConstructor) {
  Status status(-1);
  EXPECT_EQ(-1, status.code());
  EXPECT_EQ("", status.error_message());
}

TEST(Status, Copy) {
  Status original(-1, "This is wrong!");
  Status status(original);
  EXPECT_EQ(-1, status.code());
  EXPECT_EQ("This is wrong!", status.error_message());
}

TEST(Status, CopyAssign) {
  Status original(-1, "This is wrong!");
  Status status = original;
  EXPECT_EQ(-1, status.code());
  EXPECT_EQ("This is wrong!", status.error_message());
}

TEST(Status, Move) {
  Status status(Status(-1, "This is wrong!"));
  EXPECT_EQ(-1, status.code());
  EXPECT_EQ("This is wrong!", status.error_message());
}

TEST(Status, MoveAssign) {
  Status status = Status(-1, "This is wrong!");
  EXPECT_EQ(-1, status.code());
  EXPECT_EQ("This is wrong!", status.error_message());
}

TEST(Status, Equality) {
  Status original(-1, "This is wrong!");
  Status status = original;
  EXPECT_EQ(original, status);
}

TEST(Status, Inequality) {
  Status original(-1, "This is wrong!");
  Status status = Status::OK();
  EXPECT_NE(original, status);
}

}  // namespace base
