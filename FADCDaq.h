#pragma once
#include <stdio.h>
#include "AgMD1.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <new>
#include <time.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "FADCSettings.hh"
#include "sstream"
#include "string.h"
#include "math.h"

#include </home/helix/DAQ_Software/include/AcqirisImport.h>
#include </home/helix/DAQ_Software/include/AcqirisD1Import.h>


using namespace std;



class FADCDaq
{


 public:
  // Con/de structors...
  // Come back to later
  FADCDaq();
  ~FADCDaq();
  
  
  // Read the settings
  void ConfigureFADCSettings(string FADCfilename);
  
  // Read/Write Waveform
  void ReadoutWaveform( ofstream& outFile );
  void AcquireWaveform();
  void BufferWaveform();
  
  void SetupBuffer(int nRuns);
  
  
  void PrintBuffer(int count = 10);
  void TakeData(int nruns);
  
  void FindDevices();
  void Configure(int nchan = 16);
  void WriteBufferToDisk(string filename );
  void WriteHeader( string filename );

  vector <vector <vector <float> > > dataBuffVec;
  
 private:
    
  
  typedef struct timeval TV ;
  typedef struct timezone TZ;
  
  // ### Simulation flag ###
  bool simulation; // Set to true to simulate digitizers (useful for application development)
  
  // ### Global variables ###
  ViSession *InstrumentID; // Array of instrument handles
  long NumInstruments; // Number of instruments
  long nbrChannels;  // Number of channels
  long nbrIntTrigs; 
  long nbrExtTrigs; 
  long nbrModules;
  int MaxSamples;
  int units;
  int channels;
  int channels_read;
  int runs;
  int runCurrent;
  string filename;
  bool debug;

  double rate;
  
  // Some constants that need to be defined
  int NMAX_SAMPLES;
  int NMAX_CHANNELS;
  int EXTRA;


  
  // For Buffering Data
  double ***dataBuff;
  
  // FADC Settings
  struct FADCSettings fFADCSettings;
  
  // Initialization Functions
  void MakeBuffer();
  
};
