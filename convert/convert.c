/***************************************************/
/* Convert program for Bud Goode.                  */
/* Written by Jon Richards.                        */
/*                                                 */
/* This program will convert a weekly NFL Football */
/* stat files to a bud stats stat file in Buds     */
/* stat file format.                               */
/***************************************************/

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>


FILE     *fp_in;   /* File pointer for NFL stat input file.    */
FILE     *fp_out;  /* File pointer for Buds output stat file.  */
FILE     *fp_data; /* File pointer for the "convert.dat" file. */
FILE     *fp_trace;/* File pointer for "trace.dat" file.       */

char input_filename[1024];
char stats_directory[1024];
char temp_filename[1024];

int year;
int week;

#define DEBUG 1
#define TEAMS_PER_PAGE 11
#define NUM_PAGES 3

#define NUM_VARS 78

typedef struct _Info
  {
    int   team_count;    /* Number of teams listed in convert.dat.         */
    char  **tnam;        /* team names output to the out stat file.        */
    int   *torder;       /* order of the teams as they appear in the       */
			 /* NFL STATS file read in.                        */
    char  tlist[40][10][16];
			 /* Team names that are recognized in the NFL stat */

    int   vorder[NUM_VARS+1];    /* order of the variables teams as they appear in */
			 /* the NFL STATS file read in.                    */
    int   week;          /* Week number read in from the NFL stats input.  */
    char  day[16];       /* Week Day read in from the NFL stats input.     */
    int   year;          /* Year read in from the NFL stats input.         */
    int   monday1;       /* Team to play on Monday Night.                  */
    int   monday2;       /* Team to play on Monday Night.                  */
    int   missing1;      /* Team missing (if any) from the NFL stats.      */
    int   missing2;      /* Team missing (if any) from the NFL stats.      */

    char  *convert_data_filename;
			 /* Full path to the file containing the convert   */
			 /* data, usually convert.dat.           */
    int
	  sched[40][20]; /* Schedule, up to 40 teams, 20 weeks.  */
    int   scores[40][20];/* Scores, up to 40 teams, 20 weeeks.   */

  } Info;

Info info;



typedef struct _TeamData
  {
    int w;         /* Wins for a team.   */
    int l;         /* Losses for a team. */
    int t;         /* Ties for a team.   */
    float v[NUM_VARS];   /* Variable for a team. */
    char  tmm[6];  /* Team minutes read in from the NFL Stats .      */
  } TeamData;

TeamData *data;

int GetTeamCount();
void GetSchedule();
void GetScores(char *statsdir);
void read_in_convert_data();
void WriteNewScoresFile(char *statsdir);
void WriteNewStatFile(char *statsdir);

/***************************************************/
/* This is the main routine.  Live long and enjoy. */
/***************************************************/

