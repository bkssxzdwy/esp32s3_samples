set(src_dirs
            KEY
            XL9555
            LCD
            IIC
            SPISD)

set(include_dirs
            KEY
            XL9555
            LCD
            IIC
            SPISD)

set(requires
            fatfs
            driver
            esp_lcd)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)