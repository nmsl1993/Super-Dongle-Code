import cPickle as pickle
import numpy as np
import sys
print "Reading..."
target = sys.argv[1]
data = pickle.load(open(target,'r'))
print "Read done... Writing..."
np.savetxt(target.split('.')[0]+'.csv',data,delimiter=',')
