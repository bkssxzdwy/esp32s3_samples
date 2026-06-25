set(src_dirs
            LCD
            XL9555
            IIC)

set(include_dirs
            LCD
            XL9555
            IIC)

set(requires
            driver
            esp_lcd)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)