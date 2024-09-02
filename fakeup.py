#!/usr/bin/env python
  
import sys
import os
import re

import teams

def getEntirestatsVarsNames(fname: str) -> list:
    
    # Read the entirestats.txt file and return a dictionary of the variable names
    # and their values
    
    # The entirestat
    o_names = []
    d_names = []
    in_offense = False
    in_defense = False
    with open(fname) as f:
        for line in f:
            if line.startswith("OFFENSE"):
                in_offense = True
                continue
            if in_offense and len(line) < 3:
                in_offense = False
                continue
            if line.startswith("DEFENSE"):
                in_defense = True
                continue
            if in_defense and len(line) < 3:
                in_defense = False
                return [o_names, d_names]
                continue
            
            if in_offense:
                o_names.append(line[:18].strip())
            if in_defense:
                d_names.append(line[:18].strip())
                
    return [o_names, d_names]

def getTeamsFilenames(year: int, week: int) -> list:
    # Return a list of filenames for the team stats for the given year and week
    # The list is sorted by team number
    
    stats_dir = os.getenv('STATSDIR')
    if stats_dir is None:
        print("STATSDIR environment variable not set")
        sys.exit(1)
    filepath = os.path.join(stats_dir, str(year), f"Week{week:02}")
    filenames = os.listdir(filepath)
    
    filenames = [os.path.join(filepath, filename) for filename in filenames if filename.startswith("team_") and filename.endswith(".txt")]
    filenames.sort()
    return filenames

def readTeamStatsFile(team_filename) -> list:
    
    values = []
    in_statistics_section = False
    in_score_section = False
    in_scoring_td_rush = False
    in_kickoffs = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Statistic"):
                in_statistics_section = True
                continue
            if in_statistics_section and len(line) < 3:
                in_statistics_section = False
                continue
            if in_statistics_section:
                parts = _getParts(line, 2)
                #print(parts)
                values.append([parts[0] , parts[1], parts[2]])
            if line.startswith("Score By Periods"):
                in_score_section = True
                continue
            if in_score_section and len(line) < 3:
                in_score_section = False
                continue
            if in_score_section:
                parts = _getParts(line, 6)
                #print(parts)
                
                if(parts[0] == "Team"):
                    values.append(["POINTS SCORED" , parts[6], None])
                if(parts[0] == "Opponents"):
                    values.append(["POINTS ALLOWED" , None, parts[6]])
                    
            if line.startswith("Scoring	TD	Rush"):
                in_scoring_td_rush = True
                continue
            if in_scoring_td_rush and len(line) < 3:
                in_scoring_td_rush = False
                continue
            if in_scoring_td_rush:
                if(line.startswith("Team")):
                    parts = _getParts(line, 8)
                    values.append(["TOUCHDOWNS", parts[1], [parts[2], parts[3], parts[4]]])
                if(line.startswith("Opponent")):
                    parts = _getParts(line, 8)
                    values.append(["OPP TOUCHDOWNS",  parts[1], parts[2], parts[3], parts[4]])
                    
            if(line.startswith("Kickoff Returns	No.	Yds")):
                in_kickoffs = True
                continue
            if in_kickoffs and len(line) < 3:
                in_kickoffs = False
                continue
            if in_kickoffs:
                if line.startswith("Team"):
                    parts = _getParts(line, 5)
                    values.append(["KICKOFF RETURNS", parts[1], parts[3], parts[5]])
                if line.startswith("Opponent"):
                    parts = _getParts(line, 5)
                    values.append(["OPP KICKOFF RET", parts[1], parts[3]])
                
    
    return values

