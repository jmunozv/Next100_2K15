#!/usr/bin/python

from math import sqrt

import sys, ROOT
sys.path.append('/Users/javi/Development/pyutils/')
from system_of_units import *


### EXPERIMENT NAME & DATA TO ANALIZE
EXP_NAME = 'Next100'
VOXEL_SIZE = 2
ENERGY_RES = 0.7

QBB = 2458
PDF_Emin = 2400
PDF_Emax = 2500

### BACKGROUNDS Activities in Becquerels & GLOBAL EFF
BB0NU_PDF_EFF = 29.9
TL_ACTIVITY = 3
BI_ACTIVITY = 2


### Getting Files
PDF_PATH  = '/Users/Javi/Development/Next100_2K15/data/roi/'

extension = '.voxels' + str(VOXEL_SIZE) + '.energy' + str(ENERGY_RES).replace('.','')
extension += '.roi.histos.root'

bb0nu_histo_file = ROOT.TFile(PDF_PATH + EXP_NAME + '.Xe136_bb0nu' + extension)
Tl208_histo_file = ROOT.TFile(PDF_PATH + EXP_NAME + '.Tl208' + extension)
Bi214_histo_file = ROOT.TFile(PDF_PATH + EXP_NAME + '.Bi214' + extension)


### Getting PDFs
bb0nu_pdf = bb0nu_histo_file.Get("ROIFilter.PDF")
bb0nu_pdf.SetTitle('PDF of Xe136_bb0nu')

Tl208_pdf = Tl208_histo_file.Get("ROIFilter.PDF")
Tl208_pdf.SetTitle('PDF of Tl208')

Bi214_pdf = Bi214_histo_file.Get("ROIFilter.PDF")
Bi214_pdf.SetTitle('PDF of Bi214')

# Normalizing
bb0nu_pdf.Scale(1./bb0nu_pdf.Integral())
Tl208_pdf.Scale(1./Tl208_pdf.Integral())
Bi214_pdf.Scale(1./Bi214_pdf.Integral())

# Smoothing Backgrounds
Tl208_pdf.Smooth(1)
Bi214_pdf.Smooth(1)

### Plotting PDFs
canvas_title = EXP_NAME + ' Res. ' + str(ENERGY_RES) + '% @ Qbb'
pdf_canvas = ROOT.TCanvas('PDFs', canvas_title, 200, 10, 800, 900 )
pdf_canvas.Divide(1,3)

pdf_canvas.cd(1)
bb0nu_pdf.Draw()

pdf_canvas.cd(2)
Tl208_pdf.SetLineColor(ROOT.kRed)
Tl208_pdf.SetMinimum(0.)
Tl208_pdf.Draw()

pdf_canvas.cd(3)
Bi214_pdf.SetLineColor(ROOT.kRed)
Bi214_pdf.SetMinimum(0.)
Bi214_pdf.Draw()

pdf_canvas.Update()


### Background Activities
bb0nu_act = bb0nu_pdf.Clone()
bb0nu_act.Scale(BB0NU_PDF_EFF)
bb0nu_act.SetTitle('BB0nu efficienfy [Perc.]')

Tl208_act = Tl208_pdf.Clone()
Tl208_act.Scale(TL_ACTIVITY)

Bi214_act = Bi214_pdf.Clone()
Bi214_act.Scale(BI_ACTIVITY)

bkgnd_act = Tl208_act.Clone()
bkgnd_act.Add(Bi214_act)
bkgnd_act.SetTitle('Background Activity [Becq.]')
#print 'Total Background [Becq]: ' , bkgnd_act.Integral()


### Plotting Efficiency & Background activiy
act_canvas = ROOT.TCanvas( 'Activity', canvas_title, 200, 10, 800, 600 )
act_canvas.Divide(1,2)
act_canvas.cd(1)
bb0nu_pdf.Draw()

act_canvas.cd(2)
bkgnd_act.SetMinimum(0)
bkgnd_act.SetLineColor(ROOT.kRed)
bkgnd_act.Draw()
act_canvas.Update()




#################################################################################################
### Figures of merit
num_bins = bb0nu_pdf.GetNbinsX()
Emin = bb0nu_pdf.GetXaxis().GetXmin()
Emax = bb0nu_pdf.GetXaxis().GetXmax()

### Assuming ROI = PDF
eff = bb0nu_act.Integral()
bkgnds = bkgnd_act.Integral()
merit = eff / sqrt(bkgnds)
print 'ROI = PDF: [%.1f, %.1f] KeV' %(PDF_Emin, PDF_Emax)
print '   Efficiency: ', eff, '  Bkgnds [Bq]: ', bkgnds, '  ->   Fig. of Merit: ', merit

### Assuming ROI = 1FWHM arround Qbb
roi_width = (QBB * ENERGY_RES) / 100.

roi_Emin = QBB - roi_width / 2.
roi_Emax = QBB + roi_width / 2.

