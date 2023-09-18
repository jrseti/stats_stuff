#include <iostream>

#include "BudGraphClass.h"
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    int year, week;
    char s[2048];
    FILE *fp;

    if(argc != 3) {
        cout << "Usage: weekly_data <year> <week>" << endl;
        return 0;
    }
    year = atoi(argv[1]);
    week = atoi(argv[2]);

    printf("Processing year = %d, week = %d\n", year, week);

    CBudGraphClass *budClass = new CBudGraphClass(year, week);

    string weekly_team_data = "INSERT INTO weekly values(";
	weekly_team_data += to_string(year);
	weekly_team_data += ", ";
    weekly_team_data += to_string(week);//POST SEASON ONLY COMMENT THIS OUT
	//weekly_team_data += "19";//ADD IN FOR POST SEASON POST SEASON ONLY
	weekly_team_data += ", ";

	string team_data = weekly_team_data;
	string team_number;
 
    // Print weekly_team_data
    
    sprintf(s, "../stats/%d/Week%02d/week%d.sql", year, week, week);
    fp = fopen(s, "w");

    for(int teams = 0 ; teams < 32 ; teams++) {

        sprintf(s, "%d, ", teams + 1);
        team_data += s;

        for(int var = 0 ; var < 200 ; var++) {

            sprintf(s,"%.2f",(double)budClass->m_w_vars[teams][week-1][var]);
            team_data += s;
			
			if(var < 199) {
				team_data += ", ";
            }
			else {
				//team_data += ")";
				team_data += ",0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0);\n";
            }

            
        }
        cout << team_data << endl;

        
        fprintf(fp, "%s", team_data.c_str());
        

        team_data = weekly_team_data;
    }

    fclose(fp);

    return 0;
}