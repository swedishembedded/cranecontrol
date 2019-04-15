#!/bin/sh

(cd theboss/src/libfirmware/; tig; git push origin master);
(cd theboss/src/libdriver/; tig; git push origin master);
(cd theboss/src/bossinit/; tig; git push origin master);
(cd theboss/; tig; git push origin master);
tig; git push origin master
