#pragma once
#include <ostream>

namespace Sapfire
{
	enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	};

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

// From glfw3.h
#define KEY_SPACE           ::Sapfire::Key::Space
#define KEY_APOSTROPHE      ::Sapfire::Key::Apostrophe    /* ' */
#define KEY_COMMA           ::Sapfire::Key::Comma         /* , */
#define KEY_MINUS           ::Sapfire::Key::Minus         /* - */
#define KEY_PERIOD          ::Sapfire::Key::Period        /* . */
#define KEY_SLASH           ::Sapfire::Key::Slash         /* / */
#define KEY_0               ::Sapfire::Key::D0
#define KEY_1               ::Sapfire::Key::D1
#define KEY_2               ::Sapfire::Key::D2
#define KEY_3               ::Sapfire::Key::D3
#define KEY_4               ::Sapfire::Key::D4
#define KEY_5               ::Sapfire::Key::D5
#define KEY_6               ::Sapfire::Key::D6
#define KEY_7               ::Sapfire::Key::D7
#define KEY_8               ::Sapfire::Key::D8
#define KEY_9               ::Sapfire::Key::D9
#define KEY_SEMICOLON       ::Sapfire::Key::Semicolon     /* ; */
#define KEY_EQUAL           ::Sapfire::Key::Equal         /* = */
#define KEY_A               ::Sapfire::Key::A
#define KEY_B               ::Sapfire::Key::B
#define KEY_C               ::Sapfire::Key::C
#define KEY_D               ::Sapfire::Key::D
#define KEY_E               ::Sapfire::Key::E
#define KEY_F               ::Sapfire::Key::F
#define KEY_G               ::Sapfire::Key::G
#define KEY_H               ::Sapfire::Key::H
#define KEY_I               ::Sapfire::Key::I
#define KEY_J               ::Sapfire::Key::J
#define KEY_K               ::Sapfire::Key::K
#define KEY_L               ::Sapfire::Key::L
#define KEY_M               ::Sapfire::Key::M
#define KEY_N               ::Sapfire::Key::N
#define KEY_O               ::Sapfire::Key::O
#define KEY_P               ::Sapfire::Key::P
#define KEY_Q               ::Sapfire::Key::Q
#define KEY_R               ::Sapfire::Key::R
#define KEY_S               ::Sapfire::Key::S
#define KEY_T               ::Sapfire::Key::T
#define KEY_U               ::Sapfire::Key::U
#define KEY_V               ::Sapfire::Key::V
#define KEY_W               ::Sapfire::Key::W
#define KEY_X               ::Sapfire::Key::X
#define KEY_Y               ::Sapfire::Key::Y
#define KEY_Z               ::Sapfire::Key::Z
#define KEY_LEFT_BRACKET    ::Sapfire::Key::LeftBracket   /* [ */
#define KEY_BACKSLASH       ::Sapfire::Key::Backslash     /* \ */
#define KEY_RIGHT_BRACKET   ::Sapfire::Key::RightBracket  /* ] */
#define KEY_GRAVE_ACCENT    ::Sapfire::Key::GraveAccent   /* ` */
#define KEY_WORLD_1         ::Sapfire::Key::World1        /* non-US #1 */
#define KEY_WORLD_2         ::Sapfire::Key::World2        /* non-US #2 */

