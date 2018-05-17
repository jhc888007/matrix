#!/usr/bin/python

import sys
import matrixbuilder3 as mb


r = mb.reader(sys.argv[1], sys.argv[2])
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    print uid, r.get(uid, 20)


