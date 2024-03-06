#pragma once
#include <assert.h>

#include "stm32h7xx.h"
#include <Utils.h>

class hwGPIO
{
public:
	enum class eMode
	{
		Input = 0b00,
		Output = 0b01,
		//		Alternate = 0b10, // set by alt function
		Analog = 0b11,
	};
	enum class ePP
	{
		Float     = 0b0,
		PushPull  = 0b0,
		OpenDrian = 0b1,
	};
	enum class ePUPD
	{
		None     = 0b00,
		PullUp   = 0b01,
		PullDown = 0b10,
	};
	enum class eSpeed
	{
		Low      = 0b00,
		Medium   = 0b01,
		High     = 0b10,
		VeryHigh = 0b11
	};
	
	// Make GPIO Pin initialized elsewhere
	hwGPIO()
	{
	}
	// Make GPIO Pin initialized elsewhere
	hwGPIO(GPIO_TypeDef *pgpio, uint32_t pin)
	{
		assert(pin >= 0 && pin <= 15);
		assert(pgpio != nullptr); 
		pGPIO = pgpio;
		Pin = pin;
	}
	// Make GPIO pin initialized
	hwGPIO(GPIO_TypeDef *pgpio, uint32_t pin, eMode mode, ePP pp = ePP::Float, ePUPD pupd = ePUPD::None, eSpeed speed = eSpeed::High)
	{
		Init(pgpio, pin, mode, pp, pupd, speed);
	}
	void Init(GPIO_TypeDef *pgpio, uint32_t pin, eMode mode, ePP pp = ePP::Float, ePUPD pupd = ePUPD::None, eSpeed speed = eSpeed::High)
	{
		assert(pgpio != nullptr); 
		assert(pin >= 0 && pin <= 15);
		pGPIO = pgpio;
		Pin = pin;
		EnableClk();
		SetMode(mode);
		SetDriver(pp);
		SetPuPD(pupd);
		SetSpeed(speed);
	}
	void Init(GPIO_TypeDef *pgpio, uint32_t pin, uint8_t af, ePP pp = ePP::Float, ePUPD pupd = ePUPD::None, eSpeed speed = eSpeed::High)
	{
		R_t<uint8_t, 0,15>AF(af);
		assert(pgpio != nullptr); 
		assert(pin >= 0 && pin <= 15);
		pGPIO = pgpio;
		Pin = pin;
		EnableClk();
		SetMode(AF);
		SetDriver(pp);
		SetPuPD(pupd);
		SetSpeed(speed);
	}

	void On()	{ Set(); }
	void Off()	{ Reset(); }
	void High()	{ Set(); }
	void Low()	{ Reset(); }
	void Set() { assert(pGPIO != nullptr); pGPIO->BSRR = 1 << Pin; }
	void Reset() { assert(pGPIO != nullptr); pGPIO->BSRR = 1 << (Pin + 16); }
	void Toggle() {	Level() ? Reset() : Set(); }
	bool Level() { assert(pGPIO != nullptr); return pGPIO->IDR & (1 << Pin); }
	void Level(bool l) { l ? Set() : Reset(); }
	bool IsHigh() {return Level(); }
	bool IsLow() {return !Level(); }
	bool IsOn() {return Level(); }
	bool IsOff() {return !Level(); }
	
	
private:
	GPIO_TypeDef *pGPIO = nullptr;
	uint32_t Pin;
	
	void EnableClk()
	{
		if (pGPIO == GPIOA)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
		if (pGPIO == GPIOB)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;
		if (pGPIO == GPIOC)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;
		if (pGPIO == GPIOD)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN;
		if (pGPIO == GPIOE)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOEEN;
		if (pGPIO == GPIOF)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOFEN;
		if (pGPIO == GPIOG)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOGEN;
		if (pGPIO == GPIOH)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOHEN;
		if (pGPIO == GPIOI)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOIEN;
		if (pGPIO == GPIOJ)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOJEN;
#ifdef GPIOK
		if (pGPIO == GPIOK)
			RCC->AHB4ENR |= RCC_AHB4ENR_GPIOKEN;
#endif // GPIOK
		else
			assert("Wrong port");
	}

	void SetMode(R_t<uint8_t,0,15>af)
	{
		pGPIO->MODER &= ~(0b11 << 2*Pin); // 00
		pGPIO->MODER |= (uint32_t)0b10 << 2*Pin; // AF
		if (Pin < 8)
		{
			pGPIO->AFR[0] &= ~((uint32_t)0xF << 4*Pin);
			pGPIO->AFR[0] |= (uint32_t)af << 4*Pin;
		}
		else
		{
			pGPIO->AFR[1] &= ~((uint32_t)0xF << 4*(Pin-8));
			pGPIO->AFR[1] |= (uint32_t)af << 4*(Pin - 8);
		}
	}

	void SetMode(eMode mode)
	{
		pGPIO->MODER &= ~(0b11 << 2*Pin); // 00
		pGPIO->MODER |= (uint32_t)mode << 2*Pin; 
	}
	void SetDriver(ePP pp)
	{
		if (pp == ePP::OpenDrian)
			pGPIO->OTYPER |= 1 << Pin;
		else			
			pGPIO->OTYPER &= ~(1 << Pin);
	}
	void SetPuPD(ePUPD pupd)
	{
		pGPIO->PUPDR &= ~(0b11 << 2*Pin); // 00
		pGPIO->PUPDR |= (uint32_t)pupd << 2*Pin;
	}
	void SetSpeed(eSpeed speed)
	{
		pGPIO->OSPEEDR &= ~(0b11 << 2*Pin); // 00
		pGPIO->OSPEEDR |= (uint32_t)speed << 2*Pin; 
	}
};

