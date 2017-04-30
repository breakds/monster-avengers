package(default_visibility = ["//data:__subpackages__"]);

cc_library(
    name = "properties",
    hdrs = [
        "properties.h",
    ],
    srcs = [
        "properties.cc",
    ],
    deps = [
        "//data/utils",
    ],
)

cc_test(
    name = "properties_test",
    srcs = [
        "properties_test.cc",
    ],
    deps = [
        ":properties",
        "@gtest//:main",
    ],
)
