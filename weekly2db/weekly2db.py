#!/usr/bin/env python

import sys
import os
sys.path.insert(0, f'{os.getcwd()}')
from config import WEEKLY_SCP_CMD, WEEKLY_DB_CMD_1, WEEKLY_DB_CMD_2


year = int(sys.argv[1])
week = int(sys.argv[2])

stats_dir = os.getenv('STATSDIR')
if stats_dir is None:
    print("STATSDIR environment variable not set")
    sys.exit(1)
filepath = os.path.join(stats_dir, str(year), f"Week{week:02}", f"week{week}.sql")
print(filepath)
#file = open(filepath, "w") 

cmd = WEEKLY_SCP_CMD.replace("WEEKLY_FILE", filepath)
print(cmd)
os.system(cmd)
cmd = WEEKLY_DB_CMD_1.replace("WEEKLY_YEAR", str(year)) 
cmd = cmd.replace("WEEKLY_WEEK", str(week))
print(cmd)
os.system(cmd)
cmd = WEEKLY_DB_CMD_2.replace("WEEKLY_YEAR", str(year))
cmd = cmd.replace("WEEKLY_WEEK", str(week))
print(cmd)
os.system(cmd)