
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "psflib.h"
#include "psf2fs.h"
#if defined(USING_AUDIO_OVERLOAD)
#include "psx_external.h"
#endif
#if defined(USING_HIGHLY_EXPERIMENTAL)
#include "psx.h"
#include "iop.h"
#include "r3000.h"
#include "spu.h"
#include "bios.h"
#include "mkhebios.h"
#endif
#if defined(USING_SDL_AUDIO_OUTPUT)
#include <SDL.h>
#endif
#include <unistd.h>

#if defined(USING_AUDIO_OVERLOAD)
PSX_STATE *psx_state = NULL;
#endif
#if defined(USING_HIGHLY_EXPERIMENTAL)
void *psx_state = NULL;
#endif
void *psf2fs = NULL;

int version = 0;

#if !defined(USING_SDL_AUDIO_OUTPUT)
#define SAMPLES441 ((44100*2*2)/50)
#define SAMPLES480 ((48000*2*2)/50)

int16_t audiobuffer[SAMPLES480];
#endif

typedef struct _psf1_load_state
{
	void *emu;
	bool first;
	unsigned refresh;
} psf1_load_state;

static int psf1_info(void * context, const char * name, const char * value)
{
	psf1_load_state * state = (psf1_load_state *)context;

	if (!state->refresh && !strcasecmp(name, "_refresh"))
	{
		state->refresh = atoi(value);
	}

	return 0;
}

#if defined(USING_HIGHLY_EXPERIMENTAL)
typedef struct {
	uint32_t pc0;
	uint32_t gp0;
	uint32_t t_addr;
	uint32_t t_size;
	uint32_t d_addr;
	uint32_t d_size;
	uint32_t b_addr;
	uint32_t b_size;
	uint32_t s_ptr;
	uint32_t s_size;
	uint32_t sp,fp,gp,ret,base;
} exec_header_t;

typedef struct {
	char key[8];
	uint32_t text;
	uint32_t data;
	exec_header_t exec;
	char title[60];
} psxexe_hdr_t;
#endif

int psf1_load(void * context, const uint8_t * exe, size_t exe_size, const uint8_t * reserved, size_t reserved_size)
{
	psf1_load_state * state = (psf1_load_state *)context;

#if defined(USING_AUDIO_OVERLOAD)
	if (reserved && reserved_size)
		return -1;

	if (psf_load_section((PSX_STATE *)state->emu, exe, exe_size, state->first))
		return -1;

	state->first = false;

#endif
#if defined(USING_HIGHLY_EXPERIMENTAL)
	psxexe_hdr_t *psx = (psxexe_hdr_t *) exe;

	if (exe_size < 0x800)
		return -1;

	uint32_t addr = psx->exec.t_addr;
	uint32_t size = exe_size - 0x800;

	addr &= 0x1fffff;
	if ((addr < 0x10000) || (size > 0x1f0000) || (addr + size > 0x200000))
		return -1;

	void * pIOP = psx_get_iop_state( state->emu );
	iop_upload_to_ram( pIOP, addr, exe + 0x800, size );

	if (state->refresh == 0)
	{
		if (!strncasecmp((const char *) exe + 113, "Japan", 5))
			state->refresh = 60;
		else if (!strncasecmp((const char *) exe + 113, "Europe", 6))
			state->refresh = 50;
		else if (!strncasecmp((const char *) exe + 113, "North America", 13))
			state->refresh = 60;
	}

	if (state->first != 0)
	{
		void *pR3000 = iop_get_r3000_state(pIOP);
		r3000_setreg(pR3000, R3000_REG_PC, psx->exec.pc0);
		r3000_setreg(pR3000, R3000_REG_GEN + 29, psx->exec.s_ptr);
		state->first = false;
	}
#endif

	return 0;
}

static void * psf_file_fopen(void * _, const char * uri)
{
	FILE * f = fopen(uri, "rb");
	return (void *)f;
}

static size_t psf_file_fread(void * buffer, size_t size, size_t count, void * handle)
{
	return fread(buffer, size, count, (FILE *)handle);
}

static int psf_file_fseek(void * handle, int64_t offset, int whence)
{
	return fseek((FILE *)handle, offset, whence);
}

static int psf_file_fclose(void * handle)
{
	fclose((FILE *)handle);
	return 0;
}

static long psf_file_ftell(void * handle)
{
	return ftell((FILE *)handle);
}

const psf_file_callbacks psf_file_system =
{
	"\\/|:",
	NULL,
	psf_file_fopen,
	psf_file_fread,
	psf_file_fseek,
	psf_file_fclose,
	psf_file_ftell
};

#if defined(USING_AUDIO_OVERLOAD)
void psf_log(void * context, const char * message)
{
	fprintf(stderr, "%s", message);
	fflush(stderr);
}
#endif

#if defined(USING_HIGHLY_EXPERIMENTAL)
void *bios_data;
bool psf_init_and_load_bios(const char *name)
{
	FILE *fp = fopen(name, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot load %s.\n", name);
		return false;
	}

	int bios_size = 0x400000;
	bios_data = malloc(bios_size);
	if (bios_data == NULL)
	{
		fprintf(stderr, "Cannot allocate memory.\n");
		return false;
	}

	bios_size = fread(bios_data, 1, bios_size, fp);
	fclose(fp);

	if (bios_size >= 0x400000)
	{
		void *bios = mkhebios_create(bios_data, &bios_size);
		bios_data = bios;
	}

	bios_set_image((unsigned char *) bios_data, bios_size);
	int ret = psx_init();
	if (ret != 0)
	{
		fprintf(stderr, "Failed psx_init (%d).\n", ret);
		return false;
	}

	return true;
}
#endif

