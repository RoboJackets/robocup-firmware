nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS radio_protocol.proto)
add_library(robocup_protos ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(robocup_protos PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${NANOPB_INCLUDE_DIRS})
set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS}
        PROPERTIES GENERATED TRUE)
