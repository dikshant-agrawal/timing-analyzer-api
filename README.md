# Timing Analyzer API — Bare-Metal Embedded Measurement

A configurable timing-measurement API for bare-metal embedded systems (PSoC 5LP / Cortex-M3), built with object-oriented C.

## What it does
Measures execution time of code sections using selectable time bases — the DWT cycle counter, SysTick, or GPIO pin toggling for oscilloscope verification — and reports results over UART.

## Features
- Start / Stop / Pause / Resume measurement state machine
- Multiple analyzers running in parallel
- Configurations: DWT, DWT + output pin, SysTick, SysTick + output pin, pin-only
- UART-based reporting (report assembled first, then transmitted in a single shot to avoid distorting measurements)
- Object-oriented C design (structs + function pointers)
- Board-support drivers included: LEDs, seven-segment display, joystick, ring buffer, logging service

## Tech stack
C (bare-metal), PSoC Creator project for PSoC 5LP (Cortex-M3), DWT / SysTick peripherals, UART.

## Build & run
1. Open `PreDesign01.cydsn` in **PSoC Creator 4.x**
2. Build and program a PSoC 5LP target (CY8CKIT-050 or similar development board)
3. Observe measurement reports on the UART terminal (see `bsw/Services/logging.h` for config)

## Project structure
- `PreDesign01.cydsn/main.c` — demo application
- `PreDesign01.cydsn/analyzer.*` — timing analyzer API
- `PreDesign01.cydsn/bsw/` — board support (LEDs, seven-segment, joystick, ring buffer, logging)

