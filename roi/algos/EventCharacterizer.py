from Centella.AAlgo import AAlgo
from Centella.physical_constants import *

from ROOT import gSystem
gSystem.Load("$GATE_DIR/lib/libGATE")
gSystem.Load("$GATE_DIR/lib/libGATEIO")
gSystem.Load("$GATE_DIR/lib/libGATEUtils")
from ROOT import gate

"""
This algorithm characterizes Background events that have passed all filters
Parameters:
	blobRadius
	ROIMinE -> ROI Minimum  Energy
	ROIMaxE -> ROI Maximum  Energy
"""


class EventCharacterizer(AAlgo):

	############################################################
	def __init__(self, param=False, level=1, label="", **kargs):

		"""
		Event Characterizer Algorithm
		"""
		#self.m.log(1, 'Constructor()')

		### GENERAL STUFF
		self.name = 'EventCharacterizer'
		#self.level = level
		AAlgo.__init__(self, param, level, self.name, 0, label, kargs)

		### PARAMETERS
		# Blob Radius
		try:
			self.blobRadius = self.doubles['blobRadius']
			self.m.log(1, "Blob Radius: %.1f mm" %(self.blobRadius))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'blobRadius' not defined.")
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
		# Event Energy Histogram 
		histo_name = self.alabel("EventEnergy")
		histo_desc = "Event Energy"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc,
								 int(self.roiMaxE/KeV - self.roiMinE/KeV),
								 self.roiMinE, self.roiMaxE)
		self.hman.fetch(self.alabel("EventEnergy")).GetXaxis().SetTitle("Event Energy [MeV]")

		# Distance between True & Recons. extremes for events passing the filter
		histo_name = self.alabel("DistTrueRec1")
		histo_desc = "Distance True-Rec extreme 1"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 40, 0, 40) 

		# Distance between True & Recons. extremes for events passing the filter
		histo_name = self.alabel("DistTrueRec2")
		histo_desc = "Distance True-Rec extreme 2"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 40, 0, 40) 

		# Distance between True & Recons. extremes for events passing the filter
		histo_name = self.alabel("DistTrueRec")
		histo_desc = "Distance True-Rec extremes"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 40, 0, 40) 


		### Counters:
		self.numInputEvents = 0
		self.numSwappedEvents = 0

		return



	############################################################
	def execute(self, event=""):

		self.m.log(2, 'Execute()')		
	
		self.numInputEvents += 1

		### Event Energy
		evtEdep = event.GetEnergy()
		self.hman.fill(self.alabel("EventEnergy"), evtEdep)
		self.m.log(2, "Event Edep: %.3f MeV" %(evtEdep/MeV))


		### Extremes stuff
		# Getting the Hottest Track
		hTrack = 0
		maxEdep = 0
		for track in self.event.GetTracks():
			trackEdep = track.GetEnergy()
			if (trackEdep > maxEdep):
				maxEdep = trackEdep
				hTrack = track

		distExtFirst  = hTrack.fetch_dvstore("DistExtFirst")
		distExtSecond = hTrack.fetch_dvstore("DistExtSecond")

		# Ordering extremes per energy
		energyStart = -1
		ext1Pos = ext2Pos = 0
		blob1E = blob2E = 0
		
		for hit in event.GetHits():
			dist1 = distExtFirst[hit.GetID()]
			if (dist1 < self.blobRadius): blob1E += hit.GetAmplitude()

			dist2 = distExtSecond[hit.GetID()]
			if (dist2 < self.blobRadius): blob2E += hit.GetAmplitude()

		if (blob1E < blob2E):
			energyStart = hTrack.GetExtremes().first.GetID()
			ext1Pos = hTrack.GetExtremes().first.GetPosition()
			ext2Pos = hTrack.GetExtremes().second.GetPosition()
		else:
			energyStart = hTrack.GetExtremes().second.GetID()
			ext2Pos = hTrack.GetExtremes().first.GetPosition()
			ext1Pos = hTrack.GetExtremes().second.GetPosition()

		# Ordering extremes per euclidean distance
		ext1MCPos = ext2MCPos = 0
		MCTracks = event.GetMCTracks();

		# If Signal, True extremes are the second extremes of "primary" tracks
		if (event.GetMCEventType() == gate.BB0NU):
			numPrimaries = 0
			for MCTrack in MCTracks:
				if (MCTrack.GetParticle().IsPrimary()):
					numPrimaries += 1
					if (numPrimaries == 1): ext1MCPos = MCTrack.GetExtremes().second.GetPosition()
					if (numPrimaries == 2):
						ext2MCPos = MCTrack.GetExtremes().second.GetPosition()
						break

		# If Background, True extremes are the extremes of the Hottest TTrack
		else:
			# Getting the hottest True Track from the Hottest Track
			MCTrackIDs = hTrack.fetch_ivstore("mcTrackIDs")
			maxEdep = 0
			hMCTrack = 0
			for MCTrack in MCTracks:
				if MCTrack.GetID() in MCTrackIDs : 
					eDep = MCTrack.GetEnergy()
					if (eDep > maxEdep):
						maxEdep = eDep
						hMCTrack = MCTrack

			ext1MCPos = hMCTrack.GetExtremes().first.GetPosition()
			ext2MCPos = hMCTrack.GetExtremes().second.GetPosition()

		# Verbosing
		self.m.log(3, 'Extreme 1 Rec. Position: (%f, %f, %f)' %(ext1Pos.x(), ext1Pos.y(), ext1Pos.z()))
		self.m.log(3, 'Extreme 2 Rec. Position: (%f, %f, %f)' %(ext2Pos.x(), ext2Pos.y(), ext2Pos.z()))

		self.m.log(3, 'Extreme 1 MC Position: (%f, %f, %f)' %(ext1MCPos.x(), ext1MCPos.y(), ext1MCPos.z()))
		self.m.log(3, 'Extreme 2 MC Position: (%f, %f, %f)' %(ext2MCPos.x(), ext2MCPos.y(), ext2MCPos.z()))


		# Matching Rec & True extremes by euclidean distance
		dist1 = dist2 = 0
		d11 = gate.distance(ext1Pos, ext1MCPos)
		d12 = gate.distance(ext1Pos, ext2MCPos)

		# Swapped extremes ??
		if (d12 < d11):
			self.m.log(2, 'Extremes Swapped')
			self.numSwappedEvents += 1
			dist1 = d12
			dist2 = gate.distance(ext2Pos, ext1MCPos)
		# Correctly matched extrem
		else:
			dist1 = d11
			dist2 = gate.distance(ext2Pos, ext2MCPos)

		self.m.log(2, 'Distance 1:', dist1)
		self.m.log(2, 'Distance 2:', dist2)

		# Filling Histograms
		self.hman.fill(self.alabel("DistTrueRec"), dist1)
		self.hman.fill(self.alabel("DistTrueRec"), dist2)

		self.hman.fill(self.alabel("DistTrueRec1"), dist1)
		self.hman.fill(self.alabel("DistTrueRec2"), dist2)


		return True

		

	############################################################
	def finalize(self):

		self.m.log(1, 'Finalize()')

		self.m.log(1, 'Input  Events: ', self.numInputEvents)
		self.m.log(1, 'Swapped Events: ', self.numSwappedEvents)

		self.logman["USER"].ints[self.alabel("InputEvents")] = self.numInputEvents
		self.logman["USER"].ints[self.alabel("SwappedEvents")] = self.numSwappedEvents

		return
