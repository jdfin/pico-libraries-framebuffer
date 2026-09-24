
add_compile_options(-Wall -Wextra -Werror)

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


# nhd_bsxv_f_test

add_executable(nhd_bsxv_f_test
    nhd_bsxv_f_test.cpp
)

pico_enable_stdio_uart(nhd_bsxv_f_test 0)
pico_enable_stdio_usb(nhd_bsxv_f_test 1)

target_link_libraries(nhd_bsxv_f_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    framebuffer
    misc
)

pico_add_extra_outputs(nhd_bsxv_f_test)


# nhd_af_ctxp_test

add_executable(nhd_af_ctxp_test
    nhd_af_ctxp_test.cpp
)

pico_enable_stdio_uart(nhd_af_ctxp_test 0)
pico_enable_stdio_usb(nhd_af_ctxp_test 1)

target_link_libraries(nhd_af_ctxp_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    framebuffer
    misc
)

pico_add_extra_outputs(nhd_af_ctxp_test)


# nhd_af_csxp_test

add_executable(nhd_af_csxp_test
    nhd_af_csxp_test.cpp
)

pico_enable_stdio_uart(nhd_af_csxp_test 0)
pico_enable_stdio_usb(nhd_af_csxp_test 1)

target_link_libraries(nhd_af_csxp_test PRIVATE
    pico_rand
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    framebuffer
    misc
)

pico_add_extra_outputs(nhd_af_csxp_test)


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
