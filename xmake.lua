set_project("lsm-cpp")
set_version("0.0.1")
set_languages("c++20")

add_rules("mode.debug", "mode.release")
add_requires("gtest")

target("skiplist")
    set_kind("static")
    add_files("src/skiplist/*.cpp")
    add_includedirs("include", {public = true})

target("memtable")
    set_kind("static")
    add_deps("skiplist")
    add_files("src/memtable/*.cpp")
    add_includedirs("include", {public = true})


target("test_skiplist")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_skiplist.cpp")
    add_deps("skiplist")
    add_packages("gtest")

target("test_memtable")
    set_kind("binary")
    set_group("tests")
    add_files("test/test_memtable.cpp")
    add_deps("memtable")
    add_packages("gtest")