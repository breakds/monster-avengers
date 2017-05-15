package(default_visibility = ["//visibility:public"])

cc_library(
    name = "signature",
    hdrs = [
        "signature.h",
    ],
    deps = [
        "//data/types:properties",
    ],
)

cc_test(
    name = "signature_test",
    srcs = [
        "signature_test.cc",
    ],
    deps = [
        ":signature",
        "@gtest//:main",
    ],
)
