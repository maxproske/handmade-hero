#pragma once

#include <math.h>
#include <stdint.h>

#define local_persist static // Can't be used outside this translation unit (source file)
#define global_variable static 
#define internal static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

// TODO(max): Services that the platform layer provides to the game

// NOTE(max): Services that the game provides to the playform layer

// GameUpdateAndRender takes in timing, input, bitmap buffer to use, sound buffer to use, and returns bitmap and sound.
// Non-platform depedent win32_offscreen_buffer
struct game_offscreen_buffer
{
	void *Memory;
	int Width;
	int Height;
	int Pitch;
};

struct game_sound_output_buffer
{
	int SamplesPerSecond;
	int SampleCount;
	int16 *Samples;
};

struct game_button_state 
{
	int HalfTransitionCount;
	bool32 EndedDown;
};

struct game_controller_input
{
	union {
		game_button_state Buttons[6];
		struct {
			game_button_state Up;
			game_button_state Down;
			game_button_state Left;
			game_button_state Right;
			game_button_state LeftShoulder;
			game_button_state RightShoulder;
		};
	};
};

struct game_input
{
	game_controller_input Controllers[4];
};

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer);