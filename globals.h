#ifndef ATTRIBUTES_INCLUDES_H_INCLUDED
#define ATTRIBUTES_INCLUDES_H_INCLUDED

//Math
static const float PI = 3.14159258979f;

//display
static const int SCREEN_WIDTH{ 1080 };
static const int SCREEN_HEIGHT{ 720 };
static const int SCREEN_DELAY{ 10 };
//How long the frame takes in milliseconds
static const int FRAME_DURATION{ 16 };

//Player
static const unsigned int  PLAYER_MAX_LIVES{ 3 };
static const int   PLAYER_RADIUS{ 0 };
static const float PLAYER_MINIMUM_VELOCITY{ 0.01f };
static const float PLAYER_ACCELERATION{ 0.1f };
static const float PLAYER_DECELERATION{ 0.0225f };
static const int   SAFEZONE_RADIUS{ 200 };

//Projectiles
static const int   PROJECTILE_MAX_COUNT{ 200 };
static const float PROJECTILE_SPEED{ 10 };
static const int   PROJECTILE_LENGTH{ 15 };
static const int   PROJECTILE_OFFSET{ 20 };//The distance from which the projectile begins relative to the players center

//Asteroids
static const int   	INITIAL_ASTEROID_COUNT{ 10 };
static const int	ASTEROID_MAX_COUNT{ 20000 };
static const int   	ASTEROID_RADIUS{ 100 };
static const int   	ASTEROID_MIN_RADIUS{ 20 };
static const int   	ASTEROID_MINIMUM_VERT_COUNT{ 4 };
static const int 	ASTEROID_MAX_VERT_COUNT{ 8 };
static const double ASTEROID_BREAK_ANGLE{ 20.0 };
static const double ASTEROID_BREAK_SPEED_MULTIPLYER{ 1.1 };

#endif // ATTRIBUTES_INCLUDES_H_INCLUDED
