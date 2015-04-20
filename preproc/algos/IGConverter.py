
from Centella.AAlgo import AAlgo
from Centella.physical_constants import *
from Centella.gateWriter import gateWriter
from Centella.cerrors import *

from ROOT import gSystem

ok = gSystem.Load("$GATE_DIR/lib/libGATE")

if not ok: raise CImportError("GATE_LIB path not defined!")

from ROOT import gate

class IGConverter(AAlgo):


	def __init__(self,param=False,level = 1,label="",**kargs):
		"""
		Initialize members and take input arguments
		"""
		#self.m.log(1, 'Constructor()')

		self.name='IGConverter'
		self.writer = gateWriter()
		AAlgo.__init__(self,param,level,self.name,0,label,kargs)
		try: self.dstname = self.strings["GATE_DST_NAME"]
		except KeyError: self.dstname = "irene2gate.root" 



	def initialize(self):
		"""
		Create writer for gate DST
		"""
		self.m.log(1, 'Initialize()')

		### Defining histos
		# Histogram of MC Particles
		histo_name = self.alabel("NumMCParticles")
		histo_desc = "Number of MC Particles"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 60, 0, 60)

		# Histogram of MC Tracks
		histo_name = self.alabel("NumMCTracks")
		histo_desc = "Number of MC Tracks"
		self.m.log(2, "Booking ", histo_name)
		self.m.log(3, "   Description: ", histo_desc)
		self.hman.h1(histo_name, histo_desc, 30, 0, 30)

		self.writer.open(self.dstname)
		return



	def execute(self,event=""):
		"""
		Convert irene events into gate events and save into DST
		"""
		self.m.log(2, 'Execute()')
		gevent = self.irene2gate(event)
		self.writer.write(gevent,0)
		return True
			


	def irene2gate(self,event):
		gevent = gate.Event()

		# Event ID
		gevent.SetID(event.GetID())
		
		# Particles
		tparts = event.GetParticles()
		pIDs = {}

		numPrimaries = 0
		for part in tparts:
			gpart = gate.MCParticle()
			gpart.SetPDG(part.GetPDGcode())
			gpart.store("name", part.Name())
			gpart.SetPrimary(part.IsPrimary())
			gpart.SetID(part.GetParticleID())
			gpart.SetCreatorProc(part.GetCreatorProcess())

			pIDs[part.GetParticleID()] = gpart
			daus = part.GetDaughters()
			dausIDv = gate.vint()
			for i in range(daus.GetEntriesFast()): 
				dausIDv.push_back(daus[i].GetParticleID())
			gpart.store("dausID",dausIDv)
					
			if part.IsPrimary():
				numPrimaries += 1
			else:
				gpart.store("momID",part.GetMother().GetParticleID())
							
			mom = part.GetInitialMomentum()
			gpart.SetInitialMom(mom.X(),mom.Y(),mom.Z(),mom.E())

			mom = part.GetDecayMomentum()
			gpart.SetFinalMom(mom.X(),mom.Y(),mom.Z(),mom.E())

			vtx = part.GetInitialVertex()
			gpart.SetInitialVtx(vtx.X(),vtx.Y(),vtx.Z())
					
			vtx = part.GetDecayVertex()
			gpart.SetFinalVtx(vtx.X(),vtx.Y(),vtx.Z())
					
			gevent.AddMCParticle(gpart)
					
			# Tracks
			for itrk in range(part.GetTracks().GetEntriesFast()): 
				trk = part.GetTracks()[itrk]
				gtrk = gate.MCTrack()
				gtrk.SetParticle(gpart)							
				gtrk.SetLength(part.GetTrackLength())
				gtrk.SetID(part.GetParticleID())
				gpart.AddTrack(gtrk) # Add to particle
				gevent.AddMCTrack(gtrk) # add to event
							
				# Hits
				num_hits = 0
				for hit in trk.GetHits():
					ghit = gate.MCHit()
					ghit.SetID(num_hits)
					ghit.SetParticle(gpart)
					ghit.SetAmplitude(hit[1])
					ghit.SetPosition(hit[0].X(),hit[0].Y(),hit[0].Z())
					ghit.SetTime(hit[0].T())
					gtrk.AddHit(ghit)
					gevent.AddMCHit(ghit)
					num_hits += 1

				# Setting Track Energy
				gtrk.SetEnergy(gtrk.GetHitsEnergy())

				# Setting Track Extremes
				gtrk.SetExtremes(0, len(trk.GetHits())-1)

				# Verbosing
				if (self.m.level >= 3): gtrk.Info()

			
		# Setting daughter and mother particles	
		for part in gevent.GetMCParticles():			
			dausID = part.fetch_ivstore("dausID")
			for dauID in dausID: part.AddDaughter(pIDs[dauID])
			part.erase_ivstore("dausID")
			if not part.IsPrimary(): 
				part.SetMother(pIDs[part.fetch_istore("momID")])
				part.erase_istore("momID")

		# Setting Event MC Type
		if (numPrimaries == 2): gevent.SetMCEventType(gate.BB0NU)
		else: gevent.SetMCEventType(gate.BKG)

		# Setting Event MC Energy
		gevent.SetMCEnergy(gevent.GetMCTracksEnergy())

		# Filling Histos
		numMCParts = len(gevent.GetMCParticles())
		numMCTracks = len(gevent.GetMCTracks())
		self.hman.fill(self.alabel("NumMCParticles"), numMCParts)
		self.hman.fill(self.alabel("NumMCTracks"), numMCTracks)

		# Verbosing
		self.m.log(2, 'Num MC Particles:', numMCParts)
		self.m.log(2, 'Num MC Tracks:', numMCTracks)
		if (self.m.level >= 3): gevent.Info()

		return gevent



	def finalize(self):
		self.m.log(1, 'Finalize()')
		self.writer.close()
		return