left  = int(roi_Emin - PDF_Emin) + 1
right = int(roi_Emax - PDF_Emin)

eff = bb0nu_act.Integral(left, right)
bkgnds = bkgnd_act.Integral(left, right)
merit = eff / sqrt(bkgnds)
print 'ROI of 1 FWHM: [%.1f, %.1f] KeV' %(roi_Emin, roi_Emax)
print '   Efficiency: ', eff, '  Bkgnds [Bq]: ', bkgnds, '  ->   Fig. of Merit: ', merit

### Assuming ROI = 2FWHM arround Qbb
roi_width = (2. * QBB * ENERGY_RES) / 100.

roi_Emin = QBB - roi_width / 2.
roi_Emax = QBB + roi_width / 2.

left  = int(roi_Emin - PDF_Emin) + 1
right = int(roi_Emax - PDF_Emin)

eff = bb0nu_act.Integral(left, right)
bkgnds = bkgnd_act.Integral(left, right)
merit = eff / sqrt(bkgnds)
print 'ROI of 2 FWHM: [%.1f, %.1f] KeV' %(roi_Emin, roi_Emax)
print '   Efficiency: ', eff, '  Bkgnds [Bq]: ', bkgnds, '  ->   Fig. of Merit: ', merit


### All ROI possibilities
merit_histo = ROOT.TH2D('Merit', 'Figure of Merit', num_bins, PDF_Emin, PDF_Emax, num_bins, PDF_Emin, PDF_Emax)
max_left = max_right = 0
max_merit = max_eff = max_bkgnds = 0. 

for left in range(1, num_bins+1):
  for right in range(left, num_bins+1):
    eff = bb0nu_act.Integral(left, right)
    bkgnds = bkgnd_act.Integral(left, right)
    merit = eff / sqrt(bkgnds)
    merit_histo.SetBinContent(left, right, merit)
    #print 'ROI[', left, ',', right, ']:  Efficiency: ', eff, '  Bkgnds [Bq]: ', bkgnds, '  ->   Fig. of Merit: ', merit
    if (merit > max_merit):
			max_left   = left
			max_right  = right
			max_merit  = merit
			max_eff    = eff
			max_bkgnds = bkgnds

roi_Emin = PDF_Emin + max_left
roi_Emax = PDF_Emin + max_right
print 'Max Merit ROI: [%.1f, %.1f] KeV' %(roi_Emin, roi_Emax)
print '   Efficiency: ', max_eff, '  Bkgnds [Bq]: ', max_bkgnds, '  ->   Fig. of Merit: ', max_merit


### Top Efficiencies ROI possibilities
topMerit_eff_histo = ROOT.TH2D('Efficiency', 'Global Efficiency', num_bins, Emin, Emax, num_bins, Emin, Emax)
merit_perc = 5 * perCent
min_merit = (1. - merit_perc) * max_merit
max_left = max_right = 0
max_merit = max_eff = max_bkgnds = 0.
for left in range(1, num_bins+1):
  for right in range(left, num_bins+1):
  	eff = bb0nu_act.Integral(left, right)
  	bkgnds = bkgnd_act.Integral(left, right)
  	merit = eff / sqrt(bkgnds)
  	if (merit > min_merit):
			#print 'ROI[', left, ',', right, ']:  Efficiency: ', eff, '  Bkgnds [Bq]: ', bkgnds, '  ->   Fig. of Merit: ', merit
			topMerit_eff_histo.SetBinContent(left, right, eff)
			if (eff > max_eff):
				max_left   = left
				max_right  = right
				max_merit  = merit
				max_eff    = eff
				max_bkgnds = bkgnds

roi_Emin = PDF_Emin + max_left
roi_Emax = PDF_Emin + max_right
print 'Max Eff from Top Merit ROI: [%.1f, %.1f] KeV' %(roi_Emin, roi_Emax)
print '   Efficiency: ', max_eff, '  Bkgnds [Bq]: ', max_bkgnds, '  ->   Fig. of Merit: ', max_merit


### Plotting Figure of Merit
merit_canvas = ROOT.TCanvas( 'Merit', canvas_title, 200, 10, 800, 800 )
ROOT.gStyle.SetPalette(1)
ROOT.gStyle.SetNumberContours(256)
merit_histo.SetXTitle('Left Margin')
merit_histo.SetYTitle('Right Margin')
merit_histo.Draw('col')
#merit_histo.Print('all')
merit_canvas.Update()


### Plotting Efficiencies of top 5% Figure of Merit
globalEff_canvas = ROOT.TCanvas( 'Global Efficiency', canvas_title, 200, 10, 800, 800 )
ROOT.gStyle.SetPalette(1)
topMerit_eff_histo.SetXTitle('Left Margin')
topMerit_eff_histo.SetYTitle('Right Margin')
topMerit_eff_histo.Draw('col')
#topMerit_eff_histo.Print('all')
globalEff_canvas.Update()


raw_input()


