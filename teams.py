#!/usr/bin/python
  
import sys
import os

names = [ 
	    ["ATL","ATLANTA","Atl."],
	    ["IND","INDIANAPOLIS","Ind."],
	    ["BUF","BUFFALO","Buff","Buff."],
	    ["CHI","CHICAGO","Chi","Chi."],
	    ["CIN","CINCINNATI","Cin."],
	    ["BAL","BALTIMORE","Balt","Balt.","Bal"],
	    ["DAL","DALLAS","Dall","Dal."],
	    ["DEN","DENVER","Denv","Den.","Denv."],
	    ["DET","DETROIT","Det."],
	    ["GB","GREEN BAY","G.B."],
	    ["TEN","TENNESSEE","Tenn."],
	    ["KC","KANSAS CITY","K.C."],
	    #["LA","L. A. RAMS","L.A.","LA-R"],
	    ["LAR","L. A. RAMS","L.A.","LA-R","RAM"],
	    ["MIA","MIAMI","Mia.","MIA.","mia."],
	    ["MIN","MINNESOTA","Minn","Minn."],
	    ["NE","NEW ENGLAND","NEW ENGL","N.E."],
	    ["NO","NEW ORLEANS","N.O."],
	    ["NYG","N.Y. GIANTS","NY-G","NYG","New York Giants"],
	    ["NYJ","N.Y. JETS","NY-J","NYJ","New York Jets"],
	    #["OAK","OAKLAND","Raiders","Rai.","oak."],
	    ["LVR","LAS VEGAS","Raiders","Rai.","oak."],
	    ["PHI","PHILADELPHIA","Phil","Phi.","Phil."],
	    ["PIT","PITTSBURGH","Pitt","Pitt."],
	    ["ARI","ARIZONA","Phoenix","Pho.","ari.","ariz","Ariz."],
	    ["LAC","L.A. CHARGERS","L A CHARGERS","LAC","LA-C","LA Chargers","L A Chargers"],
	    ["SF","S. FRANCISCO","S.F."],
	    ["SEA","SEATTLE","Sea."],
	    ["TB","TAMPA BAY","T.B."],
	    ["WAS","WASHINGTON","Wash","Wash."],
	    ["CAR","CAROLINA","car."],
	    ["JAC","JACKSONVILLE","Jac","Jax"],
	    ["CLE","CLEVELAND","Clev.","Clev","Clv"],
	    ["HOU","HOUSTON","Hou","Hou."],
	    ["All","NFL/","NFL","NFL/Avg"],
	    ["BYE","BYE","Bye","Bye."]
	]

def get_count():
	# Don't count "NFL" as a team, thus the -1
	return len(names) - 1

def long_names_to_list():
	name_list = []
	for team_names in names:
		#print(team_names)
		if len(team_names) > 1:
			name_list.append(team_names[1])
	return name_list
	
def name_to_num(name):
	index = 0
	for name_list in names:
		index += 1
		if(name.upper() in [n.upper() for n in name_list]):
					if index == 34: # BYE team is 0
						index = 0
					return index
	return -1

def num_to_abbrev(num):
	return names[num-1][0]

def num_to_abbrev3(num):
	""" Return the abbreviation, inserting a space when the abbrev is 2 chars """
	abbrev = names[num-1][0]
	if(len(abbrev) < 3):
		return abbrev[0] + " " + abbrev[1]
	return abbrev
		
def team_name_list_to_num_list(team_list):

	return [name_to_num(name) for name in team_list]
		
if __name__== "__main__":
	
	"""for i in range(1, len(names)+1):
		print ("%d: %s,%s" % (i, num_to_abbrev(i), num_to_abbrev3(i)))"""

	for name_list in names:
		print ("%s: %d" % (name_list[1], name_to_num(name_list[1])))

	t = ["CHICAGO", "ATL"]
	print (team_name_list_to_num_list(t))
		

	

