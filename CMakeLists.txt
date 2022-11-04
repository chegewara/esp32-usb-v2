FILE(GLOB_RECURSE app_sources src/*.*)

idf_component_register(SRCS ${app_sources} 
                        PRIV_REQUIRES tinyusb driver fatfs
                        INCLUDE_DIRS "./src"  
                        )
