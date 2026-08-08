list(APPEND srcs    picture.c
                    bmp.c
                    gif.c
                    jpeg.c
                    png.c
                    tjpgd.c
                    pngle.c
                    )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES BSP
                             MYFATFS
                             esp_driver_jpeg
                             fatfs
                             )