#if !defined(FUNCTIONS_H)
#define FUNCTIONS_H

/*
#include "platform.h"
#include "game.h"
#include "globals.h"
#include "vec2.h"
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <x86intrin.h>
*/

//SPACE_SHIP
internal space_ship
CreateSpaceShip()
{
	space_ship Result = {};
	Result.Radius = PLAYER_RADIUS;
	Result.P = vec2f{ SCREEN_WIDTH / 2 - PLAYER_RADIUS  / 2, PLAYER_RADIUS / 2 - PLAYER_RADIUS / 2 };
	Result.Lives = PLAYER_MAX_LIVES;
	Result.IsAccelerating = false;

	return Result;
}

internal void
SimulateSpaceShip(space_ship *SpaceShip, const game_input *Input)
{
	vec2f mouse_pos_v = vec2f{(real32)Input->MouseX, (real32)Input->MouseY};

	SpaceShip->IsAccelerating = Input->Space.EndedDown;

	SpaceShip->Dir = mouse_pos_v - SpaceShip->P;
	SpaceShip->Dir.Normalize();

	SpaceShip->rotated_y_axis_v = -SpaceShip->Dir;
	SpaceShip->rotated_x_axis_v.X = -SpaceShip->rotated_y_axis_v.Y;
	SpaceShip->rotated_x_axis_v.Y = SpaceShip->rotated_y_axis_v.X;


	vec2f deceleration_v = -SpaceShip->dP / (vec2f::Length(SpaceShip->dP));
	deceleration_v *= PLAYER_DECELERATION;

	//when the speed is meaninglessly small, set it to zero, to stop any unwanted motion
	if (vec2f::Length(SpaceShip->dP) > PLAYER_MINIMUM_VELOCITY)
	{
		SpaceShip->dP += deceleration_v;
	}	
	else
	{
		SpaceShip->dP.X = 0;
		SpaceShip->dP.Y = 0;
	}
	if (SpaceShip->IsAccelerating)
	{
		//Temporarily use the direction vector as a acceleration vector
		SpaceShip->dP += SpaceShip->Dir.multiple(PLAYER_ACCELERATION);
		SpaceShip->Dir.Normalize();    //Normalize the direction vector before further use
	}

	SpaceShip->P += SpaceShip->dP;
	for (int i = 0; i < 8; ++i)
	{
		SpaceShip->ScreenVerts[i].x = (int)(SpaceShip->P.X + SpaceShip->LocalVerts[i].X*SpaceShip->rotated_x_axis_v.X + SpaceShip->LocalVerts[i].Y*SpaceShip->rotated_y_axis_v.X);
		SpaceShip->ScreenVerts[i].y = (int)(SpaceShip->P.Y + SpaceShip->LocalVerts[i].X*SpaceShip->rotated_x_axis_v.Y + SpaceShip->LocalVerts[i].Y*SpaceShip->rotated_y_axis_v.Y);
	}

	//Toroidal looping
	if (SpaceShip->P.X + PLAYER_RADIUS <= 0 && SpaceShip->dP.X < 0)
	{
		SpaceShip->P.X = SCREEN_WIDTH + PLAYER_RADIUS;
	}
	else if (SpaceShip->P.X - PLAYER_RADIUS > SCREEN_WIDTH && SpaceShip->dP.X > 0)
	{
		SpaceShip->P.X = -PLAYER_RADIUS;
	}
	if (SpaceShip->P.Y + PLAYER_RADIUS <= 0 && SpaceShip->dP.Y < 0)
	{
		SpaceShip->P.Y = SCREEN_HEIGHT + PLAYER_RADIUS;
	}
	else if (SpaceShip->P.Y - PLAYER_RADIUS > SCREEN_HEIGHT && SpaceShip->dP.Y > 0)
	{
		SpaceShip->P.Y = -PLAYER_RADIUS;
	}
}

internal void
DrawSpaceShip(SDL_Renderer* Renderer, const space_ship *SpaceShip)
{
	SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
	SDL_RenderDrawLines(Renderer, SpaceShip->ScreenVerts, (SpaceShip->IsAccelerating ? 8 : 5));
}

