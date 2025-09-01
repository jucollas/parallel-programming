#ifndef PROCESSOR_CLASS
#define PROCESSOR_CLASS

#include <string>

#include "engine.h"

class Processor{
  private:
    std::string input_file;
    std::string output_file; 
    std::string filter;
    Engine* engine;

  public:
    Processor(std::string input_file, std::string output_file, std::string filter = "", Engine* engine = nullptr);
    ~Processor();
    void execute();

};

#endif