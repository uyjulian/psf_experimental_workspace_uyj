cmake_minimum_required(VERSION 3.0...3.10)
project(psf_tester)

if((NOT DEFINED CMAKE_BUILD_TYPE) OR (NOT CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL ""))
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(AO_SRC
	aopsf/eng_psf.c
	aopsf/eng_psf2.c
	aopsf/psx_hw.c
	aopsf/psx.c
	aopsf/r3000dis.c
	aopsf/spu/spu.c
	aopsf/spu/spucore.c
)

set(HE_SRC
	highlyexperimental/Core/bios.c
	highlyexperimental/Core/iop.c
	highlyexperimental/Core/ioptimer.c
	highlyexperimental/Core/mkhebios.c
	highlyexperimental/Core/psx.c
	highlyexperimental/Core/r3000.c
	highlyexperimental/Core/spu.c
	highlyexperimental/Core/spucore.c
	highlyexperimental/Core/vfs.c
)

set(AO_INCLUDES
	aopsf
)

set(HE_INCLUDES
	highlyexperimental/Core
)

set(AO_DEFINITIONS
	USING_AUDIO_OVERLOAD
)

set(HE_DEFINITIONS
	USING_HIGHLY_EXPERIMENTAL
)

set(EXE_SRC
	test.c
	psflib/psf2fs.c
	psflib/psflib.c
)

set(EXE_INCLUDES
	psflib
)

set(EXE_DEFINITIONS
	EMU_COMPILE
	EMU_LITTLE_ENDIAN
	HAVE_STDINT_H
	USING_SDL_AUDIO_OUTPUT
)

find_package(ZLIB REQUIRED)

list(APPEND EXE_LIBS
	ZLIB::ZLIB
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

list(APPEND EXE_LIBS
	${SDL2_LIBRARIES}
)

# list(APPEND EXE_SRC ${AO_SRC})
# list(APPEND EXE_INCLUDES ${AO_INCLUDES})
# list(APPEND EXE_DEFINITIONS ${AO_DEFINITIONS})

list(APPEND EXE_SRC ${HE_SRC})
list(APPEND EXE_INCLUDES ${HE_INCLUDES})
list(APPEND EXE_DEFINITIONS ${HE_DEFINITIONS})

add_executable(psf_tester ${EXE_SRC})
set_target_properties(psf_tester PROPERTIES C_STANDARD 11)
target_compile_definitions(psf_tester PUBLIC ${EXE_DEFINITIONS})
target_include_directories(psf_tester PUBLIC ${EXE_INCLUDES})
target_link_libraries(psf_tester ${EXE_LIBS})
