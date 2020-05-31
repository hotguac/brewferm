# brewferm
Fermentation chamber controller running on a Particle&reg; Photon/Argon family of IoT hardware.

Use [Particle Workbench](https://www.particle.io/workbench) to build and deploy.

BrewFerm is a dual-stage, dual-loop, fermentation chamber temperature controller. Implemented as two nested PID control loops, the outer loop is setup to maintain a target wort temperature. The output of the outer loop PID is a target fermentation chamber temperature. The second, inner PID loop is setup to control heating and cooling of the chamber, as needed, to hold the chamber at the temperature set by the outer loop.

The dual-loop configuration allows maintaining the target wort temperature within +/- 0.2F, although typical usage would shoot for +/- 0.5F to reduce energy consumption.

The reference system the software is developed against is a small chest freezer with a small heating element added inside. The Particle IoT device controls two relays which turn the heating or cooling on as necessary to maintain temperatures.

The target temperature and PID tunings can be set through a clound interface using the [Particle Console](https://console.particle.io/devices) website, the command line interface with a curl command, or the [BrewFermoid Android](https://github.com/hotguac/brewfermoid) application. Each of the interfaces can also display current system status, PID tuning, or pause the system to suspend heating and cooling when not in use.

The procedure for assigning sensors to beer or to chamber roles is performed by the following procedure:

- turn power off to system
- unplug all sensors
- turn power on to system
- wait for system to indicate sensor assignment color sequence
    - dim white, bright white, green, yellow, red)
    - plugin beer temperature sensor
    - wait for color sequence (dim while, bright white, yellow, red, green)
    - plugin chamber temperature sensor
    - the system should start normal processing