unsigned int main(unsigned int argc, char *argv[])
{
  char  string[256];
  char  dummy[256];
  int   i,j,m,k;       /* General counting variable.                     */
  char  answer;      /* Yes or no answer from the user.                */
  int   page,total_pages;
  int   jstart,jstop;

  int d;

  char statsdir[256];

  // Get the env variable STATSDIR, if NULL print error and return
  if(getenv("STATSDIR")==NULL)
  {
    printf("ERROR: STATSDIR environment variable not set.\n");
    exit(0);
  }
  strcpy(statsdir,getenv("STATSDIR"));

  /****************************************/
  /* Print out help message if necessary. */
  /****************************************/

  if((argc < 2) || (strcmp(argv[1], "-h") == 0) ||
    (strcmp(argv[1], "help") == 0)) {
      fprintf(stderr,"\n**********************************************\n");
      fprintf(stderr,"\n convert syntax:\n");
      fprintf(stderr,"\n     convert <year> <week> ");
      fprintf(stderr,"\n               or for this message:");
      fprintf(stderr,"\n     convert help\n\n");
      fprintf(stderr,"\n Example:\n\n     convert in\n");
      fprintf(stderr,"\n**********************************************\n\n");
      exit(0);
  }

  year = atoi(argv[1]);
  week = atoi(argv[2]);

  sprintf(input_filename,"%s/%d/Week%02d/entirestats.txt",statsdir, year,week);

  /************************/
  /* Open the trace file. */
  /************************/
  sprintf(temp_filename,"%s/%d/Week%02d/trace.txt",statsdir, year,week);
  fp_trace = fopen(temp_filename, "w");
  //fp_trace = stdout;

  /***************************/
  /* Say hello the the user. */
  /***************************/

  printf("\n----------------------");
  printf("\n| Welcome to Convert | ");
  printf("\n----------------------\n\n");


  /*************************/
  /* Initialize stuff. . . */
  /*************************/

  /* Create the file name. */
  info.convert_data_filename = (char *)calloc(768,sizeof(char));
  strcpy(info.convert_data_filename,"./convert/convert.dat");

  info.team_count = (int)GetTeamCount();
  info.monday1  = -1;
  info.monday2  = -1;
  info.missing1 = -1;
  info.missing2 = -1;
  info.week     = 0;
  info.year     = 0;
  info.day[0]   = 0;

 // printf("\nHERE TEAMCOUNT=%d",info.team_count );getchar();
 // exit(0);

  /* Allocate the team names. */
  info.tnam     = (char **)calloc(info.team_count,sizeof(char *));
  for(i=0;i<info.team_count;i++)
    info.tnam[i] = (char *)calloc(16,sizeof(char));

  /* Allocate the team names list. */
/*
  info.tlist = (char ***)calloc(info.team_count,sizeof(char **));
  for(i=0;i<info.team_count;i++)
    info.tlist[i] = (char **)calloc(10,sizeof(char *));
  for(i=0;i<info.team_count;i++)
    info.tlist[i] = (char *)calloc(16,sizeof(char));
*/


  /* Allocate the team order. */
  info.torder = (int *)calloc(info.team_count,sizeof(int));

//printf("\nHERE 1");getchar();
  /* Clear the variable order. */
  for(i=0;i<info.team_count;i++)
    info.vorder[0]=0;
//printf("\nHERE 2");getchar();
  /* Allocate the TeamData structures. */
  data = (TeamData *)calloc(info.team_count,sizeof(TeamData));

  /**********************************************************/
  /* Open the NFL Stats input file. This file name has been */
  /* specified in the command line.                         */
  /**********************************************************/
//printf("\nHERE 21");getchar();
  sprintf(input_filename, "%s/%d/Week%02d/entirestats.txt",statsdir,year,week);
  if (NULL == (fp_in = fopen(input_filename, "r")))
    {
      fprintf(fp_trace,"\nCould not open the file \"%s\"",input_filename);
      exit(0);
    }
printf("\nHERE 3");
  /******************************************************************/
  /* Read in input file to determine the Week number, day and Year. */
  /******************************************************************/

  info.year = 0;
  while((info.year == 0)&&(fgets(string, 128, fp_in)))
    {
      /* Read in a line from the input file. */
      //fgets(string, 128, fp_in);

      /**************************************************************/
      /* If the line read in contains the word "THROUGH" then it is */
      /* The line that contains the Week, Day and Year.             */
      /**************************************************************/

      if((strstr(string,"THROUGH") != NULL) ||
	 (strstr(string,"through") != NULL) ||
	 (strstr(string,"Through") != NULL))
	{
	  /* Remove all commas in the line. */

	  for(i=0;i<(int)strlen(string);i++)
	    if(string[i] == ',')string[i] = ' ';

	  /* Parse up the string. */

	  sscanf(string,"%s %d %s %s %s %s %s %d ",
		 dummy,&info.week,dummy,dummy,info.day,dummy,dummy,&info.year);
	  //if(info.year==2000)info.year=1999;

	}
      }

      if(info.year==0)
	 {
	   printf("Warning!\nA Date/Week Line was not found in the\nIN file. Add one like:\n  WEEK 1 / THROUGH SUNDAY SEPTEMBER 3, 1995\n\nThen rerun convert again!!\n\n");
	   exit(-1);
	 }
      /****************************/
      /* rewind to the beginning. */
      /****************************/
      rewind(fp_in);

      /*******************************************************/
      /* Ask the user if the Day, Week and Year are correct. */
      /*******************************************************/

//printf("\n\nHERE 4");getchar();
      sprintf(string,"\nConvert has read the following data from the file \"%s\":",
	      argv[1]);
      printf("%s",string);
      printf("\n");
      printf("\n Week:   %d",info.week);
      printf("\n Year:   %d",info.year);
      printf("\n Through %s",info.day);
      printf("\n");
      printf("\n Is this correct? (y or n). . . ");
      answer = getchar();

      if((answer == 'N') || (answer == 'n'))
	{
	  fprintf(fp_trace,"\n\nCorrect the problem by editing \"%s\" and then",argv[1]);
	  fprintf(fp_trace," run Convert again!\n\n");
	  exit(-1);
	}

//printf("\nHERE 5");
  /***************************************************************/
  /* If the Day is Monday then ask for the 2 teams that have not */
  /* played yet.                                                 */
  /***************************************************************/
/* FIRST WEEK THURS comment out*/
  if(strncasecmp(info.day,"mon",3)) {
      printf("\n\n Enter the number of the first team that plays on Monday Night(1 thru %d). . .",info.team_count);
      fgets(string, sizeof(string), stdin);
      //gets(string);clear

      sscanf(string,"%d",&info.monday1);
      printf("\n\n Enter the number of the second team that plays on Monday Night(1 thru %d). . .",info.team_count);
      fgets(string, sizeof(string), stdin);
      sscanf(string,"%d",&info.monday2);

      printf("\n\n You entered teams %d and %d. Thank You!",
	     info.monday1,info.monday2);
    }

//printf("\nHERE 5");
  printf("\n\n Reading in convert.dat . . .");
  fprintf(fp_trace,"\n\n Reading in convert.dat . . .");
//printf("\nHERE 6");
  GetSchedule();
//printf("\nHERE 7");
  GetScores(statsdir);
//printf("\nHERE 8");  
  read_in_convert_data();
//printf("\nHERE 9");

/****************************************/
/****************************************/
/****************************************/
/****************************************/

  /*****************************/
  /* Find the number of pages. */
  /*****************************/

  //total_pages = (int)(info.team_count/8)+1;
  //JR MTEMP!!!
  // total_pages=4;

  /********************************************************/
  /* Read the "in" file to find the Word PAGE or OFFENSE. */
  /* This is the beginning of a page.                     */
  /********************************************************/

 
 total_pages=3; //Regular
 // total_pages=4; //first week, 2 teams on Monday, only 4 pages.
 // total_pages=2;//MODIFIED FOR POST SEASON
  //total_pages = 1; //Modified for first week on Thurs only one team playing
  //total_pages=1;//First week, Thursday


 //for (page = 1; page <= total_pages; page++) //Reg season
  //for(page=1;page<=4;page++) //Use this one, first week, monday

  //for(page=1;page<=2;page++)//POST
  for(page=1;page<=total_pages;page++)//MODIFIED FOR POST SEASON
  {
	  
    printf("\nHERE, page %d\n",page);
    sleep(1);
    /***********************************************/
    /* Find the top of the next page.              */
    /* read in the line containing the team names. */
    /***********************************************/
    fgets(string, 128, fp_in);

	  printf("%s\n", string);
    while(strncasecmp(string,"PAGE",4) && strncasecmp(string,"OFFE",4)) {
      fgets(string, 128, fp_in);
    }

    if(DEBUG) {
	    printf("\n\n Reading Page %d\n%s\n",page,string);
    }

    /****************************************************/
    /* Find the indices of the team group on this page. */
    /****************************************************/

    jstart = (page-1)*TEAMS_PER_PAGE;
    jstop  = page*TEAMS_PER_PAGE-1;

	  
    if(page == total_pages) /* Last page. */
	  {
		  //jstop = 28;
	 // jstart = (total_pages-1)*8;
 	 // jstop  = info.team_count-1;
	//	  jstart=32;
	//	  jstop=32;
	  //jstop=1;

	  //JR TEMP@!!!
	  //jstop = jstop-=2;

	  //if(info.missing1 > -1) /* 2 teams may be missing on week 1. */
	  //  {
	   //   jstart = jstart + 2;
	   // }
	//	  jstop = 11;
	  }

	  jstart = (page-1)*TEAMS_PER_PAGE;
    jstop  = page*TEAMS_PER_PAGE-1;
	  //jstop = 11;
	  //MODIFIED FOR POST SEASON GAMES
	  
	  /*
	 jstart = 0; 
      jstop  = 7;
	  if(page==2)
	 {
		  jstart=8;
		  jstop=11;
	  }
	  */
	  

	  
/*
	  jstart=0;
		  jstop=1;
*/
  if(page == NUM_PAGES) {
    jstop = 32; //Regular season!
  }

	  //if(page == 4) jstop = 30; //first week, monday, JRXX

	  //FIRST WEEK Monday morning, 2 teams missing.
	  //if(page==4)jstop=30;

  /***********************************/
  /* Find out which teams these are. */
  /***********************************/


  /****************************************************************/
  /* Sort thru the line and figure out which team numbers are for */
  /* each column.                                                 */
  /****************************************************************/

  /* Read past the word OFFENSE, up to the first team name. */
  char *temp_pp;
  temp_pp = strtok(string," ");

  if(DEBUG) {
    fprintf(fp_trace, "Team list: %s\n", temp_pp);
  }

  for(j=jstart;j<=jstop;j++)
	{
    char *pp;
    pp= strtok(NULL," \r\n\0");
    strcpy(dummy,pp);
    printf("TEAM = %s\n",dummy);
    fprintf(fp_trace,"***TEAM NAME %s\n",dummy);
    for(i=0;i<info.team_count;i++) {
      for(m=0;m<10;m++) {
        if(info.tlist[i][m][0]!=0){
          if(!strncasecmp(dummy,info.tlist[i][m],4)) {
            info.torder[j]=i;
            fprintf(fp_trace,"TORDER %d, %s\n",i+1,info.tlist[i][m]);
          }
        }
      }
    }

	   if(info.torder[j] == -1)
	     {
	       fprintf(fp_trace,"\n**************************************************\n");
	       fprintf(fp_trace,"\nERROR!\n");
	       fprintf(fp_trace,"On page %d the team %s is not recognized!",page+1,dummy);
	       fprintf(fp_trace,"\nAdd the name to the \"convert.dat\" file and try again.");
	       fprintf(fp_trace,"\n**************************************************\n");
	       exit(0);
	     }

	}

      /***************************************/
      /* Read in the variables on this page. */
      /***************************************/

      for(m=1;m<NUM_VARS;m++)
	{
	  fprintf(fp_trace,"TOP: vorder[%d] = %d, \n", m,
		      info.vorder[m]);

    if(m == 1)
    {
      /* Read in the Win-Lost-Tie string. */
      fgets(string, 128, fp_in);
      if(strstr(string,"------")!=NULL) {
        fgets(string, 128, fp_in); /* Read past a line of ----- */
      }
	    if(!strncasecmp(string,"-MORE-",6))fgets(string, 128, fp_in);
	    //  fprintf(fp_trace,"Page %d, var %d, \n%s",page,m,string);

      /* Parse it up. */
      //k = 16; Starting 2024 the Won-Loss is WLT
      //k = 14;
      k = strlen("GAMES (W-L-T)");
      for(i=jstart;i<=jstop;i++) {
		  while((k<(int)strlen(string))&&(string[k] !=  ' '))k++;
		  while((k<(int)strlen(string))&&(string[k] ==  ' '))k++;
		  if(k<(int)strlen(string))
			  data[info.torder[i]].w = atoi(string+k);
		  else
			  data[info.torder[i]].w =0;

		  while((k<(int)strlen(string))&&(string[k] !=  '-'))k++;k++;
		  if(k<(int)strlen(string))
			data[info.torder[i]].l = atoi(string+k);
		  else
			data[info.torder[i]].l ==0;
		  while((k<(int)strlen(string))&&((string[k] !=  '-') && (string[k] !=  ' ')))k++;k++;
		  if(string[k-1] ==  '-')
		    data[info.torder[i]].t = atoi(string+k);
		  else
			  data[info.torder[i]].t = 0;

		  data[info.torder[i]].v[1]=data[info.torder[i]].w+
					     data[info.torder[i]].l+
					     data[info.torder[i]].t;
		  fprintf(fp_trace,"WLT: %d %d %d\n", data[info.torder[i]].w,
					     data[info.torder[i]].l,
					     data[info.torder[i]].t );


	      }

	    }
	  else if(info.vorder[m] == -1)
	    {
	      /* Dont use this line, just read past it. */
	      fgets(string, 128, fp_in);
        fprintf(fp_trace, "\n***\nRead past: %s***\n", string);
	      if(!strncasecmp(string,"-MORE-",6))fgets(string, 128, fp_in);
	      //fprintf(fp_trace,"Page %d, var %d, \n%s",page,m,string);
        continue;
	    }
	  else if(info.vorder[m] == 40)
	    {
	      /* read in FIELD GOALS/FGA */
	      fgets(string, 128, fp_in);
	      if(!strncasecmp(string,"-MORE-",6))fgets(string, 128, fp_in);
	      fprintf(fp_trace,"THIS IS FGA2:\n");
	      fprintf(fp_trace,"Page %d, var %d, \n%s",
		      page,info.vorder[m],string);

	      for(i=jstart;i<=jstop;i++)
		{
 
      if(i != 32) {
        if(i==jstart) {
          //2024 they changed FIELD GOALS/FGA to FIELD GOALS MADE/ATTEMPTED
          data[info.torder[i]].v[40] = atol(strtok(string+26," /"));
        }
        else {
          data[info.torder[i]].v[40] = atol(strtok(NULL," /"));
        }
        data[info.torder[i]].v[41]   = atol(strtok(NULL," /"));
      }
		}
	    }
	  else if(info.vorder[m] == 98)
	    {
	      /* Read in the team minutes. */
	      fgets(string, 128, fp_in);
	      if(!strncasecmp(string,"-MORE-",6))fgets(string, 128, fp_in);
	      fprintf(fp_trace,"Page %d, var %d, \n%s",page,m,string);

	      for(i=jstart;i<=jstop;i++)
		{
		  if(i==jstart)
		    strcpy(data[info.torder[i]].tmm,strtok(string+16," "));
		  else
		    strcpy(data[info.torder[i]].tmm,strtok(NULL," "));
		  data[info.torder[i]].tmm[4]=0;
		  for(k=0;k<(int)strlen(data[info.torder[i]].tmm);k++)
		    if(data[info.torder[i]].tmm[k]==':')
		      data[info.torder[i]].tmm[k]='.';
		}

	    }
	  else if(info.vorder[m] == 43)
	    {
	      /* Skip to points allowed. */
	      fgets(string, 128, fp_in);
	      if(!strncasecmp(string,"-MORE-",6))fgets(string, 128, fp_in);
	      fprintf(fp_trace,"Page %d, var %d, \n%s",page,m,string);

	      while(strncasecmp(string,"POINTS ALLOWED",13))
		{
		  fgets(string, 128, fp_in);
		  if(!strncasecmp(string,"-MORE-",6))fgets(string, 128, fp_in);
		  fprintf(fp_trace,"Page %d, var %d, \n%s",page,m,string);
		}

	      for(i=jstart;i<=jstop;i++)
		{
		  if(i==jstart)
		    data[info.torder[i]].v[info.vorder[m]] =
		      (float)atof(strtok(string+16," "));
		  else
		    data[info.torder[i]].v[info.vorder[m]] =
		      (float)atof(strtok(NULL," "));
fprintf(fp_trace,"Data team %d, var %d = %f\n",info.torder[i]+1,info.vorder[m]+1,data[info.torder[i]].v[info.vorder[m]]);

		}

	    }
	  else
	    {
	      /* Read in the stats. */
	      fgets(string, 128, fp_in);
	      if(!strncasecmp(string,"-MORE-",6))fgets(string, 128, fp_in);
	      fprintf(fp_trace,"Page %d, var %d, \n%s",page,m,string);

	      if(info.vorder[m] == 39)fprintf(fp_trace,"Var 39!!!!!!");

		  for (i = jstart; i <= jstop; i++) {

			  d = 0;
			  while (!isdigit(string[d]) && (string[d] != '-'))d++;
			  d--;
			  if (i == jstart)
				  if (info.vorder[m] == 39) {
					  data[info.torder[i]].v[info.vorder[m]] =
						  (float)atof(strtok(string + d, " "));
				  }
				  else {
					  data[info.torder[i]].v[info.vorder[m]] =
						  (float)atof(strtok(string + d, " "));
				  }

			  else {
				  data[info.torder[i]].v[info.vorder[m]] =
					  (float)atof(strtok(NULL, " "));
			  }

			  if (info.vorder[m] == 39) {
				  fprintf(fp_trace, "%f ",
					  data[info.torder[i]].v[info.vorder[m]]);
			  }
		  }
		
	    }
	}

    }/* End of for(page=0;page<total_pages;page++) */

    fprintf(fp_trace,"\nDONE reading in stats\n");

  /*****************/
  /* Var 8 is 6-7. */
  /*****************/

  for(i=0;i<info.team_count;i++)
    {
      fprintf(fp_trace,"\nPRE V8:%f, %f, %f\n", 
              data[info.torder[i]].v[8],
              data[info.torder[i]].v[6],
              data[info.torder[i]].v[7]);
      data[info.torder[i]].v[8]=
	      data[info.torder[i]].v[6]-data[info.torder[i]].v[7];
      fprintf(fp_trace,"\nPOST V8:%f, %f, %f\n", 
              data[info.torder[i]].v[8],
              data[info.torder[i]].v[6],
              data[info.torder[i]].v[7]);
    }

   fprintf(fp_trace,"\nDONE doing var 8\n");

  /*****************************/
  /* Define variable 1, w+l+t. */
  /*****************************/

  for(i=0;i<info.team_count;i++)
  {
    data[i].v[1] = data[i].w+data[i].l+data[i].t;
  }

  data[32].v[1]=1.0;
  data[32].t = 1;

  fprintf(fp_trace,"\nDONE wlt\n");

  /***********/
  /* Scores. */
  /***********/
  for(i=0;i<info.team_count;i++)
  {
    info.scores[i][info.week-1]=data[i].v[42];
    if((i==(info.monday1-1)) || (i==(info.monday2-1)))
	//||	(i==(24-1)) || (i==(12-1)))//STRANGE MONDAY- JRXX
      info.scores[i][info.week-1]=data[info.team_count-1].v[42];
  }

  fprintf(fp_trace,"\nDONE scores\n");
  /**********************************************************/
  /* Any Monday nite teams will use last weeks data, as if  */
  /* it were a bye week.                                    */
  /**********************************************************/

  /**********************/
  /* Open the out file. */
  /**********************/

  sprintf(temp_filename,"%s/%d/out", statsdir, year);
  if (NULL == (fp_out = fopen(temp_filename, "w")))
    {
      fprintf(fp_trace,"\nCould not open the file \"out\"");
      fclose(fp_out);
      exit(0);
    }


  /****************************/
  /* Write out the variables. */
  /****************************/


  for(j=0;j<info.team_count-1;j++)
    {
      i = j;
      /*******************************************************************/
      /* There may be missing teams on Monday, fake it by using NFL AVG. */
      /*******************************************************************/
		if(i == (info.monday1-1))
		{
		  //i=info.team_count-1;
		  strcpy(info.tnam[i],info.tnam[j]);
		  data[i].t+=1;
		  for(k=2;k<NUM_VARS;k++)
			 data[i].v[k]=data[info.team_count-1].v[k];
		  data[i].v[1]+=1;
		}
		if(i == (info.monday2-1)) 
		{
		 // i=info.team_count-1;
		  strcpy(info.tnam[i],info.tnam[j]);
		  data[i].t+=1;
		  for(k=2;k<NUM_VARS;k++)
			 data[i].v[k]=data[info.team_count-1].v[k];
		  data[i].v[1]+=1;
		}
		//JRXX - STRANGE MONDAY
		/*
		if(i == (24-1)) 
		{
		 // i=info.team_count-1;
		  strcpy(info.tnam[i],info.tnam[j]);
		  data[i].t+=1;
		  for(k=2;k<75;k++)
			 data[i].v[k]=data[info.team_count-1].v[k];
		  data[i].v[1]+=1;
		}
		if(i == (12-1)) 
		{
		 // i=info.team_count-1;
		  strcpy(info.tnam[i],info.tnam[j]);
		  data[i].t+=1;
		  for(k=2;k<75;k++)
			 data[i].v[k]=data[info.team_count-1].v[k];
		  data[i].v[1]+=1;
		}
		*/
		
		
	/*
    if((i == (info.monday1-1)) && (info.week == 1))
	{
	  i=info.team_count-1;
	  strcpy(info.tnam[i],info.tnam[j]);
	}
    if((i == (info.monday2-1)) && (info.week == 1))
	{
	  i=info.team_count-1;
	  strcpy(info.tnam[i],info.tnam[j]);
	}
	*/
     //fprintf(fp_trace,"\nHERE 6\n");
     /******************************************/
     /* Average the monday missing teams data. */
     /******************************************/
	 /*
    if(((j == (info.monday1-1))||(j == (info.monday2-1))) &&
		(info.week != 1))
	{
	fprintf(stderr,"DATA %lf",data[i].v[2]);
	fprintf(stderr,"DATA %lf",data[i].v[3]);
	fprintf(stderr,"DATA %lf",data[i].v[4]);
	fprintf(stderr,"DATA %lf",data[i].v[5]);
	fprintf(stderr,"DATA %lf",data[i].v[6]);
	fprintf(stderr,"DATA %lf",data[i].v[7]);
	
          data[i].t+=1;
          for(k=2;k<75;k++)
			 data[i].v[k]=data[i].v[k]+data[i].v[k]/data[i].v[1];
			 data[i].v[1]+=1;
    }
	*/
/*
	if(info.sched[i][0]==0)
	{
		if(info.week==1)data[j].t+=1;
		for(k=0;k<75;k++)
			 data[i].v[k]=data[info.team_count-1].v[k];
		memcpy(data[i].tmm,data[info.team_count-1].tmm,6);
					 
	}
*/
	if((info.sched[i][0]==0)&& (info.week==1))//If week 1 and team not playing..
	{
		data[i].w=data[i].l=data[i].t=0;
		for(k=0;k<NUM_VARS;k++)
			 data[i].v[k]=0;
		strcpy(data[i].tmm,"\0");
		data[i].v[1]=1;

	fprintf(stderr,"\nDATA %lf",data[i].v[1]);
	fprintf(stderr,"\nDATA %lf",data[i].v[2]);
	fprintf(stderr,"\nDATA %lf",data[i].v[3]);
	fprintf(stderr,"\nDATA %lf",data[i].v[4]);
	fprintf(stderr,"\nDATA %lf",data[i].v[5]);
	fprintf(stderr,"\nDATA %lf",data[i].v[6]);
	fprintf(stderr,"\nDATA %lf",data[i].v[7]);
	fprintf(stderr,"\nDATA %lf",data[i].v[8]);
	fprintf(stderr,"\nDATA %s",info.tnam[i]);
	fprintf(stderr,"\nDATA %d",info.year-2000);
	fprintf(stderr,"\n%2d%2d%2d",data[i].w,data[i].l,data[i].t);

	fprintf(stderr,"\nDATA %lf",data[i].v[6]/data[i].v[1]);
	fprintf(stderr,"\nDATA %lf",data[i].v[7]/data[i].v[1]);
	fprintf(stderr,"\nDATA %lf",data[i].v[8]/data[i].v[1]);

	fprintf(stderr,"\nDATA %02d",info.year-2000);

	/*
	fprintf(fp_out,"%2d%2d%2d%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%3s %02d 1\n",
		data[i].w,data[i].l,data[i].t,
		0,
		data[i].v[2],data[i].v[3],data[i].v[4],data[i].v[5],
		data[i].v[6],
		data[i].v[6]/data[i].v[1],
		data[i].v[7],
		data[i].v[7]/data[i].v[1],
		data[i].v[8],
		data[i].v[8]/data[i].v[1],
		info.tnam[i],
		info.year-2000);
		*/

	fprintf(fp_out,"%2d%2d%2d%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f",
		data[i].w,data[i].l,data[i].t,
		0.0,
		data[i].v[2],data[i].v[3],data[i].v[4],data[i].v[5],
		data[i].v[6],
		data[i].v[6]/data[i].v[1],
		data[i].v[7],
		data[i].v[7]/data[i].v[1],
		data[i].v[8],
		data[i].v[8]/data[i].v[1]);

	fprintf(stderr,"\nDATA %s",info.tnam[i]);

	fprintf(fp_out,"%3s",info.tnam[i]); 
	fprintf(fp_out," %02d 1\n",info.year-2000);

	}
	else
	{
		fprintf(fp_out,"%2d%2d%2d%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%3s %02d 1\r\n",
		data[i].w,data[i].l,data[i].t,
		data[i].v[1],
		data[i].v[2],data[i].v[3],data[i].v[4],data[i].v[5],
		data[i].v[6],
		data[i].v[6]/data[i].v[1],
		data[i].v[7],
		data[i].v[7]/data[i].v[1],
		data[i].v[8],
		data[i].v[8]/data[i].v[1],
		info.tnam[i],
		info.year-2000);
	}
      fprintf(fp_out,"%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%3s %02d 2\r\n",
	data[i].v[9],
	data[i].v[10],
	data[i].v[11],
	data[i].v[12],
	data[i].v[13],
	data[i].v[14],
	data[i].v[15],
	data[i].v[16],
	data[i].v[17],
	data[i].v[18],
	data[i].v[19],
	data[i].v[20],
	info.tnam[i],
	info.year-2000);

      fprintf(fp_out,"%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%3s %02d 3\r\n",
	data[i].v[21],
	data[i].v[22],
	data[i].v[23],
	data[i].v[24],
	data[i].v[25],
	data[i].v[26],
	data[i].v[27],
	data[i].v[28],
	data[i].v[29],
	data[i].v[30],
	data[i].v[31],
	data[i].v[32],
	info.tnam[i],
	info.year-2000);

      fprintf(fp_out,"%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%3s %02d 4\r\n",
	data[i].v[33],
	data[i].v[34],
	data[i].v[35],
	data[i].v[36],
	data[i].v[37],
	data[i].v[38],
	data[i].v[39],
	data[i].v[40],
	data[i].v[41],
	data[i].v[42],
	data[i].v[43],
	data[i].v[44],
	info.tnam[i],
	info.year-2000);

      fprintf(fp_out,"%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%3s %02d 5\r\n",
	data[i].v[45],
	data[i].v[46],
	data[i].v[47],
	data[i].v[48],
	data[i].v[48]/data[i].v[1],
	data[i].v[49],
	data[i].v[49]/data[i].v[1],
	data[i].v[50],
	data[i].v[50]/data[i].v[1],
	data[i].v[51],
	data[i].v[52],
	data[i].v[53],
	info.tnam[i],
	info.year-2000);


      fprintf(fp_out,"%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%6.0f%3s %02d 6\r\n",
	data[i].v[54],
	data[i].v[55],
	data[i].v[56],
	data[i].v[57],
	data[i].v[59],
	data[i].v[60],
	data[i].v[61],
	data[i].v[62],
	data[i].v[63],
	data[i].v[64],
	data[i].v[65],
	data[i].v[66],
	info.tnam[i],
	info.year-2000);

      fprintf(fp_out,"%6.0f%6.0f%6.0f  %4s%51s %02d 7\r\n",
	data[i].v[67],
	data[i].v[68],
	data[i].v[69],
	data[i].tmm,
	info.tnam[i],
	info.year-2000);
	//fprintf(fp_trace,"\nHERE 7\n");
    }

  /**********/
  /* Close! */
  /**********/
  fclose(fp_out);

  WriteNewScoresFile(statsdir);
  WriteNewStatFile(statsdir);

  fprintf(fp_trace,"\nConvert Finished!\n\nThank You!");
  printf("\nConvert Finished. \n\nThank you!");

  /*********************/
  /* save the IN file. */
  /*********************/
  /*
  if(info.monday1!=-1)
    sprintf(string,"copy %s c:\\%d\\nfl_sun.%d",
	  input_filename,info.year,info.week);
  else
    sprintf(string,"copy %s c:\\%d\\nfl_mon.%d",
	    input_filename,info.year,info.week);
  system(string);
  */

  sleep(1);

  return(1);

}/* End of main(). */

