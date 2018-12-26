cmake_minimum_required (VERSION 2.8.12)
project(cm-accessories)
# Set common compiler flags. These flags will be used regardless of build configuration
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package (Threads REQUIRED)
find_package(PkgConfig)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(DLT REQUIRED automotive-dlt)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src/inc
	${CMAKE_CURRENT_SOURCE_DIR}/src/hicom/inc
	${CMAKE_CURRENT_SOURCE_DIR}/src/LIB/inc
)

set(INCLUDE 
		${CMAKE_CURRENT_SOURCE_DIR}/src/hicom/inc/WinTypes.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/hicom/inc/FTCI2C.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/hicom/inc/hicom_i2c.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/hicom/inc/hicom.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/inc/zmod44xx_types.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/inc/zmod44xx.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/inc/zmod44xx_config.h
		${CMAKE_CURRENT_SOURCE_DIR}/src/inc/main.h
)

set(LIB_INCLUDE
		${CMAKE_CURRENT_SOURCE_DIR}/LIB/inc/eco2.h
		${CMAKE_CURRENT_SOURCE_DIR}/LIB/inc/iaq.h
		${CMAKE_CURRENT_SOURCE_DIR}/LIB/inc/r_cda.h
		${CMAKE_CURRENT_SOURCE_DIR}/LIB/inc/tvoc.h
)

set(SOURCE
		${CMAKE_CURRENT_SOURCE_DIR}/src/hicom/hicom_i2c.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/hicom/hicom.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/zmod44xx.c
		${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
)

set(ZMOD44xx_SRC ${SOURCE} ${INCLUDE} ${LIB_INCLUDE})

find_library(FTCI2C_LIB_PATH ftci2c PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LIB/lib NO_CMAKE_FIND_ROOT_PATH)
#find_library(FTD2XX_A_LIB_PATH NAMES libftd2xx.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LIB/lib NO_CMAKE_FIND_ROOT_PATH)
find_library(FTD2XX_LIB_PATH NAMES libftd2xx.so.1.2.7 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LIB/lib NO_CMAKE_FIND_ROOT_PATH)
find_library(ECO2_LIB_PATH eco2 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LIB/lib NO_CMAKE_FIND_ROOT_PATH)
find_library(IAQ_LIB_PATH iaq PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LIB/lib NO_CMAKE_FIND_ROOT_PATH)
#find_library(ODOR_LIB_PATH odor PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LIB/lib NO_CMAKE_FIND_ROOT_PATH)
find_library(R_CDA_LIB_PATH r_cda PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LIB/lib NO_CMAKE_FIND_ROOT_PATH)
find_library(TVOC_LIB_PATH tvoc PATHS ${CMAKE_CURRENT_SOURCE_DIR}/LIB/lib NO_CMAKE_FIND_ROOT_PATH)

set(UNITLIB ${ECO2_LIB_PATH} ${IAQ_LIB_PATH} ${R_CDA_LIB_PATH} ${TVOC_LIB_PATH} ${FTCI2C_LIB_PATH} ${FTD2XX_LIB_PATH} )#${ODOR_LIB_PATH} )

add_executable(ZMOD44xx ${ZMOD44xx_SRC})

target_link_libraries(ZMOD44xx	pthread
				${UNITLIB}
				${GLIB_LIBRARIES}
				-lm
)

install(TARGETS ZMOD44xx RUNTIME DESTINATION bin)
