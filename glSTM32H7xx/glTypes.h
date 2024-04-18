/*
 * Copyright 2024 Søren Gullach
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * You must include the following attribution in all copies or substantial portions of the Software:
 * "Søren Gullach <STM32_R7_OS@toolsbox.dk>"
 */

#pragma once

#include <stdint.h>
#include <assert.h>
#include "MemoryDefs.h"

typedef uint16_t P_t;
typedef uint8_t C_t;

// Define a structure to represent colors in ARGB format
struct glARGB_t
{
	C_t B, G, R, A; // fits LTDC layout
} __PACKED; 

struct glColor_t : public glARGB_t
{
	glColor_t()
	{
		R = G = B = 0; A = 255;
	}
	glColor_t(C_t red, C_t green, C_t blue, C_t alpha=255)
	{
		R = red; G = green; B = blue; A = alpha;
	}
}; 
static_assert(sizeof(glColor_t) == 4, "Wrong size");

struct glPoint_t
{
	glPoint_t() { X = Y = 0; }
	glPoint_t(P_t x, P_t y) { X = x; Y = y; }
	P_t X, Y;
	
	uint32_t Pos(P_t width)	{ return (X + Y*width); }
};

#include <math.h>
#define M_PI 3.1415926535897932384626433832795
glColor_t RainbowColorGradient(float value);