//PROJECTILES
internal inline projectile
CreateProjectile(vec2f P, vec2f Dir) {
	projectile Result = {};
	Result.P = P;
	Result.Dir = Dir.Normalized();
	Result.dP = Result.Dir*PROJECTILE_SPEED;
	Result.Dir = Result.Dir*PROJECTILE_LENGTH;
	return Result;
}

internal inline void
AddProjectile(game_state *GameState, projectile newProjectile) {
	assert(GameState->ProjectileCount <= GameState->ProjectileCapacity);
	GameState->Projectiles[GameState->ProjectileCount++] = newProjectile;
}

internal inline void
DestroyProjectile(game_state *GameState, int Index) {
	assert(GameState->ProjectileCount > 0);
	GameState->Projectiles[Index] = GameState->Projectiles[--GameState->ProjectileCount];
}

internal inline bool
IsInBounds(vec2f P)
{
	if (P.X < 0 || P.X > SCREEN_WIDTH ||
		P.Y < 0 || P.Y > SCREEN_HEIGHT)
	{
		return false;
	}
	return true; }

internal inline vec2f
GetProjectileEndP(projectile Projectile) {
	return Projectile.P + Projectile.Dir;
}

internal void
SimulateAndDrawProjectiles(game_state *GameState, SDL_Renderer *Renderer, projectile *Projectiles)
{
	for (uint32 Index = 0; Index < GameState->ProjectileCount;) {
		Projectiles[Index].P += Projectiles[Index].dP;

		if (IsInBounds(GameState->Projectiles[Index].P)) {
			vec2f ProjectileEndP = GetProjectileEndP(GameState->Projectiles[Index]);
			SDL_RenderDrawLine( Renderer, (int)GameState->Projectiles[Index].P.X,
								(int)GameState->Projectiles[Index].P.Y,
								(int)ProjectileEndP.X, (int)ProjectileEndP.Y);
 			Index++;
		}
		else
		{
			DestroyProjectile(GameState, Index);
		}
	}
}

internal bool
TestProjectileAsteroid(projectile Projectile, simulation_asteroid *Asteroid){
	if((real32)vec2f::Length(Projectile.P - Asteroid->P) < Asteroid->Radius + PROJECTILE_LENGTH){
		vec2f p0 = Projectile.P - Asteroid->P;
		vec2f p1 = Projectile.P + Projectile.Dir - Asteroid->P;

		uint32 count = 0;
		for(uint32 Index = 0; Index < Asteroid->VertCount; Index++) {
			if(IsPointAboveLine(p0, p1, Asteroid->Verts[Index])){
				count++;
			}
		}
		if(count != 0 && count != Asteroid->VertCount){
			return true;
		}
	}
	return false;
}

//ASTEROIDS
internal simulation_asteroid
CreateSimAsteroid(int32 initial_x, int32 initial_y, real32 vel_x, real32 vel_y)
{
	simulation_asteroid Result = {};
	Result.P = vec2f{ (real32)initial_x, (real32)initial_y };
	Result.dP = vec2f{ vel_x, vel_y };
	Result.Radius = ASTEROID_RADIUS;
	Result.AngularVelocity = ((real32)(rand() % 200) - 100.f) / 100.f;

	uint32 AdditionalVerts = rand() % ((ASTEROID_MAX_VERT_COUNT - ASTEROID_MINIMUM_VERT_COUNT > 0)
									? ASTEROID_MAX_VERT_COUNT - ASTEROID_MINIMUM_VERT_COUNT : 1);
	Result.VertCount = ASTEROID_MINIMUM_VERT_COUNT + AdditionalVerts;

	for (uint32 i = 0; i < Result.VertCount; ++i)
	{
		Result.Verts[i].X = (real32)(ASTEROID_RADIUS*cos(2 * M_PI*((i + 1.0 + (rand() % 50 - 25) / 100.0) / (int32)(Result.VertCount))));
		Result.Verts[i].Y = (real32)(ASTEROID_RADIUS*sin(2 * M_PI*((i + 1.0 + (rand() % 50 - 25) / 100.0) / (int32)(Result.VertCount))));
	}
	return Result;
}

