# distutils: language = c++
# distutils: sources = FADCDaq.h, FADCDaq.cpp
# distutils: include_dirs = ./


from FADCDaq cimport FADCDaq


import numpy as np
from astropy.io import fits
from libcpp.vector cimport vector
import threading

# CCD Class
cdef class pyFADC:
    

    cdef FADCDaq _fadcdaq  # Hold a C++ instance which we're wrapping
    cdef nchannels
    cdef nruns
    cdef nsample
    
    # C++ initialization
    def __cinit__(self):
        

        #print ("Python Init")
        self._fadcdaq = FADCDaq()
        self.nchannels = -999
        self.nruns = -999
        self.nsample = -999
        #print ("Python Done")


	# Read/Write status for use with threading...
	self.readStatus = 0
	self.writeStatus = 0

	self.dataThread = None
	self.writeThread = None

    # Read FADC Settings
    def ReadFADCSettings(self, fsettings):
        self._fadcdaq.ConfigureFADCSettings(fsettings)


    # Configure for data taking
    def Configure(self, nchannels = 16):
        self.nchannels = nchannels
        cdef int nc = nchannels
        self._fadcdaq.Configure(nc)

        
    # Set up the Buffer
    def ConfigBuffer(self, nruns):

        self.nruns = nruns
        cdef int nr = nruns
        self._fadcdaq.SetupBuffer(nr   )


    # Thread-ed function to take data
    def ThreadTakeData(self, nruns = None):
    	 self.dataThread = threading.Thread(target=self.TakeData, args=nruns)
	 self.readStatus = 0
	 self.dataThread.start()
	 

    # Data taking command
    def TakeData(self, nruns = None):

        cdef int nr = 0
        if (nruns == None) and ( self.nruns != -999):
            nr = self.nruns
            self._fadcdaq.TakeData( nr)
	    self.readStatus = 1

        elif (nruns != None) and (nruns <= self.nruns):
            nr = nruns
            self._fadcdaq.TakeData( nr )
	    self.readStatus = 1

        else :
            print ("Error number of runs requested is invalid:\n\t%d" %(nruns))
	    self.readStatus = -1

    def GetBufferData(self):
        runs = self._fadcdaq.dataBuffVec.size()
        chan = self._fadcdaq.dataBuffVec[0].size()
        samp = self._fadcdaq.dataBuffVec[0][0].size()

        dataArray = np.asarray(self._fadcdaq.dataBuffVec)
        return dataArray


    # Thread-ed function to writing data to buffer
    def WriteBufferThread(self, filename):
    	self.writeThread = threading.Thread(target=self.WriteBuffer, args=filename)
	self.writeStatus = 0
	self.writeThread.start()

    # Writing data to buffer
    def WriteBuffer(self, filename):
        self._fadcdaq.WriteBufferToDisk(filename)
        self.writeStatus = 1


    def WriteHeader(self, filename):
        self._fadcdaq.WriteHeader(filename)
            

    # Rotate CCD by CCD angle
    def RotateCCD(self, R, ccdID):
        deltaAngle = 360. / 16.0;
        angle = float(ccdID) * deltaAngle + 1.e-9 - 90
        angle = np.deg2rad(angle)
        # Minus because clockwise
        xcomp = -np.cos(angle)
        ycomp = np.sin(angle)
        return R*xcomp, R*ycomp

                                

    def WriteCCDFits(self, filename):

        data = self.GetBufferData()
        nImages, nCCD, nPixels = np.shape(data)

        fCCDSize = 8e-6 # Size in m
        fCCDOffset = 21.4772e-2 # Size in m
        
        # X-Y coordinates of pixels
        CCD_locations = np.zeros((nPixels, nCCD, 2))
        
        # Offseting and flipping the CCD order (outside-in)
        R = fCCDOffset - np.arange(nPixels) * fCCDSize
        
        for i in range(16):
            x, y = self.RotateCCD(R, i)
            # for j in range(nPixels):
            #     CCD_locations[j][i][0] = x[j]
            #     CCD_locations[j][i][1] = y[j]
            CCD_locations[:,i,0] = x
            CCD_locations[:,i,1] = y

        # Setting Adding images to array of hdu
        hdu_arr = [0]*nImages
        for i in range(nImages):
            data_store = np.transpose(data[i])
            hdu_arr[i] = fits.ImageHDU(data_store)
            
        # Saving the CCD locations as the primary hdu
        hdu_p = fits.PrimaryHDU(CCD_locations)
        hdu_arr = [hdu_p] + hdu_arr
        
        # Create list of hdu
        hdul = fits.HDUList(hdu_arr)
        # Assign image name
        for i in range(len(hdul)):
            if (i == 0):
                hdul[i].name = "CCD Coordinates"
            else :
                hdul[i].name = "Image %d"%i
        

        hdul.writeto(filename, overwrite = True)
