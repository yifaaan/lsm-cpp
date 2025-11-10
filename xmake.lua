set_project("lsm-cpp")
set_version("0.0.1")

add_rules("mode.debug", "mode.release")
add_requires("gtest")

target("skiplist")
    set_kind("static")
    add_files("src/skiplist/skiplist.cpp")
    add_includedirs("include", {public = true})

target("memtable")
    set_kind("static")
    add_deps("skiplist")
    add_files("src/memtable/memtable.cpp")
    add_includedirs("include", {public = true})