internal inline void
DestroyAsteroid(game_state *GameState, uint32 Index) {
	assert(Index >= 0 && Index < GameState->AsteroidCount && GameState->AsteroidCount > 0);
	--GameState->AsteroidCount;
	GameState->SimAsteroids[Index] = GameState->SimAsteroids[GameState->AsteroidCount];
	GameState->ScreenAsteroids[Index] = GameState->ScreenAsteroids[GameState->AsteroidCount];
}

internal inline void
AddAsteroid(game_state *GameState, simulation_asteroid SimAsteroid) {
	assert(GameState->AsteroidCount < GameState->AsteroidCapacity);
	GameState->SimAsteroids[GameState->AsteroidCount++] = SimAsteroid;
}

internal inline void
SetAsteroid(game_state *GameState, uint32 Index, simulation_asteroid SimAsteroid) {
	assert(Index >= 0 && Index < GameState->AsteroidCount);
	GameState->SimAsteroids[Index] = SimAsteroid;
}

internal inline void
TakePlayerLife(game_state *GameState) {
	GameState->Player.Lives--;
	GameState->Player.P.X = SCREEN_WIDTH / 2;  GameState->Player.dP.X = 0.0;
	GameState->Player.P.Y = SCREEN_HEIGHT / 2; GameState->Player.dP.Y = 0.0;
}

internal void
SimulateAsteroidsCollidePlayer(game_state *GameState, simulation_asteroid *SimAsteroids, space_ship *Player) {
	vec2f PlayerP = Player->P;
	real32 PlayerRadius = Player->Radius;

	for (uint32 Index = 0; Index < GameState->AsteroidCount;) {
		simulation_asteroid *Asteroid = SimAsteroids + Index;
		if(Asteroid->Radius < ASTEROID_MIN_RADIUS || Asteroid->VertCount > ASTEROID_MAX_VERT_COUNT){
			DestroyAsteroid(GameState, Index);
			continue;
		}

		Asteroid->P += Asteroid->dP;
		
		//Looping in the X direction
		if (Asteroid->P.X + Asteroid->Radius < 0 && Asteroid->dP.X < 0)
		{
			Asteroid->P.X = (real32)(SCREEN_WIDTH + Asteroid->Radius);
		}
		else if (Asteroid->P.X - Asteroid->Radius > SCREEN_WIDTH && Asteroid->dP.X > 0)
		{
			Asteroid->P.X = (real32)-Asteroid->Radius;
		}
		//Looping in the Y direction
		if (Asteroid->P.Y + Asteroid->Radius < 0 && Asteroid->dP.Y < 0)
		{
			Asteroid->P.Y = (real32)(SCREEN_HEIGHT + Asteroid->Radius);
		}
		else if (Asteroid->P.Y - Asteroid->Radius > SCREEN_HEIGHT && Asteroid->dP.Y > 0)
		{
			Asteroid->P.Y = (real32)-Asteroid->Radius;
		}

		if ((int32)vec2f::Length(PlayerP - Asteroid->P) < Asteroid->Radius + PlayerRadius)
		{
			TakePlayerLife(GameState);
		}

		Index++;
	}

}

