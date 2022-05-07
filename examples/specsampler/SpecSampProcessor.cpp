
#include "SpecSampProcessor.h"
#include <iterator>

static const int dm = 8;

//======================================================================================
SpecSampProcessor::SpecSampProcessor() :
  win(Aurora::def_fftsize), anal(win,win.size()/dm), syn(win,win.size()/dm),
  shift(Aurora::def_sr,win.size()),
  samp(1, std::vector<Aurora::specdata<float>>(win.size())), att(0.1f), dec(0.1f),
  sus(1.f), rel(0.1f), env(att,dec,sus,rel), hcnt(anal.hsize())
{
  std::size_t n = 0;
  for(auto &s : win)
  s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}

LatticeProcessorModule::ChannelData SpecSampProcessor::createChannels()
{
  addChannel({"input", ChannelType::input });
  addChannel({"ouput", ChannelType::output });
  return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData SpecSampProcessor::createParameters()
{
  addParameter({ "Base Note", {0, 127, 60, 1, 1}});
  addParameter({ "Attack", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Decay", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Sustain", {0, 1., 1., 0.001, 1}});
  addParameter({ "Release", {0, 1., 0.1, 0.001, 1}});
  addParameter({ "Keep Formants", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
  addParameter({ "Load Sample", {0, 1, 0, 1, 1}, Parameter::Type::FileButton});
  return ParameterData(getParameters(), getNumberOfParameters());
}

void SpecSampProcessor::hostParameterChanged(const char* parameterID,
						  const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Load Sample")
    {
        std::cout << "File to load" << newValue << std::endl;
        auto samples = getSamplesFromFile(newValue);
	//std::cout << samples.numSamples << std::endl;
	if(samples.numSamples > 0) {
	  samp.resize(samples.numSamples/anal.hsize());
	  std::cout << "frames: " << samp.size() << " : " << samples.numSamples
		  << " : " << anal.hsize() << std::endl;
	  std::size_t n = 0;
	  std::vector<float> lfr(anal.hsize());
	  for(auto &frame : samp) {
	    std::copy(samples.data[0] + n,
		      samples.data[0] + n + anal.hsize(),
		      lfr.begin());
	    frame = anal(lfr);
            n += anal.hsize();
	  }
	  std::cout << "samples: " << n << "\n";
	  if(n < samples.numSamples) {
	    std::fill(lfr.begin(), lfr.end(), 0);
	    std::copy(samples.data[0] + n,
	    	      samples.data[0] + samples.numSamples,
	        lfr.begin()); 
	    samp.push_back(anal(lfr));
	    } 
	  } 
	}
}

void SpecSampProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
  anal.reset(sr);
  syn.reset(sr);
  shift.reset(sr);
  fs = sr;
}

void SpecSampProcessor::startNote(int midiNoteNumber, float velocity )
{
  setMidiNoteNumber(midiNoteNumber);
  const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
  att = getParameter("Attack");
  dec = getParameter("Decay");
  note_on = true;
}

void SpecSampProcessor::stopNote (float /* velocity */)
{
  note_on = false;
  env.release(getParameter("Release"));
}

void SpecSampProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
  updateParameter(parameterID, newValue);
}

void SpecSampProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
  syn.vsize(blockSize);
  sus = getParameter("Sustain");
  if(hcnt >= anal.hsize()) {
    const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
    const float base = getMidiNoteInHertz(getParameter("Base Note"), 440);
    shift.lock_formants(getParameter("Keep Formants"));
    shift(samp[rp++],freq/base);
    rp = rp != samp.size() ? rp : 0; 
    hcnt -= anal.hsize();
  }
  auto &s = syn(shift.frame());
  auto &e = env(s,note_on);
  std::copy(e.begin(),e.end(), buffer[0]);
  hcnt += blockSize;
}


// the class factories
#ifdef WIN32
extern "C" 
{
  __declspec(dllexport) LatticeProcessorModule* create() { return new SpecSampProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new SpecSampProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
