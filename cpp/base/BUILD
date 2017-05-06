package(default_visibility = ["//visibility:public"])

cc_library(
    name = "strcat",
    hdrs = [
        "strcat.h",
    ],
)

cc_test(
    name = "strcat_test",
    srcs = [
        "strcat_test.cc"
    ],
    deps = [
        ":strcat",
        "@gtest//:main",
    ],
)

cc_library(
    name = "status",
    hdrs = [
        "status.h",
    ],
    deps = [
        ":strcat",
    ],
)

cc_test(
    name = "status_test",
    srcs = [
        "status_test.cc",
    ],
    deps = [
        ":status",
        "@gtest//:main",
    ],
)
