cmake_minimum_required(VERSION 3.13)

set(dir ${CMAKE_CURRENT_LIST_DIR})

if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    message("Using PICO_SDK_PATH from environment ('${PICO_SDK_PATH}')")
    set(PICO_SDK_PATH "${PICO_SDK_PATH}" CACHE PATH "Path to the Raspberry Pi Pico SDK")
endif ()

if (PICO_SDK_PATH)
	include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
	project(demo C CXX ASM)
	pico_sdk_init()
	
	set(CMAKE_CXX_FLAGS "-Ofast -Wall -mcpu=cortex-m0plus -mtune=cortex-m0plus")

	add_library(codec2 STATIC 
		${dir}/src/sine.c
		${dir}/src/codec2.c
		${dir}/src/phase.c
		${dir}/src/quantise.c
		${dir}/src/helpers.c
		${dir}/src/tables.c
		${dir}/src/cmsis
		${dir}/src/cmsis/arm_cfft_radix4_q31.c
		${dir}/src/cmsis/arm_bitreversal2.S
		${dir}/src/cmsis/arm_rfft_init_q31.c
		${dir}/src/cmsis/arm_cfft_q31.c
		${dir}/src/cmsis/arm_shift_q31.c
		${dir}/src/cmsis/arm_bitreversal.c
		${dir}/src/cmsis/arm_rfft_q31.c
		${dir}/src/interpolate.c
		)

	add_executable(demo 
		${dir}/src/demo/demo-pico.c
		)
	target_link_libraries(demo 
		codec2
		pico_stdlib 
		hardware_divider
		hardware_dma
		hardware_pwm
		)

	target_link_options(demo PRIVATE
		-Xlinker
		--print-memory-usage
	)

	pico_enable_stdio_uart(demo 0)
	pico_enable_stdio_usb(demo 1)

	pico_set_float_implementation(demo pico)
	pico_set_double_implementation(demo pico)
 
	pico_add_extra_outputs(demo)

	target_include_directories(demo PRIVATE 
		${dir}/header/
		${dir}/header/cmsis/
		$ENV{PICO_SDK_PATH}/src/common/pico_base/include/
		$ENV{PICO_SDK_PATH}/rp2_common/hardware_dma/include/
		$ENV{PICO_SDK_PATH}/rp2_common/hardware_pwm/include/
	)


else ()
	project(demo C CXX ASM)
	set(CMAKE_CXX_FLAGS "-Ofast -Wall")

	add_library(codec2 STATIC 
		${dir}/src/sine.c
		${dir}/src/codec2.c
		${dir}/src/phase.c
		${dir}/src/quantise.c
		${dir}/src/helpers.c
		${dir}/src/tables.c
		${dir}/src/cmsis
		${dir}/src/cmsis/arm_cfft_radix4_q31.c
		${dir}/src/cmsis/arm_bitreversal2.c
		${dir}/src/cmsis/arm_rfft_init_q31.c
		${dir}/src/cmsis/arm_cfft_q31.c
		${dir}/src/cmsis/arm_shift_q31.c
		${dir}/src/cmsis/arm_bitreversal.c
		${dir}/src/cmsis/arm_rfft_q31.c
		${dir}/src/interpolate.c
		)
	add_executable(demo 
		${dir}/src/demo/demo-host.c
		)

	target_include_directories(demo PRIVATE
		${dir}/header/
		${dir}/header/cmsis/
	)
	target_link_libraries(demo codec2)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
 
target_link_options(codec2 PRIVATE)

target_include_directories(codec2 PUBLIC 
	${dir}/header/
	${dir}/header/cmsis/
	)

