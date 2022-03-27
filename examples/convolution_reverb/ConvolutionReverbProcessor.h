#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "Conv.h"
#include "simple_svg_1.0.0.hpp"

 using namespace Aurora;
template <typename S>
inline S sum(S a, S b) { return a + b; } 
class ConvolutionReverbProcessor : public LatticeProcessorModule
{
 



template <typename S>
struct ConvReverb {
   IR<S>   ir1;
   IR<S>   ir2;  
   IR<S>   ir3;
   Conv<S> c1;
   Conv<S> c2;
   Conv<S> c3;
   BinOp<S, sum> mix;
 

  ConvReverb(const std::vector<S> &s1,
	     const std::vector<S> &s2,
	     const std::vector<S> &s3) :
    ir1(s1,32), ir2(s2,256), ir3(s3,4096),
    c1(&ir1), c2(&ir2), c3(&ir3) { }

  void reset(const std::vector<S> &s1,
	     const std::vector<S> &s2,
	     const std::vector<S> &s3) {
    ir1.reset(s1,32);
      ir2.reset(s2,256);
        ir3.reset(s3,4096);
  }

  const std::vector<S> &operator()(const std::vector<S> &in, S g) {
    return mix(mix(c1(in,g*0.3),c2(in,g*0.3)),c3(in,g*0.3));
  }

};

template <typename S>
ConvReverb<S> create_reverb(std::vector<S> &imp) {
  if(imp.size() < 8192)
    imp.resize(8192);
  std::vector<S> s1(imp.begin()+32, imp.begin()+256);
  std::vector<S> s2(imp.begin()+256, imp.begin()+4096);
  std::vector<S> s3(imp.begin()+4096, imp.end());
  return ConvReverb(s1,s2,s3);
}

template <typename S>
void reset_reverb(ConvReverb<S> &rev, std::vector<S> &imp) {
  if(imp.size() < 8192)
    imp.resize(8192);
  std::vector<S> s1(imp.begin()+32, imp.begin()+256);
  std::vector<S> s2(imp.begin()+256, imp.begin()+4096);
  std::vector<S> s3(imp.begin()+4096, imp.end());
  rev.reset(s1,s2,s3);
}



  
public:
    ConvolutionReverbProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    void hostParameterChanged(const char* parameterID, const char* newValue) override;
    void hostParameterChanged(const char* parameterID, float newValue) override;
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    void createDescription(std::string& description)
    {
        description = "";
    }

    /* override this method if you want to draw to the Lattice generic editor viewport */
    const char* getSVGXml() override;

    /* override this method and return true if you wish to enable drawing on the generic editor viewport */
    bool canDraw() override {
        auto draw = okToDraw;
        okToDraw = false;
        return draw;
    }

    static float remap(float value, float from1, float to1, float from2, float to2)
    {
        return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
    }
    
    const char* getModuleName() override {    return "Convolution Reverb";     }
    
private:
    std::vector<float> irTable;
    bool okToDraw = true;
    std::string svgText;
    bool isPlaying = false;
    int sampleIndex = 0;
    ConvReverb<float> delay;
    Mix<float> mix;
    std::vector<float> inL, inR;
    bool bypass = false;
    
};

