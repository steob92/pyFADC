#include "FADCDaq.h"



void FADCDaq::FindDevices()
{
  
  cout << "FindDevices..." << endl;
  //TV *tStatusStart = new TV;
  //TV *tStatusStop = new TV;
  //TZ *tz = new TZ;
  //long d_usec;
  //long d_sec;
  
  // Find all digitizers
  ViString  options = "cal=0 dma=1";

  // The following call will automatically detect ASBus connections between digitizers and
  // combine connected digitizers (they must be the same model!) into multi-instruments.
  // The call returns the number of multi-instruments and/or single instruments.
  

  ViStatus status = AcqrsD1_multiInstrAutoDefine(options, &NumInstruments);

  cout << " Creating new VISession ..." << endl;
  if (InstrumentID) { delete InstrumentID;}

  InstrumentID = new ViSession[10];
  cout << " Done " << endl;

  
  status = Acqrs_InitWithOptions("PCI::INSTR0", VI_FALSE, VI_FALSE, options, &(InstrumentID[0]));
  if (debug){cout<<"Initialization Status"<<status<<endl;}
  
  //gettimeofday(tStatusStop,tz);
  
  
  if(status != VI_SUCCESS)
    {
      char message[256];
      Acqrs_errorMessage(InstrumentID[0], status, message, 256);
      cout << message << endl;
    }

  
  Acqrs_getNbrChannels(InstrumentID[0], &nbrChannels);
  
  Acqrs_getInstrumentInfo(InstrumentID[0], "NbrInternalTriggers", &nbrIntTrigs);
  Acqrs_getInstrumentInfo(InstrumentID[0], "NbrExternalTriggers", &nbrExtTrigs);
  //  Acqrs_getInstrumentInfo(InstrumentID[0], "NbrExternalTriggers", &nbrExtTrigs); // Does this need to run twice? SOB

  if (debug)
    {
      cout << "Instrument ID is" << InstrumentID[0] << endl;
      cout << "Number channels is: " << (int) nbrChannels << endl;
      cout << "nbrIntTrigs " << nbrIntTrigs << endl;
      cout << "nbrExtTrigs " << nbrExtTrigs << endl;
      cout << "I have found " << NumInstruments
	   << " Agilent - Acqiris Digitizer(s) on your PC" << endl;
    }
}



void FADCDaq::Configure(int nchan)
{
  // ### Configuration of the FADC settings ###      
  long nbrSegments = 1;
  //Coupling indicator is as follow:

  //1 - 1M DC
  //2 - 1M AC 
  //3 - 50 DC
  //4 - 50 AC 

  channels = nchan;
  channels_read = nchan;

  int j;
  // Configure timebase
  AcqrsD1_configHorizontal(InstrumentID[0], fFADCSettings.sampInterval, fFADCSettings.delayTime);
  AcqrsD1_configMemory(InstrumentID[0], fFADCSettings.nbrSamples, nbrSegments);	

  if (channels>0)
    {
      for (j=0; j < channels; j++)
	{
	  // Configure vertical settings of channel 1
	  AcqrsD1_configVertical(InstrumentID[0], j+1, fFADCSettings.fullScale, fFADCSettings.offset,fFADCSettings.channel_coupling,0.0);
	}
    }
  else if (channels<0 and channels>-17)
    {
      j=abs(channels);
      AcqrsD1_configVertical(InstrumentID[0], j, fFADCSettings.fullScale, fFADCSettings.offset,fFADCSettings.channel_coupling,0.0);
    }
  else
    {
      cout << "Incorrect Channel Usage. (+)ve for collecting from all channels between 1 and n, (-)ve for that single CCD" << endl;
      cout << "Channels = " << channels << endl;
    }
  
  //  // ////To use a channel other than channel 1 to trigger off of for a single trigger
  //      AcqrsD1_configVertical(InstrumentID[0], channel_number, fFADCSettings.fullScale, fFADCSettings.offset,fFADCSettings.channel_coupling,0.0);
	    
  // Configure edge trigger on triggerx800...0
  AcqrsD1_configTrigClass(InstrumentID[0], 0, 0x80000000, 0, 0, 0.0, 0.0);
  // Configure the trigger conditions of channel 1 internal trigger (-1,triglevelmv)
  
  //  [status infoValue] = Acqrs_getInstrumentInfo(InstrumentID[0],"ASBus_2_PosInCrate",double);
  //  cout<<infoValue<<endl;

  //Trigscoupling is now defined HERE as 3.0 (DC 50 ohm)
  //TrigSlope is set to positive (0)
  //*******************************************//
  AcqrsD1_configTrigSource(InstrumentID[0], -1,0.0,1, fFADCSettings.trigLevelmv, 0.0);
  //*******************************************//
  ////If you want to use a trigger other than ext. 1, use the following trigsource and comment out the previous one
  // int trigsource_number = 1;
  // AcqrsD1_configTrigSource(InstrumentID[0], trigsource_number,0.0,1, fFADCSettings.trigLevelmv, 0.0);


  cout << "Configuring FADCs..." << endl
       << "Number of Channels: " << channels << endl
       << "Instrument ID: " << InstrumentID[0] << endl
       << "Trigger Level: " << fFADCSettings.trigLevelmv << endl
       << "Full Scale: " << fFADCSettings.fullScale << endl
       << "Offset: " << fFADCSettings.offset << endl
       << "Channel Coupling: " <<  fFADCSettings.channel_coupling << endl;
}




