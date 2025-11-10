#pragma once
#include <cstddef>

namespace BaseAdder {
	constexpr std::ptrdiff_t EntityLiseBase = 0x18AC04;
	constexpr std::ptrdiff_t LocalPlayerPawn = 0x18AC00;
	constexpr std::ptrdiff_t ViewMatrix = 0x57DFD0;
	constexpr std::ptrdiff_t i_MaxPlayer = 0x191FD4;
}
namespace offset {
	namespace PlayerBase {
		constexpr std::ptrdiff_t C_Name = 0x205;
		constexpr std::ptrdiff_t i_Health = 0xEC;
		constexpr std::ptrdiff_t i_Team = 0x30C;
		constexpr std::ptrdiff_t i_Id = 0x1C4;
		constexpr std::ptrdiff_t f_CollisionBox = 0x50;
		constexpr std::ptrdiff_t f_CollisionBoxMax = 0x54;
		constexpr std::ptrdiff_t f_PosX = 0x28;
		constexpr std::ptrdiff_t f_PosY = 0x30;
		constexpr std::ptrdiff_t f_PosZ = 0x2c;
		constexpr std::ptrdiff_t f_CameraYaw = 0x34;
		constexpr std::ptrdiff_t f_CameraPitch = 0x38;
		constexpr std::ptrdiff_t P_WeaponService = 0x364;//*Base_PlayerPawn
	}
	namespace WeaponBase {//WeaponService
		constexpr std::ptrdiff_t i_1Ammo = 0x12C;//Pistol
		constexpr std::ptrdiff_t i_2Ammo = 0x13C;//Main
		constexpr std::ptrdiff_t i_1LoadAmmo = 0x108;//Pistol
		constexpr std::ptrdiff_t i_2LoadAmmo = 0x118;//Main
		constexpr std::ptrdiff_t i_ShotCooldown = 0x18;
	}
}