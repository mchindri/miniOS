import os, sys, time

floppyPath    = ".\\bin\\disk.img"
flpSector     = 64
flpSize       = 512 * flpSector

out = open(floppyPath, "wb")

ch = 'ABCDFGHIJKLMNOPQRSTUVWXYZ'
for j in range(0, flpSector):
    for i in range(0, 512):
        out.write(ch[j % 24].encode('ascii'))

#cleanup
out.close()