internal inline void
AddAsteroidTopSplit(game_state *GameState, int ParentIndex, projectile Projectile)
{
	simulation_asteroid *SimParent = GameState->SimAsteroids + ParentIndex;
	simulation_asteroid SimResult = *SimParent;

	//Create the Projectiles' lines' points in the asteroids coordinate system
	vec2f P1 = Projectile.P - SimResult.P;
	vec2f P2 = GetProjectileEndP(Projectile) - SimResult.P;

	uint32 VertCount = 0;
	//If the first point is above the line, add it to the point array
	if (IsPointAboveLine(P1, P2, SimParent->Verts[0]))
	{
		SimResult.Verts[VertCount++] = SimParent->Verts[0];
	}
	for (uint32 i = 1; i < SimParent->VertCount; i++)
	{
		if (IsPointAboveLine(P1, P2, SimParent->Verts[i]) != IsPointAboveLine(P1, P2, SimParent->Verts[i - 1]))
		{
			SimResult.Verts[VertCount++] = GetIntersection(P1, P2, SimParent->Verts[i], SimParent->Verts[i - 1]);
		}
		if (IsPointAboveLine(P1, P2, SimParent->Verts[i]))
		{
			SimResult.Verts[VertCount++] = SimParent->Verts[i];
		}
	}
	if (IsPointAboveLine(P1, P2, SimParent->Verts[0]) != IsPointAboveLine(P1, P2, SimParent->Verts[SimParent->VertCount - 1]))
	{
		SimResult.Verts[VertCount++] = GetIntersection(P1, P2, SimParent->Verts[0], SimParent->Verts[SimParent->VertCount - 1]);
	}

	if(VertCount < 2 || VertCount > ASTEROID_MAX_VERT_COUNT){
		return;
	}
	assert(VertCount >= 2);
	assert(VertCount <= ASTEROID_MAX_VERT_COUNT);

	SimResult.VertCount = VertCount;

	vec2f CenterOfMass = GetAverageP(SimResult.Verts, SimResult.VertCount);
	SimResult.P = SimParent->P + CenterOfMass;
	SimResult.Radius = 0;
	for (uint32 i = 0; i < SimResult.VertCount; i++)
	{
		SimResult.Verts[i] -= CenterOfMass;
		real32 VertOffsetFromCOM = vec2f::Length(SimResult.Verts[i]);
		if (VertOffsetFromCOM > SimResult.Radius) {
			SimResult.Radius = VertOffsetFromCOM;
		}
	}
	AddAsteroid(GameState, SimResult);
}

internal inline void
AddAsteroidBottomSplit(game_state *GameState, int ParentIndex, projectile Projectile)
{
	simulation_asteroid *SimParent = GameState->SimAsteroids + ParentIndex;
	simulation_asteroid SimResult = *SimParent;

	//Create the Projectiles' lines' points in the asteroids coordinate system
	vec2f P1 = Projectile.P - SimResult.P;
	vec2f P2 = GetProjectileEndP(Projectile) - SimResult.P;

	uint32 VertCount = 0;
	//If the first point is above the line, add it to the point array
	if (!IsPointAboveLine(P1, P2, SimParent->Verts[0]))
	{
		SimResult.Verts[VertCount++] = SimParent->Verts[0];
	}
	for (uint32 i = 1; i < SimParent->VertCount; i++)
	{
		if ((IsPointAboveLine(P1, P2, SimParent->Verts[i]) != IsPointAboveLine(P1, P2, SimParent->Verts[i - 1])))
		{
			SimResult.Verts[VertCount++] = GetIntersection(P1, P2, SimParent->Verts[i], SimParent->Verts[i - 1]);
		}
		if (!IsPointAboveLine(P1, P2, SimParent->Verts[i]))
		{
			SimResult.Verts[VertCount++] = SimParent->Verts[i];
		}
	}
	if (IsPointAboveLine(P1, P2, SimParent->Verts[0]) != IsPointAboveLine(P1, P2, SimParent->Verts[SimParent->VertCount - 1]))
	{
		SimResult.Verts[VertCount++] = GetIntersection(P1, P2, SimParent->Verts[0], SimParent->Verts[SimParent->VertCount - 1]);
	}

	if(VertCount < 2 || VertCount > ASTEROID_MAX_VERT_COUNT){
		return;
	}
	assert(VertCount >= 2);
	assert(VertCount <= ASTEROID_MAX_VERT_COUNT);

	SimResult.VertCount = VertCount;

	vec2f CenterOfMass = GetAverageP(SimResult.Verts, SimResult.VertCount);
	SimResult.P = SimParent->P + CenterOfMass;
	SimResult.Radius = 0;
	for (uint32 i = 0; i < SimResult.VertCount; i++)
	{
		SimResult.Verts[i] -= CenterOfMass;
		real32 VertOffsetFromCOM = vec2f::Length(SimResult.Verts[i]);
		if (VertOffsetFromCOM > SimResult.Radius) {
			SimResult.Radius = VertOffsetFromCOM;
		}
	}
	AddAsteroid(GameState, SimResult);
}

