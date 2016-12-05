#if !defined(TYPES_H)
#define TYPES_H

struct projectile {
	vec2f P;
	vec2f dP;
	vec2f Dir;
};

struct simulation_asteroid {
	vec2f 		P;
	vec2f 		dP;
	int32 		Radius;
	real32 		AngularVelocity;
};

struct shape_asteroid {
	vec2f 		Verts[ASTEROID_MAX_VERT_COUNT];
};

struct screen_asteroid {
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
	shape_asteroid		*LocalAsteroids;
	screen_asteroid		*ScreenAsteroids;
	int32				*AsteroidVertCounts;
	int32 				AsteroidCount;
	int32 				AsteroidCapacity;

	projectile			*Projectiles;
	int32 				ProjectileCount;
	int32 				ProjectileCapacity;
	bool32				Started;
	bool32 				UseRappidFire{ false };
};

#endif //TYPES_H
