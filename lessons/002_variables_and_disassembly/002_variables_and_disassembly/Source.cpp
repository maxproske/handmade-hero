#include <windows.h>

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	char SmallS; // 8 bits - 256 different values [-128,127]
	char unsigned SmallU; // 8 bits unsigned - 256 values [0,255]

	short MediumS; // 16 bits - 65536 values
	short unsigned MediumU;

	int LargeS; // 32 bits - 4 billion values
	int unsigned LargeU;

	// Press Alt+G to open Disassembly

	char unsigned Test;
	Test = 255; // 000000FF

	// 00AB171E  mov         byte ptr [Test],0FFh 

	Test = Test + 1; // 00000100 and al gets the two least significant bits (woah!)

	// 00AB1722  movzx       eax,byte ptr [Test]  
	// 00AB1726  add         eax, 1
	// 00AB1729  mov         byte ptr[Test], al

	// Bonus: A saturating adder (not in C) would stay at its maximum value
}