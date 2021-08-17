
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "psflib.h"
#include "psf2fs.h"
#include "psx_external.h"
#include <SDL.h>

void *psx_state = NULL;
void *psf2fs = NULL;

int version = 0;

typedef struct _psf1_load_state
{
	void * emu;
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

int psf1_load(void * context, const uint8_t * exe, size_t exe_size,
	const uint8_t * reserved, size_t reserved_size)
{
	psf1_load_state * state = (psf1_load_state *)context;

	if (reserved && reserved_size)
		return -1;

	if (psf_load_section((PSX_STATE *)state->emu, exe, exe_size, state->first))
		return -1;

	state->first = false;

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

void psf_log(void * context, const char * message)
{
	fprintf(stderr, "%s", message);
	fflush(stderr);
}

bool init(char *name)
{

	version = psf_load(name, &psf_file_system, 0, 0, 0, 0, 0, 0, 0, 0);

	if (version <= 0) return FALSE;

	if (version == 1 || version == 2)
	{
		int state_size = psx_get_state_size(version);
		psx_state = malloc(state_size);
		if (!psx_state) return FALSE;
		memset(psx_state, 0, state_size);

		psx_register_console_callback((PSX_STATE *)psx_state, psf_log, 0);

		if (version == 1)
		{
			psf1_load_state state;

			state.emu = psx_state;
			state.first = true;
			state.refresh = 0;

			if (psf_load(name, &psf_file_system, 1, psf1_load, &state, psf1_info, &state, 1, NULL, NULL) < 0)
				return FALSE;

			if (state.refresh)
				psx_set_refresh((PSX_STATE *)psx_state, state.refresh);

			psf_start((PSX_STATE *)psx_state);
		}
		else if (version == 2)
		{
			psf2fs = psf2fs_create();
			if (!psf2fs) return FALSE;

			psf1_load_state state;

			state.refresh = 0;

			if (psf_load(name, &psf_file_system, 2, psf2fs_load_callback, psf2fs, psf1_info, &state, 1, NULL, NULL) < 0)
				return FALSE;

			if (state.refresh)
				psx_set_refresh((PSX_STATE *)psx_state, state.refresh);

			psf2_register_readfile((PSX_STATE *)psx_state, psf2fs_virtual_readfile, psf2fs);

			psf2_start((PSX_STATE *)psx_state);
		}
	}

	return TRUE;
}

void audio_callback(void * arg, Uint8 * stream, int len)
{
	int32(*mixChunk)(PSX_STATE *state, int16 *buffer, uint32 count);

	mixChunk = (version == 2) ? psf2_gen : psf_gen;
	int CHANNELS = 2;

	mixChunk((PSX_STATE *)psx_state, (int16 *)stream, len / CHANNELS / sizeof(int16));
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: psftest <tune.psf>\n");
		printf("ESC closes.\n");
		printf("\n");
		return 0;
	}

	if (init(argv[1]))
	{
		if (SDL_Init(SDL_INIT_AUDIO) != 0)
		{
			SDL_Log("Failed to init audio: %s", SDL_GetError());
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
			SDL_PauseAudioDevice(dev, 0); /* start audio playing. */
			SDL_Event ev;
			while (SDL_WaitEvent(&ev))
			{
				if (ev.type == SDL_QUIT)
				{
					break;
				}
			}
			SDL_CloseAudioDevice(dev);
			return 0;
		}
	}

	if (psf2fs) psf2fs_delete(psf2fs);
	if (psx_state) {
		if (version == 1) psf_stop((PSX_STATE *)psx_state);
		else if (version == 2) psf2_stop((PSX_STATE *)psx_state);
		free(psx_state);
	}

	return 0;
}
