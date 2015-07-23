from Centella.AAlgo import AAlgo
from Centella.physical_constants import *
from math import sqrt
from ROOT import TCanvas, TH2F, TH3F

from ROOT import gSystem
gSystem.Load("$GATE_DIR/lib/libGATE")
gSystem.Load("$GATE_DIR/lib/libGATEIO")
gSystem.Load("$GATE_DIR/lib/libGATEUtils")
from ROOT import gate

"""
This algorithm draws the event hits via ROOT
Parameters:
   hitsType -> MC / Smeared
"""


class DrawHits(AAlgo):

	############################################################
	def __init__(self, param=False, level=1, label="", **kargs):

		"""
		True Energy Filter Algorithm
		"""

		#self.m.log(1, 'Constructor()')

		### GENERAL STUFF
		self.name = 'DrawHits'
		#self.level = level
		AAlgo.__init__(self, param, level, self.name, 0, label, kargs)

    ### PARAMETERS
		# Select if MCEvent Resumed Info is printed
		try:
			self.mcInfo = self.ints['mcInfo']
			self.m.log(1, "Printing MC Info: %i" %(self.mcInfo))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'mcInfo' not defined.")
			exit(0) 


	############################################################		
	def initialize(self):

		self.m.log(1, 'Initialize()')

		return



	############################################################
	def execute(self, event=""):

		self.m.log(2, 'Execute()')

		### Getting the Hits
		mcHits = event.GetMCHits()
		smHits = event.GetHits()
		

		### Identifying area of interest
		minX = minY = minZ = 10000
		maxX = maxY = maxZ = -10000
		for hit in smHits:
			posX = hit.GetPosition().x() 
			posY = hit.GetPosition().y() 
			posZ = hit.GetPosition().z() 

			if (posX > maxX): maxX = posX
			if (posX < minX): minX = posX
			if (posY > maxY): maxY = posY
			if (posY < minY): minY = posY
			if (posZ > maxZ): maxZ = posZ
			if (posZ < minZ): minZ = posZ

		self.m.log(2, "Area of interest:   X: [%.1f, %.1f]  Y: [%.1f, %.1f]  Z: [%.1f, %.1f]" \
		           %(minX, maxX, minY, maxY, minZ, maxZ))


		### Create & Fill the Histograms
		margin = 20

		# MC
		bin_size = 1
		histoXYZ_MC = TH3F("histoXYZ_MC", "XYZ MC hits",
		               			int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               			int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin,
		               			int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		histoXY_MC  = TH2F("histoXY_MC", "XY MC hits",
		               			int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               			int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin)

		histoXZ_MC  = TH2F("histoXZ_MC", "XZ MC hits",
		               			int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               			int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		for hit in mcHits:
			posX = hit.GetPosition().x() 
			posY = hit.GetPosition().y() 
			posZ = hit.GetPosition().z()
			hitE = hit.GetAmplitude()
			histoXYZ_MC.Fill(posX, posY, posZ, hitE)
			histoXY_MC.Fill(posX, posY, hitE)
			histoXZ_MC.Fill(posX, posZ, hitE)


		# Smeared
		bin_size = 2
		histoXYZ_SM = TH3F("histoXYZ_SM", "XYZ Smeared hits",
		               			int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               			int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin,
		               			int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		histoXY_SM  = TH2F("histoXY_SM", "XY Smeared hits",
		               			int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               			int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin)

		histoXZ_SM  = TH2F("histoXZ_SM", "XZ Smeared hits",
		                   int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		                   int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		for hit in smHits:
			posX = hit.GetPosition().x() 
			posY = hit.GetPosition().y() 
			posZ = hit.GetPosition().z()
			hitE = hit.GetAmplitude()
			histoXYZ_SM.Fill(posX, posY, posZ, hitE)
			histoXY_SM.Fill(posX, posY, hitE)
			histoXZ_SM.Fill(posX, posZ, hitE)


		### Create & Fill the Extremes Histograms
		# MC
		bin_size = 1
		histoXYZextremes_MC = TH3F("histoXYZextremes_MC", "XYZ Smeared extremes",
		               					int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               					int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin,
		               					int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		histoXYextremes_MC  = TH2F("histoXYextremes_MC", "XY Smeared extremes", 
		                        int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		                        int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin)

		histoXZextremes_MC  = TH2F("histoXZextremes_MC", "XZ Smeared extremes",
		             						int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		             						int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		# If Signal, True extremes are the second extremes of "primary" tracks
		MCTracks = event.GetMCTracks()

		if (event.GetMCEventType() == gate.BB0NU):
			numPrimaries = 0
			for MCTrack in MCTracks:
				if (MCTrack.GetParticle().IsPrimary()):
					numPrimaries += 1
					if (numPrimaries == 1): extreme1Pos = MCTrack.GetExtremes().second.GetPosition()
					if (numPrimaries == 2):
						extreme2Pos = MCTrack.GetExtremes().second.GetPosition()
						break

		# If Background, True extremes are the extremes of the Hottest TTrack
		else:
			# Getting the hottest True Track from the Hottest Track
			MCTrackIDs = event.GetTracks()[0].fetch_ivstore("mcTrackIDs")
			maxEdep = 0
			hMCTrack = 0
			for MCTrack in MCTracks:
				if MCTrack.GetID() in MCTrackIDs : 
					eDep = MCTrack.GetEnergy()
					if (eDep > maxEdep):
						maxEdep = eDep
						hMCTrack = MCTrack

			extreme1Pos = hMCTrack.GetExtremes().first.GetPosition()
			extreme2Pos = hMCTrack.GetExtremes().second.GetPosition()

		self.m.log(2, "MC Extreme 1: [%.1f, %.1f, %.1f]" % (extreme1Pos.x(), extreme1Pos.y(), extreme1Pos.z()))
		self.m.log(2, "MC Extreme 2: [%.1f, %.1f, %.1f]" % (extreme2Pos.x(), extreme2Pos.y(), extreme2Pos.z()))

		# Filling
		histoXYZextremes_MC.Fill(extreme1Pos.x(), extreme1Pos.y(), extreme1Pos.z())
		histoXYZextremes_MC.Fill(extreme2Pos.x(), extreme2Pos.y(), extreme2Pos.z())

		histoXYextremes_MC.Fill(extreme1Pos.x(), extreme1Pos.y())
		histoXYextremes_MC.Fill(extreme2Pos.x(), extreme2Pos.y())

		histoXZextremes_MC.Fill(extreme1Pos.x(), extreme1Pos.z())
		histoXZextremes_MC.Fill(extreme2Pos.x(), extreme2Pos.z())

		# SM
		bin_size = 2
		histoXYZextremes_SM = TH3F("histoXYZextremes_SM", "XYZ MC extremes",
		               					int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               					int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin,
		               					int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		histoXYextremes_SM  = TH2F("histoXYextremes_SM", "XY MC extremes", 
		                        int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		                        int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin)

		histoXZextremes_SM  = TH2F("histoXZextremes_SM", "XZ MC extremes",
		             						int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		             						int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		evtExtremes = event.GetTracks()[0].GetExtremes()
		extreme1Pos = evtExtremes.first.GetPosition()
		extreme2Pos = evtExtremes.second.GetPosition()

		self.m.log(2, "Rec Extreme 1: [%.1f, %.1f, %.1f]" % (extreme1Pos.x(), extreme1Pos.y(), extreme1Pos.z()))
		self.m.log(2, "Rec Extreme 2: [%.1f, %.1f, %.1f]" % (extreme2Pos.x(), extreme2Pos.y(), extreme2Pos.z()))

		histoXYZextremes_SM.Fill(extreme1Pos.x(), extreme1Pos.y(), extreme1Pos.z())
		histoXYZextremes_SM.Fill(extreme2Pos.x(), extreme2Pos.y(), extreme2Pos.z())

		histoXYextremes_SM.Fill(extreme1Pos.x(), extreme1Pos.y())
		histoXYextremes_SM.Fill(extreme2Pos.x(), extreme2Pos.y())

		histoXZextremes_SM.Fill(extreme1Pos.x(), extreme1Pos.z())
		histoXZextremes_SM.Fill(extreme2Pos.x(), extreme2Pos.z())


		### Drawing the Histograms
		canvas = TCanvas("myCanvas", "Hits", 10, 10, 1200, 800)
		canvas.Divide(3,2)

		canvas.cd(1)
		histoXYZ_MC.GetXaxis().SetTitle("X")
		histoXYZ_MC.GetYaxis().SetTitle("Y")
		histoXYZ_MC.GetZaxis().SetTitle("Z");
		histoXYZ_MC.SetTitle("Event %i - MC Hits" %(event.GetID()))
		histoXYZ_MC.Draw("ISO")
		histoXYZextremes_MC.Draw("same ISO")

		canvas.cd(2)
		histoXY_MC.GetXaxis().SetTitle("X")
		histoXY_MC.GetYaxis().SetTitle("Y")
		histoXY_MC.SetTitle("Event %i - MC Hits" %(event.GetID()))
		histoXY_MC.Draw("colz")
		histoXYextremes_MC.Draw("same CONT")

		canvas.cd(3)
		histoXZ_MC.GetXaxis().SetTitle("X")
		histoXZ_MC.GetYaxis().SetTitle("Z")
		histoXZ_MC.SetTitle("Event %i - MC Hits" %(event.GetID()))
		histoXZ_MC.Draw("colz")
		histoXZextremes_MC.Draw("same CONT")

		canvas.cd(4)
		histoXYZ_SM.GetXaxis().SetTitle("X")
		histoXYZ_SM.GetYaxis().SetTitle("Y")
		histoXYZ_SM.GetZaxis().SetTitle("Z");
		histoXYZ_SM.SetTitle("Event %i - Smeared Hits" %(event.GetID()))
		histoXYZ_SM.Draw("ISO")
		histoXYZextremes_SM.Draw("same ISO")

		canvas.cd(5)
		histoXY_SM.GetXaxis().SetTitle("X")
		histoXY_SM.GetYaxis().SetTitle("Y")
		histoXY_SM.SetTitle("Event %i - Smeared Hits" %(event.GetID()))
		histoXY_SM.Draw("colz")
		histoXYextremes_SM.Draw("same CONT")

		canvas.cd(6)
		histoXZ_SM.GetXaxis().SetTitle("X")
		histoXZ_SM.GetYaxis().SetTitle("Z")
		histoXZ_SM.SetTitle("Event %i - Smeared Hits" %(event.GetID()))
		histoXZ_SM.Draw("colz")
		histoXZextremes_SM.Draw("same CONT")

		canvas.Update()


		if (self.mcInfo): print gate.ResumedInfo(event)

		raw_input("Press Key to continue ...")


		return True

		

	############################################################
	def finalize(self):

		self.m.log(1, 'Finalize()')

		return
