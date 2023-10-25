#!/usr/bin/env python
  
import sys
import os
import re

import teams
from config import PREDICT_SCP_CMD, PREDICT_DB_CMD_1, PREDICT_DB_CMD_2, PREDICT_DB_CMD_3

debug = True

def get_report(year, week):

	cmd = "wget -O - http://mule.he.net/~budsport/pub/%d/predict.php?usewk=wk%d 2>/dev/null" % (year, week);
	#print(f'http://mule.he.net/~budsport/pub/{year}/predict.php?usewk=wk{week}')
	print(cmd)
	text = os.popen(cmd).read()
	return text

def parse_leaders(line):
	#print(line)
	leaders = []

	line = line.replace("AND", "")
	i = line.index(':')
	j = line.index('(')
	leaders.append(line[i+1:j-1].strip())
	line = line[j+1:-1]
	i = line.index(')')
	j = line.index('(')
	leaders.append(line[i+1:j-1].strip())
	line = line[j+1:-1]
	i = line.index(')')
	j = line.index('(')
	leaders.append(line[i+1:j-1].strip())
	line = line[j+1:-1]
	i = line.index(')')
	j = line.index('(')
	leaders.append(line[i+1:j-1].strip())

	return leaders

def parse_wildcards(line):

	wildcards = []
 
	parts = line.split('ARE')
	parts = parts[1].split('AND')
	wildcards.append(parts[0].strip())
	wildcards.append(parts[1].strip())
 
	return wildcards

class PredictPost:

	def __init__(self, year, week):
		self.year = year
		self.week = week
		self.nfc_leaders = []
		self.afc_leaders = []
		self.nfl_leaders = []
		self.nfc_wildcards = []
		self.afc_wildcards = []
		self.nfc_superbowl = ""
		self.afc_superbowl = ""
		self.winning_margin = ""
		self.super_bowl_champ = ""

	def parse(self, line):

		global debug

		line = line.replace(",", "")

		#if "THE MOST LIKELY NFC LEADERS ARE:" in line:
		if "THE MOST LIKELY NFC LEADERS ARE:" in line:
			self.nfc_leaders = parse_leaders(line)
			if(debug == True):
				print("NFC_LEADERS: |" + self.nfc_leaders[0] + "|")
				print("NFC_LEADERS: |" + self.nfc_leaders[1] + "|")
				print("NFC_LEADERS: |" + self.nfc_leaders[2] + "|")
				print("NFC_LEADERS: |" + self.nfc_leaders[3] + "|")
		#elif "THE MOST LIKELY AFC LEADERS ARE:" in line:
		elif "THE MOST LIKELY AFC LEADERS ARE:" in line:
			self.afc_leaders = parse_leaders(line)
			if(debug == True):
				print("AFC_LEADERS: |" + self.afc_leaders[0] + "|")
				print("AFC_LEADERS: |" + self.afc_leaders[1] + "|")
				print("AFC_LEADERS: |" + self.afc_leaders[2] + "|")
				print("AFC_LEADERS: |" + self.afc_leaders[3] + "|")
		#elif "THE MOST LIKELY NFC WILD CARD TEAMS ARE ") in line:
		elif "THE MOST LIKELY NFC WILD CARD TEAMS ARE" in line:
			self.nfc_wildcards = parse_wildcards(line)
			if(debug == True):
				print("NFC_WILDCARDS: |" + self.nfc_wildcards[0] + "|")
				print("NFC_WILDCARDS: |" + self.nfc_wildcards[1] + "|")
		elif "THE MOST LIKELY AFC WILD CARD TEAMS ARE" in line:
			self.afc_wildcards = parse_wildcards(line)
			if(debug == True):
				print("AFC_WILDCARDS: |" + self.afc_wildcards[0] + "|")
				print("AFC_WILDCARDS: |" + self.afc_wildcards[1] + "|")
		#elif "THE NFC CHAMPION SHOULD BE" in line:
		elif "THE NFC CHAMPION SHOULD BE" in line:
			# THE NFC CHAMPION SHOULD BE S FRANCISCO   THE AFC CHAMP KANSAS CITY
			exp = re.compile(r"THE NFC CHAMPION SHOULD BE\s+(.+)\s+THE AFC CHAMP\s+(.+)")
			self.nfc_superbowl, self.afc_superbowl = exp.findall(line)[0]
			self.nfc_superbowl = self.nfc_superbowl.strip()
			self.afc_superbowl = self.afc_superbowl.strip()
			"""line = line.replace("THE AFC CHAMP", ",");
			i = line.index(" BE ") + 3
			j = line.index(',')
			self.nfc_superbowl = line[i+1:j-1].strip()
			self.afc_superbowl = line[j+1:-1].strip()"""
			if(debug == True):
				print("NFC_SUPERBOWL: |" + self.nfc_superbowl + "|")
				print("AFC_SUPERBOWL: |" + self.afc_superbowl + "|")
		#elif "FAVORED BY" in line:
		elif "FAVORED BY" in line:
			# FAVORED BY  7 POINTS, THE SUPER BOWL CHAMP WILL BE S. FRANCISCO.
   			# Use regular expressions to parse this line FAVORED BY  7 POINTS, THE SUPER BOWL CHAMP WILL BE S. FRANCISCO.
			exp = re.compile(r" FAVORED BY\s+(\d+)\s+POINTS THE SUPER BOWL CHAMP WILL BE\s+(.+)\.")
			self.winning_margin, self.super_bowl_champ = exp.findall(line)[0]
			self.winning_margin = self.winning_margin.strip()
			self.super_bowl_champ = self.super_bowl_champ.strip()
			if(debug == True):
				print("SUPERBOWL CHAMP: " + self.super_bowl_champ + ", Winning margin: " + self.winning_margin)

	def to_sql(self):
		sql = "insert into predict_post set year=" + str(self.year) + ", week=" + str(int(week) + 1);
		sql += ", nfc_leader1=" + str(teams.name_to_num(self.nfc_leaders[0]));
		sql += ", nfc_leader2=" + str(teams.name_to_num(self.nfc_leaders[1]));
		sql += ", nfc_leader3=" + str(teams.name_to_num(self.nfc_leaders[2]));
		sql += ", nfc_leader4=" + str(teams.name_to_num(self.nfc_leaders[3]));
		sql += ", afc_leader1=" + str(teams.name_to_num(self.afc_leaders[0]));
		sql += ", afc_leader2=" + str(teams.name_to_num(self.afc_leaders[1]));
		sql += ", afc_leader3=" + str(teams.name_to_num(self.afc_leaders[2]));
		sql += ", afc_leader4=" + str(teams.name_to_num(self.afc_leaders[3]));
		sql += ", nfc_wildcard1=" + str(teams.name_to_num(self.nfc_wildcards[0]));
		sql += ", nfc_wildcard2=" + str(teams.name_to_num(self.nfc_wildcards[1]));
		sql += ", afc_wildcard1=" + str(teams.name_to_num(self.afc_wildcards[0]));
		sql += ", afc_wildcard2=" + str(teams.name_to_num(self.afc_wildcards[1]));
		sql += ", nfc_superbowl=" + str(teams.name_to_num(self.nfc_superbowl));
		sql += ", afc_superbowl=" + str(teams.name_to_num(self.afc_superbowl));
		sql += ", super_bowl_champ=" + str(teams.name_to_num(self.super_bowl_champ));
		sql += ", super_bowl_champ_winning_margin=" + self.winning_margin + ";";

		return sql;


