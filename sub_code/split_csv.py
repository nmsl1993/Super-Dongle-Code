import numpy as np

pyout = np.genfromtxt('pyout.csv',delimiter=',')
print pyout.size

transA = pyout.flatten()[0::3]
transB = pyout.flatten()[1::3]
transC = pyout.flatten()[2::3]


np.savetxt('samplesA.csv',transA,delimiter=',')
np.savetxt('samplesB.csv',transB,delimiter=',')
np.savetxt('samplesC.csv',transC,delimiter=',')