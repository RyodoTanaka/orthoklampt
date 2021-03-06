import sys
import numpy as np
from cspace_visualizer import *

font_size = 45
showSamples = True
output_name = "2dof_manip"

folder_name = "../data/samples/"
qsname = folder_name+output_name+"_qspace.png"
csname = folder_name+output_name+"_cspace.png"
qsname_samples = folder_name+output_name+"_qspace_volumes.png"
csname_samples = folder_name+output_name+"_cspace_volumes.png"

qfname = folder_name+"cspace_robot_2dof_inner.samples"
cfname = folder_name+"cspace_robot_2dof.samples"

[Q1,Q2] = generateInfeasibleSamplesOneDim(qfname, dim1=0)
[C1,C2] = generateInfeasibleSamplesTwoDim(cfname, dim1=0, dim2=1)

###############################################################################
## QUOTIENT-SPACE
###############################################################################
fig = plt.figure(0)
fig.patch.set_facecolor('white')
ax = fig.gca()
ax.set_xlabel(r'\theta_1',fontsize=font_size)
ax.set_ylabel(r'\theta_2',rotation=1.57,fontsize=font_size)
ax.tick_params(axis='both', which='major', pad=15)
plt.axis([-3.14,3.14,-3.14,3.14])
plotCSpaceDelaunayGrey(Q1,Q2,0.15)
plt.savefig(qsname, bbox_inches='tight')
if showSamples:
  plotSamplesOneDim(qfname)
  plt.savefig(qsname_samples, bbox_inches='tight')

###############################################################################
## CSPACE
###############################################################################
fig = plt.figure(1)
fig.patch.set_facecolor('white')
ax = fig.gca()
ax.set_xlabel(r'\theta_1',fontsize=font_size)
ax.set_ylabel(r'\theta_2',rotation=1.57,fontsize=font_size)
ax.tick_params(axis='both', which='major', pad=15)
plt.axis([-3.14,3.14,-1.57,1.57])
plotCSpaceDelaunayGrey(C1,C2,0.15)
plt.savefig(csname, bbox_inches='tight')
if showSamples:
  plotSamplesOneDim(qfname)
  plt.savefig(csname_samples, bbox_inches='tight')
plt.show()