/* Function keys */
#define KEY_ESCAPE          ::Sapfire::Key::Escape
#define KEY_ENTER           ::Sapfire::Key::Enter
#define KEY_TAB             ::Sapfire::Key::Tab
#define KEY_BACKSPACE       ::Sapfire::Key::Backspace
#define KEY_INSERT          ::Sapfire::Key::Insert
#define KEY_DELETE          ::Sapfire::Key::Delete
#define KEY_RIGHT           ::Sapfire::Key::Right
#define KEY_LEFT            ::Sapfire::Key::Left
#define KEY_DOWN            ::Sapfire::Key::Down
#define KEY_UP              ::Sapfire::Key::Up
#define KEY_PAGE_UP         ::Sapfire::Key::PageUp
#define KEY_PAGE_DOWN       ::Sapfire::Key::PageDown
#define KEY_HOME            ::Sapfire::Key::Home
#define KEY_END             ::Sapfire::Key::End
#define KEY_CAPS_LOCK       ::Sapfire::Key::CapsLock
#define KEY_SCROLL_LOCK     ::Sapfire::Key::ScrollLock
#define KEY_NUM_LOCK        ::Sapfire::Key::NumLock
#define KEY_PRINT_SCREEN    ::Sapfire::Key::PrintScreen
#define KEY_PAUSE           ::Sapfire::Key::Pause
#define KEY_F1              ::Sapfire::Key::F1
#define KEY_F2              ::Sapfire::Key::F2
#define KEY_F3              ::Sapfire::Key::F3
#define KEY_F4              ::Sapfire::Key::F4
#define KEY_F5              ::Sapfire::Key::F5
#define KEY_F6              ::Sapfire::Key::F6
#define KEY_F7              ::Sapfire::Key::F7
#define KEY_F8              ::Sapfire::Key::F8
#define KEY_F9              ::Sapfire::Key::F9
#define KEY_F10             ::Sapfire::Key::F10
#define KEY_F11             ::Sapfire::Key::F11
#define KEY_F12             ::Sapfire::Key::F12
#define KEY_F13             ::Sapfire::Key::F13
#define KEY_F14             ::Sapfire::Key::F14
#define KEY_F15             ::Sapfire::Key::F15
#define KEY_F16             ::Sapfire::Key::F16
#define KEY_F17             ::Sapfire::Key::F17
#define KEY_F18             ::Sapfire::Key::F18
#define KEY_F19             ::Sapfire::Key::F19
#define KEY_F20             ::Sapfire::Key::F20
#define KEY_F21             ::Sapfire::Key::F21
#define KEY_F22             ::Sapfire::Key::F22
#define KEY_F23             ::Sapfire::Key::F23
#define KEY_F24             ::Sapfire::Key::F24
#define KEY_F25             ::Sapfire::Key::F25

/* Keypad */
#define KEY_KP_0            ::Sapfire::Key::KP0
#define KEY_KP_1            ::Sapfire::Key::KP1
#define KEY_KP_2            ::Sapfire::Key::KP2
#define KEY_KP_3            ::Sapfire::Key::KP3
#define KEY_KP_4            ::Sapfire::Key::KP4
#define KEY_KP_5            ::Sapfire::Key::KP5
#define KEY_KP_6            ::Sapfire::Key::KP6
#define KEY_KP_7            ::Sapfire::Key::KP7
#define KEY_KP_8            ::Sapfire::Key::KP8
#define KEY_KP_9            ::Sapfire::Key::KP9
#define KEY_KP_DECIMAL      ::Sapfire::Key::KPDecimal
#define KEY_KP_DIVIDE       ::Sapfire::Key::KPDivide
#define KEY_KP_MULTIPLY     ::Sapfire::Key::KPMultiply
#define KEY_KP_SUBTRACT     ::Sapfire::Key::KPSubtract
#define KEY_KP_ADD          ::Sapfire::Key::KPAdd
#define KEY_KP_ENTER        ::Sapfire::Key::KPEnter
#define KEY_KP_EQUAL        ::Sapfire::Key::KPEqual

#define KEY_LEFT_SHIFT      ::Sapfire::Key::LeftShift
#define KEY_LEFT_CONTROL    ::Sapfire::Key::LeftControl
#define KEY_LEFT_ALT        ::Sapfire::Key::LeftAlt
#define KEY_LEFT_SUPER      ::Sapfire::Key::LeftSuper
#define KEY_RIGHT_SHIFT     ::Sapfire::Key::RightShift
#define KEY_RIGHT_CONTROL   ::Sapfire::Key::RightControl
#define KEY_RIGHT_ALT       ::Sapfire::Key::RightAlt
#define KEY_RIGHT_SUPER     ::Sapfire::Key::RightSuper
#define KEY_MENU            ::Sapfire::Key::Menu