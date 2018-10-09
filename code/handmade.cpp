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

		tSine += 2.0f*Pi32*1.0f / (real32)WavePeriod; // Period of sine is 2pi 
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
internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, game_sound_output_buffer *SoundBuffer)
{
	local_persist int BlueOffset = 0;
	local_persist int GreenOffset = 0;
	local_persist int ToneHz = 256;

	game_controller_input *Input0 = &Input.PlayerControllers[0];
	if(Input0->IsAnalog)
	{
		// TODO(max): Use analog movement tuning
		ToneHz = 256 + (int)(128.0f*(Input0->EndX));
		BlueOffset += (int)4.0f*(Input0->EndY);
	}
	else
	{
		// TODO(max): Use digital movement tuning
	}

	// These two values add up to what the button was at the start of the frame
	//Input.AButtonEndedDown;
	//Input.AButtonHalfTransitionCount;
	if (Input0->AButtonEndedDown)
	{
		GreenOffset += 1;
	}

	GameOutputSound(SoundBuffer, ToneHz); // How many samples of sound to output
	RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}