void FADCDaq::AcquireWaveform()
{
  // ### Acquiring a waveform ###
  ViBoolean done = 0;
   
  AcqrsD1_acquire(InstrumentID[0]); // Start the acquisition
   
  int count = 0;
  while (!done) //&& ...
    {
      //    cout << "\t\tNot Done... " << InstrumentID[0] << " " << done <<  endl;
      AcqrsD1_acqDone(InstrumentID[0], &done); // Poll for the end of the acquisition

      // High value to catch a break...
      if (count > 1234567890)
	{
	  cout << "Unsuccessful..." << endl;
	  break;
	}
      count++;
    }	
}


void FADCDaq::TakeData(int nruns)
{
  // Timing for rate
  clock_t tick = clock();
  
  int i=0;
  runCurrent = 0;
  while(i<nruns)
    {
      
      //	cout << "Run : " << i << endl;
      //Order the FADC to acquire a trace
      //cout << "\tAcquire..." << endl;
      AcquireWaveform();
      //Readout the trace acquired and write it to file
      //ReadoutWaveform(outFile, myset);
      
      //cout << "\tBuffer..." << endl;
      BufferWaveform();
      //cout << "\tNext!..." << endl;
      runCurrent++;
      i++;
      
      //if (runCurrent >= runs)
      //{
      //  cout << "Larger that buffer size..." << endl;
      //  break;
      //}
    }
  clock_t tock = clock();
  rate = 1. / (double(tock - tick) / CLOCKS_PER_SEC / nruns);
}



// ### Readout the data in ASCII format into output file ###