def readTeamStatsFile2(team_filename) -> list:
    
    o_data = []
    d_data = []
    o_data.append(('OFFENSE', _getTeamName(team_filename)))
    d_data.append(('DEFENSE', _getTeamName(team_filename)))
    
    scores = _getScores(team_filename)
    d_data.append(('POINTS ALLOWED', scores[1]))
    
    o_data.append(('GAMES (Won-Lost)', _getGamesWonLost(team_filename)))
    
    first_downs = getFirstDowns(team_filename)
    o_data.append(('FIRST DOWNS', first_downs[0]))
    o_data.append((' Rushing', first_downs[2]))
    o_data.append((' Passing', first_downs[4]))
    o_data.append((' Penalty', first_downs[6]))
    
    d_data.append(('OPP FIRST DOWNS', first_downs[1]))
    d_data.append((' Rushing', first_downs[3]))
    d_data.append((' Passing', first_downs[5]))
    d_data.append((' Penalty', first_downs[7]))
    
    total_net_yards = _getTotalYards(team_filename)
    o_data.append(('YDS GAINED (tot)', total_net_yards[0]))
    o_data.append((' Avg per Game', total_net_yards[2]))
    d_data.append(('OPP YARDS GAINED', total_net_yards[1]))
    d_data.append((' Avg per Game', total_net_yards[3]))
    
    rushing = _getRushing(team_filename)
    o_data.append(('RUSHING (net)', rushing[0]))
    o_data.append((' Avg per Game', rushing[2]))
    o_data.append((' Rushes', rushing[4]))
    o_data.append((' Yards per Rush', rushing[6]))
    d_data.append(('OPP RUSHING (net)', rushing[1]))
    d_data.append((' Avg per Game', rushing[3]))
    d_data.append((' Rushes', rushing[5]))
    d_data.append((' Yards per Rush', rushing[7]))
    
    """

    Passing data is like
    PASSING (net)     2597    3252    3795    3765    2656    3484    3040    2739
 Avg per Game      173.1   216.8   253.0   251.0   177.1   232.3   202.7   182.6
 Passes Att.       536     569     539     536     494     503     463     465
 Completed         316     323     365     368     308     330     289     290
 Pct Completed      59.0    56.8    67.7    68.7    62.3    65.6    62.4    62.4
 Yards Gained     3042    3536    3984    4006    2936    3680    3277    3005
 Sacked             61      41      29      37      40      34      37      42
 Yards Lost        445     284     189     241     280     196     237     266
 Had intercepted    14      20      10       8      11      13      11      15
 Yards Opp Ret     278     375     166     107     120     183     141     137
 Opp TDs on Int      1       3       2       0       2       2       1       2
    """
    passing = _getPassing(team_filename)
    o_data.append(('PASSING (net)', passing[0]))
    o_data.append((' Avg per Game', passing[2]))
    o_data.append((' Passes Att.', passing[4]))
    o_data.append((' Completed', passing[6]))
    o_data.append((' Pct Completed', passing[8]))
    o_data.append((' Yards Gained', passing[10]))
    o_data.append((' Sacked', passing[12]))
    o_data.append((' Yards Lost', passing[14]))
    o_data.append((' Had Intercepted', passing[16]))
    o_data.append((' Yards Opp Ret', passing[19]))
    o_data.append((' Opp TDs on Int', passing[20]))
    
    """
        d_data is like:
        OPP PASSING(net)  2529    2402    3523    2760    3240    3762    2963    3562
 Avg per Game      168.6   160.1   234.9   184.0   216.0   250.8   197.5   237.5
 Passes Att.       463     462     506     446     454     587     489     552
 Completed         282     254     315     268     312     375     298     370
 Pct Completed      60.9    55.0    62.3    60.1    68.7    63.9    60.9    67.0
 Sacked             40      44      34      41      33      40      36      27
 Yards Lost        256     310     298     262     233     246     212     187
    """
    
    d_data.append(('OPP PASSING(net)', passing[1]))
    d_data.append((' Avg per Game', passing[3]))
    d_data.append((' Passes Att.', passing[5]))
    d_data.append((' Completed', passing[7]))
    d_data.append((' Pct Completed', passing[9]))
    d_data.append((' Sacked', passing[13]))
    d_data.append((' Yards Lost', passing[15]))
    
    punts = _getPunts(team_filename)
    o_data.append(('PUNTS', punts[0]))
    o_data.append((' Avg Yards', punts[1]))
    
    punt_returns = _getPuntReturns(team_filename)
    o_data.append(('PUNT RETURNS', punt_returns[0]))
    o_data.append((' Avg Return', punt_returns[2]))
    o_data.append((' Returned for TD', punt_returns[4]))
    
    kickoff_returns = _getKickoffReturns(team_filename)
    o_data.append(('KICKOFF RETURNS', kickoff_returns[0]))
    o_data.append((' Avg Return', kickoff_returns[2]))
    o_data.append((' Returned for TD', kickoff_returns[4]))
    
    interceptions = _getInterceptions(team_filename)
    d_data.append(('INTERCEPTIONS BY', interceptions[0]))
    d_data.append((' Yards Returned', interceptions[2]))
    d_data.append((' Returned for TD', interceptions[4]))
    
    d_data.append(('OPP PUNT RETURNS', punt_returns[1]))
    d_data.append((' Avg return', punt_returns[3]))
    
    d_data.append(('OPP KICKOFF RETURNS', kickoff_returns[1]))
    d_data.append((' Avg return', kickoff_returns[3]))
    
    penalties = _getPenalties(team_filename)
    o_data.append(('PENALTIES', penalties[0]))
    o_data.append((' Yards Penalized', penalties[2]))
    
    fumbles = _getFumbles(team_filename)
    o_data.append(('FUMBLES BY', fumbles[0]))
    o_data.append((' Fumbles Lost', fumbles[2]))
    o_data.append((' Opp Fumbles', fumbles[1]))
    o_data.append((' Opp Fum Lost', fumbles[3]))
    
    poss_avg = _getPosAvg(team_filename)
    o_data.append(('POSS. TIME (avg)', poss_avg[0]))
    
    touchdowns = _getTouchdowns(team_filename)
    o_data.append(('TOUCHDOWNS', touchdowns[0]))
    o_data.append((' Rushing', touchdowns[2]))
    o_data.append((' Passing', touchdowns[4]))
    o_data.append((' Returns', touchdowns[6]))
    d_data.append(('OPP TOUCHDOWNS', touchdowns[1]))
    d_data.append((' Rushing', touchdowns[3]))
    d_data.append((' Passing', touchdowns[5]))
    d_data.append((' Returns', touchdowns[7]))

    extra = _getExtra(team_filename)
    o_data.append(('EXTRA-PT KICKS', extra[0]))
    o_data.append(('2-PT CONVERSIONS', '1/1'))
    o_data.append(('FIELD GOALS/FGA', extra[1]))
    o_data.append(('POINTS SCORED', extra[2]))

    

    
    
    return [o_data, d_data]

