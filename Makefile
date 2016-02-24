OS := $(shell uname)
ifeq ($(OS), Darwin)
CXXFLAGS = -std=c++11 -stdlib=libc++
OSXGL = -framework OpenGL
PORTAUDIO = bin/libportaudio.a -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices -framework Carbon
else
CXXFLAGS = -std=c++11
PORTAUDIO = bin/libportaudio.a -lasound -pthread
endif

all: libRijndael libPsarcReader libSettings libWem libAudio exeopensmith exeSetup exeTest exeTuner

libRijndael:
	gcc -c Rijndael/rijndael.c -o bin/rijndael.o
	ar rcs bin/libRijndael.a bin/rijndael.o
	rm bin/rijndael.o

libPsarcReader:
	g++ $(CXXFLAGS) -c PsarcReader/BigEndianReader.cpp -o bin/BigEndianReader.o
	g++ $(CXXFLAGS) -c -I. -Iinclude PsarcReader/PSARC.cpp -o bin/PSARC.o
	g++ $(CXXFLAGS) -c -I. -Iinclude PsarcReader/Sng.cpp -o bin/Sng.o
	ar rcs bin/libPsarcReader.a bin/BigEndianReader.o bin/PSARC.o bin/Sng.o
	rm bin/BigEndianReader.o bin/PSARC.o bin/Sng.o

libSettings:
	g++ $(CXXFLAGS) -c Settings/Settings.cpp -o bin/Settings.o
	ar rcs bin/libSettings.a bin/Settings.o
	rm bin/Settings.o

libWem:
	g++ $(CXXFLAGS) -c Wem/bitstream.cpp -o bin/bitstream.o
	g++ $(CXXFLAGS) -c -I. Wem/codebook.cpp -o bin/codebook.o
	g++ $(CXXFLAGS) -c Wem/crc.cpp -o bin/crc.o
	g++ $(CXXFLAGS) -c -I. Wem/Wem.cpp -o bin/Wem.o
	ar rcs bin/libWem.a bin/bitstream.o bin/codebook.o bin/crc.o bin/Wem.o
	rm bin/bitstream.o bin/codebook.o bin/crc.o bin/Wem.o

libAudio:
	g++ $(CXXFLAGS) -c -I. -Iinclude Audio/Audio.cpp -o bin/Audio.o
	g++ $(CXXFLAGS) -c -I. -Iinclude Audio/NoteDetector.cpp -o bin/NoteDetector.o
	g++ $(CXXFLAGS) -c -I. -Iinclude Audio/VorbisDecoder.cpp -o bin/VorbisDecoder.o
	ar rcs bin/libAudio.a bin/Audio.o bin/NoteDetector.o bin/VorbisDecoder.o
	rm bin/Audio.o bin/NoteDetector.o bin/VorbisDecoder.o

exeopensmith:
	g++ $(CXXFLAGS) -I. -Iinclude opensmith/Camera.cpp opensmith/Controller.cpp opensmith/Hud.cpp opensmith/main.cpp opensmith/Mesh.cpp opensmith/Model.cpp opensmith/Sprite.cpp opensmith/Text2D.cpp opensmith/util.cpp opensmith/View.cpp bin/libAudio.a bin/libPsarcReader.a bin/libRijndael.a bin/libSettings.a bin/libWem.a $(PORTAUDIO) -lz `pkg-config --static --libs glew` `pkg-config --static --libs glfw3` `pkg-config --static --libs ogg` `pkg-config --static --libs vorbis` $(OSXGL) -o bin/opensmith

exeSetup:
	g++ $(CXXFLAGS) -I. -Iinclude Setup/Setup.cpp bin/libAudio.a bin/libSettings.a $(PORTAUDIO) -o bin/Setup

exeTest:
	g++ $(CXXFLAGS) -I. -Iinclude Test/test.cpp bin/libAudio.a bin/libWem.a bin/libPsarcReader.a bin/libRijndael.a -lz `pkg-config --static --libs ogg` `pkg-config --static --libs vorbis` -o bin/Test

exeTuner:
	g++ $(CXXFLAGS) -I. -Iinclude Tuner/libfft.c Tuner/tuner.cpp bin/libAudio.a bin/libSettings.a $(PORTAUDIO) -o bin/Tuner

clean:
	rm bin/libAudio.a bin/libPsarcReader.a bin/libRijndael.a bin/libSettings.a bin/libWem.a bin/opensmith bin/Setup bin/Test bin/Tuner