/************************************************************************/
/************************************************************************/
/************************************************************************/

void read_in_convert_data()

{

  int tnum;
  int i,j,k;
  char *string;
  int count;
  FILE *fp;

  /******************************/
  /* Open the convert.dat file. */
  /******************************/

  if (NULL == (fp = fopen(info.convert_data_filename, "r")))
    {
      printf("\nIn GetTeamCount(). Could not open the file %s",
	     info.convert_data_filename);
      fclose(fp);
      exit(0);
    }

  /*************/
  /* Allocate. */
  /*************/

  string  = (char *)calloc(256,sizeof(char));

  /****************************/
  /* Read in the tlist array. */
  /****************************/

  /* search for the first team name line. */
  string[0]=0;
  while(!isdigit(string[0]))
    {
      /* Read in a line. */
      fgets(string, 256, fp);
    }

fprintf(stderr,"\n");

  /* Read in the lines. */

  while(isdigit(string[0]))
    {
      sscanf(string,"%d",&tnum);
      sscanf(string,"%d %s %s %s %s %s %s %s %s %s %s",
	     &tnum,
	     info.tlist[tnum-1][0],
	     info.tlist[tnum-1][1],
	     info.tlist[tnum-1][2],
	     info.tlist[tnum-1][3],
	     info.tlist[tnum-1][4],
	     info.tlist[tnum-1][5],
	     info.tlist[tnum-1][6],
	     info.tlist[tnum-1][7],
	     info.tlist[tnum-1][8],
	     info.tlist[tnum-1][9]);
      for(j=0;j<10;j++)
	for(i=0;i<(int)strlen(info.tlist[tnum-1][j]);i++)
	  if(info.tlist[tnum-1][j][i]=='*')info.tlist[tnum-1][j][i]=' ';

      /* Read in a line. */
      fgets(string, 256, fp);

    }

  /***************************/
  /* Read in the tnam array. */
  /***************************/

  /* search for the first team name line. */
  string[0]=0;
  while(!isdigit(string[0]))
    {
      /* Read in a line. */
      fgets(string, 256, fp);
    }

  /* Read in the lines. */

  while(isdigit(string[0]))
    {
      sscanf(string,"%d",&tnum);
      sscanf(string,"%d %s",
	     &tnum,
	     info.tnam[tnum-1]);
      for(i=0;i<(int)strlen(info.tnam[tnum-1]);i++)
	if(info.tnam[tnum-1][i]=='*')info.tnam[tnum-1][i]=' ';

      /* Read in a line. */
      fgets(string, 256, fp);

    }

  /*******************************/
  /* Read in the variable order. */
  /*******************************/

  /* search for the first team name line. */
  string[0]=0;
  while(!isdigit(string[0]))
    {
      /* Read in a line. */
      fgets(string, 256, fp);
    }

  count = 0;
  while(isdigit(string[0]))
    {
      sscanf(string,"%d",&info.vorder[count]);
      info.vorder[count]--;
      count++;

      /* Read in a line. */
      fgets(string, 256, fp);
    }


  /*********************/
  /* Close and return. */
  /*********************/

  fclose(fp);
  free(string);

  return;

}/* End of read_in_convert_data(). */


