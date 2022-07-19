'''
Created on 2022/07/19

Synopsis:
  ./gen_nums_int.py 8192    //random
  ./gen_nums_int.py 8192 0  //equal

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
  with open('int_rand_'+n+'.txt', "w") as f:
      for i in range(num):
          r = random.randint(1, 1000)
          f.write(str(r)+'\n')
else:
  with open('int_eq_'+n+'.txt', "w") as f:
      for i in range(num):
          s = "12"
          f.write(s+'\n')   