void FADCDaq::ReadoutWaveform(std::ofstream& outFile)
{
  // ### Readout the data ###
    //channels refers to the user input parameter of the ReadInFADC main function
    //int nchannel = channels;
    long nbrSegments = 1;

    double streamlined_waveform[NMAX_CHANNELS][NMAX_SAMPLES+EXTRA];

//    vector <double *> waveFormArray_Vector = vector<double *>(nchannel);
   // long extra

    AqReadParameters		readParams;
    AqDataDescriptor		wfDesc; //contains waveform information that is common to all segments
    AqSegmentDescriptor		segDesc[nbrSegments]; //Describes te data structure of every segment

	
    readParams.dataType			= 3.0; //(8-bytes resolution waveform);
    readParams.readMode			= 0.0; //(Standard acquisition in one segment) ReadModeStdW;
    readParams.nbrSegments		= nbrSegments;
    readParams.firstSampleInSeg		= 0;
    readParams.firstSegment		= 0;
    readParams.segmentOffset		= 0;
    readParams.segDescArraySize		= (double)sizeof(AqSegmentDescriptor) * nbrSegments;
    readParams.nbrSamplesInSeg		= fFADCSettings.nbrSamples;
    readParams.dataArraySize		= (sizeof(double)*(readParams.nbrSamplesInSeg + EXTRA)  * (nbrSegments + 1) ); // This is the formula implemented inside the driver
    
    //nchannel corresponds to a number between 1 and 16.
    //Instead of creating a vector containing the channel number, use directly the iterator value in the function
     if (channels>0){
	     for (int j=0; j < channels; j++){
	     	//Read the waveform recorded by the FADC for every channel
	     	AcqrsD1_readData(InstrumentID[0], (j+1), &readParams, streamlined_waveform[j], &wfDesc, &segDesc);
	     }
     } else if (channels<0 and channels>-17){
		int j=abs(channels);
		AcqrsD1_readData(InstrumentID[0], (j), &readParams, streamlined_waveform[0], &wfDesc, &segDesc);
     } else {
	cout << "Incorrect Channel Usage. (+)ve for collecting from all channels between 1 and n, (-)ve for that single CCD" << endl;
  }

    //    ////For tests that involve reading a single but not starting with channel 1 (ex. only want data from channel 5 or any other) uncomment the following and comment out the previous loop 

    //   AcqrsD1_readData(InstrumentID[0], channel_number, &readParams, streamlined_waveform[0], &wfDesc, &segDesc);

    //returnedSamplesPerSeg gives the actual nuber of data points recorded for one trace
    long j=0; int i=0; int ch=0;
    while(j < wfDesc.returnedSamplesPerSeg){
        i=0;
	if (channels<0){
		ch=1;
	} else if (channels>0){
		ch=channels;
	}
	//The jth data point of the traces is written down for every recording channel
	while(i < ch){
		outFile<<streamlined_waveform[i][j];
	    //outFile << waveFormArray_Vector.at(i)[j];
	    if (i != (channels-1)) outFile << "\t";
            i++;
	}
	outFile << endl;
    j++;
    }

}

// ### Readout the data in ASCII format into output file ###

void FADCDaq::BufferWaveform()
{
  // ### Readout the data ###
  //channels refers to the user input parameter of the ReadInFADC main function
  //int nchannel = channels;
  long nbrSegments = 1;
  
  double streamlined_waveform[NMAX_CHANNELS][NMAX_SAMPLES+EXTRA];
  
  //    vector <double *> waveFormArray_Vector = vector<double *>(nchannel);
  // long extra
  
  AqReadParameters		readParams;
  AqDataDescriptor		wfDesc; //contains waveform information that is common to all segments
  AqSegmentDescriptor		segDesc[nbrSegments]; //Describes te data structure of every segment
  
  
  readParams.dataType			= 3.0; //(8-bytes resolution waveform);
  readParams.readMode			= 0.0; //(Standard acquisition in one segment) ReadModeStdW;
  readParams.nbrSegments		= nbrSegments;
  readParams.firstSampleInSeg		= 0;
  readParams.firstSegment		= 0;
  readParams.segmentOffset		= 0;
  readParams.segDescArraySize		= (double)sizeof(AqSegmentDescriptor) * nbrSegments;
  readParams.nbrSamplesInSeg		= fFADCSettings.nbrSamples;
  readParams.dataArraySize		= (sizeof(double)*(readParams.nbrSamplesInSeg + EXTRA)  * (nbrSegments + 1) ); // This is the formula implemented inside the driver
    
  //nchannel corresponds to a number between 1 and 16.
  //Instead of creating a vector containing the channel number, use directly the iterator value in the function
  if (channels>0)
    {
      for (int j=0; j < channels; j++)
	{
	  //Read the waveform recorded by the FADC for every channel
	  AcqrsD1_readData(InstrumentID[0], (j+1), &readParams, streamlined_waveform[j], &wfDesc, &segDesc);
	}
    }

  else if (channels<0 and channels>-17)
    {
      int j=abs(channels);
      AcqrsD1_readData(InstrumentID[0], (j), &readParams, streamlined_waveform[0], &wfDesc, &segDesc);
    }
  else
    {
      cout << "Incorrect Channel Usage. (+)ve for collecting from all channels between 1 and n, (-)ve for that single CCD" << endl;
    }
  
  //    ////For tests that involve reading a single but not starting with channel 1 (ex. only want data from channel 5 or any other) uncomment the following and comment out the previous loop 
  
  //   AcqrsD1_readData(InstrumentID[0], channel_number, &readParams, streamlined_waveform[0], &wfDesc, &segDesc);
  
  //returnedSamplesPerSeg gives the actual nuber of data points recorded for one trace
  long j=0; int i=0; int ch=0;

  //cout << "Buffering to dataBuff" << endl;
  //cout << wfDesc.returnedSamplesPerSeg << " " << channels << " " << runCurrent << endl;
  while(j < wfDesc.returnedSamplesPerSeg)
    {
      i=0;
      if (channels<0)
	{
	  ch=1;
	}
      else if (channels>0)
	{
	  ch=channels;
	}
      //The jth data point of the traces is written down for every recording channel
      while(i < ch)
	{
	  //outFile<<streamlined_waveform[i][j];
	  //outFile << waveFormArray_Vector.at(i)[j];
	  //if (i != (channels-1)) outFile << "\t";
	  // Writing to the data buffer
	  dataBuffVec[runCurrent][i][j] = streamlined_waveform[i][j];
	  //dataBuffV[runCurrent][i][j] = streamlined_waveform[i][j];
	  i++;
	}
      
      //outFile << endl;
      j++;
    }
  //cout << " Done!" << endl;
  
}



