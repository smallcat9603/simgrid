'''
Created on 2018/12/04

@author: huyao
'''

hosts = 1024
fn = "../output/hostfile.txt"
for i in range(hosts):
    s = "node-" + str(i) + ".simgrid.org" + "\n"
    f = open(fn, "a")
    f.write(s)
    f.close()