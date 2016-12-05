#include <SDL.h>
#include "timer.cpp"
#include <random>
#include "vec2.h"
#include "assert.h"

#include <stdint.h>
#include "globals.h"
#include "platform.h"
#include "game.h"
#include "game.cpp"
#include "immintrin.h"
#include <sys/mman.h>

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
	return true;
}

internal void
InitGameMemory(game_memory *GameMemory){
	GameMemory->Size = Megabytes(64);
	GameMemory->BaseAddress = (void*)Gigabytes(10);
	GameMemory->BaseAddress = mmap(	GameMemory->BaseAddress, GameMemory->Size,
							PROT_READ | PROT_WRITE,
							MAP_ANONYMOUS | MAP_PRIVATE,
							-1, 0);
}

internal void
HandleEvents(game_state *GameState, SDL_Event* e)
{
	if (e->type == SDL_QUIT)
	{
		//GameState->Running = false;
	}
	if (e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		switch (e->key.keysym.sym)
		{
		case SDLK_ESCAPE:
			//GameState->Running = false; break;
		case SDLK_SPACE:
			GameState->Player.IsAccelerating = true;
		}
	}
	if (e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		switch (e->key.keysym.sym)
		{
		case SDLK_SPACE:
			GameState->Player.IsAccelerating = false;
		}
	}
	if (e->type == SDL_MOUSEBUTTONDOWN)
	{
		switch (e->button.button)
		{
		case(SDL_BUTTON_LEFT) :
			GameState->UseRappidFire = true;
			break;
		case(SDL_BUTTON_RIGHT) :
			AddProjectile(GameState, CreateProjectile(GameState->Player.P,
				GameState->Player.Dir));
			break;
		}
	}
	if (e->type == SDL_MOUSEBUTTONUP)
	{
		switch (e->button.button)
		{
		case(SDL_BUTTON_LEFT) :
			GameState->UseRappidFire = false;
			break;
		}
	}
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


int main(int argc, char *args[])
{
	platform_state PlatformState = {};
	assert(InitPlatform(&PlatformState));

	game_memory GameMemory  = {};
	InitGameMemory(&GameMemory);
	assert(GameMemory.BaseAddress);

	while (true)
	{
		PlatformState.FPS.start();
		while (SDL_PollEvent(&PlatformState.Event) != 0)
		{
			HandleEvents(&GameMemory, &PlatformState.Event);
		}
		UpdateAndRender(&GameMemory, &PlatformState, &CurrentInput);

		SDL_Delay(FRAME_DURATION - ((PlatformState.FPS.get_time() <= FRAME_DURATION) ? PlatformState.FPS.get_time() : FRAME_DURATION));
		PlatformState.FPS.update_avg_fps();
	}

	CleanUp(&PlatformState);
	return 0;
}

//printf("fps: %f;\tprojectiles: %d of %d;\t asteroids: %d of %d\n", GameState.FPS.get_average_fps(), GameState.ProjectileCount, GameState.ProjectileCapacity, GameState.AsteroidCount, GameState.AsteroidCapacity);