void FADCDaq::ConfigureFADCSettings(string FADCfilename){

    //variables for reading the user settings file!
    ifstream infile;
    string buf;
    int firstline = 1;
    string tempstr;  

    //open header file and read variables into headersettings class 
    infile.open(FADCfilename.c_str()); 
  
    if(!infile.is_open())
      {

	cout << "Problem opening " << FADCfilename << endl;

      }
    else
      {

	while(getline(infile, buf))
	  {
	  
	    //if (buf[0] == '#') int nothing;// cout << "found header. skipping comments" << endl;
	    if (buf[0] == '#') {continue;}// cout << "found header. skipping comments" << endl;

	    else
	      {
		istringstream bufstream(buf);

		while(!bufstream.eof())
		  {
		    
		    if (firstline == 1)
		      {
		       
			bufstream >> fFADCSettings.nbrSamples >> fFADCSettings.channel_coupling >> fFADCSettings.delayTime >> fFADCSettings.fullScale >> fFADCSettings.trigLevelmv >> fFADCSettings.sampInterval >> fFADCSettings.offset;
		        if (debug)
			  {
			    cout<<"****************************************"<<endl;
			    cout<<"Number of samples = "<<fFADCSettings.nbrSamples<<endl;
			    cout<<"Channel coupling = "<<fFADCSettings.channel_coupling<<endl;
			    cout<<"Delay time = "<<fFADCSettings.delayTime<<endl;
			    cout<<"Voltage range (fullscale) = "<<fFADCSettings.fullScale<<endl;
			    cout<<"Trigger level = "<<fFADCSettings.trigLevelmv<<" mV"<<endl;
			    cout<<"Time resolution = "<<fFADCSettings.sampInterval<<" s"<<endl;
			    cout<<"Vertical offset = "<<fFADCSettings.offset << " V"<<endl;
			  }
			
			firstline = 0;
		      }
		    
		    bufstream >> tempstr;
		    //cout << tempstr << endl;
		  }
	      }
	  }
      }
    
    
    //cout << "Looking for Devices..." << endl;
    FindDevices();
    //cout << "...Done!" << endl;	
		
}




FADCDaq::FADCDaq()
{

  NMAX_SAMPLES = 50000;
  NMAX_CHANNELS = 16;
  EXTRA = 270;
  InstrumentID = 0;
  simulation = false;
  debug = 1;
  dataBuff = 0;
  runCurrent = 0;
  //cout << "Looking for Devices..." << endl;
  //FindDevices();
  //cout << "...Done!" << endl;	
  
}

FADCDaq::~FADCDaq()
{

  Acqrs_closeAll(); // ### Exit Digitizer gracefully ##

  
  if (InstrumentID) {delete InstrumentID;}
  dataBuffVec.clear();
  //if (dataBuff){delete dataBuff;}
}