/*********************************/
/*********************************/
/*********************************/

int GetTeamCount()

{
  FILE *fp;
  int count;
  char *string;

  /******************************/
  /* Open the convert.dat file. */
  /******************************/

  if (NULL == (fp = fopen(info.convert_data_filename, "r")))
    {
      printf("\nIn GetTeamCount(). Could not open the file %s",
	     info.convert_data_filename);
      fclose(fp);
      exit(0);
    }

  /*************/
  /* Allocate. */
  /*************/

  string = (char *)calloc(256,sizeof(char));

  /*******************************************/
  /* Read in the tlist array, counting them. */
  /*******************************************/

  /* search for the first team name line. */
  string[0]=0;
  while(!isdigit(string[0]))
    {
      /* Read in a line. */
      fgets(string, 256, fp);
    }

  /* Read in the lines, counting as you go. */

  count = 0;
  while(isdigit(string[0]))
    {
      count++;
      /* Read in a line. */
      fgets(string, 256, fp);
    }


  free(string);
  fclose(fp);
  return(count);


}/* End of GetTeamCount(). */

/*********************************/
/*********************************/
/*********************************/

void GetSchedule()
{
  FILE *fp;
  int  i,j,ok;
  char *string;
  char *p;
  int first;
  char schedsdir[256];

  /*************/
  /* Allocate. */
  /*************/

  string = (char *)calloc(768,sizeof(char));

  /***************************/
  /* Open the schedule file. */
  /***************************/

  printf("\r\nOpening scheds");
  //Sleep(2000);

  //Get the scheds directory env variable only if not null
  if (NULL == (p = getenv("SCHEDSDIR")))
  {
    printf("\nIn GetSchedule(). Could not get the SCHEDSDIR environment variable. Exiting\n\n");
    free(string);
    exit(0);
  }
  else
  {
    strcpy(schedsdir, p);
  }
  
  sprintf(string,"scheds/sched_%d.txt",info.year);
  if (NULL == (fp = fopen(string, "r")))
    {
      printf("\nIn GetSchedule(). Could not open the file %s", string);
	    sleep(2);
      //fclose(fp);
      free(string);
      exit(0);
    }

 // Sleep(2000);
  /*******************************/
  /* Read in the schedule array. */
  /*******************************/

  /* Skip the first line. */
  fgets(string, 256, fp);

  fprintf(fp_trace,"\nSchedule Read in:");
  /* Read in the schedule lines. */
  for(i=0;i<info.team_count-1;i++)
    {
      /* Read in a line. */
      fgets(string, 256, fp);

      /* Skip past the team name and number. */
     // strtok(string+4," ,");
	  //strtok(string+7," ");

      /* Read in the schedule data. */
      j=0;
      ok=1;
      fprintf(fp_trace,"\n");
	  first=1;
      while(ok)
	{
		  if(first)p=strtok(string+7,",");
	  else p = strtok(NULL,",");
	  first=0;
	  if(p != NULL)
	    {
	      info.sched[i][j]=atol(p);
	      fprintf(fp_trace,"%d ",info.sched[i][j]);
	   }
	  else ok=0;

	  j++;
	}
    }

  fprintf(fp_trace,"\n\n");

  /****************************/
  /* Free, close, and return. */
  /****************************/
  free(string);
  fclose(fp);
  return;

}
/* End of GetSchedule(). */

