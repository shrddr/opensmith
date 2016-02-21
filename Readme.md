Opensmith - free guitar learning game.
It aims to fix Rocksmith shortcomings: drop the branded cable requirement, add ASIO support, improve interface responsiveness and loading times.

Dependencies:
* glfw - window creation
* glew - OpenGL extension loader
* glm - matrix math
* gli - texture loader
* zlib - psarc entries unpacker
* portaudio - soundcard interface
* ogg - song container
* vorbis - song codec

Contents:
* PSARC and SNG parser
* WEM to OGG converter
* OpenGL visuals
* Audio input and output
* Note detector
* Tuner executable
* Setup executable
* Main executable

Building:
VS 2015 solution and prebuilt dependencies included.
Mac/Linux should work too.

Usage:
Run setup first and select the fastest audio device. After that, run `opensmith songfile.psarc`. Add `-rhythm` parameter to play rhythm instead of lead, `-dx` to set difficulty x (0-30 depending on the song), `-f` to go fullscreen.
You can use Ubisoft DLCs as well as custom DLCs (https://github.com/rscustom/rocksmith-custom-song-toolkit).