// Define the shape of the data buffer
// To whom it may concern:
/*
      What we want to do is to remove the overhead associated with writing each run to disk.
      To do this we want to store the read values in a buffer.
      The buffer with have to be sufficient size to store all the data.
      We also want to have the program flexible so that it is agnostic to number of channels being read
      and number of runs.
      Hence we need to dynamically assign the size of the buffer at run time, not at compile time.
      
      To achieve this dataBuff is a pointer to an array of pointers to an array of pointers....
      - dataBuff points to an array of points (lets call it a1) this is length runs
      - Each of the entries in a1 then points to an array of pointers (lets call this a2) of length channels
      - Each of the entries in a2 then point to an array of doubles of length nbrSamples.

      The net results is a 3D array of size (runs, channels, nbrSamples) such that:
        dataBuff[12][4][100] will be the value of sample 100 in channel 4 (really ch 5 as n = 0,1,...) taken during run 12.

*/

void FADCDaq::MakeBuffer()
{
  //if (dataBuff){delete dataBuff;}
  //dataBuff = new double**[runs];

  // Single Channel Acquisition
  if (channels < 0)
    {
      dataBuffVec.resize( runs, vector <vector <float> > (1, vector <float> (fFADCSettings.nbrSamples) ) ); 
    }
  else
    {
      dataBuffVec.resize( runs, vector <vector <float> > (channels, vector <float> (fFADCSettings.nbrSamples) ) ); 
    }


  //cout << "Vector Buffer size: " << dataBuffVec.size() << " " << dataBuffVec[0].size() << " " << dataBuffVec[0][0].size() << endl; 
  //nbrSamples = myset.nbrSamples;

  //  cout << "Creating data buffer" << endl
  //   << "Number of Runs: " << runs << endl
  //   << "Number of Channels: " << channels << endl
  //   << "Number of samples: " << fFADCSettings.nbrSamples << endl;

  /*
  for (int i_runs = 0; i_runs < runs; i_runs++)
    {
      if (channels > 0)
	{
	  dataBuff[i_runs] = new double *[channels];
	  channels_read = channels;
	}
      else 
	{
	  dataBuff[i_runs] = new double *[1];
	  channels_read = 1;
        }
      for ( int i_channels  = 0; i_channels < channels_read; i_channels ++)
	{
	  dataBuff[i_runs][i_channels] = new double [fFADCSettings.nbrSamples];
	}
    }
  */
}

void FADCDaq::SetupBuffer( int nRuns)
{
  // Worry about error messages later...
  
  runs = nRuns;
  
  MakeBuffer();
  
  
}


void FADCDaq::PrintBuffer(int count )
{
  for (int i = 0 ; i < count; i ++)
    {
      for (int  j = 0 ; j < channels; j++)
	{
	  
	  cout << dataBuffVec[0][i][j] << " " ;
	}
      cout << endl;
      //	  dataBuff[runCurrent][i][j]
      
    } 
}






void FADCDaq::WriteBufferToDisk(string filename )
{

  ofstream outFile(filename.c_str(),ios::binary);
  
  for (int i = 0; i < runs; i++)
    {
      for (int k = 0; k < fFADCSettings.nbrSamples;  k++)
	{
	  for (int j = 0; j < channels_read; j++)
	    {
	      float out  = dataBuffVec[i][j][k];
	      outFile.write((char*)&out,sizeof(float));
	    } 
	}
    }
  outFile.close();
}




void FADCDaq::WriteHeader( string filename )
{
  ofstream outFile(filename.c_str());
  outFile << "#timediv\tevents\tnsamp\tAcq.rate\tCHcoupling\tOffset\tfullscale" << endl;
  outFile << fFADCSettings.sampInterval << "\t"
	  << runs << "\t"
	  << fFADCSettings.nbrSamples << "\t"
	  << fFADCSettings.sampInterval << "\t"
	  << rate << "\t"
	  << fFADCSettings.channel_coupling << "\t"
	  << fFADCSettings.offset << "\t"
	  << fFADCSettings.fullScale << endl;
  outFile.close();
}