bool psf_do_init(const char *name)
{
#if defined(USING_HIGHLY_EXPERIMENTAL)
	if (psf_init_and_load_bios("hebios.bin") == false)
		return false;
#endif
	version = psf_load(name, &psf_file_system, 0, 0, 0, 0, 0, 0, 0, 0);

	if (version <= 0)
		return false;

	if (version == 1 || version == 2)
	{
		int state_size = psx_get_state_size(version);
		psx_state = malloc(state_size);
		if (!psx_state)
			return false;
		memset(psx_state, 0, state_size);
#if defined(USING_HIGHLY_EXPERIMENTAL)
		psx_clear_state(psx_state, version);
#endif

#if defined(USING_AUDIO_OVERLOAD)
		psx_register_console_callback(psx_state, psf_log, 0);
#endif

		if (version == 1)
		{
			psf1_load_state state;

			state.emu = psx_state;
			state.first = true;
			state.refresh = 0;

			if (psf_load(name, &psf_file_system, 1, psf1_load, &state, psf1_info, &state, 1, NULL, NULL) < 0)
				return false;

			if (state.refresh != 0)
				psx_set_refresh(psx_state, state.refresh);

#if defined(USING_AUDIO_OVERLOAD)
			psf_start(psx_state);
#endif
		}
		else if (version == 2)
		{
			psf2fs = psf2fs_create();
			if (psf2fs == NULL)
				return false;

			psf1_load_state state;

			state.refresh = 0;

			if (psf_load(name, &psf_file_system, 2, psf2fs_load_callback, psf2fs, psf1_info, &state, 1, NULL, NULL) < 0)
				return false;

			if (state.refresh != 0)
				psx_set_refresh(psx_state, state.refresh);

#if defined(USING_AUDIO_OVERLOAD)
			psf2_register_readfile(psx_state, psf2fs_virtual_readfile, psf2fs);
#endif
#if defined(USING_HIGHLY_EXPERIMENTAL)
			psx_set_readfile(psx_state, psf2fs_virtual_readfile, psf2fs);
#endif
			

#if defined(USING_AUDIO_OVERLOAD)
			psf2_start(psx_state);
#endif
		}
	}

#if defined(USING_HIGHLY_EXPERIMENTAL)
	{
		void *pIOP = psx_get_iop_state(psx_state);
		iop_set_compat(pIOP, IOP_COMPAT_FRIENDLY);
		spu_enable_reverb(iop_get_spu_state(pIOP), 1);
	}
#endif

	return true;
}

unsigned int psf_generate_audio(int16_t *stream, unsigned int samples)
{
	int CHANNELS = 2;
	unsigned int audio_size = samples / CHANNELS;
#if defined(USING_AUDIO_OVERLOAD)
	int32(*psf_gen_ptr)(PSX_STATE *state, int16 *buffer, uint32 count);

	psf_gen_ptr = (version == 2) ? psf2_gen : psf_gen;

	psf_gen_ptr(psx_state, (int16 *)stream, audio_size);
#endif
#if defined(USING_HIGHLY_EXPERIMENTAL)
	psx_execute(psx_state, 0x7FFFFFFF, (int16_t *)stream, &audio_size, 0);
#endif
	return audio_size;
}

#if defined(USING_SDL_AUDIO_OUTPUT)
void audio_callback(void * arg, Uint8 * stream, int len)
{
	psf_generate_audio((int16_t *)stream, len / sizeof(int16_t));
}
#endif

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		char *progname = "psf_tester";
		if (argc >= 1)
		{
			progname = argv[0];
		}
		printf("Usage: %s <infile.psf>\n", progname);
		printf("\n");
		return 0;
	}

	if (psf_do_init(argv[1]))
	{
#if defined(USING_SDL_AUDIO_OUTPUT)
		if (SDL_Init(SDL_INIT_AUDIO) != 0)
		{
			SDL_Log("Failed to psf_do_init audio: %s", SDL_GetError());
			return 1;
		}

		SDL_AudioSpec specs;
		memset(&specs, 0, sizeof(specs));
		specs.freq = (version == 2) ? 48000 : 44100;
		specs.format = AUDIO_S16;
		specs.channels = 2;
		specs.samples = 4096;
		specs.callback = audio_callback;

		int dev = SDL_OpenAudioDevice(NULL, 0, &specs, NULL, 0);
		if (dev == 0)
		{
			SDL_Log("Failed to open audio: %s", SDL_GetError());
			return 1;
		}
		else
		{
			SDL_PauseAudioDevice(dev, 0);
			SDL_Event ev;
			while (SDL_WaitEvent(&ev))
			{
				if (ev.type == SDL_QUIT)
				{
					goto done;
				}
			}
done:
			SDL_CloseAudioDevice(dev);
			return 0;
		}
#else
		if (!isatty(STDOUT_FILENO))
		{
			int SAMPLES = (version == 2) ? SAMPLES480 : SAMPLES441;

			// Pipe for playback: ffplay -f s16le -ar 48000 -ac 2 -

			for (;;)
			{
				psf_generate_audio((int16_t *)audiobuffer, SAMPLES);
				fwrite(audiobuffer, SAMPLES, sizeof(int16_t), stdout);
			}
		}
#endif
	}

	if (psf2fs)
		psf2fs_delete(psf2fs);
	if (psx_state)
	{
#if defined(USING_AUDIO_OVERLOAD)
		if (version == 1)
			psf_stop(psx_state);
		else if (version == 2)
			psf2_stop(psx_state);
#endif
		free(psx_state);
	}

	return 0;
}