internal void
CollideProjectilesWithAsteroids(game_state *GameState, simulation_asteroid *SimAsteroids,  projectile *Projectiles){
	for(uint32 ProjectileIndex = 0; ProjectileIndex < GameState->ProjectileCount; ProjectileIndex++){
		for(uint32 AsteroidIndex = 0; AsteroidIndex < GameState->AsteroidCount; AsteroidIndex++){
			if(TestProjectileAsteroid(Projectiles[ProjectileIndex], &SimAsteroids[AsteroidIndex])){
				AddAsteroidBottomSplit(GameState, AsteroidIndex, Projectiles[ProjectileIndex]);
				AddAsteroidTopSplit(GameState, AsteroidIndex, Projectiles[ProjectileIndex]);
				DestroyAsteroid(GameState, AsteroidIndex);
				DestroyProjectile(GameState, ProjectileIndex);
				ProjectileIndex--;
				break;
			}
		}
	}
}

internal void
RotateAndTranslateAsteroids(simulation_asteroid *Input, screen_asteroid *Output, uint32 AsteroidCount) {
	const real32 AngleRad = PI / 180.f;
	real32 Cosine;
	real32 Sine;
	vec2f NewUnitX;
	vec2f NewUnitY;

	for (uint32 AsteroidIndex = 0; AsteroidIndex < AsteroidCount; AsteroidIndex++) {
		Cosine = (real32)cos((real64)(AngleRad*Input[AsteroidIndex].AngularVelocity));
		Sine = (real32)sin((real64)(AngleRad*Input[AsteroidIndex].AngularVelocity)); 
		NewUnitX = vec2f{ Cosine,  Sine};
		NewUnitY =  vec2f{ -Sine,  Cosine};
		for (uint32 Index = 0; Index < Input[AsteroidIndex].VertCount; ++Index)
		{
			Input[AsteroidIndex].Verts[Index].X = Input[AsteroidIndex].Verts[Index].X*NewUnitX.X + Input[AsteroidIndex].Verts[Index].Y*NewUnitY.X;
			Input[AsteroidIndex].Verts[Index].Y = Input[AsteroidIndex].Verts[Index].X*NewUnitX.Y + Input[AsteroidIndex].Verts[Index].Y*NewUnitY.Y;
			Output[AsteroidIndex].Verts[Index].x = (int32)(Input[AsteroidIndex].P.X + Input[AsteroidIndex].Verts[Index].X);
			Output[AsteroidIndex].Verts[Index].y = (int32)(Input[AsteroidIndex].P.Y + Input[AsteroidIndex].Verts[Index].Y);
		}
		Output[AsteroidIndex].VertCount = Input[AsteroidIndex].VertCount;
	}
}

void
RenderAsteroids(SDL_Renderer *Renderer, screen_asteroid *ScreenAsteroids, uint32 Count)
{
	for (uint32 Index = 0; Index < Count; Index++) {
		assert(ScreenAsteroids[Index].VertCount <= ASTEROID_MAX_VERT_COUNT);

		ScreenAsteroids[Index].Verts[ScreenAsteroids[Index].VertCount] = ScreenAsteroids[Index].Verts[0];
		SDL_RenderDrawLines(Renderer, ScreenAsteroids[Index].Verts, ScreenAsteroids[Index].VertCount + 1);
	}
}

internal void
InitGameState(game_state *GameState, void *MemoryEnd)
{
	assert(GameState);
	char *BaseAddress = ((char*)GameState) + sizeof(game_state);
	assert((BaseAddress + sizeof(game_state) +	PROJECTILE_MAX_COUNT*sizeof(projectile) +
			ASTEROID_MAX_COUNT*(sizeof(simulation_asteroid) + sizeof(screen_asteroid))) <= (char*)MemoryEnd);

	GameState->Player = CreateSpaceShip();

	GameState->Projectiles = (projectile*)BaseAddress;
	GameState->SimAsteroids = (simulation_asteroid*)(GameState->Projectiles + PROJECTILE_MAX_COUNT);
	GameState->ScreenAsteroids = (screen_asteroid*)(GameState->SimAsteroids + ASTEROID_MAX_COUNT);

	GameState->AsteroidCapacity = ASTEROID_MAX_COUNT; 
	GameState->ProjectileCapacity = PROJECTILE_MAX_COUNT;
	GameState->AsteroidCount = 0;
	GameState->ProjectileCount = 0;
}

