# STM32_NIHI_SimpleInverter

A simple open-loop 3-phase SPWM (Sinusoidal Pulse Width Modulation) inverter project using the STM32G474RE's High-Resolution Timer (HRTIM).

## Overview

This project demonstrates how to implement a 3-phase inverter on the **Nucleo-G474RE** board without complex feedback control. It utilizes the HRTIM peripheral to generate high-precision PWM signals with deadtime insertion.

## Features

* **MCU:** STM32G474RE (Nucleo-G474RE)
* **Peripheral:** HRTIM (High-Resolution Timer)
* **Control Method:** Open Loop SPWM (V/f constant not implemented, pure sine wave generation)
* **Waveform Generation:** Look-Up Table (LUT) based sine wave synthesis
* **Protection:** Hardware deadtime insertion via HRTIM

## Parameters

| Parameter | Symbol | Value |
| :--- | :--- | :--- |
| **Fundamental Freq** | $f_m$ | 50 Hz |
| **Switching Freq** | $f_{sw}$ | 20 kHz |
| **Deadtime** | $t_{dt}$ | 50 ns |
| **Modulation Index** | $M$ | 0.85 |

## Requirements

* **IDE:** STM32CubeIDE
* **Hardware:** Nucleo-G474RE board

## Usage

1.  Open the project in STM32CubeIDE.
2.  Build and flash the code to the Nucleo-G474RE.
3.  Check the PWM outputs on the designated HRTIM pins (Timer A, B, and C outputs).

## License

This project is open source.
