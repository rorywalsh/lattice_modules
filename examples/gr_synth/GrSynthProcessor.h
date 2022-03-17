#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "grain.h"
#include "Env.h"
#include <cstdlib>
#include <functional>
#include <iostream>
#include "simple_svg_1.0.0.hpp"


class GrSynthProcessor : public LatticeProcessorModule
{
    
 public:
  GrSynthProcessor();
    
  void createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs) override;
    
  /* This function is called by he host to populate the parameter vector */
  void createParameters(std::vector<ModuleParameter> &parameters) override;
    
  /*  This function is called by the host whenever a parameter changes */
  void hostParameterChanged(const std::string& parameterID, float newValue) override;
    
  /*  This function is called by the host before playback/performance */
  void prepareProcessor(int sr, std::size_t block) override;
    
  /* Call this method to trigger host callback */
  void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
  /* Called by the host when a note is started */
  void startNote(int midiNoteNumber, float velocity) override;
    
  /* Called by the host when a note is stoped */
  void stopNote (float velocity) override;
    
  /*  Main processing function called continuously by the host on the audio thread.
      paramValues is a list of parameter values passed from the host in order of their creation */
  void processSynthVoice(float** buffer, int numChannels, std::size_t blockSize) override;
    
  /* Is a synth */
  bool isSynth() override
  {
    return true;
  }

    ModuleType getModuleType() override
    {
        return ModuleType::synthProcessor;
    }

    std::string getModuleName() override
    {
        return "Grain Synth";
    }

	/* override this method if you want to draw to the Lattice generic editor viewport */
	std::string getSVGXml() override;

	/* override this method and return true if you wish to enable drawing on the generic editor viewport */
	bool canDraw() override { return true; }

	static float remap(float value, float from1, float to1, float from2, float to2)
	{
		return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
	}

	double getRMS(std::vector<float> const& v)
	{
		double sum = 0;
		auto const count = static_cast<float>(v.size());

		for (int i = 0; i < count; i++)
			sum += pow(v[i], 2);

		return sqrt(sum / count);
	}

 private:
  std::vector<float> wave;
  float att, dec, sus, rel;
  Aurora::GrainGen<float> grain;
  Aurora::Env<float> env;
  float amp;
  float siglevel;
  std::vector<float> am;
  std::vector<float> fm;
  float sr;
  float fac;
  bool isNoteOn = false;
  bool okToDraw = true;
  float rms = 0;
	
  std::vector<svg::Color> colours;
};
