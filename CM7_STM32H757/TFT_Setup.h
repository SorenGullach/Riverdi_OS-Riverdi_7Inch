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

/*
 * from data sheet
  HV MODE - Horizontal input timing  |         |        |         |        |
+------------------------------------+---------+--------+---------+--------+
|          Parameter Symbol          |   Min   |  Typ   |   Max   |  Unit  |
+------------------------------------+---------+--------+---------+--------+
|   Horizontal display area (thd)    |               1024         |  DCLK  |
|          DCLK frequency            |   44.9  |  51.2  |   63    |   MHz  |
|     (frame rate 60Hz) (fclk)       |         |        |         |        |
|     1 Horizontal Line (th)         |   1200  |  1344  |  1400   |  DCLK  |
|       HSYNC pulse width (thpw)     |    1    |   70   |   140   |  DCLK  |
|        HSYNC blanking (thb)        |   160   |  160   |   160   |  DCLK  |
|      HSYNC front porch (thfp)      |    16   |  160   |   260   |  DCLK  |
+------------------------------------+---------+--------+---------+--------+

|   HV MODE - Vertical input timing  |         |        |         |        |
+------------------------------------+---------+--------+---------+--------+
|   Parameter Symbol                 |   Min   |  Typ   |   Max   |  Unit  |
+------------------------------------+---------+--------+---------+--------+
|   Vertical display area (tvd)      |               600          |    H   |
|    VSYNC period time (tv)          |   624   |  635   |   750   |    H   |
|   VSYNC pulse width (tvpw)         |    1    |   10   |    20   |    H   |
|    VSYNC back porch (tvb)          |    23   |   23   |    23   |    H   |
|   VSYNC front porch (tvfp)         |    1    |   12   |   127   |    H   |
+------------------------------------+---------+--------+---------+--------+
*/

/*
	The general blending formula is : BC = BF1 x C + BF2 x Cs
	• BC = blended color
	• BF1 = blend factor 1
	• C = current layer color
	• BF2 = blend factor 2
	• Cs = subjacent layers blended color
*/
enum class eBlendingFactor1
{
	F1_CA    = 0B100U /* Constant Alpha */,
	F1_PAxCA = 0B110U /* Constant Alpha x Pixel Alpha */,
	F2_CA    = 0B101U /* Constant Alpha */,
	F2_PAxCA = 0B111U /* Constant Alpha x Pixel Alpha */
};
enum class eBlendingFactor2
{
	F2_CA    = 0B101U /* Constant Alpha */,
	F2_PAxCA = 0B111U /* Constant Alpha x Pixel Alpha */
};

// TFT display specifications
const uint16_t HSW = 70;	/*!< Horizontal Synchronization Width */
const uint16_t HBP = 160; /*!< Horizontal Back Porch */
const uint16_t HFP = 160; /*!< Horizontal Front Porch */
const uint16_t HACT = 1024; /*!< Active Width */
const uint16_t VSH = 10; /*!< Vertical Synchronization Height  */
const uint16_t VBP = 23; /*!< Vertical Back Porch   */
const uint16_t VFP = 12; /*!< Vertical Front Porch   */
const uint16_t VACT = 600; /*!< Active height */

#ifdef DSI_SETUP

enum class eDSI_PLL_IN
{
	DIV1 = 1U,
	DIV2 = 2U,
	DIV3 = 3U,
	DIV4 = 4U,
	DIV5 = 5U,
	DIV6 = 6U,
	DIV7 = 7U,
};
enum class eDSI_PLL_OUT
{
	DIV1 = 0U,
	DIV2 = 1U,
	DIV4 = 2U,
	DIV8 = 3U
};
// Clock Pll setup
const eDSI_PLL_IN IDF = eDSI_PLL_IN::DIV1;
const eDSI_PLL_OUT ODF = eDSI_PLL_OUT::DIV1;
const R_t<uint32_t, 10, 125> NDIV = 62;
const uint32_t TXClkEscape = 4;

// DSI Host timing
const uint32_t TimeoutCkdiv = 1;

// Data & clock lanes
const uint32_t NumberOfLanes = 2;
const bool AutomaticClockLaneControl = true; 
const bool ClockLaneRunsInHighSpeedMode = true;

// PHY timings
const uint32_t ClockLaneHS2LPTime = 28;
const uint32_t ClockLaneLP2HSTime = 32;
const uint32_t DataLaneHS2LPTime = 15;
const uint32_t DataLaneLP2HSTime = 24;
const uint32_t DataLaneMaxReadTime = 0;
const uint32_t StopWaitTime = 0;

enum class eColorCoding 
{
	RGB565 = 0U,
	RGB666 = 3U, 
	RGB888 = 5U,
};

enum class eVideoMode
{
	NoBurst_PULSES = 0U,
	NoBurst_EVENTS = 1U,
	BURST          = 2U,
};

// Display interface
const uint32_t VirtualChannelID = 0;
const eColorCoding ColorCoding = eColorCoding::RGB888;

const bool LooselyPacked = false;

const eVideoMode Mode = eVideoMode::BURST;
const uint32_t PacketSize = HACT;
const uint32_t NumberOfChunks = 0;
const uint32_t NullPacketSize = 0;
const bool FrameBTAAcknowledgeEnable = false;

const bool LPCommandEnable = true; 
const uint32_t LPLargestPacketSize = 23;
const uint32_t LPVACTLargestPacketSize = 0;

const bool LPHorizontalFrontPorchEnable = true;
const bool LPHorizontalBackPorchEnable = true; 
const bool LPVerticalActiveEnable = true; 
const bool LPVerticalFrontPorchEnable = true; 
const bool LPVerticalBackPorchEnable = true;
const bool LPVerticalSyncActiveEnable = true;

#endif

const bool HSPolarityHigh = false; 
const bool VSPolarityHigh = false;
const bool DEPolarityHigh = true;
const bool PCPolarityInverted = false;

#ifdef LTDC_SETUP
// LTDC settings

enum class ePixelFormat {
	
	ARGB8888 = 0B000U /* ARGB8888 LTDC pixel format */,
	RGB888   = 0B001U /* RGB888 LTDC pixel format */,
	RGB565   = 0B010U /* RGB565 LTDC pixel format */,
	ARGB1555 = 0B011U /* ARGB1555 LTDC pixel format */,
	ARGB4444 = 0B100U /* ARGB4444 LTDC pixel format */,
	L8       = 0B101U /* L8 LTDC pixel format */,
	AL44     = 0B110U /* AL44 LTDC pixel format */,
	AL88     = 0B111U  /* AL88 LTDC pixel format */
};

struct Layer_t
{
//	uint16_t WindowX0 = 0;
//	uint16_t WindowX1 = 1024;
//	uint16_t WindowY0 = 0;
//	uint16_t WindowY1 = 600;
//	uint16_t ImageWidth = 1024;
//	uint16_t ImageHeight = 600;
//	ePixelFormat PixelFormat = ePixelFormat::ARGB8888;
//	uint8_t ConstantAlpha = 255; // used when BFx == Fx_CA
//	eBlendingFactor BlendingFactor1 = eBlendingFactor1::F1_CA;
//	eBlendingFactor BlendingFactor2 = eBlendingFactor2::F2_CA;
//	uint8_t BackColorAlpha = 255; // used when BFx == Fx_PAXCA
//	uint8_t BackColorRed = 0;	
//	uint8_t BackColorGreen = 0;
//	uint8_t BackColorBlue = 0;
} Layers[2];
#endif