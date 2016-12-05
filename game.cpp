#if !defined(FUNCTIONS_H)
#define FUNCTIONS_H

internal simulation_asteroid SimulationAsteroidMemory[ASTEROID_MAX_COUNT];;
internal shape_asteroid LocalAsteroidPolygonMemory[ASTEROID_MAX_COUNT];
internal screen_asteroid ScreenAsteroidPolygonMemory[ASTEROID_MAX_COUNT];
internal int32 AsteroidVertCountMemory[ASTEROID_MAX_COUNT];
internal projectile ProjectileMemory[PROJECTILE_MAX_COUNT];

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
SimulateSpaceShip(space_ship *SpaceShip)
{
	static int MouseX, MouseY;
	SDL_GetMouseState(&MouseX, &MouseY);

	vec2f mouse_pos_v = vec2f{ (float)MouseX, (float)MouseY };

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
	return true;
}

internal inline vec2f
GetProjectileEndP(projectile Projectile) {
	return Projectile.P + Projectile.Dir;
}

internal void
UpdateAndRenderProjectiles(game_state *GameState, SDL_Renderer *Renderer, projectile *Projectiles)
{
	for (int Index = 0; Index < GameState->ProjectileCount;) {
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

bool
TestProjectileAsteroid(projectile Projectile, simulation_asteroid Asteroid){
	if(vec2f::Length(Projectile.P - Asteroid.P) < Asteroid.Radius + PROJECTILE_LENGTH){
		return true;	
	}
	return false;
}

//ASTEROIDS
internal simulation_asteroid
CreateSimAsteroid(int initial_x, int initial_y, float vel_x, float vel_y)
{
	simulation_asteroid Result;
	Result.P = vec2f{ (float)initial_x, (float)initial_y };
	Result.dP = vec2f{ vel_x, vel_y };
	Result.Radius = ASTEROID_RADIUS;
	Result.AngularVelocity = (rand() % 200 - 100) / 100.0;
	return Result;
}

shape_asteroid
CreateAsteroidShape(int VertCount) {
	shape_asteroid Result;
	//Assign the initial vertex locations
	for (int i = 0; i < VertCount; ++i)
	{
		//Set the initial points for the asteroid polygon
		Result.Verts[i].X = ASTEROID_RADIUS*cos(2 * M_PI*((i + 1.0 + (rand() % 50 - 25) / 100.0) / (VertCount)));
		Result.Verts[i].Y = ASTEROID_RADIUS*sin(2 * M_PI*((i + 1.0 + (rand() % 50 - 25) / 100.0) / (VertCount)));
	}
	return Result;
}

/*
simulation_asteroid
CreateAsteroid(const simulation_asteroid *parent, float angle)
{
	assert(&parent);
	simulation_asteroid Result = simulation_asteroid{};
	Result.P = parent->P;
	Result.dP = parent->dP.rotated(angle)*ASTEROID_BREAK_SPEED_MULTIPLYER;
	Result.Radius = parent->Radius / 2;

	//Number of points in the polygon,(the first and last are the same)
	Result.VertCount = rand() % 3 + ASTEROID_MINIMUM_VERT_COUNT;
	Result.AngularVelocity = (rand() % 200 - 100) / 100.0;
	for (int i = 0; i < Result.VertCount; ++i)
	{
		//Set the initial points for the asteroid polygon
		Result.LocalVerts[i].X = Result.Radius*cos(2 * M_PI*((i + 1.0 + (rand() % 50 - 25) / 100.0) / (Result.VertCount)));
		Result.LocalVerts[i].Y = Result.Radius*sin(2 * M_PI*((i + 1.0 + (rand() % 50 - 25) / 100.0) / (Result.VertCount)));
	}
	return Result;
}
*/


inline void
DestroyAsteroid(game_state *GameState, int Index) {
	assert(Index >= 0 && Index < GameState->AsteroidCount && GameState->AsteroidCount > 0);
	--GameState->AsteroidCount;
	GameState->AsteroidVertCounts[Index] = GameState->AsteroidVertCounts[GameState->AsteroidCount];
	GameState->LocalAsteroids[Index] = GameState->LocalAsteroids[GameState->AsteroidCount];
	GameState->SimAsteroids[Index] = GameState->SimAsteroids[GameState->AsteroidCount];
}
/*
inline void
AddAsteroid(game_state *GameState, simulation_asteroid SimAsteroid, shape_asteroid ShapeAsteroid) {
	assert(GameState->AsteroidCount < GameState->AsteroidCapacity);
	GameState->SimAsteroids[GameState->AsteroidCount] = SimAsteroid;
	GameState->LocalAsteroids[GameState->AsteroidCount] = ShapeAsteroid;
	GameState->AsteroidCount++;
}*/

inline void
AddAsteroid(game_state *GameState, simulation_asteroid SimAsteroid, shape_asteroid ShapeAsteroid, int VertCount) {
	assert(GameState->AsteroidCount < GameState->AsteroidCapacity);
	GameState->SimAsteroids[GameState->AsteroidCount] = SimAsteroid;
	GameState->LocalAsteroids[GameState->AsteroidCount] = ShapeAsteroid;
	GameState->AsteroidVertCounts[GameState->AsteroidCount] = VertCount;
	GameState->AsteroidCount++;
}

inline void
SetAsteroid(game_state *GameState, int Index, simulation_asteroid SimAsteroid, shape_asteroid ShapeAsteroid, int VertCount) {
	assert(Index >= 0 && Index < GameState->AsteroidCount);
	GameState->SimAsteroids[Index] = SimAsteroid;
	GameState->LocalAsteroids[Index] = ShapeAsteroid;
	GameState->AsteroidVertCounts[Index] = VertCount;
}

inline void
TakePlayerLife(game_state *GameState) {
	GameState->Player.Lives--;
	GameState->Player.P.X = SCREEN_WIDTH / 2;  GameState->Player.dP.X = 0.0;
	GameState->Player.P.Y = SCREEN_HEIGHT / 2; GameState->Player.dP.Y = 0.0;
}

void
MoveAsteroids(game_state *GameState, simulation_asteroid *SimAsteroids, space_ship *Player) {
	vec2f PlayerP = Player->P;
	float PlayerRadius = Player->Radius;

	for (int Index = 0; Index < GameState->AsteroidCount;) {
		simulation_asteroid *Asteroid = SimAsteroids + Index;
		if(Asteroid->Radius < ASTEROID_MIN_RADIUS){
			DestroyAsteroid(GameState, Index);
			continue;
		}
		Asteroid->P += Asteroid->dP;
		
		//Looping in the X direction
		if (Asteroid->P.X + Asteroid->Radius < 0 && Asteroid->dP.X < 0)
		{
			Asteroid->P.X = SCREEN_WIDTH + Asteroid->Radius;
		}
		else if (Asteroid->P.X - Asteroid->Radius > SCREEN_WIDTH && Asteroid->dP.X > 0)
		{
			Asteroid->P.X = -Asteroid->Radius;
		}
		//Looping in the Y direction
		if (Asteroid->P.Y + Asteroid->Radius < 0 && Asteroid->dP.Y < 0)
		{
			Asteroid->P.Y = SCREEN_HEIGHT + Asteroid->Radius;
		}
		else if (Asteroid->P.Y - Asteroid->Radius > SCREEN_HEIGHT && Asteroid->dP.Y > 0)
		{
			Asteroid->P.Y = -Asteroid->Radius;
		}

		if (vec2f::Length(PlayerP - Asteroid->P) < Asteroid->Radius + PlayerRadius)
		{
			TakePlayerLife(GameState);
		}

		Index++;
	}

}

inline void
AddAsteroidTopSplit(game_state *GameState, int ParentIndex, projectile Projectile)
{
	simulation_asteroid *SimParent = GameState->SimAsteroids + ParentIndex;
	shape_asteroid *ShapeParent = GameState->LocalAsteroids + ParentIndex;
	int ParentVertCount = GameState->AsteroidVertCounts[ParentIndex];

	simulation_asteroid SimResult = *SimParent;
	shape_asteroid ShapeResult = *ShapeParent;

	//Create the Projectiles' lines' points in the asteroids coordinate system
	vec2f P1 = Projectile.P - SimResult.P;
	vec2f P2 = GetProjectileEndP(Projectile) - SimResult.P;

	int VertCount = 0;
	//If the first point is above the line, add it to the point array
	if (IsPointAboveLine(P1, P2, ShapeParent->Verts[0]))
	{
		ShapeResult.Verts[VertCount++] = ShapeParent->Verts[0];
	}
	for (int i = 1; i < ParentVertCount; i++)
	{
		if (IsPointAboveLine(P1, P2, ShapeParent->Verts[i]) != IsPointAboveLine(P1, P2, ShapeParent->Verts[i - 1]))
		{
			ShapeResult.Verts[VertCount++] = GetIntersection(P1, P2, ShapeParent->Verts[i], ShapeParent->Verts[i - 1]);
		}
		if (IsPointAboveLine(P1, P2, ShapeParent->Verts[i]))
		{
			ShapeResult.Verts[VertCount++] = ShapeParent->Verts[i];
		}
	}
	if (IsPointAboveLine(P1, P2, ShapeParent->Verts[0]) != IsPointAboveLine(P1, P2, ShapeParent->Verts[ParentVertCount - 1]))
	{
		ShapeResult.Verts[VertCount++] = GetIntersection(P1, P2, ShapeParent->Verts[0], ShapeParent->Verts[ParentVertCount - 1]);
	}

	if (VertCount < 1){
		return;
	}

	vec2f CenterOfMass = GetAverageP(ShapeResult.Verts, VertCount);
	SimResult.P = SimParent->P + CenterOfMass;
	SimResult.Radius = 0;
	for (int i = 0; i < VertCount; i++)
	{
		ShapeResult.Verts[i] -= CenterOfMass;
		float VertOffsetFromCOM = vec2f::Length(ShapeResult.Verts[i]);
		if (VertOffsetFromCOM > SimResult.Radius) {
			SimResult.Radius = VertOffsetFromCOM;
		}
	}
	AddAsteroid(GameState, SimResult, ShapeResult, VertCount);
}

inline void
AddAsteroidBottomSplit(game_state *GameState, int ParentIndex, projectile Projectile)
{
	simulation_asteroid *SimParent = GameState->SimAsteroids + ParentIndex;
	shape_asteroid *ShapeParent = GameState->LocalAsteroids + ParentIndex;
	int ParentVertCount = GameState->AsteroidVertCounts[ParentIndex];

	simulation_asteroid SimResult = *SimParent;
	shape_asteroid ShapeResult = *ShapeParent;

	//Create the Projectiles' lines' points in the asteroids coordinate system
	vec2f P1 = Projectile.P - SimResult.P;
	vec2f P2 = GetProjectileEndP(Projectile) - SimResult.P;

	int VertCount = 0;
	//If the first point is above the line, add it to the point array
	if (!IsPointAboveLine(P1, P2, ShapeParent->Verts[0]))
	{
		ShapeResult.Verts[VertCount++] = ShapeParent->Verts[0];
	}
	for (int i = 1; i < ParentVertCount; i++)
	{
		if ((IsPointAboveLine(P1, P2, ShapeParent->Verts[i]) != IsPointAboveLine(P1, P2, ShapeParent->Verts[i - 1])))
		{
			ShapeResult.Verts[VertCount++] = GetIntersection(P1, P2, ShapeParent->Verts[i], ShapeParent->Verts[i - 1]);
		}
		if (!IsPointAboveLine(P1, P2, ShapeParent->Verts[i]))
		{
			ShapeResult.Verts[VertCount++] = ShapeParent->Verts[i];
		}
	}
	if (IsPointAboveLine(P1, P2, ShapeParent->Verts[0]) != IsPointAboveLine(P1, P2, ShapeParent->Verts[ParentVertCount - 1]))
	{
		ShapeResult.Verts[VertCount++] = GetIntersection(P1, P2, ShapeParent->Verts[0], ShapeParent->Verts[ParentVertCount - 1]);
	}

	if (VertCount < 1){
		return;
	}

	vec2f CenterOfMass = GetAverageP(ShapeResult.Verts, VertCount);
	SimResult.P = SimParent->P + CenterOfMass;
	SimResult.Radius = 0;
	for (int i = 0; i < VertCount; i++)
	{
		ShapeResult.Verts[i] -= CenterOfMass;
		float VertOffsetFromCOM = vec2f::Length(ShapeResult.Verts[i]);
		if (VertOffsetFromCOM > SimResult.Radius) {
			SimResult.Radius = VertOffsetFromCOM;
		}
	}
	AddAsteroid(GameState, SimResult, ShapeResult, VertCount);
}

internal void
CollideAsteroidsWithProjectiles(game_state *GameState, simulation_asteroid *SimAsteroids,  projectile *Projectiles){
	for(int ProjectileIndex = 0; ProjectileIndex < GameState->ProjectileCount; ProjectileIndex++){
		for(int AsteroidIndex = 0; AsteroidIndex < GameState->AsteroidCount; AsteroidIndex++){
			if(TestProjectileAsteroid(Projectiles[ProjectileIndex], SimAsteroids[AsteroidIndex])){
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

void
RotateAndTranslateAsteroidsToScreen(game_state *GameState, shape_asteroid *Input, simulation_asteroid *SimAsteroids, int *VertCounts, screen_asteroid *Output) {
	const float AngleRad = M_PI / 180.0;
	float Cosine;
	float Sine;
	vec2f NewUnitX;
	vec2f NewUnitY;

	for (int AsteroidIndex = 0; AsteroidIndex < GameState->AsteroidCount; AsteroidIndex++) {
		Cosine = cos(AngleRad*SimAsteroids[AsteroidIndex].AngularVelocity);
		Sine = sin(AngleRad*SimAsteroids[AsteroidIndex].AngularVelocity); 
		NewUnitX = vec2f{ Cosine,  Sine};
		NewUnitY =  vec2f{ -Sine,  Cosine};
		for (int Index = 0; Index < VertCounts[AsteroidIndex]; ++Index)
		{
			Input[AsteroidIndex].Verts[Index].X = Input[AsteroidIndex].Verts[Index].X*NewUnitX.X + Input[AsteroidIndex].Verts[Index].Y*NewUnitY.X;
			Input[AsteroidIndex].Verts[Index].Y = Input[AsteroidIndex].Verts[Index].X*NewUnitX.Y + Input[AsteroidIndex].Verts[Index].Y*NewUnitY.Y;
			Output[AsteroidIndex].Verts[Index].x = SimAsteroids[AsteroidIndex].P.X + Input[AsteroidIndex].Verts[Index].X;
			Output[AsteroidIndex].Verts[Index].y = SimAsteroids[AsteroidIndex].P.Y + Input[AsteroidIndex].Verts[Index].Y;
		}
	}
}

void
DrawAsteroids(SDL_Renderer *Renderer, screen_asteroid *ScreenAsteroids, int *VertCounts, int Count)
{
	for (int Index = 0; Index < Count; Index++) {
		assert(VertCounts[Index] <= ASTEROID_MAX_VERT_COUNT);

		ScreenAsteroids[Index].Verts[VertCounts[Index]] = ScreenAsteroids[Index].Verts[0];
		SDL_RenderDrawLines(Renderer, ScreenAsteroids[Index].Verts, VertCounts[Index] + 1);
	}
}

internal void
InitGameState(game_state *GameState)
{
	GameState->ProjectileCapacity = sizeof(ProjectileMemory) / sizeof(projectile);
	GameState->AsteroidCapacity = sizeof(SimulationAsteroidMemory) / sizeof(simulation_asteroid);

	GameState->SimAsteroids = SimulationAsteroidMemory;
	GameState->LocalAsteroids = LocalAsteroidPolygonMemory;
	GameState->ScreenAsteroids = ScreenAsteroidPolygonMemory;
	GameState->AsteroidVertCounts = AsteroidVertCountMemory;
	GameState->Projectiles = ProjectileMemory;
	GameState->Player = CreateSpaceShip();
}

internal void
NewGame(game_state *GameState)
{
	GameState->ProjectileCount = 0;
	GameState->AsteroidCount = 0;

	printf("NEW GAME! Player'S LIVES: %d\n", GameState->Player.Lives);
	GameState->Player.P = vec2f{ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	GameState->Player.dP = vec2f{ 0, 0 };
	GameState->Player.Lives = PLAYER_MAX_LIVES;
	srand(time(nullptr));

	for (int Index = 0; Index < INITIAL_ASTEROID_COUNT; Index++)
	{
		int x, y;
		do {
			x = rand() % SCREEN_WIDTH;
			y = rand() % SCREEN_HEIGHT;
		} while (sqrt((SCREEN_WIDTH / 2 - x)*(SCREEN_WIDTH / 2 - x) + (SCREEN_HEIGHT / 2 - y)*(SCREEN_HEIGHT / 2 - y)) <=
			SAFEZONE_RADIUS + ASTEROID_RADIUS);

	int AdditionalVerts = rand() % ((ASTEROID_MAX_VERT_COUNT - ASTEROID_MINIMUM_VERT_COUNT > 0)
									? ASTEROID_MAX_VERT_COUNT - ASTEROID_MINIMUM_VERT_COUNT
									: 1);
		GameState->AsteroidVertCounts[Index] = ASTEROID_MINIMUM_VERT_COUNT + AdditionalVerts;
		simulation_asteroid newSimAsteroid = CreateSimAsteroid(x, y, (rand() % 200 - 100) / 100.f, (rand() % 200 - 100) / 100.f);
		shape_asteroid newAsteroidShape = CreateAsteroidShape(GameState->AsteroidVertCounts[Index]);
		
		AddAsteroid(GameState, newSimAsteroid, newAsteroidShape, GameState->AsteroidVertCounts[Index]);
	}
}

internal void
UpdateAndRender(game_memory *Memory, platform_state *Platform)
{
	game_state *GameState = (game_state*)Memory->BaseAddress;
	if(!Memory->IsGameStateInitialized){
		InitGameState(GameState);
		NewGame(GameState);
	}

	SDL_SetRenderDrawColor(Platform->Renderer, 20, 20, 20, 255);
	SDL_RenderClear(Platform->Renderer);
	SDL_SetRenderDrawColor(Platform->Renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(Platform->Renderer, &Platform->screen_outline);

	{
		SimulateSpaceShip(&GameState->Player);
		if (GameState->UseRappidFire)
		{
			AddProjectile(GameState, CreateProjectile(GameState->Player.P, GameState->Player.Dir));
		}
		UpdateAndRenderProjectiles(GameState, Platform->Renderer, GameState->Projectiles);
		CollideAsteroidsWithProjectiles(GameState, GameState->SimAsteroids, GameState->Projectiles);
		MoveAsteroids(GameState, GameState->SimAsteroids, &GameState->Player);
		if( (GameState->Player.Lives <= 0) || (GameState->AsteroidCount == 0 && (GameState->AsteroidCount != INITIAL_ASTEROID_COUNT)) )
		{
			NewGame(GameState);
			return;
		}
		RotateAndTranslateAsteroidsToScreen(GameState, GameState->LocalAsteroids, GameState->SimAsteroids, GameState->AsteroidVertCounts, GameState->ScreenAsteroids);
		DrawAsteroids(Platform->Renderer, GameState->ScreenAsteroids, GameState->AsteroidVertCounts, GameState->AsteroidCount);
	}

	DrawSpaceShip(Platform->Renderer, &GameState->Player);
	SDL_RenderPresent(Platform->Renderer);
}

#endif //FUNCTIONS_H
