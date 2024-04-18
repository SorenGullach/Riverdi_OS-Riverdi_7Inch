cmake_minimum_required(VERSION 3.15)

project(CM7_STM32H757 LANGUAGES C CXX ASM)

find_bsp(
	ID com.sysprogs.arm.stm32
	VERSION 2024.01
	MCU STM32H757XI
	HWREGISTER_LIST_FILE STM32H7xxxx/DeviceDefinitions/stm32h757xx.xml
	CXX_STANDARD 17
	DISABLE_GNU_EXTENSIONS
	FRAMEWORKS com.sysprogs.embedded.semihosting_and_profiler
	FWCONFIGURATION com.sysprogs.efp.profiling.debugger_check=SYSPROGS_PROFILER_DEBUGGER_CHECK_MODE=1)

bsp_include_directories(../../../Users/45201/AppData/Local/VisualGDB/EmbeddedBSPs/arm-eabi/com.sysprogs.arm.stm32/STM32H7xxxx/CMSIS_HAL/Core/Include ../../../Users/45201/AppData/Local/VisualGDB/EmbeddedBSPs/arm-eabi/com.sysprogs.arm.stm32/STM32H7xxxx/CMSIS_HAL/Device/ST/STM32H7xx/Include)
add_bsp_based_executable(
	NAME CM7_STM32H757
	GENERATE_BIN
	GENERATE_MAP
	SOURCES
		Main.cpp
		../../osSTM32H7xx/SystemInit.h
		../../osSTM32H7xx/SystemInitCM7.cpp
		../../osSTM32H7xx/Utils.h
		../../osSTM32H7xx/RCC.cpp
		../../osSTM32H7xx/RCC.h
		../../osSTM32H7xx/StartupSemafor.cpp
		../../osSTM32H7xx/StartupSemafor.h
		../../osSTM32H7xx/Utils.cpp
		../../osSTM32H7xx/SysTick.h
		../../osSTM32H7xx/SysTick.cpp
		../../osSTM32H7xx/PWR.h
		../../osSTM32H7xx/GPIO.h
		../../osSTM32H7xx/LTDC.h
		../../osSTM32H7xx/LTDC.cpp
		../../osSTM32H7xx/DSI.h
		../../osSTM32H7xx/DSI.cpp
		../../osSTM32H7xx/Timers.h
		../../osSTM32H7xx/Timers.cpp
		../../osSTM32H7xx/FMC.cpp
		../../osSTM32H7xx/FMC.h
		TFTDisplay.h
		TFT_Setup.h
		../../glSTM32H7xx/glMain.h
		../../glSTM32H7xx/glTypes.h
		../../glSTM32H7xx/glMain.cpp
		../../glSTM32H7xx/glPrimitives.cpp
		../../glSTM32H7xx/glPrimitives.h
	LINKER_SCRIPT STM32H757XI_flash.lds)

	target_include_directories(CM7_STM32H757 PRIVATE
	../../osSTM32H7xx
	../../glSTM32H7xx)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
target_compile_definitions(CM7_STM32H757 PRIVATE STM32H757xx DEBUG DEBUG_DEFAULT_INTERRUPT_HANDLERS)
    message(STATUS "Setting debug flags...")
	target_compile_options(CM7_STM32H757 PRIVATE -O0 -Wall -Wextra)
	message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS_DEBUG})
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
target_compile_definitions(CM7_STM32H757 PRIVATE STM32H757xx RELEASE)
    message(STATUS "Setting release flags...")
	target_compile_options(CM7_STM32H757 PRIVATE -O3)
	message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS_RELEASE})
endif()

