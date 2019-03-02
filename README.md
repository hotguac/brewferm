# brewferm
Fermentation chamber controller running on a Particle Photon

Use Local Build method. See vendor's local build page.
`https://docs.particle.io/support/particle-tools-faq/local-build/`

Requires the firmware source downloaded from Particle.io

Developed on Debian Stretch

To run ferm.py script, MySql-python must be installed
  `sudo apt-get install python-pip python-dev libmysqlclient-dev gcc-arm-none-eabi make`
  `pip install MySQL-python`

To build the application use this command line
`make PLATFORM=photon APPDIR=~/Documents/brewferm`
where APPDIR points to the brewferm source directory.

To build and deploy to a Particle Photon in dfu mode
'make PLATFORM=photon APPDIR=~/Documents/brewferm program-dfu'
