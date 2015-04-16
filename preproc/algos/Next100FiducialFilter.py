from Centella.AAlgo import AAlgo
from Centella.physical_constants import *
from math import sqrt

"""
This algorithm filters Non Fiducial events.
Parameters:
   vetoWidth -> Width of Veto Volume
   minEnergy -> Minimum Deposited Energy in Veto
"""


class Next100FiducialFilter(AAlgo):

	############################################################
	def __init__(self, param=False, level=1, label="", **kargs):

		"""
		True Energy Filter Algorithm
		"""

		#self.m.log(1, 'Constructor()')

		### GENERAL STUFF
		self.name = 'Next100FiducialFilter'
		#self.level = level
		AAlgo.__init__(self, param, level, self.name, 0, label, kargs)

    ### PARAMETERS
		# Veto Width
		try:
			self.vetoWidth = self.doubles['vetoWidth']
			self.m.log(1, "Veto Width: %.1f mm." %(self.vetoWidth/mm))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'vetoWidth' not defined.")
			exit(0) 
		
    # Minimum Energy
		try:
			self.minEnergy = self.doubles['minEnergy']
			self.m.log(1, "Minimum Energy: %.1f KeV." %(self.minEnergy/KeV))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'minEnergy' not defined.")
			exit(0)


		### DETECTOR DIMENSIONS
		self.detectorLength  = 1300.
		self.detectorRadius  = 534.5
		self.detectorZcenter = 14.



	############################################################		
	def initialize(self):

		self.m.log(1, 'Initialize()')
		
		### Defining histos
		# Histogram of Veto Edep
		histo_name = self.alabel("vetoEdep")
		histo_desc = "Veto Energy"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 50, 0., 0.5)
		
		# Histogram of Event Edep After Filter
		histo_name = self.alabel("evtEdepAfter")
		histo_desc = "Event Energy Dep. After Filter"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 70, 2.3, 3.0)

        
    ### Counters:
		self.numInputEvents = 0
		self.numOutputEvents = 0


		### USEFUL VARIABLES
		self.zPosLimit = self.detectorLength/2. + self.detectorZcenter - self.vetoWidth
		self.zNegLimit = -self.detectorLength/2. + self.detectorZcenter + self.vetoWidth
		self.radLimit  = self.detectorRadius - self.vetoWidth

		return



	############################################################
	def execute(self, event=""):

		self.m.log(2, 'Execute()')
			
		self.numInputEvents += 1   

		fiducial = False

		vetoEdep = 0.;
		evtEdep = 0.;
		for iPart in self.event.GetParticles():
			for trkIdx in range(iPart.GetTracks().GetEntriesFast()): 
				iTrack = iPart.GetTracks()[trkIdx]
				for iHit in iTrack.GetHits():
					evtEdep += iHit[1]
					# Computing vetoEdep
					hitPos = iHit[0]
					hitPosZ = hitPos.Z()
					hitRad = sqrt(hitPos.X()**2 + hitPos.Y()**2)

					if ((hitPosZ > self.zPosLimit) or
					    (hitPosZ < self.zNegLimit) or
					    (hitRad  > self.radLimit)):
						vetoEdep += iHit[1]


		# Histograming
		if (vetoEdep > 0.): self.hman.fill(self.alabel("vetoEdep"), vetoEdep)

		if (vetoEdep < self.minEnergy):
			fiducial = True
			self.numOutputEvents += 1   
			self.m.log(2, "Veto Edep: %.1f KeV  ->  Filter Passed" %(vetoEdep/KeV))
			self.hman.fill(self.alabel("evtEdepAfter"), evtEdep)
		else:
			self.m.log(2, "Veto Edep: %.1f KeV  ->  Filter Failed" %(vetoEdep/KeV))


		return fiducial

		

	############################################################
	def finalize(self):

		self.m.log(1, 'Finalize()')

		self.m.log(1, 'Input  Events: ', self.numInputEvents)
		self.m.log(1, 'Output Events: ', self.numOutputEvents)

		self.logman["USER"].ints[self.alabel("InputEvents")] = self.numInputEvents
		self.logman["USER"].ints[self.alabel("OutputEvents")] = self.numOutputEvents
		
		return
