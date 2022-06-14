
#include "TWProcessor.h"
#include <iterator>
#include <sstream>


TWProcessor::TWProcessor()
	:LatticeProcessorModule()
{

}

LatticeProcessorModule::ChannelData TWProcessor::createChannels()
{
	addChannel({ "output", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData TWProcessor::createParameters()
{
    addParameter({"16'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    addParameter({"5 1/3'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    addParameter({"8'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    addParameter({"4'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    addParameter({"2 2/3'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    addParameter({"2'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    addParameter({"1 5/3'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    addParameter({"1 1/3'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    addParameter({"1'", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.f, 0.001f, 1.f)});
    return {getParameters(), getNumberOfParameters()};
}

void TWProcessor::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
}



void TWProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    std::size_t n = 0;
    const float g[] = {getParameter("16'"),
		       getParameter("5 1/3'"),
		       getParameter("8'"),
		       getParameter("4'"),
		       getParameter("2 2/3'"),
		       getParameter("2'"),
		       getParameter("1 3/5'"),
		       getParameter("1 1/3'"),
		       getParameter("1'")};
    float scal = 1./(12.+g[0]+g[1]+g[2]+g[3]+g[4]+g[5]+g[6]+g[7]+g[8]);
    std::fill(buffer[0],buffer[0]+blockSize,0);
    tg(blockSize);
    for(auto key: keys) {
      if(key) {	
	for(std::size_t j = 0; j < blockSize; j++) {
	  // lower foldback
	   buffer[0][j] += (n - 12 >= 12) ?
	     tg.wheel(n-12)[j]*g[0] : tg.wheel(n)[j]*g[0];
	   buffer[0][j] += tg.wheel(n)[j]*g[2] + tg.wheel(n+7)[j]*g[1] +
	    tg.wheel(n+12)[j]*g[3];
	   // top foldbacks
	   buffer[0][j] += (n + 19 < 91) ? tg.wheel(n+19)[j]*g[4] :
	     tg.wheel(n+7)[j]*g[4];
           buffer[0][j] += (n + 24 < 91) ? tg.wheel(n+24)[j]*g[5] :
	     tg.wheel(n+12)[j]*g[5];
           buffer[0][j] += (n + 28 < 91) ? tg.wheel(n+28)[j]*g[6] :
	     tg.wheel(n+16)[j]*g[6];
           buffer[0][j] += (n + 31 < 91) ? tg.wheel(n+31)[j]*g[7] :
	     ((n + 19 < 91) ? tg.wheel(n+19)[j]*g[7] :
	      tg.wheel(n+7)[j]*g[7]);
           buffer[0][j] += (n + 48 < 91) ? tg.wheel(n+48)[j]*g[8] :
	     ((n + 24 < 91) ? tg.wheel(n+24)[j]*g[8] :
	      tg.wheel(n+12)[j]*g[8]);
	}
      }
      n++;
    }
    for(std::size_t j = 0; j < blockSize; j++) buffer[0][j++] *= scal;
    
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new TWProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new TWProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
