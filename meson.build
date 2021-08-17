project('psf_kek', ['c'], default_options: ['optimization=2', 'debug=true', 'warning_level=1'], version: 'unknown')


zlib_dep = dependency('zlib')

exe_src = [
	'test.c',
	'psflib/psf2fs.c',
	'psflib/psflib.c',
	'aopsf/eng_psf.c',
	'aopsf/eng_psf2.c',
	'aopsf/psx_hw.c',
	'aopsf/psx.c',
	'aopsf/r3000dis.c',
	'aopsf/spu/spu.c',
	'aopsf/spu/spucore.c',
]

exe_includes = [
	'psflib/',
	'aopsf/',
]

exe_flags = [
	'-DEMU_COMPILE',
	'-DEMU_LITTLE_ENDIAN',
	'-DHAVE_STDINT_H',
]

exe_dependencies = [
	zlib_dep
]

executable(
	'psf2pcm',
	sources: exe_src,
	include_directories: include_directories(exe_includes),
	c_args: exe_flags,
	dependencies: exe_dependencies,
)
