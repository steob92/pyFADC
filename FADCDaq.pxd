from libcpp.vector cimport vector
from libcpp cimport bool
from libcpp.string cimport string

from FADCDaq cimport FADCDaq

cdef extern from "FADCDaq.cpp":
    pass



    # Declare the class with cdef
    cdef extern from "FADCDaq.h":
        cdef cppclass FADCDaq:

            # Constructor/dectructor
            FADCDaq () except +


            # Configure FADC Settings
            void ConfigureFADCSettings(string FADCfilename)
            
            void AcquireWaveform()
            void BufferWaveform()
            
            void SetupBuffer(int nRuns)
            
        
            void PrintBuffer(int count)
            void TakeData(int nruns)
            
            void FindDevices();
            void Configure(int nchan)
            void WriteBufferToDisk(string filename )
            void WriteHeader( string filename )
            vector [vector [vector [float] ] ] dataBuffVec
