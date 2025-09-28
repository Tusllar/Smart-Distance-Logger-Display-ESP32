set(srcs "sd_card_spi.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include"
                       REQUIRES fatfs sd_card
                       WHOLE_ARCHIVE)