def _getTeamName(team_filename) -> str:
   
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Statistic"): 
                parts = line.split()
                return parts[1]

    return "???"

def _getGamesWonLost(team_filename) -> str:
    
    with open(team_filename) as f:
        for line in f:
            #print(line)
            if line.startswith("Won") and "Lost" in line:
                parts = line.replace(",", "").split()
                #print(parts)
                return  f"{parts[1]}-{parts[3]}"

    return "?-?"

def _getParts(text, num_parts_at_end):
    
    parts_to_return = []
    parts = text.split()
    for i in range(num_parts_at_end):
        parts_to_return.append(parts.pop())
    # reverse the list
    parts_to_return.reverse()
    
    # now add the beginning part of the string to the list
    name = " ".join(parts)
    parts_to_return.insert(0, name)
    
    return parts_to_return
    
def getFirstDowns(team_filename) -> list:
    
    values = []
    in_statistics_section = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Statistic"):
                in_statistics_section = True
                continue
            if in_statistics_section and len(line) < 3:
                in_statistics_section = False
                continue
            if in_statistics_section:
                """
                Total First Downs	230	272
                Rushing	62	99
                Passing	143	138
                Penalty	25	35
                """
                #print(line)
                if (line.startswith("Total First Downs") or 
                    line.startswith("Rushing") or 
                    line.startswith("Passing") or
                    line.startswith("Penalty")):
                    parts = _getParts(line, 2)
                    #print(parts)
                    values.append(parts[1])
                    values.append(parts[2])
                    if line.startswith("Penalty"):
                        return values
    
    return values

