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
year_2digit = str(year)[2:4]
filepath1 = os.path.join(stats_dir, str(year), f"{year_2digit}summar.txt")
print(filepath1)
filepath2 = os.path.join(stats_dir, str(year), "stats.txt")
print(filepath2)
filepath3 = os.path.join(stats_dir, str(year), "scores.txt")
print(filepath3)

zip_filepath = os.path.join(stats_dir, str(year), f"Week{week:02}", f"week{week:02}.zip")

cmd = f"rm -rf {zip_filepath}"
print(cmd)
os.system(cmd)
cmd = f"zip -j -r -D {zip_filepath} {filepath1} {filepath2} {filepath3}"
print(cmd)
os.system(cmd)

print(f"Zip file is at {zip_filepath}")