/*********************************/
/*********************************/
/*********************************/

void GetScores(char *statsdir)
{
  FILE *fp;
  int  i,j;
  char *string;
  int  dummy;
  int k,l;

  /*************/
  /* Allocate. */
  /*************/

  string = (char *)calloc(768,sizeof(char));

  /*************************/
  /* Open the scores file. */
  /*************************/

  sprintf(temp_filename,"%s/%d/scores.txt",statsdir,year);
  if (NULL == (fp = fopen(temp_filename, "r")))
  {
    printf("\nIn GetScores(). Could not open the file %s", temp_filename);
    return;
  }

  /*****************************/
  /* Read in the scores array. */
  /*****************************/

  fprintf(fp_trace,"\nScores Read in:");
  /* Read in the schedule lines. */
  for(i=0;i<info.week-1;i++)
    {
      /* Read in a line. */
      fgets(string, 256, fp);

      /* Read in the scores data. */
      j=0;
      sscanf(string,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        &info.scores[j][i],&dummy,
        &info.scores[j+1][i],&dummy,
        &info.scores[j+2][i],&dummy,
        &info.scores[j+3][i],&dummy,
        &info.scores[j+4][i],&dummy,
        &info.scores[j+5][i],&dummy,
        &info.scores[j+6][i],&dummy,
        &info.scores[j+7][i],&dummy,
        &info.scores[j+8][i],&dummy,
        &info.scores[j+9][i],&dummy,
        &info.scores[j+10][i],&dummy,
        &info.scores[j+11][i],&dummy);

      /* Read in a line. */
      fgets(string, 256, fp);

      /* Read in the scores data. */
      j=12;
      sscanf(string,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
	 &info.scores[j][i],&dummy,
	 &info.scores[j+1][i],&dummy,
	 &info.scores[j+2][i],&dummy,
	 &info.scores[j+3][i],&dummy,
	 &info.scores[j+4][i],&dummy,
	 &info.scores[j+5][i],&dummy,
	 &info.scores[j+6][i],&dummy,
	 &info.scores[j+7][i],&dummy,
	 &info.scores[j+8][i],&dummy,
	 &info.scores[j+9][i],&dummy,
	 &info.scores[j+10][i],&dummy,
	 &info.scores[j+11][i],&dummy);

	 

      /* Read in a line. */
      fgets(string, 256, fp);

      /* Read in the scores data. */
      j=24;
      sscanf(string,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
	 &info.scores[j][i],&dummy,
	 &info.scores[j+1][i],&dummy,
	 &info.scores[j+2][i],&dummy,
	 &info.scores[j+3][i],&dummy,
	 &info.scores[j+4][i],&dummy,
	 &info.scores[j+5][i],&dummy,
	 &info.scores[j+6][i],&dummy,
	 &info.scores[j+7][i],&dummy,
	 &info.scores[j+8][i],&dummy,
	 &info.scores[j+9][i],&dummy,
	 &info.scores[j+10][i],&dummy,
	 &info.scores[j+11][i],&dummy);

  }
  fprintf(fp_trace,"\nDone Reading Scores\n\n");

  /****************************/
  /* Free, close, and return. */
  /****************************/
  free(string);
  fclose(fp);
  return;

}
/* End of GetScores(). */