def _getScores(team_filename) -> list:
    
    in_scores_section = False
    team_points = 0
    with open(team_filename) as f:
        for line in f:
            #print(line)
            if line.startswith("Score By Periods"):
                in_scores_section = True
                continue
            if in_scores_section and len(line) < 3:
                in_scores_section = False
                continue
            if in_scores_section:
                if line.startswith("Team"): 
                    parts = _getParts(line, 6)
                    #print(parts)
                    team_points = parts[6]
                if line.startswith("Opponents"): 
                    parts = _getParts(line, 6)
                    #print(parts)
                    return [team_points, parts[6]]

    
    return [None, None]

def _getTotalYards(team_filename) -> list:
    
    """
    Total Net Yards	3952	4422
    Avg. Per Game	263.5	294.8
    """
    got_total_net_yards = False
    total_net_yards = []
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Total Net Yards"):
                parts = _getParts(line, 2)
                total_net_yards = [parts[1], parts[2]]
                got_total_net_yards = True
            if got_total_net_yards and line.startswith("Avg. Per Game"):
                parts = _getParts(line, 2)
                total_net_yards.append(parts[1])
                total_net_yards.append(parts[2])
                return total_net_yards


    return [None, None, None, None]

def _getRushing(team_filename:str) -> list:
    
    """Rushing	No.	Yds
    """
    # rushing_net, opp_rushing_net, 
    # rushing_avg, opp_rushing_avg, 
    # rushes, opp_rushes, 
    # yards_per_rush, opp_yards_per_rush
    in_rushing_section = False
    next_is_avg = False
    values = [0] * 4
    with open(team_filename) as f:
        for line in f:
            if "Net Yards Rushing" in line:
                next_is_avg = True
                continue
            if next_is_avg:
                parts = _getParts(line, 2)
                rushing_avg = parts[1]
                opp_rushing_avg = parts[2]
                next_is_avg = False
                continue
            if line.startswith("Rushing	No.	Yds	Avg	Long"):
                in_rushing_section = True
                continue
            if in_rushing_section and len(line) < 3:
                in_rushing_section = False
                continue
            if in_rushing_section:
                if line.startswith("Team"): 
                    parts = _getParts(line, 5)
                    rushes = parts[1]
                    rushing_net = parts[2]
                    yards_per_rush = parts[3]
                    values
                if line.startswith("Opponents"): 
                    parts = _getParts(line, 5)
                    opp_rushes = parts[1]
                    opp_rushing_net = parts[2]
                    opp_yards_per_rush = parts[3]
                    return [rushing_net, opp_rushing_net, rushing_avg, opp_rushing_avg, rushes, opp_rushes, yards_per_rush, opp_yards_per_rush]
                
    return None     
            
