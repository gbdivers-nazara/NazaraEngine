// Copyright (C) 2015 Jérôme Leclercq
// This file is part of the "Nazara Engine - Core module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Core/StringStream.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <Nazara/Core/Debug.hpp>

namespace Nz
{
	inline Color::Color()
	{
	}

	inline Color::Color(UInt8 red, UInt8 green, UInt8 blue, UInt8 alpha) :
	r(red),
	g(green),
	b(blue),
	a(alpha)
	{
	}

	inline Color::Color(UInt8 lightness) :
	r(lightness),
	g(lightness),
	b(lightness),
	a(255)
	{
	}

	inline Color::Color(UInt8 vec[3], UInt8 alpha) :
	r(vec[0]),
	g(vec[1]),
	b(vec[2]),
	a(alpha)
	{
	}

	inline String Color::ToString() const
	{
		StringStream ss;
		ss << "Color(" << static_cast<int>(r) << ", " << static_cast<int>(g) << ", " << static_cast<int>(b);

		if (a != 255)
			ss << ", " << static_cast<int>(a);

		ss << ')';

		return ss;
	}

	inline Color Color::operator+(const Color& color) const
	{
		///TODO: Improve this shit
		Color c;
		c.r = static_cast<UInt8>(std::min(static_cast<unsigned int>(r) + static_cast<unsigned int>(color.r), 255U));
		c.g = static_cast<UInt8>(std::min(static_cast<unsigned int>(g) + static_cast<unsigned int>(color.g), 255U));
		c.b = static_cast<UInt8>(std::min(static_cast<unsigned int>(b) + static_cast<unsigned int>(color.b), 255U));
		c.a = static_cast<UInt8>(std::min(static_cast<unsigned int>(a) + static_cast<unsigned int>(color.a), 255U));

		return c;
	}

	inline Color Color::operator*(const Color& color) const
	{
		///TODO: Improve this shit
		Color c;
		c.r = static_cast<UInt8>((static_cast<unsigned int>(r) * static_cast<unsigned int>(color.r)) / 255U);
		c.g = static_cast<UInt8>((static_cast<unsigned int>(g) * static_cast<unsigned int>(color.g)) / 255U);
		c.b = static_cast<UInt8>((static_cast<unsigned int>(b) * static_cast<unsigned int>(color.b)) / 255U);
		c.a = static_cast<UInt8>((static_cast<unsigned int>(a) * static_cast<unsigned int>(color.a)) / 255U);

		return c;
	}

	inline Color Color::operator+=(const Color& color)
	{
		return operator=(operator+(color));
	}

	inline Color Color::operator*=(const Color& color)
	{
		return operator=(operator*(color));
	}

	inline bool Color::operator==(const Color& color) const
	{
		return r == color.r && g == color.g && b == color.b && a == color.a;
	}

	inline bool Color::operator!=(const Color& color) const
	{
		return !operator==(color);
	}

	// Algorithmes venant de http://www.easyrgb.com/index.php?X=MATH

	inline Color Color::FromCMY(float cyan, float magenta, float yellow)
	{
		return Color(static_cast<UInt8>((1.f-cyan)*255.f), static_cast<UInt8>((1.f-magenta)*255.f), static_cast<UInt8>((1.f-yellow)*255.f));
	}

	inline Color Color::FromCMYK(float cyan, float magenta, float yellow, float black)
	{
		return FromCMY(cyan * (1.f - black) + black,
					   magenta * (1.f - black) + black,
					   yellow * (1.f - black) + black);
	}

	inline Color Color::FromHSL(UInt8 hue, UInt8 saturation, UInt8 lightness)
	{
		if (saturation == 0)
		{
			// RGB results from 0 to 255
			return Color(lightness * 255,
						   lightness * 255,
						   lightness * 255);
		}
		else
		{
			// Norme Windows
			float l = lightness/240.f;
			float h = hue/240.f;
			float s = saturation/240.f;

			float v2;
			if (l < 0.5f)
				v2 = l * (1.f + s);
			else
				v2 = (l + s) - (s*l);

			float v1 = 2.f * l - v2;

			return Color(static_cast<UInt8>(255.f * Hue2RGB(v1, v2, h + (1.f/3.f))),
						   static_cast<UInt8>(255.f * Hue2RGB(v1, v2, h)),
						   static_cast<UInt8>(255.f * Hue2RGB(v1, v2, h - (1.f/3.f))));
		}
	}

