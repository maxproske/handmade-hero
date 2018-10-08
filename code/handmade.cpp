#include "handmade.h"

internal void GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	local_persist real32 tSine; // Output it directly from here
	int16 ToneVolume = 3000;
	int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz; // How many samples do we need to fill to get 256Hz

	int16 *SampleOut = SoundBuffer->Samples;
	for (int SampleIndex = 0;
		SampleIndex < SoundBuffer->SampleCount;
		++SampleIndex)
	{
		real32 SineValue = sinf(tSine);
		int16 SampleValue = (int16)(SineValue * ToneVolume);
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;

		tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod; // Period of sine is 2pi 
	}
}

internal void RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
	// Cast void pointer to unsigned char (typedef uint8)
	uint8 *Row = (uint8 *)Buffer->Memory;
	for (int Y = 0; Y < Buffer->Height; ++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int X = 0; X < Buffer->Width; ++X)
		{
			// Little endian (255,0,0,0 will draw blue)
			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
	}
}

// Platform-independent update loop
internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	GameOutputSound(SoundBuffer, ToneHz); // How many samples of sound to output
	RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}