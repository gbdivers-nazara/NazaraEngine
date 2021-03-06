// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Mathematics module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/Error.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Math/Config.hpp>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <Nazara/Core/Debug.hpp>

namespace Nz
{
	namespace Detail
	{
		namespace
		{
			// https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
			static const unsigned int MultiplyDeBruijnBitPosition[32] =
			{
				0,  9,  1, 10, 13, 21,  2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
				8, 12, 20, 28, 15, 17, 24,  7, 19, 27, 23,  6, 26,  5, 4, 31
			};

			static const unsigned int MultiplyDeBruijnBitPosition2[32] =
			{
				 0,  1, 28,  2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17,  4, 8,
				31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18,  6, 11,  5, 10, 9
			};
		}

		template<typename T>
		typename std::enable_if<sizeof(T) <= sizeof(UInt32), unsigned int>::type IntegralLog2(T number)
		{
			// https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
			number |= number >> 1; // first round down to one less than a power of 2
			number |= number >> 2;
			number |= number >> 4;
			number |= number >> 8;
			number |= number >> 16;

			return MultiplyDeBruijnBitPosition[static_cast<UInt32>(number * 0x07C4ACDDU) >> 27];
		}

		template<typename T>
		// Les parenthèses autour de la condition sont nécesaires pour que GCC compile ça
		typename std::enable_if<(sizeof(T) > sizeof(UInt32)), unsigned int>::type IntegralLog2(T number)
		{
			static_assert(sizeof(T) % sizeof(UInt32) == 0, "Assertion failed");

			// L'algorithme pour le logarithme base 2 (au dessus) ne fonctionne qu'avec des nombres au plus 32bits
			// ce code décompose les nombres plus grands en nombres 32 bits par masquage et bit shifting
			for (int i = sizeof(T)-sizeof(UInt32); i >= 0; i -= sizeof(UInt32))
			{
				// Le masque 32 bits sur la partie du nombre qu'on traite actuellement
				T mask = T(std::numeric_limits<UInt32>::max()) << i*8;
				T val = (number & mask) >> i*8; // Masquage et shifting des bits vers la droite (pour le ramener sur 32bits)

				// Appel de la fonction avec le nombre 32bits, si le résultat est non-nul nous avons la réponse
				unsigned int log2 = IntegralLog2<UInt32>(val);
				if (log2)
					return log2 + i*8;
			}

			return 0;
		}

		template<typename T>
		typename std::enable_if<sizeof(T) <= sizeof(UInt32), unsigned int>::type IntegralLog2Pot(T number)
		{
			// https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
			return MultiplyDeBruijnBitPosition2[static_cast<UInt32>(number * 0x077CB531U) >> 27];
		}

		template<typename T>
		// Les parenthèses autour de la condition sont nécesaires pour que GCC compile ça
		typename std::enable_if<(sizeof(T) > sizeof(UInt32)), unsigned int>::type IntegralLog2Pot(T number)
		{
			static_assert(sizeof(T) % sizeof(UInt32) == 0, "Assertion failed");

			// L'algorithme pour le logarithme base 2 (au dessus) ne fonctionne qu'avec des nombres au plus 32bits
			// ce code décompose les nombres plus grands en nombres 32 bits par masquage et bit shifting
			for (int i = sizeof(T)-sizeof(UInt32); i >= 0; i -= sizeof(UInt32))
			{
				// Le masque 32 bits sur la partie du nombre qu'on traite actuellement
				T mask = T(std::numeric_limits<UInt32>::max()) << i*8;
				UInt32 val = UInt32((number & mask) >> i*8); // Masquage et shifting des bits vers la droite (pour le ramener sur 32bits)

				// Appel de la fonction avec le nombre 32bits, si le résultat est non-nul nous avons la réponse
				unsigned int log2 = IntegralLog2Pot<UInt32>(val);
				if (log2)
					return log2 + i*8;
			}

			return 0;
		}
	}

	/*!
	* \brief Approaches the objective, beginning with value and with increment
	* \return The nearest value of the objective you can get with the value and the increment for one step
	*
	* \param value Initial value
	* \param objective Target value
	* \parma increment One step value
	*/

	template<typename T>
	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline T Approach(T value, T objective, T increment)
	{
		if (value < objective)
			return std::min(value + increment, objective);
		else if (value > objective)
			return std::max(value - increment, objective);
		else
			return value;
	}

