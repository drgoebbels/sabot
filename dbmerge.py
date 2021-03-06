#Script for merging sabot databases 
#For now, this only merges the user table

import os
import sys
import sqlite3

def mergeDB(dbName):
    dbCon = None
    try:
        if os.path.isfile(dbName):
            dbCon = sqlite3.connect(dbName)
            db = dbCon.cursor()

            db.execute('SELECT name,password FROM user')
            for row in db:
                sadb.execute('SELECT id FROM user WHERE name=\'%s\'' % row[0])
                data = sadb.fetchone()
                if data is None:
                    sadb.execute('INSERT INTO user(name,password) VALUES(?,?)', row)             
        else:
            sys.stderr.write('Database %s does not exist\n' % dbName)
    except:
        sys.stderr.write('Failed to access Database: %s\n' % dbName)
    finally:
        if dbCon:
            dbCon.close()

if len(sys.argv) < 2:
    sys.stderr.write('No database specified to merge\n')
    sys.exit(1)
else:
    sadbCon = None
    try:
        if os.path.isfile('sabot.db'):
            sadbCon = sqlite3.connect('sabot.db')
            sadb = sadbCon.cursor()
        
            for arg in sys.argv[1:]:
                print 'merging: %s\n' % arg
                mergeDB(arg)
            sadbCon.commit()
        else:
            sys.stderr.write('Failed to access Database sabot.db\n')
            sys.exit(1) 
    except sqlite3.Error, e:
        sys.stderr.write('Failed to access Database sabot.db\n')
        sys.exit(1)
    finally:
        if sadbCon:
            sadbCon.close()

