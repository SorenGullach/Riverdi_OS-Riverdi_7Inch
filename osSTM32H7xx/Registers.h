#pragma once

#include <stdint.h>

/*
 A type that checks for range in debug mode
*/
template <typename T, T min, T max>
	struct R_t
	{
		R_t(T value)
		{
			assert(value >= min && value <= max);
			Value = value;
		}
		T& operator=(const uint32_t other)
		{
			assert(other >= min && other <= max);
			Value = other;
			return Value;
		}
		T& operator=(const uint8_t other)
		{
			assert(other >= min && other <= max);
			Value = other;
			return Value;
		}
		/*
		R_t<T, min, max>& operator=(const R_t<T, min, max>& other)
		{
			assert(other.Value >= min && other.Value <= 15);
			Value = other.Value;
			return Value;
		}
		*/
		bool operator==(const int other)
		{
			assert(other >= min && other <= max);
			return Value == other;
		}
		bool operator==(const uint32_t other)
		{
			assert(other >= min && other <= max);
			return Value == other;
		}
		bool operator==(const uint8_t other)
		{
			assert(other >= min && other <= max);
			return Value == other;
		}
		/*
		bool operator==(const R_t<T, min, max>& other)
		{
			assert(other.Value >= min && other.Value <= 15);
			return Value == other.Value;
		}
		*/
//		template<typename C>
		operator T() const
		{
			return Value;
		}
	private:
		T Value;
	};



