package(default_visibility = ["//visibility:public"])

cc_library(
    name = "types",
    deps = [
        ":properties",
        ":armor",
        ":jewel",
        ":skill",
    ],
)

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

cc_library(
    name = "armor",
    hdrs = [
        "armor.h",
    ],
    deps = [
        ":properties",
    ],
)

cc_library(
    name = "jewel",
    hdrs = [
        "jewel.h",
    ],
    deps = [
        ":properties",
    ],
)

cc_library(
    name = "skill",
    hdrs = [
        "skill.h",
    ],
    deps = [
        ":properties",
    ],
)

cc_test(
    name = "types_test",
    srcs = [
        "types_test.cc",
    ],
    deps = [
        ":armor",
        ":jewel",
        ":skill",
        "@gtest//:main",
    ],
)
