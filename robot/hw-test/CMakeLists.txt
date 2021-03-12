function(add_test test_file)
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name}.elf
        ${test_file}
    )

    target_link_libraries(${test_name}.elf firm-lib)

    add_custom_target(${test_name} ALL
        ${CMAKE_OBJCOPY} -Obinary "${test_name}.elf" "${test_name}.bin"
        WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        DEPENDS ${test_name}.elf
        COMMENT "objcopying to make binary executable for ${test_name}"
    )

endfunction()

file(GLOB files "./*.cpp")
foreach(file ${files})
    add_test(${file})
endforeach()
