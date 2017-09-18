##################################################################################################################################
### The Rainbow Parser, by Shashi Narayan and Shay Cohen (University of Edinburgh)
###
###
### This script is used to train a grammar with the expecation-maximization algorithm.
### Its main driver is the lpcfgparser binary in the decode/src directory.
### Usage: python lpcfgem.py [treebank-file] [output-prefix-grammar-file] [vocab-file] [vocab-threshold] [latent-state-number] [iterations-number] [scale-factor]
### See README.md file for more information
###
##################################################################################################################################


import os
import sys

def convertFileToTrees(sentenceFile, outputFile, root, POS):

	with open(sentenceFile, "r") as f:
		with open(outputFile, "w") as g:
			for sent in f:
				sent = sent.rstrip()
				words = sent.split(" ")

				print >> g, "("+root,

				for w in words:
					print >> g, "("+POS+" "+w+")",

				print >> g, ")"


def createExecutionFile(outputCommandFile, inputTreebank, outGrammarPrefix, vocabFile, vocabThreshold, m, iterationsNumber, scaleFactor):

	with open(outputCommandFile, "w") as f:
		print >> f, "command string lpcfgem"
		print >> f, "m int", m
		print >> f, "vocabThreshold int", vocabThreshold
		print >> f, "vocabFile string", vocabFile
		print >> f, "treebankInput string", inputTreebank
		print >> f, "grammarOutputPrefix string", outGrammarPrefix
		print >> f, "iterationsNumber int",iterationsNumber
		print >> f, "scalingFactor double",scaleFactor



def checkCompiled():
	dir = os.path.dirname(os.path.realpath(__file__))

	myExec = dir + '/../decode/src/lpcfgparser'
	myExec2 = dir + '/norm-trees.sh'
	myExec3 = dir + '/denorm-trees.sh'

	if not os.path.isfile(myExec):
		sys.exit("Cannot find the executable "+myExec+". Did you compile the parser? See README.md.")
	else:
		print >> sys.stderr, "Found parser binary in " + myExec

	if not os.path.isfile(myExec2):
		sys.exit("Cannot find the executable "+myExec2+". Did you compile the parser? See README.md.")
	else:
		print >> sys.stderr, "Found parser binary in " + myExec2

	if not os.path.isfile(myExec3):
		sys.exit("Cannot find the executable "+myExec3+". Did you compile the parser? See README.md.")
	else:
		print >> sys.stderr, "Found parser binary in " + myExec3
	

	return [myExec, myExec2, myExec3]



if (len(sys.argv) < 7):
	sys.exit("usage: python lpcfgem.py [treebank-file] [output-prefix-grammar-file] [vocab-file] [vocab-threshold] [latent-state-number] [iterations-number] [scale-factor]")

inputFile = sys.argv[1]
grammarFile = sys.argv[2]
vocabFile = sys.argv[3]
vocabThreshold = sys.argv[4]
m = sys.argv[5]
iterationsNumber = sys.argv[6]

if not os.path.isfile(inputFile):
	sys.exit("Cannot find file "+inputFile)

if (len(sys.argv) < 6):
	scaleFactor = 1.5
else:
	scaleFactor = sys.argv[7]

try:
	vocabThreshold = int(vocabThreshold)
except ValueError:
	sys.exit("vocab-threshold should be an integer")

try:
	iterationsNumber = int(iterationsNumber)
except ValueError:
	sys.exit( "iterations-number should be an integer")

try:
	m = int(m)
except ValueError:
	sys.exit( "latent-state-number should be an integer")

try:
	scaleFactor = float(scaleFactor)
except ValueError:
	sys.exit( "scale-factor should be a float number")



[myExec, myExecNorm, myExecDenorm] = checkCompiled()

createExecutionFile(inputFile+".cmd", inputFile+".norm", grammarFile, vocabFile, vocabThreshold, m, iterationsNumber, scaleFactor)

os.system(myExecNorm+" --originalparsetrees "+inputFile+" --normalisedparsetrees "+inputFile+".norm")

os.system(myExec+" "+inputFile+".cmd")

print >> sys.stderr, "lpcfgem.py says: I should have left a trace of my execution:"
print >> sys.stderr, "The file "+inputFile+".cmd"+" which is the command file fed into the lpcfgparser binary as input"
print >> sys.stderr, "The file "+inputFile+".norm"+" which is the binarized treebank feed into the lpcfgparser binary as input"
