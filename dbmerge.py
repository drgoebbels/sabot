#Script for merging sabot databases 

import sys
import sqlite3


if len(sys.argv) < 2:
    sys.stderr.write('No database specified to merge')
else:
    try:
        sadbCon = sqlite3.connect('sabot.db')
        sadb = sadbCon.cursor()
    except line.Error, e:
        sys.stderr.write('Failed to Connect to Database')
        sys.exit(1)
         

    
