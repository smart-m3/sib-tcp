sib-tcp
=======
Configure with --enable-hipdex to enable hipdex encryption before make.
After that sib-tcp will work only with hipdex compatible KP (e.g. ANSI-C-KPI).
sib-tcp will need key.dat file, which can be generated with /hip-dex/binaries/gen utility. It will also generate HIT.txt file with Host Identity Tag for generated key, which have to be used on KP together with SIB ip address.