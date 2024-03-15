Hi this is a start of an OS for the Riverdi 7" stm32H757 displays.
The SysTick and Clock setup works :-)
Some of the intercomunication is in place.
Some of the Power control is working.
A new linker file is also made.
The rest is stil to come :-|

It is based on the VisualGDB platform. The goal is to make 2 projects one for the CM7 and one for the CM4 core.
First the goal is to make the CM7 handle the setup and have the Diplay working.

I want to make some sort of platform (Visual studio plugin) to setup the clocksystem (cubeMX clon) and to design the display layout (LVGL clon).
If you have any knowlege of making Visual studio plugin please let me know :-)



