package(default_visibility = ["//visibility:public"])

cc_library(
    name = "data",
    hdrs = [
        "data.h",
    ],
    deps = [
        "//data/types",
    ],
)

cc_library(
    name = "sqlite_loader",
    hdrs = [
        "sqlite_loader.h",
    ],
    srcs = [
        "sqlite_loader.cc",
    ],
    deps = [
        ":data",
        "//data/types:properties",
        "//base:status",
        "//base:error_code",
        "@glog//:glog",
    ],
    linkopts = [
        # Requires installation of libsqlite3-dev. On Debian (and its
        # friends), this can be done with:
        #
        # sudo apt install libsqlite3-dev
        "-lsqlite3",
    ],
)

filegroup(
    name = "testdata",
    srcs = glob([
        "testdata/*"
    ]),
)

cc_test(
    name = "sqlite_loader_test",
    srcs = [
        "sqlite_loader_test.cc",
    ],
    size = "small",
    data = [
        ":testdata",
    ],
    deps = [
        ":sqlite_loader",
        "//base:status_test_util",
        "@gtest//:main",
    ],
)
