#include "audio/AudioBuffer.h"
#include <AL/alext.h>
#include <AL/al.h>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>

static bool IsBigEndian(void)
{
    int a = 1;
    return !((char*)&a)[0];
}

static int ConvertToInt(char* buffer, int len)
{
    int a = 0;

    if(!IsBigEndian())
    {
        for(int i = 0; i < len; ++i)
        {
            ((char*)&a)[i] = buffer[i];
        }
    }
    else
    {
        for(int i = 0; i < len; ++i)
        {
            ((char*)&a)[3-i] = buffer[i];
        }
    }

    return a;
}

//Location and size of data is found here: http://www.topherlee.com/software/pcm-tut-wavformat.html
static char* LoadWAV(const char* filename, int& channels, int& sampleRate, int& bps, int& size)
{
    char buffer[4];

    std::ifstream in(filename);
    in.read(buffer, 4);

    if(strncmp(buffer, "RIFF", 4) != 0)
    {
        std::cout << "Error here, not a valid WAV file, RIFF not found in header\n This was found instead: "
                  << buffer[0] << buffer[1] << buffer[2] << buffer[3] << std::endl;
    }

    in.read(buffer, 4);//size of file. Not used. Read it to skip over it.  

    in.read(buffer, 4);//Format, should be WAVE

    if(strncmp(buffer, "WAVE", 4) != 0)
    {
        std::cout << "Error here, not a valid WAV file, WAVE not found in header.\n This was found instead: "
                  << buffer[0] << buffer[1] << buffer[2] << buffer[3] << std::endl;
    }

    in.read(buffer, 4);//Format Space Marker. should equal fmt (space)

    if(strncmp(buffer, "fmt ", 4) != 0)
    {
        std::cout << "Error here, not a valid WAV file, Format Marker not found in header.\n This was found instead: "
                  << buffer[0] << buffer[1] << buffer[2] << buffer[3] << std::endl;
    }

    in.read(buffer, 4);//Length of format data. Should be 16 for PCM, meaning uncompressed.

    if(ConvertToInt(buffer, 4) != 16)
    {
        std::cout << "Error here, not a valid WAV file, format length wrong in header.\n This was found instead: "
                  << ConvertToInt(buffer, 4) << std::endl;
    }

    in.read(buffer, 2);//Type of format, 1 = PCM

    if(ConvertToInt(buffer, 2) != 1)
    {
        std::cout << "Error here, not a valid WAV file, file not in PCM format.\n This was found instead: "
                  << ConvertToInt(buffer, 4) << std::endl;
    }

    in.read(buffer, 2);//Get number of channels. 

    //Assume at this point that we are dealing with a WAV file. This value is needed by OpenAL
    channels = ConvertToInt(buffer, 2);

    in.read(buffer, 4);//Get sampler rate. 

    sampleRate = ConvertToInt(buffer, 4);

    //Skip Byte Rate and Block Align. Maybe use later?
    in.read(buffer, 4);//Block Align
    in.read(buffer, 2);//ByteRate

    in.read(buffer, 2);//Get Bits Per Sample

    bps = ConvertToInt(buffer, 2);

    //Skip character data, which marks the start of the data that we care about. 
    in.read(buffer, 4);//"data" chunk. 

    in.read(buffer, 4); //Get size of the data

    size = ConvertToInt(buffer, 4);

    if(size < 0)
    {
        std::cout << "Error here, not a valid WAV file, size of file reports 0.\n This was found instead: "
                  << size << std::endl;
    }

    char* data = new char[size];

    in.read(data, size);//Read audio data into buffer, return.

    in.close();

    return data;    
}

namespace sound {
	AudioBuffer::AudioBuffer(const char* filename) : buffer_handle_(0) {
		alGenBuffers(1, &buffer_handle_);

		int channels;
		int sample_rate;
		int bps;
		int size;
		char* data = LoadWAV(filename, channels, sample_rate, bps, size);

		ALenum format = AL_NONE;
		if (channels == 1) {
			format = AL_FORMAT_MONO16;
		}
		else if (channels == 2) {
			format = AL_FORMAT_STEREO16;
		}
		else if (channels == 3) {
			format = AL_FORMAT_BFORMAT2D_16;
		}
		else if (channels == 4) {
			format = AL_FORMAT_BFORMAT3D_16;
		}

		alBufferData(buffer_handle_, format, data, size, sample_rate);
		//buffer_handle_ = alutCreateBufferFromFile(filename);
		//std::cout << "create hande: " << buffer_handle_ << "\n";
		//std::cout << alutGetErrorString(alutGetError()) << "\n";
	}

	AudioBuffer::~AudioBuffer() {
		alDeleteBuffers(1, &buffer_handle_);
	}

} //namespace sound
