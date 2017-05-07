#pragma once

#include "gtest/gtest.h"

#define EXPECT_OK(statement) EXPECT_EQ(::base::Status::OK(), (statement))
#define ASSERT_OK(statement) ASSERT_EQ(::base::Status::OK(), (statement))