/*********************************/
/*********************************/
/*********************************/

void WriteNewScoresFile(char *statsdir)
{
  FILE *fp;
  int  i,j,k,first;
  char *string;
  int  total[40];

  /*************/
  /* Allocate. */
  /*************/

  string = (char *)calloc(768,sizeof(char));

  /*************************/
  /* Open the scores file. */
  /*************************/

  /* Save the old one. */
  sprintf(string,"cp %s/%d/scores.txt %s/%d/scores%d.txt",
	  statsdir,info.year,statsdir,info.year,info.week-1);

  sprintf(string,"%s/%d/scores.txt",statsdir,info.year);

  if (NULL == (fp = fopen(string, "w")))
  {
      printf("\nIn WriteNewScoresFile(). Could not open the file %s", string);
      fclose(fp);
      free(string);
      exit(0);
  }


  /*******************************/
  /* Write out the scores array. */
  /*******************************/

	  for(j=0;j<info.team_count-1;j++) total[j]=0;

  fprintf(fp_trace,"\nScores Written to scores file:");
  /* Write the scores. */
  first = 0;
  for(i=0;i<info.week-1;i++)
    for(j=0;j<info.team_count-1;j++)
      {
	if((j%12==0)&&(first))fprintf(fp,"\r\n");
	first = 1;

	/* Total up all the scores. */
	total[j]=0;
	for(k=0;k<info.week-1;k++)
	  total[j] += info.scores[j][k];

	if((int)abs(info.sched[j][i])==0)
	  fprintf(fp,"  0  0");
	else
	  fprintf(fp,"%3d%3d",
		  info.scores[j][i],
		  info.scores[(int)abs(info.sched[j][i])-1][i]);
      }
  //fprintf(fp,"\n");
  /************************/
  /* Write out this week. */
  /************************/
  first = 1;
  if(info.week==1)first = 0;
  i = info.week-1;
  for(j=0;j<info.team_count-1;j++)
    {
      if((j%12==0)&&(first))fprintf(fp,"\r\n");
      first = 1;
      if((int)abs(info.sched[j][i])==0)
      {
		fprintf(fp,"  0  0");
      }
      else
	fprintf(fp,"%3d%3d",
	  info.scores[j][i]-total[j],
	  info.scores[(int)abs(info.sched[j][i])-1][i]-
	  total[(int)abs(info.sched[j][i])-1]);
   }

for(i=0;i<32;i++)
  fprintf(fp_trace,"\nscores[%d]=%d scores[%d]=%d",i+1,info.scores[i][0],
	  (int)abs(info.sched[i][0]),
	   info.scores[(int)abs(info.sched[i][0])-1][0]);
  /***************************/
  /* Free, close and return. */
  /***************************/
  free(string);
  fclose(fp);

  return;

}/* End of WriteNewScoresFile(). */

