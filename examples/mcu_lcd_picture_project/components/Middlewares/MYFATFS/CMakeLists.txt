list(APPEND srcs    fstools.c)

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "."
                    REQUIRES fatfs
                    )