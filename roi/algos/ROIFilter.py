from Centella.AAlgo import AAlgo
from Centella.physical_constants import *

"""
This algorithm filters events which energy are out of the ROI
It also generates the PDF & ROI pdfs
Parameters:
   pdfMinE -> PDF Minimum  Energy
   pdfMaxE -> PDF Maximum  Energy
   ROIMinE -> ROI Minimum  Energy
   ROIMaxE -> ROI Maximum  Energy
"""


class ROIFilter(AAlgo):

	############################################################
	def __init__(self, param=False, level=1, label="", **kargs):

		"""
		ROI Filter Algorithm
		"""
		#self.m.log(1, 'Constructor()')

		### GENERAL STUFF
		self.name = 'ROIFilter'
		#self.level = level
		AAlgo.__init__(self, param, level, self.name, 0, label, kargs)

    ### PARAMETERS
    # PDF Minimum Energy
		try:
			self.pdfMinE = self.doubles['pdfMinE']
			self.m.log(1, "PDF Minimum Energy: %.3f MeV." %(self.pdfMinE/MeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'pdfMinE' not defined.")
			exit(0)

    # PDF Maximum Energy
		try:
			self.pdfMaxE = self.doubles['pdfMaxE']
			self.m.log(1, "PDF Maximum Energy: %.3f MeV." %(self.pdfMaxE/MeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'pdfMaxE' not defined.")
			exit(0)

    # ROI Minimum Energy
		try:
			self.roiMinE = self.doubles['roiMinE']
			self.m.log(1, "ROI Minimum Energy: %.3f MeV." %(self.roiMinE/MeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'roiMinE' not defined.")
			exit(0)

    # ROI Maximum Energy
		try:
			self.roiMaxE = self.doubles['roiMaxE']
			self.m.log(1, "ROI Maximum Energy: %.3f MeV." %(self.roiMaxE/MeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'roiMaxE' not defined.")
			exit(0)



	############################################################		
	def initialize(self):

		self.m.log(1, 'Initialize()')
		
		### Defining histos
		# PDF Histogram 
		histo_name = self.alabel("PDF")
		histo_desc = "PDF"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc,
		             int(self.pdfMaxE/KeV - self.pdfMinE/KeV),
		             self.pdfMinE, self.pdfMaxE)
		self.hman.fetch(self.alabel("PDF")).GetXaxis().SetTitle("Event Energy [MeV]")

		# ROI Histogram 
		histo_name = self.alabel("ROI")
		histo_desc = "ROI"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc,
		             int(self.roiMaxE/KeV - self.roiMinE/KeV),
		             self.roiMinE, self.roiMaxE)
		self.hman.fetch(self.alabel("ROI")).GetXaxis().SetTitle("Event Energy [MeV]")


    ### Counters:
		self.numInputEvents = 0
		self.numOutputEvents = 0

		return



	############################################################
	def execute(self, event=""):

		self.m.log(2, 'Execute()')		
	
		self.numInputEvents += 1   

		evtEdep = event.GetEnergy()
		self.hman.fill(self.alabel("PDF"), evtEdep)

		if (self.roiMinE < evtEdep < self.roiMaxE):
			self.hman.fill(self.alabel("ROI"), evtEdep)
			self.numOutputEvents += 1   
			self.m.log(2, "Event Edep: %.3f MeV  ->  Filter Passed" %(evtEdep/MeV))
			return True
		else:
			self.m.log(2, "Event Edep: %.3f MeV  ->  Filter Failed" %(evtEdep/MeV))
			return False

		return False

		

	############################################################
	def finalize(self):

		self.m.log(1, 'Finalize()')

		self.m.log(1, 'Input  Events: ', self.numInputEvents)
		self.m.log(1, 'Output Events: ', self.numOutputEvents)

		self.logman["USER"].ints[self.alabel("InputEvents")] = self.numInputEvents
		self.logman["USER"].ints[self.alabel("OutputEvents")] = self.numOutputEvents

		return