	inline Color Color::FromHSV(float hue, float saturation, float value)
	{
		if (NumberEquals(saturation, 0.f))
			return Color(static_cast<UInt8>(value * 255.f));
		else
		{
			float h = hue/360.f * 6.f;
			float s = saturation/360.f;

			if (NumberEquals(h, 6.f))
				h = 0; // hue must be < 1

			int i = static_cast<unsigned int>(h);
			float v1 = value * (1.f - s);
			float v2 = value * (1.f - s * (h - i));
			float v3 = value * (1.f - s * (1.f - (h - i)));

			float r, g, b;
			switch (i)
			{
				case 0:
					r = value;
					g = v3;
					b = v1;
					break;

				case 1:
					r = v2;
					g = value;
					b = v1;
					break;

				case 2:
					r = v1;
					g = value;
					b = v3;
					break;

				case 3:
					r = v1;
					g = v2;
					b = value;
					break;

				case 4:
					r = v3;
					g = v1;
					b = value;
					break;

				default:
					r = value;
					g = v1;
					b = v2;
					break;
			}

			// RGB results from 0 to 255
			return Color(static_cast<UInt8>(r*255.f), static_cast<UInt8>(g*255.f), static_cast<UInt8>(b*255.f));
		}
	}
	inline Color Color::FromXYZ(const Vector3f& vec)
	{
		return FromXYZ(vec.x, vec.y, vec.z);
	}

	inline Color Color::FromXYZ(float x, float y, float z)
	{
		x /= 100.f; // X from 0 to  95.047
		y /= 100.f; // Y from 0 to 100.000
		z /= 100.f; // Z from 0 to 108.883

		float r = x *  3.2406f + y * -1.5372f + z * -0.4986f;
		float g = x * -0.9689f + y *  1.8758f + z *  0.0415f;
		float b = x *  0.0557f + y * -0.2040f + z *  1.0570f;

		if (r > 0.0031308f)
			r = 1.055f * (std::pow(r, 1.f/2.4f)) - 0.055f;
		else
			r *= 12.92f;

		if (g > 0.0031308f)
			g = 1.055f * (std::pow(r, 1.f/2.4f)) - 0.055f;
		else
			g *= 12.92f;

		if (b > 0.0031308f)
			b = 1.055f * (std::pow(r, 1.f/2.4f)) - 0.055f;
		else
			b *= 12.92f;

		return Color(static_cast<UInt8>(r * 255.f), static_cast<UInt8>(g * 255.f), static_cast<UInt8>(b * 255.f));
	}

	inline void Color::ToCMY(const Color& color, float* cyan, float* magenta, float* yellow)
	{
		*cyan = 1.f - color.r/255.f;
		*magenta = 1.f - color.g/255.f;
		*yellow = 1.f - color.b/255.f;
	}

	inline void Color::ToCMYK(const Color& color, float* cyan, float* magenta, float* yellow, float* black)
	{
		float c, m, y;
		ToCMY(color, &c, &m, &y);

		float k = std::min({1.f, c, m, y});

		if (NumberEquals(k, 1.f))
		{
			//Black
			*cyan = 0.f;
			*magenta = 0.f;
			*yellow = 0.f;
		}
		else
		{
			*cyan = (c-k)/(1.f-k);
			*magenta = (m-k)/(1.f-k);
			*yellow = (y-k)/(1.f-k);
		}

		*black = k;
	}

