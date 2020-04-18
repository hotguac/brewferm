# brewferm
Fermentation chamber controller running on a Particle Photon &#174 /Argon family of IoT hardware.

Use [Particle Workbench](https://www.particle.io/workbench) to build and deploy.

BrewFerm is a fermentation chamber temperature controller implemented as two nested PID control loops. The outer loop is setup to maintain a target wort temperature. The output of the outer loop PID is a target fermentation chamber temperature. The second, inner PID loop is setup to control heating and cooling of the chamber, as needed, to hold the chamber at the temperature set by the outer loop.

The reference chamber is a small chest freezer with a small heating element added inside. The Particle IoT device controls two relays which turn the heat and cooling on and off as necessary to maintain temperatures.

The target temperature can be set through a clound interface using the [Particle Console](https://console.particle.io/devices) website, the command line interface with a curl command, or the [BrewFermoid Android](https://github.com/hotguac/brewfermoid) application. Each of the interfaces can also display current system status or pause the system to suspend heating and cooling when not in use.

