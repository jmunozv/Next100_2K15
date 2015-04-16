from Centella.AAlgo import AAlgo
from Centella.physical_constants import *

"""
This algorithm filters events which energy make them unrelevant.
Its only purpose is speed up porterior analysis.
Parameters:
   minEnergy -> Minimum Event Energy
   maxEnergy -> Maximum Event Energy
"""


class TrueEnergyFilter(AAlgo):

	############################################################
	def __init__(self, param=False, level=1, label="", **kargs):

		"""
		True Energy Filter Algorithm
		"""
		#self.m.log(1, 'Constructor()')

		### GENERAL STUFF
		self.name = 'TrueEnergyFilter'
		#self.level = level
		AAlgo.__init__(self, param, level, self.name, 0, label, kargs)

    ### PARAMETERS
    # Minimum Energy
		try:
			self.minEnergy = self.doubles['minEnergy']
			self.m.log(1, "Minimum Energy: %.2f MeV." %(self.minEnergy/MeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'minEnergy' not defined.")
			exit(0)

		# Maximum Energy
		try:
			self.maxEnergy = self.doubles['maxEnergy']
			self.m.log(1, "Maximum Energy: %.2f MeV." %(self.maxEnergy/MeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'maxEnergy' not defined.")
			exit(0) 
		


	############################################################		
	def initialize(self):

		self.m.log(1, 'Initialize()')
		
		### Defining histos
		# Histogram of Event Edep Before Filter
		histo_name = self.alabel("evtEdepBefore")
		histo_desc = "Event Energy Dep. Before Filter"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 100, 2.0, 4.0)
		
		# Histogram of Event Edep After Filter
		histo_name = self.alabel("evtEdepAfter")
		histo_desc = "Event Energy Dep. After Filter"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 100, self.minEnergy, self.maxEnergy)

        
    ### Counters:
		self.numInputEvents = 0
		self.numOutputEvents = 0

		return



	############################################################
	def execute(self, event=""):

		self.m.log(2, 'Execute()')		
	
		self.numInputEvents += 1   

		inLimits = False

		#evtEdep = 2.5;
		#evtEdep = self.event.fetch_dproperty("TrueEdep")
		evtEdep = 0.;
		for iPart in self.event.GetParticles():
			for trkIdx in range(iPart.GetTracks().GetEntriesFast()): 
				iTrack = iPart.GetTracks()[trkIdx]
				for iHit in iTrack.GetHits():
					evtEdep += iHit[1]

		self.hman.fill(self.alabel("evtEdepBefore"), evtEdep)

		if (self.minEnergy < evtEdep < self.maxEnergy):
			self.hman.fill(self.alabel("evtEdepAfter"), evtEdep)
			self.numOutputEvents += 1   
			inLimits = True
			self.m.log(2, "Event Edep: %.3f MeV  ->  Filter Passed" %(evtEdep/MeV))
		else:
			self.m.log(2, "Event Edep: %.3f MeV  ->  Filter Failed" %(evtEdep/MeV))


		return inLimits

		

	############################################################
	def finalize(self):

		self.m.log(1, 'Finalize()')

		self.m.log(1, 'Input  Events: ', self.numInputEvents)
		self.m.log(1, 'Output Events: ', self.numOutputEvents)
		
		self.logman["USER"].ints[self.alabel("InputEvents")] = self.numInputEvents
		self.logman["USER"].ints[self.alabel("OutputEvents")] = self.numOutputEvents

		return
