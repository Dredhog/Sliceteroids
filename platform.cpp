#include <SDL.h>
#include <random>
#include "vec2.h"
#include "assert.h"

#include <stdint.h>
#include "globals.h"
#include "platform.h"
#include "game.h"
#include "immintrin.h"
#include "circular_buffer.h"
#include <sys/mman.h>
//#include <Windows.h>

internal bool32
InitPlatform(platform_state *Platform)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL Initialization error: %s\n", SDL_GetError());
		return false;
	}
	if ((Platform->Window = SDL_CreateWindow("Data Driven Asteroids", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == NULL) {
		printf("Window creation error: %s\n", SDL_GetError());
		return false;
	}
	if ((Platform->Renderer = SDL_CreateRenderer(Platform->Window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
		printf("Renderer creation error: %s\n", SDL_GetError());
		return false;
	}
	Platform->Running = true;
	return true;
}

internal void
AllocateGameMemory(game_memory *GameMemory) {
	GameMemory->Size = Megabytes(1);
	GameMemory->BaseAddress = (void*)Gigabytes(10);
	//LPVOID BaseAddress = (LPVOID*)Gigabytes(10);
	GameMemory->BaseAddress = mmap(GameMemory->BaseAddress, GameMemory->Size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	//GameMemory->BaseAddress = VirtualAlloc(BaseAddress, GameMemory->Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

internal bool32
ProcessInput(game_input *OldInput, game_input *NewInput, SDL_Event* Event)
{
	*NewInput = *OldInput;
	while (SDL_PollEvent(Event) != 0)
	{
		switch (Event->type) {
		case SDL_QUIT:
			return false;
		case SDL_KEYDOWN:
			if (Event->key.keysym.sym == SDLK_ESCAPE) {
				NewInput->Escape.EndedDown = true;
				return false;
			}
			else if (Event->key.keysym.sym == SDLK_SPACE) {
				NewInput->Space.EndedDown = true;
			}
			else if (Event->key.keysym.sym == SDLK_p) {
				NewInput->p.EndedDown = true;
			}
			break;
		case SDL_KEYUP:
			if (Event->key.keysym.sym == SDLK_SPACE) {
				NewInput->Space.EndedDown = false;
			}
			else if (Event->key.keysym.sym == SDLK_p) {
				NewInput->p.EndedDown = false;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (Event->button.button == SDL_BUTTON_LEFT) {
				NewInput->MouseLeft.EndedDown = true;
			}
			else if (Event->button.button == SDL_BUTTON_RIGHT) {
				NewInput->MouseRight.EndedDown = true;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (Event->button.button == SDL_BUTTON_LEFT) {
				NewInput->MouseLeft.EndedDown = false;
			}
			else if (Event->button.button == SDL_BUTTON_RIGHT) {
				NewInput->MouseRight.EndedDown = false;
			}
			break;
		}
	}
	SDL_GetMouseState(&NewInput->MouseX, &NewInput->MouseY);

	for (int Index = 0; Index < 5; Index++) {
		NewInput->Buttons[Index].Changed = (OldInput->Buttons[Index].EndedDown ==
			NewInput->Buttons[Index].EndedDown) ? false : true;
	}
	return true;
}

internal void
CleanUp(platform_state *Platform)
{
	SDL_DestroyWindow(Platform->Window);
	Platform->Window = nullptr;

	SDL_DestroyRenderer(Platform->Renderer);
	Platform->Renderer = nullptr;
	SDL_Quit();
}


int main(int Count, char *Arguments[])
{
	platform_state Platform = {};
	assert(InitPlatform(&Platform));

	game_memory GameMemory = {};
	AllocateGameMemory(&GameMemory);
	assert(GameMemory.BaseAddress);

	playback_buffer PlaybackBuffer = NewPlaybackBuffer(500, 10, GameMemory.Size);

	game_input OldInput = {};
	game_input NewInput = {};

	while (Platform.Running)
	{
		Platform.FPS.start();

		Platform.Running = ProcessInput(&OldInput, &NewInput, &Platform.Event);
		if (NewInput.p.EndedDown && NewInput.p.Changed) {
			Platform.PlaybackStarted = !Platform.PlaybackStarted;
			if (!Platform.PlaybackStarted) {
				ClearPlaybackBuffer(&PlaybackBuffer);
				for(int ButtonIndex = 0; ButtonIndex < 6; ButtonIndex++){
					NewInput.Buttons[ButtonIndex].EndedDown = false;
					NewInput.Buttons[ButtonIndex].Changed = false;
				}
			}
		}

		if (Platform.PlaybackStarted) {
			PeekAndStepPlaybackBuffer(&PlaybackBuffer, &NewInput, GameMemory.BaseAddress, Platform.FrameCount);
		}
		else {
			PushPlaybackBuffer(&PlaybackBuffer, &NewInput, GameMemory.BaseAddress, Platform.FrameCount);
		}

		UpdateAndRender(&GameMemory, &Platform, &NewInput);

		SDL_Delay(FRAME_DURATION - ((Platform.FPS.get_time() <= FRAME_DURATION) ? Platform.FPS.get_time() : FRAME_DURATION));
		Platform.FPS.update_avg_fps();

		OldInput = NewInput;
		Platform.FrameCount++;
	}

	CleanUp(&Platform);
	return 0;
}

//printf("fps: %f;\tprojectiles: %d of %d;\t asteroids: %d of %d\n", GameState.FPS.get_average_fps(), GameState.ProjectileCount, GameState.ProjectileCapacity, GameState.AsteroidCount, GameState.AsteroidCapacity);