class PredictInfo:

	def __init__(self, year, week, team_num, line1, line2):

		self.year = year;
		self.week = week + 1
		self.team_num = team_num
		
		line1 = line1.replace( "- ", "-");
		line2 = line2.replace( "- ", "-");
		line1 = line1.replace( "+ ", "+");
		line2 = line2.replace( "+ ", "+");
		line2 = line2[24:-1];
		line1 = line1.strip();
		line2 = line2.strip();

		line1Parts = line1.split();
		montePart = line1Parts[len(line1Parts) - 2];
		self.montePredicts = montePart.split("-");

		line2Parts = line2.split();
		regularPart = line2Parts[len(line2Parts) - 2];
		self.regularPredicts = regularPart.split("-");

		self.nextWeekScore = int(line2Parts[week]);

	def to_sql(self):

		sql = "insert into predict_wlt set year=" + str(self.year) + ", week=" + str(self.week)
		sql += ", team=" + str(self.team_num) 
		sql += ", monte_win=" + self.montePredicts[0]
		sql += ", monte_lose=" + self.montePredicts[1]
		sql += ", monte_tie=" + self.montePredicts[2]
		sql += ", predicted_win=" + self.regularPredicts[0]
		sql += ", predicted_lose=" + self.regularPredicts[0]
		sql += ", predicted_tie=" + self.regularPredicts[0]
		sql += ", winning_margin=" + str(self.nextWeekScore)
		sql += ";"

	
		return sql

if __name__== "__main__":

	if len(sys.argv) != 3 :
		print(f'Syntax: {sys.argv[0]} <year> <week>')
		sys.exit(1)

	

	year = int(sys.argv[1])
	week = int(sys.argv[2])
	predict_text = get_report(year,week)


	stats_dir = os.getenv('STATSDIR')
	if stats_dir is None:
		print("STATSDIR environment variable not set")
		sys.exit(1)
	filepath = os.path.join(stats_dir, str(year), f"Week{week:02}", "predict.sql")
	print(filepath)
	file = open(filepath, "w") 

	pp = PredictPost(year, week)

	for line in predict_text.splitlines():
                #print(line)

		if(len(line) < 6):
			continue

		pp.parse(line)

	line1 = ""
	line2 = ""

	for tname in teams.long_names_to_list():

		print(tname)
		tnum = teams.name_to_num(tname)

		for line in predict_text.splitlines():

			line = line.strip()[3:-1]

			if line.startswith(tname) and line.find("PREDICTED") > -1:
				pi = PredictInfo(year, week, tnum, line2, line)
				file.write(pi.to_sql() + "\n")

			line2 = line1
			line1 = line
			

	file.write(pp.to_sql())
	file.close()

	print(f"Predictions for {year} week {week+1} written to {filepath}")
 
	print("\nCopying to database")
	cmd = PREDICT_SCP_CMD.replace("PREDICT_FILE", filepath)
	print(cmd)
	os.system(cmd)
	cmd = PREDICT_DB_CMD_1.replace("PREDICT_YEAR", str(year))
	cmd = cmd.replace("PREDICT_WEEK", str(week+1))
	print(cmd)
	os.system(cmd)
	cmd = PREDICT_DB_CMD_2.replace("PREDICT_YEAR", str(year))
	cmd = cmd.replace("PREDICT_WEEK", str(week+1))
	print(cmd)
	os.system(cmd)
	cmd = PREDICT_DB_CMD_3
	print(cmd)
	os.system(cmd)


