lmu-sysprak-ws2012
==================

HowTo Port-Forwarding:
----------------------

Um von außerhalb des MWN den Gameserver benutzen zu können, verwende bitte das
Skript "portforwarding.sh" mit deinem CIP-Usernamen als Argument:

./portforwarding.sh neujo

Dann werden zwei Weiterleitungen eingerichtet:

localhost:8080 <===> sysprak.priv.lab.nm.ifi.lmu.de:80 (damit kannst du den
Gameserver steuern, Spiele anlegen etc.)

localhost:1357 <===> sysprak.priv.lab.nm.ifi.lmu.de:1357 (der eigentliche
Gameserver lauscht hinter diesem Port)

Die Weiterleitung funktioniert natürlich auch von innerhalb des MWN und sollte
deshalb immer verwendet werden. Deshalb wurde auch in network.h vorübergehend
"localhost" als Hostname des Gameservers eingetragen.
