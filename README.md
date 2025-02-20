# Base Edge Router (BER) in Field4D (F4D) Outdoor Plant Phenotyping iIoT Framework

| ![Field4D Logo](https://avatars.githubusercontent.com/u/71197432?v=4) | ![Yellow Ribbon](https://upload.wikimedia.org/wikipedia/commons/thumb/d/d0/Yellow_ribbon.svg/200px-Yellow_ribbon.svg.png) |
|:----------------------:|:----------------------:|

## Description

[F4D (Field4D) outdoor plant phenotyping iIoT framework](https://github.com/field4d/field4d): Base Edge Router (BER) firmware source code.

---

## Overview

The Base Edge Router (BER) functions as the RPL root node, TSCH coordinator, and UDP server within a Wireless Sensor Network (WSN). It ensures efficient power management, minimal data loss, and continuous spatiotemporal data flow through UDP-to-JSON encapsulation, supporting seamless integration within the Field4D (F4D) ecosystem.

## Hardware Implementation

- **TI CC2650 LaunchPad:** A wireless MCU featuring a 32-bit ARM® Cortex®-M3 processor at 48 MHz, designed for reliable wireless communication and low-power operations in outdoor phenotyping applications.

## SimpleLink Software Environment

The SimpleLink software environment offers modular wireless connectivity with TI Drivers, NoRTOS support, and a Core SDK provided as a git submodule. It supports UART, SPI, I2C, and other peripherals, with configurable options in `project-conf.h` for memory and code optimization.

## Usage

This firmware is tailored for `TARGET=simplelink BOARD=launchpad/cc2650`.

### Prerequisites

Install all required toolchains as per the Contiki-NG documentation.

### Building the Firmware

In the project root, execute:

```shell
make distclean
make TARGET=simplelink BOARD=launchpad/cc2650
```

## License

MIT License

## Contributors

- **Idan Ifrach**, Hebrew University of Jerusalem, Israel
  [idan.ifrachi@mail.huji.ac.il](mailto:idan.ifrachi@mail.huji.ac.il) | [LinkedIn](https://www.linkedin.com/in/ifrachi/) | [ORCID](https://orcid.org/0009-0000-0552-0935)

## References

![Logo](https://raw.githubusercontent.com/contiki-ng/contiki-ng.github.io/master/images/logo/Contiki_logo_1RGB.png)

**Contiki-NG: The OS for Next Generation IoT Devices**
Contiki-NG is an open-source OS for IoT devices, focusing on secure, reliable low-power communication with standards like IPv6/6LoWPAN, 6TiSCH, RPL, and CoAP.

- [GitHub repository](https://github.com/contiki-ng/contiki-ng)
- [Documentation](https://docs.contiki-ng.org/)
- [Releases](https://github.com/contiki-ng/contiki-ng/releases)
- [Website](http://contiki-ng.org)

## Contact

For support, contact <greenroom.lab@mail.huji.ac.il>
