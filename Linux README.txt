If not already installed, the program will require the following packages:

libxrandr-dev 
libxcursor-dev 
libudev-dev 
libopenal-dev 
libflac-dev 
libvorbis-dev 
libgl1-mesa-dev 
libegl1-mesa-dev 
libfontconfig1-dev 
libfreetype-dev

You may be able to copy and paste this command:

sudo apt-get update && sudo apt-get install -y libxrandr-dev libxcursor-dev libudev-dev libopenal-dev libflac-dev libvorbis-dev libgl1-mesa-dev libegl1-mesa-dev libfontconfig1-dev libfreetype-dev


UBUNTU <22.x NOTE: At the time of publishing this program, the only required library that was not available via apt was libFLAC8, which has been bundled. It is possible that over time other of the packages in the above list will become incompatible (too new) when loaded via apt. In that case the older versions would have to be located and installed.