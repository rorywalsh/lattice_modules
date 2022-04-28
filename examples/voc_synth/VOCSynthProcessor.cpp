
#include "VOCSynthProcessor.h"
#include <iterator>



//======================================================================================
VOCSynthProcessor::VOCSynthProcessor() :
  win(Aurora::def_fftsize), anal(win,128), syn(win,128), ptrack(200), ceps(win.size()),
  in(Aurora::def_vsize), ftmp(win.size()/2 + 1), buf(win.size()/2 + 1), note_on(false)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}

LatticeProcessorModule::ChannelData VOCSynthProcessor::createChannels()
{
  addChannel({"input", ChannelType::input });
  addChannel({"ouput", ChannelType::output });
  return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData VOCSynthProcessor::createParameters()
{
  addParameter({ "Extract Formants", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
  return ParameterData(getParameters(), getNumberOfParameters());
}



void VOCSynthProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
  anal.reset(sr);
  syn.reset(sr);
  fs = sr;
}

void VOCSynthProcessor::startNote(int midiNoteNumber, float velocity )
{
  setMidiNoteNumber(midiNoteNumber);
  note_on = true;
}

void VOCSynthProcessor::stopNote (float /* velocity */)
{
  note_on = false;
}

void VOCSynthProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
  updateParameter(parameterID, newValue);
}

void VOCSynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
  //if(!note_on) {
	//std::fill(buffer[0], buffer[0]+blockSize, 0);
  //return;
  //} 
  
  const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
  in.resize(blockSize);
  syn.vsize(blockSize);
  float check;
    
  for(std::size_t i = 0; i < blockSize ; i++) { 
    in[i] = buffer[0][i];
    check += fabs(in[i]);
  }

  // auto &spec = anal(in);
  // if(anal.framecount() > framecount) {
  //   auto size = anal.size();  
  //   std::size_t n = 0;
  //   bool preserve = getParameter("Extract Formants");
  //   float scl = 1;
  //   std::fill(buf.begin(),buf.end(),Aurora::specdata<float>(0,0));
    
  //   if (preserve) {
  //     if(check) {
  // 	auto &senv = ceps(spec, 30);
  // 	float max = 0;
  // 	for(auto &m : senv) 
  // 	  if(m > max) max = m;
  // 	for(auto &amp : ftmp) {
  // 	  float cf = n/float(size);
  // 	  amp = spec[n].amp();
  // 	  if(senv[n] > 0)	
  // 	    amp *= max/senv[n++];
  // 	}
  //     } else preserve = false;
  //   }
    
  //   n = 0;	
  //   for(auto &bin : spec) {
  //     int k = round(scl*n);
  //     auto &senv = ceps.vector();
  //     if(k > 0  && k < spec.size()) {
  // 	preserve == false ? buf[k].amp(bin.amp())
  // 	  :  buf[k].amp(ftmp[n]*senv[k]);
  // 	buf[k].freq(bin.freq()*scl);
  //     }	
  //     n++;
  //   }
  //   framecount = anal.framecount();
  // }
  //auto &s = syn(spec);
  std::copy(in.begin(), in.end(), buffer[0]);

}


// the class factories
#ifdef WIN32
extern "C" 
{
  __declspec(dllexport) LatticeProcessorModule* create() { return new VOCSynthProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new VOCSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
