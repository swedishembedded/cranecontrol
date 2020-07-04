cat include/py/qstrdefs.h src/qstrdefs.h include/py/genhdr/qstrdefs.collected.h | 
	sed 's/^Q(.*)/"&"/' | 
	gcc -E -m32 -I. -Iinclude -Ibuild -Wall -Werror -Wdouble-promotion -Wfloat-conversion -std=c99  -Os -DNDEBUG -fdata-sections -ffunction-sections -DMICROPY_ROM_TEXT_COMPRESSION=1 - | 
	sed 's/^\"\(Q(.*)\)\"/\1/' > include/py/genhdr/qstrdefs.preprocessed.h
python3 makeqstrdata.py include/py/genhdr/qstrdefs.preprocessed.h > include/py/genhdr/qstrdefs.generated.h
python3 makemoduledefs.py --vpath="src/" modfb.c > include/py/genhdr/moduledefs.h
