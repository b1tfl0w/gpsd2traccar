# gpsd2traccar
A C program that reads gps data from [gpsd](https://gpsd.io/) server and sends them to [Traccar](https://www.traccar.org/) server using [OsmAnd](https://www.traccar.org/osmand/) protocol.

# setup
set device id, server and port for gpsd and traccar in gpsd2traccar.c defaults to localhost and default ports.

# compile
Install gps library first:
sudo apt install libgps-dev
then run:
make
and run the program using the sh script which runs in a while loop, so if program exits it will start again.
./gpsd2traccar
