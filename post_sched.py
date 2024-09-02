#!/usr/bin/env python
# 
# # After week 18 we need to put the schedule starting at week 1

import os
import sys
import re



def print_help():
	print("Syntax error:")
	print("\t%s: <year (like 2018)> <week number (like 5)>" % sys.argv[0])
	print("\n")
	sys.exit(1)
 
if(len(sys.argv) <= 2):
	print_help()

year = int(sys.argv[1])
week = int(sys.argv[2])

SCHED_FILENAME = f"scheds/{year}_post/sched.txt"
DB_FILENAME = f"scheds/{year}_post/week{week}.db"

def getTeamNames():
    team_names = []
    with open(SCHED_FILENAME, "r") as f:
        lines = f.readlines()
        for line in lines[1:]:
            line = line.strip()
            parts = line.split()
            if len(parts) > 0:
                team_names.append(parts[0])
    return team_names[:32]

def readSchedule():
    sched = []
    with open(SCHED_FILENAME, "r") as f:
        lines = f.readlines()
        for line in lines[1:]:
            line = line.strip()
            line = line[:3] + ',' + line[3 + 1:]
            line = line[:7] + ',' + line[7 + 1:]
            parts = line.split(',')
            if len(parts) > 0:
                sched.append(parts[2:])
    return sched[:-3]

def readGames():
    games = []
    with open(DB_FILENAME, "r") as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            if line.startswith("|"):
                parts = line.split("|")
                team = parts[2].strip()
                opponent = parts[4].strip()
                if int(team) < 33 and int(opponent) < 33:
                    #print(f"{team} vs {opponent}")
                    games.append((team, opponent))
    return games

def getOppForTeam(team_num, games):
    for game in games:
        if int(game[0]) == team_num:
            return int(game[1])
        if int(game[1]) == team_num:
            return -int(game[0])
        
    return 0

                
games = readGames()
print(games)
team_names = getTeamNames()
print(team_names)
sched = readSchedule()
print("Before sched change:")
for sch in sched:
    print(sch)

column_to_change = week - 19
team_num_column = 1
print(f"Changing column {column_to_change}")

index = 1
for sch in sched:
    print(sch)
    team_num = index
    opp_num = getOppForTeam(team_num, games)
    sch[column_to_change] = str(opp_num)
    index += 1
    #print(f"Team {team_num} vs {opp_num} -> {team_names[abs(opp_num) - 1]}")
    
print("After column change:")
for sch in sched:
    print(sch)   
    
#sys.exit(0)
    
print(f"{year}      1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18") 
index = 0
for sch in sched:
    line = f"{team_names[index]:4} {str(index+1).rjust(2):2} "
    #print(f"{team_names[index]:4} {str(index+1):2} ", end='')
    #print(f"{sch[0]:4} {sch[1]:2} ", end='')
    for opp in sch:
        line += f"{opp.rjust(3):3},"
        #print(f"{opp.rjust(3):3},", end='')
    line = line[:-1]
    print(line)
    index += 1
    
#sys.exit(0)

    
# Write the new schedule to a file
with open(f"scheds/{year}_post/sched.txt", "w") as f:
    f.write(f"{year}      1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18\n")
    index = 0
    for sch in sched:
        line = f"{team_names[index]:4} {str(index+1).rjust(2):2} "
        for opp in sch:
            line += f"{opp.rjust(3):3},"
        line = line[:-1]
        f.write(line + "\n")
        index += 1
    f.write("\n")
    f.write("37526618227845253158164844313767\n")
    f.write("11211122222211211212121121111211\n")
    f.close()
            