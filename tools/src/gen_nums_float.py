'''
Created on 2022/07/19

Synopsis:
  ./gen_nums_float.py 8192    //random
  ./gen_nums_float.py 8192 0  //equal

@author: huyao
'''

import sys
import random

if len(sys.argv) < 2:
  print("Data number is not specified")
  sys.exit()

n = sys.argv[1]
num = int(n)

diff = True
if len(sys.argv) > 2 and sys.argv[2] == '0':
  diff = False

if diff == True:
  with open('float_rand_'+n+'.txt', "w") as f:
      for i in range(num):
          r = random.uniform(0, 10)
          f.write(str(r)+'\n')
          # f.write(str(round(r, 9))+'\n') # .9
else:
  with open('float_eq_'+n+'.txt', "w") as f:
      for i in range(num):
          s = "0.123456789"
          f.write(s+'\n')   