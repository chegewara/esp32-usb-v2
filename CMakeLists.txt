FILE(GLOB_RECURSE app_sources src/*.*)

if(CONFIG_TINYUSB)
    list(APPEND add_tinyusb tinyusb) # add old tinyusb component, like in v4.4
else()
    list(APPEND add_tinyusb esp_tinyusb) # add managed espressif component
endif()

idf_component_register(SRCS ${app_sources} 
                        PRIV_REQUIRES ${add_tinyusb} driver fatfs
                        INCLUDE_DIRS "./src"  
                        )

# workaround to build with espressif components
if(${target} STREQUAL "esp32s2")
    target_compile_options(${COMPONENT_TARGET} PUBLIC -DCONFIG_TINYUSB)
endif()
if(${target} STREQUAL "esp32s3")
    target_compile_options(${COMPONENT_TARGET} PUBLIC -DCONFIG_TINYUSB)
endif()