def _getPassing(team_filename: str) ->list:
    
    """
    PASSING (net)     2597    3252    3795    3765    2656    3484    3040    2739
    Avg per Game      173.1   216.8   253.0   251.0   177.1   232.3   202.7   182.6
    Passes Att.       536     569     539     536     494     503     463     465
    Completed         316     323     365     368     308     330     289     290
    Pct Completed      59.0    56.8    67.7    68.7    62.3    65.6    62.4    62.4
    Yards Gained     3042    3536    3984    4006    2936    3680    3277    3005
    Sacked             61      41      29      37      40      34      37      42
    Yards Lost        445     284     189     241     280     196     237     266
    Had intercepted    14      20      10       8      11      13      11      15
    Yards Opp Ret     278     375     166     107     120     183     141     137
    Opp TDs on Int      1       3       2       0       2       2       1       2
    """
    # Return: [passing_net, opp_passing_net, avg_per_game, opp_avg_per_game, passes_att, opp_passes_att,
    #         completed, opp_completed, pct_completed, opp_pct_completed, yards_gained, opp_yards_gained,
    #         sacked, opp_sacked, yards_lost, opp_yards_lost, had_intercepted, opp_had_intercepted,
    #         yards_ret, opp_yards_ret, tds_on_int, opp_tds_on_int]
    in_section = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Net Yards Passing"):
                parts = _getParts(line, 2)
                passing_net = parts[1]
                opp_passing_net = parts[2]
                in_section = True
                continue
            
            if not in_section:
                continue
            
            if line.startswith("Avg. Per Game"):
                parts = _getParts(line, 2)
                avg_per_game = parts[1]
                opp_avg_per_game = parts[2]
                continue
            
            #Att./Completions	536/316	463/282
            if line.startswith("Att./Completions"):
                parts = _getParts(line, 2)
                passes_att = parts[1].split('/')[0]
                completed = parts[1].split('/')[1]
                opp_passes_att = parts[2].split('/')[0]
                opp_completed = parts[2].split('/')[1]
                continue
            
            if line.startswith("Sacked/Yards Lost"):
                parts = _getParts(line, 2)
                # Line is like: Sacked/Yards Lost	61/445	40/256
                sacked = parts[1].split('/')[0]
                opp_sacked = parts[2].split('/')[0]
                yards_lost = parts[1].split('/')[1]
                opp_yards_lost = parts[2].split('/')[1]
                continue
            
            
            
            #Completion Pct.	59.0	60.9
            if line.startswith("Completion Pct."):
                parts = _getParts(line, 2)
                pct_completed = parts[1]
                opp_pct_completed = parts[2]
                continue
            
            #Had Intercepted	14	14
            if line.startswith("Had Intercepted"):
                parts = _getParts(line, 2)
                had_intercepted = parts[1]
                opp_had_intercepted = parts[2]
                break
            
            #Gross Yards	3042	2785
            if line.startswith("Gross Yards"):
                parts = _getParts(line, 2)
                yards_gained = parts[1]
                opp_yards_gained = parts[2]
                continue
            
            #Sacked/Yards Lost	61/445	40/256
            if line.startswith("Sacked/Yards Lost"):
                parts = _getParts(line, 2)
                sacked = parts[1].split('/')[0]
                opp_sacked = parts[2].split('/')[0]
                yards_lost = parts[1].split('/')[1]
                opp_yards_lost = parts[2].split('/')[1]
                continue
            
    in_section = False
    with open(team_filename) as f:
        for line in f: 
            if line.startswith("Interceptions	No.	Yds	Avg	Long"):
                in_section = True
                continue   
            if in_section and len(line) < 3:
                in_section = False
                continue
            if in_section:
                if line.startswith("Team"):
                    parts = _getParts(line, 5)
                    yards_ret = parts[2]
                    tds_on_int = parts[5]  
                if line.startswith("Opponents"):
                    parts = _getParts(line, 5)
                    opp_yards_ret = parts[2]
                    opp_tds_on_int = parts[5]
                    in_section = False
                    break
                
    return [passing_net, opp_passing_net, avg_per_game, opp_avg_per_game, passes_att, opp_passes_att,
            completed, opp_completed, pct_completed, opp_pct_completed, yards_gained, opp_yards_gained,
            sacked, opp_sacked, yards_lost, opp_yards_lost, had_intercepted, opp_had_intercepted,
            yards_ret, opp_yards_ret, tds_on_int, opp_tds_on_int]
    
def _getPunts(team_filename: str) -> list:
    
    in_section = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Punting	No.	Yds	Avg	"):
                in_section = True
                continue
            if in_section and len(line) < 3:
                in_section = False
                continue
            if in_section:
                if line.startswith("Team"):
                    parts = _getParts(line, 8)
                    punts = parts[1]
                    avg = parts[3]
                    return [punts, avg]
                
    return [None, None]

