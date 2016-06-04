### Ubuntu 15.10

#### zlib
* `sudo apt-get install zlib1g-dev`

#### ogg
* `sudo apt-get install libogg-dev`

#### vorbis
* `sudo apt-get install libvorbis-dev`

#### glew
* `sudo apt-get install libglew-dev`

#### glfw
There's an apt-get package named libglfw3-dev, but it doesn't seem to work.
* `sudo apt-get install cmake xorg-dev libgl1-mesa-dev`
* `git clone https://github.com/glfw/glfw.git`
* `cd glfw`
* `cmake .`
* `make`
* `sudo make install`
* `cd .. && rm -rf glfw` (cleanup)

#### PortAudio
This one also needs building from source.
* `sudo apt-get install libasound-dev`
* `wget http://www.portaudio.com/archives/pa_stable_v19_20140130.tgz`
* `tar -xvzf pa_stable_v19_20140130.tgz && rm pa_stable_v19_20140130.tgz`
* `cd portaudio`
* `./configure && make`
* `cp lib/.libs/libportaudio.a PATH/TO/opensmith/bin`
* `cd .. && rm -rf portaudio` (cleanup)

#### and finally
* `cd opensmith`
* `make`

### Mac OS X 10.8

#### glew
* `git clone https://github.com/nigels-com/glew.git`
* `cd glew`
* `cd auto`
* `make`
* `cd ..`
* `make glew.lib`
* `sudo make GLEW_NO_GLU="-DGLEW_NO_GLU" install`
* `cd .. && rm -rf glew` (cleanup)

#### glfw3
* Probably needs xcode installed (app store) 
* Install cmake (cmake.org), run, go to tools menu - how to install for command line use, follow directions, close cmake.
* `git clone https://github.com/glfw/glfw.git`
* `cd glfw`
* `cmake .`
* `make`
* `sudo make install`
* `cd .. && rm -rf glfw` (cleanup)

#### PortAudio
* `curl "http://www.portaudio.com/archives/pa_stable_v19_20140130.tgz" -o "pa.tgz"`
* `tar -xvzf pa.tgz && rm pa.tgz`
* `cd portaudio`
* `./configure && make`
* `cp lib/.libs/libportaudio.a PATH/TO/opensmith/bin`
* `cd .. && rm -rf portaudio` (cleanup)

#### ogg
* `curl "http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz" -o "ogg.tgz"`
* `tar -xvzf ogg.tgz && rm ogg.tgz`
* `cd libogg-1.3.2`
* `./configure`
* `sudo make install`
* `cd .. && rm -rf libogg-1.3.2` (cleanup)

#### vorbis
* `curl "http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.gz" -o "vorbis.tgz"`
* `tar -xvzf vorbis.tgz && rm vorbis.tgz`
* `cd libvorbis-1.3.5`
* `./configure`
* `sudo make install`
* `cd .. && rm -rf libvorbis-1.3.5` (cleanup)

#### and finally
* `cd opensmith`
* `make`