	/*!
	* \brief Clamps value between min and max and returns the expected value
	* \return If value is not in the interval of min..max, value obtained is the nearest limit of this interval
	*
	* \param value Value to clamp
	* \param min Minimum of the interval
	* \param max Maximum of the interval
	*/

	template<typename T>
	constexpr T Clamp(T value, T min, T max)
	{
		return std::max(std::min(value, max), min);
	}

	/*!
	* \brief Gets number of bits set in the number
	* \return The number of bits set to 1
	*
	* \param value The value to count bits
	*/

	template<typename T>
	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline T CountBits(T value)
	{
		// https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
		unsigned int count = 0;
		while (value)
		{
			value &= value - 1;
			count++;
		}

		return count;
	}

	/*!
	* \brief Converts degree to radian
	* \return The representation in radian of the angle in degree (0..2*pi)
	*
	* \param degrees Angle in degree (this is expected between 0..360)
	*/

	template<typename T>
	constexpr T DegreeToRadian(T degrees)
	{
		return degrees * T(M_PI/180.0);
	}

	/*!
	* \brief Gets the unit from degree and convert it according to NAZARA_MATH_ANGLE_RADIAN
	* \return Express the degrees
	*
	* \param degrees Convert degree to NAZARA_MATH_ANGLE_RADIAN unit
	*/

	template<typename T>
	constexpr T FromDegrees(T degrees)
	{
		#if NAZARA_MATH_ANGLE_RADIAN
		return DegreeToRadian(degrees);
		#else
		return degrees;
		#endif
	}

	/*!
	* \brief Gets the unit from radian and convert it according to NAZARA_MATH_ANGLE_RADIAN
	* \return Express the radians
	*
	* \param radians Convert radian to NAZARA_MATH_ANGLE_RADIAN unit
	*/

	template<typename T>
	constexpr T FromRadians(T radians)
	{
		#if NAZARA_MATH_ANGLE_RADIAN
		return radians;
		#else
		return RadianToDegree(radians);
		#endif
	}

	/*!
	* \brief Gets the nearest power of two for the number
	* \return First power of two containing the number
	*
	* \param number Number to get nearest power
	*/

