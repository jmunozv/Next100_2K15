from Centella.AAlgo import AAlgo
from Centella.physical_constants import *
from math import sqrt
from ROOT import TCanvas, TH2F, TH3F

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
		# 
		try:
			self.hitsType = self.strings['hitsType']
			self.m.log(1, "Representing Hits: %s" %(self.hitsType))
		except KeyError:
			self.m.log(1, "WARNING!! Parameter: 'hitsType' not defined.")
			exit(0) 
		


	############################################################		
	def initialize(self):

		self.m.log(1, 'Initialize()')

		return



	############################################################
	def execute(self, event=""):

		self.m.log(2, 'Execute()')

		### Getting the right Hits
		if (self.hitsType == "MC"):
			hits = event.GetMCHits()

		elif (self.hitsType == "Smeared"):
			hits = event.GetHits()
		
		else:
			self.m.log(1, "ERROR: hits type: '%s'   NOT VALID" %(self.hitsType))
			exit(0)


		### Identifying area of interest
		minX = minY = minZ = 10000
		maxX = maxY = maxZ = -10000
		for hit in hits:
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
		margin = 10

		bin_size = 1
		if (self.hitsType == "Smeared"): bin_size = 2

		histoXYZ = TH3F("histoXYZ", "XYZ hits",
		               	int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               	int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin,
		               	int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		histoXY  = TH2F("histoXY", "XY hits",
		               	int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               	int((maxY-minY)+2*margin)/bin_size, minY-margin, maxY+margin)

		histoXZ  = TH2F("histoXZ", "XZ hits",
		               	int((maxX-minX)+2*margin)/bin_size, minX-margin, maxX+margin,
		               	int((maxZ-minZ)+2*margin)/bin_size, minZ-margin, maxZ+margin)

		for hit in hits:
			posX = hit.GetPosition().x() 
			posY = hit.GetPosition().y() 
			posZ = hit.GetPosition().z()
			hitE = hit.GetAmplitude()
			histoXYZ.Fill(posX, posY, posZ, hitE)
			histoXY.Fill(posX, posY, hitE)
			histoXZ.Fill(posX, posZ, hitE)


		### Drawing the Histograms
		canvas = TCanvas("myCanvas", "Hits", 10, 10, 1200, 600)
		canvas.Divide(2,1)

		#histoXYZ.GetXaxis().SetTitle("X");
		#histoXYZ.GetYaxis().SetTitle("Y");
		#histoXYZ.GetZaxis().SetTitle("Z");
		#histoXYZ.SetTitle("Event %i - %s Hits" %(event.GetID(), self.hitsType))
		#histoXYZ.Draw()

		canvas.cd(1)
		histoXY.GetXaxis().SetTitle("X");
		histoXY.GetYaxis().SetTitle("Y");
		histoXY.SetTitle("Event %i - %s Hits" %(event.GetID(), self.hitsType))
		histoXY.Draw("colz")

		canvas.cd(2)
		histoXZ.GetXaxis().SetTitle("X");
		histoXZ.GetYaxis().SetTitle("Z");
		histoXZ.SetTitle("Event %i - %s Hits" %(event.GetID(), self.hitsType))
		histoXZ.Draw("colz")


		canvas.Update()
		raw_input("Press Key to continue ...")


		return True

		

	############################################################
	def finalize(self):

		self.m.log(1, 'Finalize()')

		return
