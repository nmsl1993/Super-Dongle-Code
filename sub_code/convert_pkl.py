import cPickle as pickle
import numpy as np
print "Reading..."
data = pickle.load(open('pyout.pkl','r'))
print "Read done... Writing..."
np.savetxt('pyout.csv',data,delimiter=',')