
add_compile_options(-Wall -Wextra -Werror)


# spi3_test

add_executable(spi3_test
    spi3_test.cpp
)

pico_generate_pio_header(spi3_test ${CMAKE_CURRENT_LIST_DIR}/../src/spi3.pio)

pico_enable_stdio_uart(spi3_test 0)
pico_enable_stdio_usb(spi3_test 1)

target_link_libraries(spi3_test PRIVATE
    pico_stdlib
    pico_stdio_usb
    hardware_dma
    hardware_pio
)

pico_add_extra_outputs(spi3_test)


# ws35_test

add_executable(ws35_test
    ws35_test.cpp
)

pico_enable_stdio_uart(ws35_test 0)
pico_enable_stdio_usb(ws35_test 1)

target_link_libraries(ws35_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    framebuffer
    misc
)

pico_add_extra_outputs(ws35_test)


# hy35_test

add_executable(hy35_test
    hy35_test.cpp
)

pico_enable_stdio_uart(hy35_test 0)
pico_enable_stdio_usb(hy35_test 1)

target_link_libraries(hy35_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    framebuffer
    misc
)

pico_add_extra_outputs(hy35_test)


# ws24_test

add_executable(ws24_test
    ws24_test.cpp
)

pico_enable_stdio_uart(ws24_test 0)
pico_enable_stdio_usb(ws24_test 1)

target_link_libraries(ws24_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    framebuffer
    misc
)

pico_add_extra_outputs(ws24_test)


# nhd_cf_bsxv_test

add_executable(nhd_cf_bsxv_test
    nhd_cf_bsxv_test.cpp
)

pico_enable_stdio_uart(nhd_cf_bsxv_test 0)
pico_enable_stdio_usb(nhd_cf_bsxv_test 1)

target_link_libraries(nhd_cf_bsxv_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    framebuffer
    misc
)

pico_add_extra_outputs(nhd_cf_bsxv_test)


# nhd_cf_bsxv_3wire_test

add_executable(nhd_cf_bsxv_3wire_test
    nhd_cf_bsxv_3wire_test.cpp
)

pico_enable_stdio_uart(nhd_cf_bsxv_3wire_test 0)
pico_enable_stdio_usb(nhd_cf_bsxv_3wire_test 1)

target_link_libraries(nhd_cf_bsxv_3wire_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    framebuffer
    misc
)

pico_add_extra_outputs(nhd_cf_bsxv_3wire_test)


# nhd_af_cxxx_test (covers both AF-CTXP and AF-CSXP - identical code)

add_executable(nhd_af_cxxx_test
    nhd_af_cxxx_test.cpp
)

pico_enable_stdio_uart(nhd_af_cxxx_test 0)
pico_enable_stdio_usb(nhd_af_cxxx_test 1)

target_link_libraries(nhd_af_cxxx_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    framebuffer
    misc
)

pico_add_extra_outputs(nhd_af_cxxx_test)


# fd24_test

add_executable(fd24_test
    fd24_test.cpp
)

pico_enable_stdio_uart(fd24_test 0)
pico_enable_stdio_usb(fd24_test 1)

target_link_libraries(fd24_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    framebuffer
    misc
)

pico_add_extra_outputs(fd24_test)
