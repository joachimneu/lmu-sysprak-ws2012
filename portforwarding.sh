#! /bin/sh

ssh -N -L 127.0.0.1:1357:sysprak.priv.lab.nm.ifi.lmu.de:1357 -L 127.0.0.1:8080:sysprak.priv.lab.nm.ifi.lmu.de:80 $1@remote.cip.ifi.lmu.de
