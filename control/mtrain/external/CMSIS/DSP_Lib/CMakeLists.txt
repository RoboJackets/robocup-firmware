project(DSP_lib LANGUAGES C)

aux_source_directory(BasicMathFunctions BASIC_MATH_FUNCTIONS_SRC)
aux_source_directory(CommonTables COMMON_TABLES_SRC)
aux_source_directory(ComplexMathFunctions COMPLEX_MATH_FUNCTIONS_SRC)
aux_source_directory(ControllerFunctions CONTROLLER_FUNCTIONS_SRC)
aux_source_directory(FastMathFunctions FAST_MATH_FUNCTIONS_SRC)
aux_source_directory(FilteringFunctions FILTERING_FUNCTIONS_SRC)
aux_source_directory(MatrixFunctions MATRIX_FUNCTIONS_SRC)
aux_source_directory(StatisticsFunctions STATISTICS_FUNCTIONS_SRC)
aux_source_directory(SupportFunctions SUPPORT_FUNCTIONS_SRC)
aux_source_directory(TransformFunctions TRANSFORM_FUNCTIONS_SRC)

add_library(DSP
    ${BASIC_MATH_FUNCTIONS_SRC}
    ${COMMON_TABLES_SRC}
    ${COMPLEX_MATH_FUNCTIONS_SRC}
    ${CONTROLLER_FUNCTIONS_SRC}
    ${FAST_MATH_FUNCTIONS_SRC}
    ${FILTERING_FUNCTIONS_SRC}
    ${MATRIX_FUNCTIONS_SRC}
    ${STATISTICS_FUNCTIONS_SRC}
    ${SUPPORT_FUNCTIONS_SRC}
    ${TRANSFORM_FUNCTIONS_SRC}
)

target_include_directories(DSP PUBLIC ../Include)
# TODO: Verify these are correct
target_compile_definitions(DSP PRIVATE ARM_MATH_CM7=1 ARM_MATH_MATRIX_CHECK=1 ARM_MATH_ROUNDING=1 __FPU_PRESENT=1U
    ARM_MATH_BIG_ENDIAN=1)
