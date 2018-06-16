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
	//   powerpc, xbox 360, photoshop
	// Little endian mode (if low order byte comes first) 
	//   x86, arm (android/ios), x64
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
}