internal void
NewGame(game_state *GameState)
{
	GameState->ProjectileCount = 0;
	GameState->AsteroidCount = 0;

	//printf("NEW GAME! Player'S LIVES: %d\n", GameState->Player.Lives);
	GameState->Player.P = vec2f{ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	GameState->Player.dP = vec2f{ 0, 0 };
	GameState->Player.Lives = PLAYER_MAX_LIVES;
	srand(0); //Makeing the game deterministic for replay

	for (uint32 Index = 0; Index < INITIAL_ASTEROID_COUNT; Index++)
	{
		int x, y;
		do {
			x = rand() % SCREEN_WIDTH;
			y = rand() % SCREEN_HEIGHT;
		} while (sqrt((SCREEN_WIDTH / 2 - x)*(SCREEN_WIDTH / 2 - x) + (SCREEN_HEIGHT / 2 - y)*(SCREEN_HEIGHT / 2 - y)) <= SAFEZONE_RADIUS + ASTEROID_RADIUS);

		simulation_asteroid newSimAsteroid = CreateSimAsteroid(x, y, (real32)(rand() % 200 - 100) / 100.f, (real32)(rand() % 200 - 100) / 100.f);
		AddAsteroid(GameState, newSimAsteroid);
	}
}

void
UpdateAndRender(game_memory *Memory, platform_state *Platform, game_input *Input)
{
	BEGIN_TIMED_BLOCK(UpdateAndRender);

	game_state *GameState = (game_state*)Memory->BaseAddress;
	if(GameState->MagicChecksum != 11789){
		InitGameState(GameState, (char*)Memory->BaseAddress + Memory->Size);
		NewGame(GameState);
		GameState->MagicChecksum = 11789;
	}

	SDL_SetRenderDrawColor(Platform->Renderer, 20, 20, 20, 255);
	SDL_RenderClear(Platform->Renderer);

	SDL_SetRenderDrawColor(Platform->Renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(Platform->Renderer, &Platform->screen_outline);

	BEGIN_TIMED_BLOCK(Simulation);
	{
		SimulateSpaceShip(&GameState->Player, Input);
		if(Input->MouseLeft.EndedDown || (Input->MouseRight.EndedDown && Input->MouseRight.Changed)){
			AddProjectile(GameState, CreateProjectile(GameState->Player.P, GameState->Player.Dir));
		}

		BEGIN_TIMED_BLOCK(SimulateAndDrawProjectiles);
		SimulateAndDrawProjectiles(GameState, Platform->Renderer, GameState->Projectiles);
		END_TIMED_BLOCK(SimulateAndDrawProjectiles);

		BEGIN_TIMED_BLOCK(CollideProjectilesWithAsteroids);
		CollideProjectilesWithAsteroids(GameState, GameState->SimAsteroids, GameState->Projectiles);
		END_TIMED_BLOCK(CollideProjectilesWithAsteroids);

		BEGIN_TIMED_BLOCK(SimulateAsteroidsCollidePlayer);
		SimulateAsteroidsCollidePlayer(GameState, GameState->SimAsteroids, &GameState->Player);
		END_TIMED_BLOCK(SimulateAsteroidsCollidePlayer);

		if( (GameState->Player.Lives <= 0) || (GameState->AsteroidCount == 0 && (GameState->AsteroidCount != INITIAL_ASTEROID_COUNT)) )
		{
			NewGame(GameState);
			return;
		}
		BEGIN_TIMED_BLOCK(RotateAndTranslateAsteroids);
		RotateAndTranslateAsteroids(GameState->SimAsteroids, GameState->ScreenAsteroids, GameState->AsteroidCount);
		END_TIMED_BLOCK(RotateAndTranslateAsteroids);

		BEGIN_TIMED_BLOCK(RenderAsteroids);
		RenderAsteroids(Platform->Renderer, GameState->ScreenAsteroids, GameState->AsteroidCount);
		END_TIMED_BLOCK(RenderAsteroids);
	}
	END_TIMED_BLOCK(Simulation);

	DrawSpaceShip(Platform->Renderer, &GameState->Player);
	BEGIN_TIMED_BLOCK(SwapBuffer);
	SDL_RenderPresent(Platform->Renderer);
	END_TIMED_BLOCK(SwapBuffer);

	END_TIMED_BLOCK(UpdateAndRender);
}

#endif //FUNCTIONS_H
