import numpy as np
import sys,os
target = sys.argv[1]
pyout = np.genfromtxt(target,delimiter=',')

transA = pyout.flatten()[0::3]
transB = pyout.flatten()[1::3]
transC = pyout.flatten()[2::3]


target_dir = '/'.join(os.path.realpath(target).split('/')[0:-1:])
print(target_dir)
np.savetxt(target_dir+'/samples_1_0.csv',transA,delimiter=',')
np.savetxt(target_dir+'/samples_0_0.csv',transB,delimiter=',')
np.savetxt(target_dir+'/samples_0_1.csv',transC,delimiter=',')