	inline void Color::ToHSL(const Color& color, UInt8* hue, UInt8* saturation, UInt8* lightness)
	{
		float r = color.r / 255.f;
		float g = color.g / 255.f;
		float b = color.b / 255.f;

		float min = std::min({r, g, b}); // Min. value of RGB
		float max = std::max({r, g, b}); // Max. value of RGB

		float deltaMax = max - min; //Delta RGB value

		float l = (max + min)/2.f;

		if (NumberEquals(deltaMax, 0.f))
		{
			//This is a gray, no chroma...
			*hue = 0; //HSL results from 0 to 1
			*saturation = 0;
		}
		else
		{
			//Chromatic data...
			if (l < 0.5f)
				*saturation = static_cast<UInt8>(deltaMax/(max+min)*240.f);
			else
				*saturation = static_cast<UInt8>(deltaMax/(2.f-max-min)*240.f);

			*lightness = static_cast<UInt8>(l*240.f);

			float deltaR = ((max - r)/6.f + deltaMax/2.f)/deltaMax;
			float deltaG = ((max - g)/6.f + deltaMax/2.f)/deltaMax;
			float deltaB = ((max - b)/6.f + deltaMax/2.f)/deltaMax;

			float h;
			if (NumberEquals(r, max))
				h = deltaB - deltaG;
			else if (NumberEquals(g, max))
				h = (1.f/3.f) + deltaR - deltaB;
			else
				h = (2.f/3.f) + deltaG - deltaR;

			if (h < 0.f)
				h += 1.f;
			else if (h > 1.f)
				h -= 1.f;

			*hue = static_cast<UInt8>(h*240.f);
		}
	}

	inline void Color::ToHSV(const Color& color, float* hue, float* saturation, float* value)
	{
		float r = color.r / 255.f;
		float g = color.g / 255.f;
		float b = color.b / 255.f;

		float min = std::min({r, g, b});    //Min. value of RGB
		float max = std::max({r, g, b});    //Max. value of RGB

		float deltaMax = max - min; //Delta RGB value

		*value = max;

		if (NumberEquals(deltaMax, 0.f))
		{
			//This is a gray, no chroma...
			*hue = 0; //HSV results from 0 to 1
			*saturation = 0;
		}
		else
		{
			//Chromatic data...
			*saturation = deltaMax/max*360.f;

			float deltaR = ((max - r)/6.f + deltaMax/2.f)/deltaMax;
			float deltaG = ((max - g)/6.f + deltaMax/2.f)/deltaMax;
			float deltaB = ((max - b)/6.f + deltaMax/2.f)/deltaMax;

			float h;

			if (NumberEquals(r, max))
				h = deltaB - deltaG;
			else if (NumberEquals(g, max))
				h = (1.f/3.f) + deltaR - deltaB;
			else
				h = (2.f/3.f) + deltaG - deltaR;

			if (h < 0.f)
				h += 1.f;
			else if (h > 1.f)
				h -= 1.f;

			*hue = h*360.f;
		}
	}

	inline void Color::ToXYZ(const Color& color, Vector3f* vec)
	{
		return ToXYZ(color, &vec->x, &vec->y, &vec->z);
	}

	inline void Color::ToXYZ(const Color& color, float* x, float* y, float* z)
	{
		float r = color.r/255.f;        //R from 0 to 255
		float g = color.g/255.f;        //G from 0 to 255
		float b = color.b/255.f;        //B from 0 to 255

		if (r > 0.04045f)
			r = std::pow((r + 0.055f)/1.055f, 2.4f);
		else
			r /= 12.92f;

		if (g > 0.04045f)
			g = std::pow((g + 0.055f)/1.055f, 2.4f);
		else
			g /= 12.92f;

		if (b > 0.04045f)
			b = std::pow((b + 0.055f)/1.055f, 2.4f);
		else
			b /= 12.92f;

		r *= 100.f;
		g *= 100.f;
		b *= 100.f;

		//Observer. = 2°, Illuminant = D65
		*x = r*0.4124f + g*0.3576f + b*0.1805f;
		*y = r*0.2126f + g*0.7152f + b*0.0722f;
		*z = r*0.0193f + g*0.1192f + b*0.9505f;
	}

	inline float Color::Hue2RGB(float v1, float v2, float vH)
	{
		if (vH < 0.f)
			vH += 1;

		if (vH > 1.f)
			vH -= 1;

		if ((6.f * vH) < 1.f)
			return v1 + (v2-v1)*6*vH;

		if ((2.f * vH) < 1.f)
			return v2;

		if ((3.f * vH) < 2.f)
			return v1 + (v2 - v1)*(2.f/3.f - vH)*6;

		return v1;
}
}

inline std::ostream& operator<<(std::ostream& out, const Nz::Color& color)
{
	return out << color.ToString();
}

#include <Nazara/Core/DebugOff.hpp>
