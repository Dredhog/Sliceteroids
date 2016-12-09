#if !defined(TYPES_H)
#define TYPES_H

/*
#include "vec2.h"
#include "platform.h"
#include "globals.h"
*/

struct projectile {
	vec2f P;
	vec2f dP;
	vec2f Dir;
};

struct simulation_asteroid {
	vec2f 		P;
	vec2f 		dP;
	real32		Radius;
	real32 		AngularVelocity;
	uint32		VertCount;
	vec2f 		Verts[ASTEROID_MAX_VERT_COUNT];
};

struct screen_asteroid {
	uint32		VertCount;
	SDL_Point 	Verts[ASTEROID_MAX_VERT_COUNT+1];
};

struct space_ship
{
	vec2f P;
	vec2f dP;
	vec2f Dir;
	real32 Radius;
	vec2f LocalVerts[8]{{15, 15}, {0, 5}, {-15, 15}, {0, -20}, {15, 15}, {7.5, 10}, {0, 15}, {-7.5, 10}};
	vec2f rotated_x_axis_v;
	vec2f rotated_y_axis_v;
	SDL_Point ScreenVerts[8];
	int32 Lives;
	bool32 IsAccelerating;
};

struct game_state
{
	space_ship 			Player;

	simulation_asteroid *SimAsteroids;
	screen_asteroid		*ScreenAsteroids;
	uint32 				AsteroidCount;
	uint32 				AsteroidCapacity;

	projectile			*Projectiles;
	uint32 				ProjectileCount;
	uint32 				ProjectileCapacity;

	bool32				Started;
	bool32 				UseRappidFire{ false };
	uint32				MagicChecksum;
};

enum{
	DEBUG_UpdateAndRender,
	DEBUG_Simulation,
	DEBUG_SimulateAndDrawProjectiles,
	DEBUG_CollideProjectilesWithAsteroids,
	DEBUG_SimulateAsteroidsCollidePlayer,
	DEBUG_RotateAndTranslateAsteroids,
	DEBUG_RenderAsteroids,
	DEBUG_SwapBuffer,
	DEBUG_Last,
};

struct debug_cycle_counter {
	uint64 CycleCount;
	uint64 Calls;
};

#if 1
debug_cycle_counter DEBUG_CYCLE_TABLE[DEBUG_Last] = {};
#define BEGIN_TIMED_BLOCK(ID) uint64 StartCycleCount##ID = _rdtsc();
#define END_TIMED_BLOCK(ID) DEBUG_CYCLE_TABLE[DEBUG_##ID].CycleCount += _rdtsc() - StartCycleCount##ID; \
							DEBUG_CYCLE_TABLE[DEBUG_##ID].Calls++;
char DEBUG_TABLE_NAMES[][40] = {
	"UpdateAndRender",
	"Simulation",
	"SimulateAndDrawProjectiles",
	"CollideProjectilesWithAsteroids",
	"SimulateAsteroidsCollidePlayer",
	"RotateAndTranslateAsteroids",
	"RenderAsteroids",
	"SwapBuffer",
};

#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

void
UpdateAndRender(game_memory *Memory, platform_state *Platform, game_input *Input);

#endif //TYPES_H
