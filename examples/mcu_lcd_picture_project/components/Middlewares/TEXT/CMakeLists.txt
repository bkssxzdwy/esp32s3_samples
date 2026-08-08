list(APPEND srcs    text.c
                    font_lib.c
                    convert.c)

idf_component_register(SRCS "text.c" "convert.c" "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES esp_partition
                             spi_flash
                             fatfs
                             BSP)