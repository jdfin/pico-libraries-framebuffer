add_library(framebuffer INTERFACE)

target_sources(framebuffer INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/framebuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/tft.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/tft_spi_if.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/tft_spi3_if.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ws24.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ws35.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/hy35.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/nhd_cf_bsxv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/nhd_af_cxxx.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/fd24.cpp
)

target_include_directories(framebuffer INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/fonts
)

target_link_libraries(framebuffer INTERFACE
    pico_stdlib
    hardware_dma
    hardware_spi
    hardware_pio
    misc
)

pico_generate_pio_header(framebuffer ${CMAKE_CURRENT_LIST_DIR}/src/spi3.pio)

add_subdirectory(test)
