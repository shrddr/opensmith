=== Ubuntu 15.10
==== glew
* `sudo apt-get install libglew-dev`
==== glfw
There's an apt-get package named libglfw3-dev, but it doesn't seem to work.
* `sudo apt-get install cmake xorg-dev libgl1-mesa-dev`
* `git clone https://github.com/glfw/glfw.git`
* `cd glfw`
* `cmake .`
* `make`
* `sudo make install`
* `cd .. && rm -rf glfw` (cleanup)
==== zlib
* `sudo apt-get install zlib1g-dev`
==== PortAudio
* `sudo apt-get install libasound-dev` (ALSA audio library)
* `wget http://www.portaudio.com/archives/pa_stable_v19_20140130.tgz`
* `tar -xvzf pa_stable_v19_20140130.tgz`
* `cd portaudio`
* `./configure && make`
* `cp lib/.libs/libportaudio.a PATH/TO/opensmith/lib`
* `cd .. && rm -rf portaudio` (cleanup)
==== ogg
* `sudo apt-get install libogg-dev`
==== vorbis
* `sudo apt-get install libvorbis-dev`