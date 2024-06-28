# Low-power Wireless Networking

Low-power wireless networking (e.g., wireless sensor networks (WSN)) is an important research field and application area. Apart from traditional routing-based approaches to exchange messages, communication protocols based on synchronous transmissions (ST) have had a significant impact in the last decade. Such protocols, for example, Mixer, Glossy, Chaos, Crystal, etc., achieve an outstanding performance in key metrics, such as, latency, reliabiliy, and energy efficiency. In addition, network dynamics (e.g., moving nodes), which cause high overhead in routing-based communication protocols, are inherently supported by ST-based protocols what makes them a promising building block of emerging cyber-physical systems.

Unfortunately, *all* ST-based communication protocols suffer from a single point of failure. The problem is based on the need for tight time synchronization between the nodes. So far, all ST-based protocols use a dedicated node (typically called initiator or host) as time reference for the entire network. A failure of this node leads to a communication outage and its detection is unreliable and costs time and energy.

# Butler

Butler is a distributed synchronization protocol that addresses this single point of failure. Butler accurately time synchronizes all connected nodes in the network using a fully distributed approach without special nodes or roles. By synchronizing all nodes, Butler eliminates the need for a dedicated initiator in ST-based protocols, thus, all nodes in the network are able to start communication, which dramatically increases availability and dependability.

Learn more about Butler in our [paper](https://nes-lab.org/wordpress/wp-content/uploads/2022/08/mager22butler.pdf):

> Fabian Mager, Andreas Biri, Lothar Thiele, and Marco Zimmerling. "BUTLER: Increasing the Availability of Low-Power Wireless Communication Protocols." In *International Conference on Embedded Wireless Systems and Networks (EWSN)*, pages 108-119, Linz, Austria, 2022. URL https://dl.acm.org/doi/10.5555/3578948.3578958

# Building and Running Butler

Butler is implemented on Nordic Semiconductor's nRF52840 DK development board and nRF52840 Dongle. As a starting point, you can find an example application of Butler and the ST-based protocol [Mixer](https://gitlab.com/nes-lab/mixer) in the `app/` folder.

## Code Layout

`app/` example application (Mixer + Butler)

`butler/` source code of Butler

`gpi/` generic platform interface (internally used by Butler)


## Example Application

In the example application, Mixer communication rounds are executed in an infinite loop. Before each Mixer round, Butler is executed (`butler_start()`) to synchronize all nodes, such that *any* subset of nodes can initiate communication.

## Setup (Prerequisites)

Install the [Segger Embedded Studio (SES)](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/Segger-Embedded-Studio) IDE, which is free for use with nRF devices. It is available for Windows, Linux, and OSX. Note: Butler was developed with **SES V5.42**. Newer versions could cause problems due to updates in the C runtime libraries.

Inside SES, we suggest to install the "nRF CPU Support Package" (menu "Tools" -> "Package Manager"), which simplifies the creation of new projects targeting nRF SoCs.

## Open Example Project

Open the example project `app/butler_example.emProject` in the SES.

When you setup experiments, please make sure to check and understand the settings in `app/butler_config.h` and `app/mixer_config.h`, and to adapt them based on your needs.

## Build and Run

Build the tutorial project in the SES. The output, including .elf and .hex file, is placed in subfolder `Output/<configuration name>`.

There are three different configuration: `PCA10056_Release` (nRF52840 DK), `PCA10059_Release` (nRF52840 Dongle), and `PCA10059_FlockLab_Release` (nRF52840 Dongle on [FlockLab](https://gitlab.ethz.ch/tec/public/flocklab/wiki) testbed).  

The simplest way to program your device is to just copy the .hex file to the device via file system. For this purpose, the nRF52840 DK gets mounted as a storage device when you plug in the USB cable.

After programming, you can connect to the virtual COM port provided by the nRF52840 DK. The serial connection parameters are 115200,8,N,1.

The tutorial project stores the individual node ID in the SoC's "User Information Configuration Registers (UICR)". Programming the device typically erases the UICR region. Therefore, the demo application asks you to enter a node ID when it boots for the first time. The ID is then stored in the UICR so that it can be reused later on.

## Notes

There must be no external interrupts (outside of Butler) during the execution of Butler.

Butler automatically redirects radio and timer ISR's upon start, and restores the previous ISR's when finished. However, Butler also changes the configuration of the radio and timer, which does not get restored. This is unproblematic with Mixer, which configures all peripherals upon start, but needs to be considered when using other communication protocols.