/*****************************/
/*****************************/
/*****************************/

void WriteNewStatFile(char *statsdir)

{
  long int i;
  char *string;
  FILE *fp_old,*fp_new;

  /*************/
  /* Allocate. */
  /*************/

  string = (char *)calloc(1024,sizeof(char));

  /*************************/
  /* Open the scores file. */
  /*************************/

  /* Save the old one. */
  sprintf(string,"cp %s/%d/stats.txt %s/%d/stats.old",
	  statsdir,info.year,statsdir, info.year);
  system(string);

  /* Copy OUT to stats file. */
  sprintf(string,"cp %s/%d/out %s/%d/%02dsummar.txt",
	  statsdir,info.year,statsdir,info.year,info.year-2000);
  system(string);

  /* Open the stats file. */
  sprintf(string,"%s/%d/stats.txt",statsdir,info.year);
  if (NULL == (fp_new = fopen(string, "w+t")))
    {
      printf("\nIn WriteNewStatFile(). Could not open the file %s",
	     string);
      fclose(fp_new);
      free(string);
      exit(0);
    }

  /****************************/
  /* Open the old stat file.  */
  /****************************/
  sprintf(string,"%s/%d/stats.old",statsdir,info.year);
  if(info.week != 1)
  {
	  if (NULL == (fp_old = fopen(string, "r")))
		{
		  printf("\nIn WriteNewStatFile(). Could not open the file %s",
			 string);
		  fclose(fp_old);
      fp_old = NULL;
		  free(string);
		  exit(0);
		}
  }
  else
  {
	  fp_old=NULL;
  }
  fprintf(fp_trace,"\nStats Written to stats file:");

  /*******************************/
  /* Read up to last weeks data. */
  /*******************************/

  if(fp_old != NULL) {
    for(i=0;i<224*(info.week-1);i++)
      {
        fgets(string,256, fp_old);
        printf("%s\n", string);
        fprintf(fp_new,"%s",string);
      }
    fclose(fp_old);
    fp_old = NULL;
  }

  /* Close 9495stat and the out file. */
  fclose(fp_new);

  /***************************/
  /* Append out to 9495stat. */
  /***************************/

  sprintf(string,"cat %s/%d/stats.txt %s/%d/out >> %s/%d/stats.txt",
	  statsdir, info.year, statsdir, info.year, statsdir, info.year);
  system(string);

  sprintf(string,"rm %s/%d/out", statsdir, info.year);
  system(string);

  /*******************/
  /* Close and free. */
  /*******************/
  if(fp_old)fclose(fp_old);
  free(string);

 return;

}/* End of WriteNewStatFile. */