def _getPuntReturns(team_filename: str) -> list:
    
    # Return [returns, opp_returns, avg, opp_avg, td, opp_td]
    in_section = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Punt Returns	Ret	FC	Yds"):
                in_section = True
                continue
            if in_section and len(line) < 3:
                in_section = False
                continue
            if in_section:
                if line.startswith("Team"):
                    parts = _getParts(line, 6)
                    returns = parts[1]
                    avg = parts[4]
                    td = parts[6]
                if line.startswith("Opponents"):
                    parts = _getParts(line, 6)
                    opp_returns = parts[1]
                    opp_avg = parts[4]
                    opp_td = parts[6]
                    in_section = False
                    break
                    
                
    return [returns, opp_returns, avg, opp_avg, td, opp_td]

def _getKickoffReturns(team_filename: str) -> list:
    
    
    # Returns [returns, opp_returns, avg, opp_avg, td, opp_td]
    returns, opp_returns, avg, opp_avg, td, opp_td = '0', '0', '0', '0', '0', '0'
    in_section = False
    next_should_be_opp = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Kickoff Returns	No.	Yds"):
                in_section = True
                continue
            if in_section and len(line) < 3:
                in_section = False
                continue
            if in_section:
                if line.startswith("Team"):
                    parts = _getParts(line, 5)
                    returns = parts[1]
                    avg = parts[3]
                    td = parts[5]
                    next_should_be_opp = True
                    continue
                    
                if line.startswith("Opponents"):
                    parts = _getParts(line, 5)
                    opp_returns = parts[1]
                    opp_avg = parts[3]
                    opp_td = parts[5]
                    in_section = False
                    next_should_be_opp = False
                    break
                
                if next_should_be_opp:
                    
                    break
                
                    
                
    return [returns, opp_returns, avg, opp_avg, td, opp_td]

def _getInterceptions(team_filename: str) -> list:
    
    # Returns [number, opp_number, avg, opp_avg, td, opp_td]
    in_section = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Interceptions	No.	Yds	Avg"):
                in_section = True
                continue
            if in_section and len(line) < 3:
                in_section = False
                continue
            if in_section:
                if line.startswith("Team"):
                    parts = _getParts(line, 5)
                    number = parts[1]
                    avg = parts[2]
                    td = parts[5]
                if line.startswith("Opponents"):
                    parts = _getParts(line, 5)
                    opp_number = parts[1]
                    opp_avg = parts[2]
                    opp_td = parts[5]
                    in_section = False
                    break
                    
                
    return [number, opp_number, avg, opp_avg, td, opp_td]
           
def _getPenalties(team_filename: str) -> list:
    # Penalties/Yards	111/866	85/719
    # return [penalties, opp_penalties, yards, opp_yards]

    with open(team_filename) as f:
        for line in f:
            if line.startswith("Penalties/Yards"):
                parts = _getParts(line, 3)
                #print(parts)
                penalties = parts[2].split('/')[0]
                yards = parts[2].split('/')[1]
                opp_penalties = parts[3].split('/')[0]
                opp_yards = parts[3].split('/')[1]
                return [penalties, opp_penalties, yards, opp_yards]
                
    return [None, None, None, None]

def _getFumbles(team_filename: str) -> list:
    # Fumbles/Ball Lost	28/16	26/8
    # return [fumbles, opp_fumbles, lost, opp_lost]

    with open(team_filename) as f:
        for line in f:
            if line.startswith("Fumbles/Ball Lost"):
                parts = _getParts(line, 3)
                #print(parts)
                fumbles = parts[2].split('/')[0]
                lost = parts[2].split('/')[1]
                opp_fumbles = parts[3].split('/')[0]
                opp_lost = parts[3].split('/')[1]
                return [fumbles, opp_fumbles, lost, opp_lost]
                
    return [None, None, None, None]
                
def _getPosAvg(team_filename: str) -> list:
    # Possession Avg.	30:00	30:00
    # return [pos_avg, opp_pos_avg]

    with open(team_filename) as f:
        for line in f:
            if line.startswith("Possession Avg."):
                parts = _getParts(line, 2)
                #print(parts)
                pos_avg = parts[1]
                opp_pos_avg = parts[2]
                return [pos_avg, opp_pos_avg]
                
    return [None, None]
            
