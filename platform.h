#if !defined(PLATFORM_H)
#define PLATFORM_H

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32 bool32;

typedef float real32;
typedef double real64;


#define Kilobytes(Count) ((Count)*1024ll)
#define Megabytes(Count) (Kilobytes(Count)*1024ll)
#define Gigabytes(Count) (Megabytes(Count)*1024ll)

struct game_memory{
	void 	*BaseAddress;
	uint64	Size;
	bool32 	IsGameStateInitialized;
};

struct platform_state{
	SDL_Event 		Event;
	Timer 			FPS;
	SDL_Window*		Window;
	SDL_Renderer*	Renderer;
	SDL_Rect 		screen_outline{ 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	bool32 	GlobalRunning;
};

#endif //PLATFORM_H
