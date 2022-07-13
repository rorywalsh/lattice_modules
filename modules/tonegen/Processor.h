#pragma once
#include "LatticeProcessorModule.h"
#include "Tonegen.h"
#include "Env.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"

class TWProcessor : public LatticeProcessorModule
{
 public:
  TWProcessor();
    
  ChannelData createChannels() override;
    
  /* This function is called by he host to populate the parameter vector */
  ParameterData createParameters() override;
    
  /*  This function is called by the host before playback/performance */
  void prepareProcessor(int sr, std::size_t block) override;

  /*  Main processing function called continuously by the host on the audio thread.
      paramValues is a list of parameter values passed from the host in order of their creation */
  void process(float** buffer, std::size_t blockSize) override;

  void startNote(int n, float vel) override
  {
     keys[n] = 1;
  }

  void stopNote(int n, float vel) override
  {
     keys[n] = 0;
  }

  /* override this method and return true if you wish to enable drawing on the generic editor viewport */
  bool canDraw() override { return true; }
    
  const char* getModuleName() override {    return "Sawtooth Tone Generator";     }
    
 private:
  Aurora::Wavegen<float> tg;
  float att, dec, sus, rel;
  Aurora::Env<float> env;
  bool keys[128] = {0};
  float fs = 44100;

};