	template<typename T>
	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline T GetNearestPowerOfTwo(T number)
	{
		T x = 1;
		while (x < number)
			x <<= 1; // We multiply by 2

		return x;
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits
	*
	* \param number Number to get number of digits
	*/

	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline unsigned int GetNumberLength(signed char number)
	{
		// Char is expected to be 1 byte
		static_assert(sizeof(number) == 1, "Signed char must be one byte-sized");

		if (number >= 100)
			return 3;
		else if (number >= 10)
			return 2;
		else if (number >= 0)
			return 1;
		else if (number > -10)
			return 2;
		else if (number > -100)
			return 3;
		else
			return 4;
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits
	*
	* \param number Number to get number of digits
	*/

	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline unsigned int GetNumberLength(unsigned char number)
	{
		// Char is expected to be 1 byte
		static_assert(sizeof(number) == 1, "Unsigned char must be one byte-sized");

		if (number >= 100)
			return 3;
		else if (number >= 10)
			return 2;
		else
			return 1;
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits
	*
	* \param number Number to get number of digits
	*/

	inline unsigned int GetNumberLength(int number)
	{
		if (number == 0)
			return 1;

		return static_cast<unsigned int>(std::log10(std::abs(number))) + (number < 0 ? 2 : 1);
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits
	*
	* \param number Number to get number of digits
	*/

	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline unsigned int GetNumberLength(unsigned int number)
	{
		if (number == 0)
			return 1;

		return static_cast<unsigned int>(std::log10(number))+1;
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits
	*
	* \param number Number to get number of digits
	*/

	inline unsigned int GetNumberLength(long long number)
	{
		if (number == 0)
			return 1;

		return static_cast<unsigned int>(std::log10(std::abs(number))) + (number < 0 ? 2 : 1);
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits
	*
	* \param number Number to get number of digits
	*/

	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline unsigned int GetNumberLength(unsigned long long number)
	{
		if (number == 0)
			return 1;

		return static_cast<unsigned int>(std::log10(number)) + 1;
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits + 1 for the dot
	*
	* \param number Number to get number of digits
	* \param precision Number of digit after the dot
	*/

	inline unsigned int GetNumberLength(float number, UInt8 precision)
	{
		// The imprecision of floats need a cast (log10(9.99999) = 0.99999)
		return GetNumberLength(static_cast<long long>(number)) + precision + 1; // Plus one for the dot
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits + 1 for the dot
	*
	* \param number Number to get number of digits
	* \param precision Number of digit after the dot
	*/

	inline unsigned int GetNumberLength(double number, UInt8 precision)
	{
		// The imprecision of floats need a cast (log10(9.99999) = 0.99999)
		return GetNumberLength(static_cast<long long>(number)) + precision + 1; // Plus one for the dot
	}

	/*!
	* \brief Gets the number of digits to represent the number in base 10
	* \return Number of digits + 1 for the dot
	*
	* \param number Number to get number of digits
	* \param precision Number of digit after the dot
	*/

	inline unsigned int GetNumberLength(long double number, UInt8 precision)
	{
		// The imprecision of floats need a cast (log10(9.99999) = 0.99999)
		return GetNumberLength(static_cast<long long>(number)) + precision + 1; // Plus one for the dot
	}

	/*!
	* \brief Gets the log in base 2 of integral number
	* \return Log of the number (floor)
	*
	* \param number To get log in base 2
	*
	* \remark If number is 0, 0 is returned
	*/

	template<typename T>
	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline unsigned int IntegralLog2(T number)
	{
		// Proxy needed to avoid an overload problem
		return Detail::IntegralLog2<T>(number);
	}

	/*!
	* \brief Gets the log in base 2 of integral number, only works for power of two !
	* \return Log of the number
	*
	* \param number To get log in base 2
	*
	* \remark Only works for power of two
	* \remark If number is 0, 0 is returned
	*/

	template<typename T>
	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline unsigned int IntegralLog2Pot(T pot)
	{
		return Detail::IntegralLog2Pot<T>(pot);
	}

	/*!
	* \brief Gets the power of integrals
	* \return base^exponent for integral
	*
	* \param base Base of the exponentation
	* \parma exponent Power for the base
	*/

	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline unsigned int IntegralPow(unsigned int base, unsigned int exponent)
	{
		unsigned int r = 1;
		for (unsigned int i = 0; i < exponent; ++i)
			r *= base;

		return r;
	}

	/*!
	* \brief Interpolates the value to other one with a factor of interpolation
	* \return A new value which is the interpolation of two values
	*
	* \param from Initial value
	* \param to Target value
	* \param interpolation Factor of interpolation
	*
	* \remark interpolation is meant to be between 0 and 1, other values are potentially undefined behavior
	* \remark With NAZARA_DEBUG, a NazaraWarning is produced
	*
	* \see Lerp
	*/

	template<typename T, typename T2>
	constexpr T Lerp(const T& from, const T& to, const T2& interpolation)
	{
		return from + interpolation * (to - from);
	}

	/*!
	* \brief Multiplies X and Y, then add Z
	* \return The result of X * Y + Z
	*
	* \param x is X
	* \param y is Y
	* \param z is Z
	*
	* \remark This function is meant to use a special faster instruction in CPU if possible
	*/

	template<typename T>
	constexpr T MultiplyAdd(T x, T y, T z)
	{
		return x * y + z;
	}

	#ifdef FP_FAST_FMAF
	template<>
	constexpr float MultiplyAdd(float x, float y, float z)
	{
		return std::fmaf(x, y, z);
	}
	#endif

	#ifdef FP_FAST_FMA
	template<>
	constexpr double MultiplyAdd(double x, double y, double z)
	{
		return std::fma(x, y, z);
	}
	#endif

	#ifdef FP_FAST_FMAL
	template<>
	constexpr long double MultiplyAdd(long double x, long double y, long double z)
	{
		return std::fmal(x, y, z);
	}
	#endif

	/*!
	* \brief Normalizes the angle
	* \return Normalized value between 0..2*(pi if radian or 180 if degrees)
	*
	* \param angle Angle to normalize
	*/

	template<typename T>
	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline T NormalizeAngle(T angle)
	{
		#if NAZARA_MATH_ANGLE_RADIAN
		const T limit = T(M_PI);
		#else
		const T limit = T(180.0);
		#endif
		const T twoLimit = limit * T(2);

		angle = std::fmod(angle + limit, twoLimit);
		if (angle < T(0))
			angle += twoLimit;

		return angle - limit;
	}

	/*!
	* \brief Checks whether two numbers are equal
	* \return true if they are equal within a certain epsilon
	*
	* \param a First value
	* \param b Second value
	*/

	template<typename T>
	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline bool NumberEquals(T a, T b)
	{
		return NumberEquals(a, b, std::numeric_limits<T>::epsilon());
	}

	/*!
	* \brief Checks whether two numbers are equal
	* \return true if they are equal within the max difference
	*
	* \param a First value
	* \param b Second value
	* \param maxDifference Epsilon of comparison (expected to be positive)
	*/

	template<typename T>
	//TODO: Mark as constexpr when supported by all major compilers
	/*constexpr*/ inline bool NumberEquals(T a, T b, T maxDifference)
	{
		if (b > a)
			std::swap(a, b);

		T diff = a - b;
		return diff <= maxDifference;
	}

	/*!
	* \brief Converts the number to String
	* \return String representation of the number
	*
	* \param number Number to represent
	* \param radix Base of the number
	*
	* \remark radix is meant to be between 2 and 36, other values are potentially undefined behavior
	* \remark With NAZARA_MATH_SAFE, a NazaraError is produced and String() is returned
	*/

	inline String NumberToString(long long number, UInt8 radix)
	{
		#if NAZARA_MATH_SAFE
		if (radix < 2 || radix > 36)
		{
			NazaraError("Base must be between 2 and 36");
			return String();
		}
		#endif

		if (number == 0)
			return String('0');

		static const char* symbols = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

		bool negative;
		if (number < 0)
		{
			negative = true;
			number = -number;
		}
		else
			negative = false;

		String str;
		str.Reserve(GetNumberLength(number)); // Prends en compte le signe négatif

		do
		{
			str.Append(symbols[number % radix]);
			number /= radix;
		}
		while (number > 0);

		if (negative)
			str.Append('-');

		return str.Reverse();
	}

	/*!
	* \brief Converts radian to degree
	* \return The representation in degree of the angle in radian (0..360)
	*
	* \param radians Angle in radian (this is expected between 0..2*pi)
	*/

	template<typename T>
	constexpr T RadianToDegree(T radians)
	{
		return radians * T(180.0/M_PI);
	}

	/*!
	* \brief Converts the string to number
	* \return Number which is represented by the string
	*
	* \param str String representation
	* \param radix Base of the number
	* \param ok Optional argument to know if convertion is correct
	*
	* \remark radix is meant to be between 2 and 36, other values are potentially undefined behavior
	* \remark With NAZARA_MATH_SAFE, a NazaraError is produced and 0 is returned
	*/

	inline long long StringToNumber(String str, UInt8 radix, bool* ok)
	{
		#if NAZARA_MATH_SAFE
		if (radix < 2 || radix > 36)
		{
			NazaraError("Radix must be between 2 and 36");

			if (ok)
				*ok = false;

			return 0;
		}
		#endif

		static const char* symbols = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

		str.Simplify();
		if (radix > 10)
			str = str.ToUpper();

		bool negative = str.StartsWith('-');

		char* digit = &str[(negative) ? 1 : 0];
		unsigned long long total = 0;
		do
		{
			if (*digit == ' ')
				continue;

			total *= radix;
			const char* c = std::strchr(symbols, *digit);
			if (c && c-symbols < radix)
				total += c-symbols;
			else
			{
				if (ok)
					*ok = false;

				return 0;
			}
		}
		while (*++digit);

		if (ok)
			*ok = true;

		return (negative) ? -static_cast<long long>(total) : total;
	}

	/*!
	* \brief Gets the degree from unit and convert it according to NAZARA_MATH_ANGLE_RADIAN
	* \return Express in degrees
	*
	* \param angle Convert degree from NAZARA_MATH_ANGLE_RADIAN unit to degrees
	*/

	template<typename T>
	constexpr T ToDegrees(T angle)
	{
		#if NAZARA_MATH_ANGLE_RADIAN
		return RadianToDegree(angle);
		#else
		return angle;
		#endif
	}

	/*!
	* \brief Gets the radian from unit and convert it according to NAZARA_MATH_ANGLE_RADIAN
	* \return Express in radians
	*
	* \param angle Convert degree from NAZARA_MATH_ANGLE_RADIAN unit to radians
	*/

	template<typename T>
	constexpr T ToRadians(T angle)
	{
		#if NAZARA_MATH_ANGLE_RADIAN
		return angle;
		#else
		return DegreeToRadian(angle);
		#endif
	}
}

#include <Nazara/Core/DebugOff.hpp>
