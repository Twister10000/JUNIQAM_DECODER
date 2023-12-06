# JUNIQAM (JUventus New Integrated QAM) README

## Overview 📡

JUNIQAM is a custom Quadrature Amplitude Modulation (QAM) solution designed to facilitate communication through Digital-to-Analog Converters (DAC) and Analog-to-Digital Converters (ADC). This innovative system is tailored for robust digital signal processing and is a product of our dedicated engineering efforts.

## Features 🌟

- Custom QAM implementation for specialized communication needs.
- Integration with DAC for signal transmission.
- ADC utilization for signal reception and decoding.
- Built with FreeRTOS for efficient multitasking.
- Utilizes DMA for optimized data handling.
- Includes a user-friendly display interface for status and debugging.

## Installation 🛠️

To get started with JUNIQAM, clone the repository to your local machine using:

```sh
git clone https://github.com/annoyedmilk/JUNIQAM.git
```

## Hardware Setup 🔌

Ensure that your hardware is compatible with the system requirements of JUNIQAM. This includes a functioning DAC for transmitting signals and an ADC for receiving them.

## Software Dependencies 📦

- AVR Toolchain
- FreeRTOS 10.0.1
- Compatible C compiler (e.g., avr-gcc)

## Configuration ⚙️

Before running the system, configure the clock, display, DAC, DAC Timer, Generator DMA, ADC, ADC Timer, and Decoder DMA by following the initialization sequence in the `main` function.

## Usage 💻

1. Initialize the system by calling the `vInitClock`, `vInitDisplay`, and other initialization functions.
2. Create the QAM Generator and Decoder tasks using `xTaskCreate`.
3. Start the FreeRTOS scheduler with `vTaskStartScheduler`.

## Example Code 📄

```c
int main(void)
{
	resetReason_t reason = getResetReason();

	vInitClock();
	vInitDisplay();
	
	initDAC();
	initDACTimer();
	initGenDMA();
	initADC();
	initADCTimer();
	initDecDMA();
	
	xTaskCreate(vQuamGen, NULL, configMINIMAL_STACK_SIZE+500, NULL, 2, NULL);
	xTaskCreate(vQuamDec, NULL, configMINIMAL_STACK_SIZE+100, NULL, 1, NULL);

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"FreeRTOS 10.0.1");
	vDisplayWriteStringAtPos(1,0,"EDUBoard 1.0");
	vDisplayWriteStringAtPos(2,0,"QAMDECGEN-Base");
	vDisplayWriteStringAtPos(3,0,"ResetReason: %d", reason);
	vTaskStartScheduler();
	return 0;
}
```

## Contributing 🤝

We welcome contributions! If you have suggestions or would like to improve the JUNIQAM, please fork the repository and submit a pull request.

## License 📜

JUNIQAM is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## Support 🆘

If you encounter any issues or require assistance, please open an issue on the GitHub repository.
