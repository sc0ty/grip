find_package(Catch2)
find_package(Boost COMPONENTS regex filesystem system)

if(Catch2_FOUND AND Boost_FOUND)
    include_directories(${Catch2_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

    if(MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    endif()

    add_executable (tests test.cpp ids.cpp compressedids.cpp pattern.cpp ../grip/pattern.cpp)

    if(MINGW)
        set_target_properties(tests PROPERTIES LINK_SEARCH_START_STATIC 1)
        set_target_properties(tests PROPERTIES LINK_SEARCH_END_STATIC 1)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    endif()

    target_link_libraries (tests ${Boost_LIBRARIES} Catch2::Catch2 External General)
    include(CTest)
    include(Catch)
    catch_discover_tests(tests)
    add_test(NAME tests COMMAND tests)
else()
    message("Test dependencies (Catch2, Boost) not satisfied!")
endif()
