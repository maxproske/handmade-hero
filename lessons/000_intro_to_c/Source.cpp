#include <windows.h>

struct projectile {
	char unsigned IsThisOnFire; // 1 byte
	int Damage; // + 4 bytes
	int ParticlesPerSecond; // + 4 bytes
	short HowManyCooks; // + 2 bytes
	// = 11 bytes reserved for our projectile (actually 16)
	// There are ways to turn off this padding, even though there are performance loss
	// Pragmas, attributes
};

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	// 	unsigned short Test; // 16 bits/2 bytes
	// Test = 500;

	// Why is 500 represented as 244 1 in code bytes?
	// ----------------------------------------------
	//       52 1
	//       15 2631 
	//       26 84268421
	// 00000001 11110100
	// 
	// high order byte = 1
	// low order byte = 244
	//
	// Why does low order byte come first?
	// ----------------------------------------------
	// Endianness, an obscure reference from Guliver's Travels
	// Arbitrary choice of the x86 designer
	//
	// Big endian mode (if high order byte comes first)
	//   powerpc, xbox 360, photoshop, ps3. economics trumped intel (x86)
	// Little endian mode (if low order byte comes first) 
	//   x86, arm (android/ios can choose endianess), x64
	//
	// File formats may need to be switched around to be operable on the CPU

	projectile Test;

	int SizeOfChar = sizeof(char unsigned); // 1 (bytes)
	int SizeOfInt = sizeof(int); // 4

	// These should match, but the compiler thinks
	// it is more efficient for the cpu to reserve 32 bits for each
	// If cpu is slower at accessing 8 bit boundaries
	// 
	int SizeOfProjectile = sizeof(projectile); // 16 bytes, not 11
	int Size = sizeof(Test); // 16

	Test.IsThisOnFire = 1; // 1 204 204 204 (CC added as padding)
	Test.Damage = 2; // 2 0 0 0
	Test.ParticlesPerSecond = 3; // 3 0 0 0
	Test.HowManyCooks = 4; // 4 0 204 204

	// How do I read hexadecimal?
	// ----------------------------------------------
	// It lines up with binary. Decimal does not.
	// 0xA = 10
	// 0xAA = 16*10 + 10
	// 0xAAA = 16*16*10 + 16*10 + 10

	// Look at test struct as if it was a short
	// Static typed language = C keeps track of what you said something was
	// Good because compiler catches a lot of mistakes!
	//projectile *ProjectilePointer = &Test; // OK
	unsigned short *MrPointerMan = (unsigned short *)&Test; // Will grab 1 and 204 padding!

	// C secretly understands we can treat arrays as pointers
	projectile Projectiles[40]; // 640 bytes (4 bytes * 4 * 40)
	projectile *ProjectilePointer = Projectiles; // A pointer, 4 bytes
	
	Projectiles[30].Damage = 60;
	// Is the same as this catastrophic line
	((projectile *)((char *)ProjectilePointer + 30 * sizeof(projectile)))->Damage = 100;

	/*
	Wonkiness with C

	// Actually on the stack
	projectile Test;
	Test.Damage;

	// Declared to be a pointer
	projectile *Test;
	Test->Damage
	*/

	// Code is encoded in .exe, get paged into memory (pages) as they need to be executed
	// Fixup tables replace syscalls (OutputdebugString) to pointers in Windows code
	// Between intel encoded memory and CPU is iCache. It preps the code for you to store CODE
	// Stack allocates pages and free thems as needed. It can never free out of the middle, because functions nest.
	// Heap can be freed out of the middle.

	// Bitwise operators
	int X = 5;
	int Y = 0;
	X = 0xA; // 0x0000000a
	X = X << 4; // 0x000000a0 (shift a 1 place to the left)
	X = X << 4;
	X = X << 4;

	// What does this actually do?
	// ---------------------------
	//     8421
	// 00000001

	// << 1 00000010 // multiply/divide by 2
	// << 2 00000100 // multiply/divide by 4
	// << 3 00001000 // multiply/divide by 8
	// << 4 00010000 // multiply/divide by 16 (0x0a -> 0xa0)

	// Fill it with bits
	X = 0;
	X = X | (1 << 4);
	X = X | (1 << 3);
	X = X | (1 << 30);
	X = X | (1 << 1); // 0x4000001a

	// Create a mask
	Y = ((1 << 4) | (1 << 31)); // 0x80000010

	// Show bits in common (4th bit)
	X = X & Y; // 0x00000010

	// Not
	X = 0;
	X = ~X; // 0xffffffff

	// Xor
	X = (1 << 4) | (1 << 8);
	Y = (1 << 8) | (1 << 16);
	X = X ^ Y; // excludes and, so (1 << 8) gets cleared out
	X = X ^ Y; // get the original back!

	// Note(max): With < > == !=, the compiler is not onligated to give a 1 for true!
	// || is a logical Or used in predication (if/else)
	// Logical XOR ^^ is the same as !=

	X = 0;
	Y = 0;
	for (X = 0xa; X != 0; X = X << 4) {
		OutputDebugStringA("We are in the loop\n");
	}

	// Switch for comparing to a bunch of constants
	// Cases don't start executing after the jump!
	Y = 0;
	if (Y == 0) {
		int Y = 4; // Shadow the Y (different place in memory)
	}

	// VirtualLock can help you prevent pages from being swapped to disk
	// sdl = C++ standard library
	// fopen = C standard library

	// C++ Classes are glorified structs. RAII = Resource acquisition is initialization
	// It's bad, because trying to catch errors on everything makes a program brittle
	// If file A fails to open, maybe we want to do something different and smarter when file B fails (wait a few seconds)
	// Get the code to WORK, don't worry about trying to close the file handle. Wrong problem definition.

	// Opinion: Full mixin > Polymorphism/interitence is useless
	// We will use C++ function overloading

	Y = 0;
	int Z = 0;
	// Short circuit, compiler will not reach z==5
	int Z = (Y == 5) || (Z == 5);

	// Stack overflow = run out of guard pages on stack to grow the pages
	// Strength reduction (2 * 2 -> 2 << 1)
	
	// Class vs struct?
	// No differnce in C++, except struct is public: and class has private:

	struct foo {
		char bar;
	private:
	};

	class foo {
	public:
		char bar;
	};
}