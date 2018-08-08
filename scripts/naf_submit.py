#!/usr/bin/python
import os
import glob
from argparse import ArgumentParser
from shutil import copyfile, rmtree, move
from time import sleep

# --- functions ---
def getConfigParameter( config, parameter ):
   ntuples = None
   with open(config) as f:
      for line in f:
         line = line.lstrip(' ')
         line = line.rstrip('\n')
         if len(line.split()) != 3:
            continue
         if line[0] == '#':
            continue
         if line.split()[0] == parameter:
            ntuples = line.split()[2]
   return ntuples

# --- main code ---

# parsing arguments
parser = ArgumentParser()
parser.add_argument("-e", "--exe", dest="exe", help="Executable")
parser.add_argument("-n", "--ntuples", dest="ntuples", help="List of ntuples file")
parser.add_argument("-x", "--nfiles", dest="nfiles", type=int, default=1, help="Number of ntuple files per job")
parser.add_argument("-c", "--config", dest="config", help="Configuration file")
parser.add_argument("-j", "--json", dest="json", help="JSON file with certified data")
args = parser.parse_args()
if not args.exe:
   print "nothing to be done" 
   quit()
   
ntuples = args.ntuples
json = args.json
config = args.config

# get parameter from configuration 
if config:
   if not os.path.isfile(config):
      print "Configuration file does not exist"
      quit()
   if not ntuples:
      ntuples = getConfigParameter( config, "ntuplesList" )
   if not json:
      json    = getConfigParameter( config, "json" )
      
# checking if require files exist
if ntuples:
   if not os.path.isfile(ntuples):      
      print "Ntuples list file does not exist"
      quit()
if json:
   if not os.path.isfile(json):      
      print "Json  file does not exist"
      quit()
      
# directory where the jobs will be stored
maindir = "Condor_"+args.exe
if config:
   maindir = maindir+"_"+ os.path.splitext(os.path.basename(config))[0]
cwd = os.getcwd()
if os.path.exists(cwd+"/"+maindir):
   print maindir + "already exists. Rename or remove it and then resubmit"
   quit()
os.mkdir(maindir)

# splitting the file list
if ntuples:
   pid = os.getpid()
   tmpdir = ".splitdir_" + str(pid)
   os.mkdir(tmpdir)
   copyfile(ntuples, tmpdir+"/"+ntuples)
   os.chdir(tmpdir)
   splitcmd = "split.csh" + " " + str(args.nfiles) + " " + ntuples
   os.system(splitcmd)
   files = glob.glob('.*_x????.txt')
   files.sort()
   os.chdir(cwd)

   # loop over the splitted files, each will correspond to a job on the NAF
   for f in files:
      jobid = "job_"+os.path.splitext(f)[0][-4:]
      exedir = maindir+"/"+jobid
      os.mkdir(exedir)
      # moving stuff to the proper directories
      move(tmpdir+"/"+f,exedir+"/"+ntuples)
      if json:
         copyfile(json, exedir+"/"+json)
      if config:
         copyfile(config, exedir+"/"+config)
      # make the submissions
      os.chdir(exedir)
      if config:
         condorcmd = "condor_submit.csh" + " " + jobid + " " + args.exe + " " + config
      else:
         condorcmd = "condor_submit.csh" + " " + jobid + " " + args.exe
      os.system(condorcmd)
      sleep(0.2)
      # back to original directory
      os.chdir(cwd)

else:
   exedir = maindir+"/job_0000"
   os.mkdir(exedir)
   os.chdir(exedir)
   condorcmd = "condor_submit.csh job_0000" + " " + args.exe
   os.system(condorcmd)
   os.chdir(cwd)
         
# remove the temporary directory
os.chdir(cwd)
if ntuples: 
   rmtree(tmpdir)


