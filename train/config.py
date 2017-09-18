import os

dir = os.path.dirname(os.path.realpath(__file__))

print "Creating javaclasspath.txt in the src/ directory..."

with open(dir+"/src/javaclasspath.txt", "w") as f:
	print >> f, dir+"/src/jar/berkeleyParser.jar"
	print >> f, dir+"/src/jar/colt.jar"
	print >> f, dir+"/src/jar/trove.jar"
	print >> f, dir+"/src/jar/commons-cli-1.2.jar"
	print >> f, dir+"/src/jar/lpcfg-smoothclusterestimation-universal.jar"