def _getTouchdowns(team_filename: str) -> list:
    
    """
    Touchdowns	19	31
    Rushing	6	13
    Passing	10	16
    Returns	3	2
    """
    # returns [touchdowns, opp_touchdowns, rushing, opp_rushing, passing, opp_passing, returns, opp_returns]
    in_section = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Touchdowns"):
                in_section = True
            if in_section and len(line) < 3:
                in_section = False
                continue
            if in_section:
                if line.startswith("Touchdowns"):
                    parts = _getParts(line, 2)
                    touchdowns = parts[1]
                    opp_touchdowns = parts[2]
                if line.startswith("Rushing"):
                    parts = _getParts(line, 2)
                    rushing = parts[1]
                    opp_rushing = parts[2]
                if line.startswith("Passing"):
                    parts = _getParts(line, 2)
                    passing = parts[1]
                    opp_passing = parts[2]
                if line.startswith("Returns"):
                    parts = _getParts(line, 2)
                    returns = parts[1]
                    opp_returns = parts[2]
                    in_section = False
                    break
    return [touchdowns, opp_touchdowns, rushing, opp_rushing, passing, opp_passing, returns, opp_returns]

def _getExtra(team_filename: str) -> list:
    # Return [extra_pt_kicks, field_goals_fga, points_scored]
    in_section = False
    with open(team_filename) as f:
        for line in f:
            if line.startswith("Scoring	TD	Rush	Rec	Ret	K-PAT"):
                in_section = True
            if in_section and len(line) < 3:
                in_section = False
                continue
            if in_section:
                if line.startswith("Team"):
                    parts = _getParts(line, 8)
                    #line)
                    #print(parts)
                    extra_pt_kicks = parts[5]
                    field_goals_fga = parts[6]
                    points_scored = parts[8]
                    return [extra_pt_kicks, field_goals_fga, points_scored]
            
    return [None, None, None]
                
if __name__== "__main__":

    if len(sys.argv) != 3 :
        print(f'Syntax: {sys.argv[0]} <year> <week>')
        sys.exit(1)

    year = int(sys.argv[1])
    week = int(sys.argv[2])

    stats_dir = os.getenv('STATSDIR')
    if stats_dir is None:
        print("STATSDIR environment variable not set")
        sys.exit(1)
    filepath = os.path.join(stats_dir, str(year), f"Week{week:02}", "entirestats.txt")
    print(filepath)
    #o_names, d_names = getEntirestatsVarsNames(filepath)
    
    #print(o_names)
    #print(d_names)
    
    team_filenames = getTeamsFilenames(year, week)
    #print(team_filenames)
    
    """f = '/home/jrseti/stats_stuff/stats/2023/Week16/team_nyj.txt'
    team_stats = readTeamStatsFile2(team_filenames[2])
    offense = team_stats[0]
    defense = team_stats[1]
    #print(team_stats)
    for ts in offense:
        print(ts[0].ljust(18, ' '), ts[1].rjust(5, ' '))
        
    
        
    print("")"""
        
    """for ts in defense:
        print(ts[0].ljust(18, ' '), ts[1].rjust(5, ' '))"""
        
    for i in range(0, 32, 8):
        o_data = []
        d_data = []
        for j in range(i, i+8):
            print(team_filenames[j])
            offense, defense = readTeamStatsFile2(team_filenames[j])
            o_data.append(offense)
            d_data.append(defense) 
            
        for k in range(len(o_data[0])):
            for j in range(8):
                if j == 0:
                    print(o_data[j][k][0].ljust(18, ' '), end = '  ')
                print(o_data[j][k][1].rjust(5, ' '), end = '   ')
            print("")
            
        print("")
        for k in range(len(d_data[0])):
            for j in range(8):
                if j == 0:
                    print(d_data[j][k][0].ljust(18, ' '), end = '  ')
                #print("!!!!",i,j,k)
                print(d_data[j][k][1].rjust(5, ' '), end = '   ')
            print("")
            
       
        
