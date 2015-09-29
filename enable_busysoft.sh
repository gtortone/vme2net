#!/bin/bash

# la scheda va in busysoft e ci resta fino a che non viene tolto via software
./sendvme W E8 1

# reset busy software
./sendvme W F8 0
#./sendvme W F8 1

# legge se siamo in busysoft
./sendvme R 108
