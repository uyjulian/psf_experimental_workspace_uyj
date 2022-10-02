project('psf_kek', ['c'], default_options: ['optimization=2', 'debug=true', 'warning_level=1'], version: 'unknown')

sdl2_dep = dependency('sdl2', method: 'auto')

zlib_dep = dependency('zlib')

ao_src = [
	'aopsf/eng_psf.c',
	'aopsf/eng_psf2.c',
	'aopsf/psx_hw.c',
	'aopsf/psx.c',
	'aopsf/r3000dis.c',
	'aopsf/spu/spu.c',
	'aopsf/spu/spucore.c',
]

he_src = [
	'highlyexperimental/Core/bios.c',
	'highlyexperimental/Core/iop.c',
	'highlyexperimental/Core/ioptimer.c',
	'highlyexperimental/Core/mkhebios.c',
	'highlyexperimental/Core/psx.c',
	'highlyexperimental/Core/r3000.c',
	'highlyexperimental/Core/spu.c',
	'highlyexperimental/Core/spucore.c',
	'highlyexperimental/Core/vfs.c',
]

ao_includes = [
	'aopsf/',
]

he_includes = [
	'highlyexperimental/Core/',
]

ao_flags = [
	'-DUSING_AUDIO_OVERLOAD',
]

he_flags = [
	'-DUSING_HIGHLY_EXPERIMENTAL',
]

exe_src = [
	'test.c',
	'psflib/psf2fs.c',
	'psflib/psflib.c',
]

exe_includes = [
	'psflib/',
]

exe_flags = [
	'-DEMU_COMPILE',
	'-DEMU_LITTLE_ENDIAN',
	'-DHAVE_STDINT_H',
	'-DUSING_SDL_AUDIO_OUTPUT',
]

exe_dependencies = [
	zlib_dep,
	sdl2_dep,
]

# exe_src += ao_src
# exe_includes += ao_includes
# exe_flags += ao_flags
exe_src += he_src
exe_includes += he_includes
exe_flags += he_flags

executable(
	'psf_tester',
	sources: exe_src,
	include_directories: include_directories(exe_includes),
	c_args: exe_flags,
	dependencies: exe_dependencies,
)
