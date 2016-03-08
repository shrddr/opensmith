Opensmith - free guitar learning game.
It aims to fix Rocksmith shortcomings: drop the branded cable requirement, add ASIO support, improve interface responsiveness and loading times.

Dependencies:
* [glfw](https://github.com/glfw/glfw) - window creation
* [glew](https://github.com/nigels-com/glew) - OpenGL extension loader
* [glm](https://github.com/g-truc/glm) - matrix math
* [gli](https://github.com/g-truc/gli) - texture loader
* [zlib](http://www.zlib.net/) - psarc entries unpacker
* [portaudio](http://www.portaudio.com/) - audio API
* [ogg](https://xiph.org/ogg/) - song container
* [vorbis](https://xiph.org/vorbis/) - song codec
* [dirent](https://github.com/tronkko/dirent) - filesystem api

Contents:
* PSARC and SNG parser
* WEM to OGG converter
* OpenGL visuals
* Audio input and output
* Note detector
* Main executable
* Tuner executable
* Setup executable

Building:
VS 2015 solution and prebuilt dependencies included.
Also comes with GCC makefile, tested on Ubuntu and Mac OS X ([Building guide](Install.md)).

Usage:
Run setup first and select the fastest audio device. After that, run `opensmith songfile.psarc`. Add `-rhythm` parameter to play rhythm instead of lead, `-dx` to set difficulty x (0-30 depending on the song), `-f` to go fullscreen.
You can use Ubisoft DLCs as well as custom DLCs (https://github.com/rscustom/rocksmith-custom-song-toolkit), PC/Mac format.