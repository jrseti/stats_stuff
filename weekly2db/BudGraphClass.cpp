// BudGraphClass.cpp: implementation of the CBudGraphClass class.
//
//////////////////////////////////////////////////////////////////////


#include "BudGraphClass.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*************************************************
 *
 * Constructor and Destructor.
 *
 *************************************************/

CBudGraphClass::CBudGraphClass(int year, int week)
{
	m_this_year = year;
	m_this_week = week;
	m_status_string[0]=0;

	memset(m_w_vars,0,NUM_TEAMS*NUM_WEEKS*201);

	/* Read in the data. */
	ReadInData();

}

CBudGraphClass::~CBudGraphClass()
{

}

/*************************************************
 *
 * ReadInData()
 *
 * Reads Data in.
 *
 *************************************************/

void CBudGraphClass::ReadInData()

{
	char status_string[1024];

	/************************************************/
	/* Read in the schedule for this and last year. */
	/************************************************/
	if(ReadInSchedule(m_this_year,m_sched_this)==0)
	{
		printf("Trouble reading sched for this year.\r\n");
		return;
	}

	ReadWeekData();
	
	/****************************/
	/* Make the status strings. */
	/****************************/
	sprintf(m_status_string,"Read Data\n");
	sprintf(status_string,"This year = %d\n",m_this_year);
	strcat(m_status_string,status_string);
	sprintf(status_string,"This week = %d\n",m_this_week);
	strcat(m_status_string,status_string);
	sprintf(status_string,"scheds %d %d %d %d %d %d\n",
		m_sched_this[1][0],
		m_sched_this[1][1],
		m_sched_this[1][2],
		m_sched_this[1][3],
		m_sched_this[1][4],
		m_sched_this[1][5]);
	strcat(m_status_string,status_string);

	printf("%s\r\n", m_status_string);

	if(m_this_week>NUM_WEEKS)
	{
		m_this_week=NUM_WEEKS;
	}



}/* End of ReadInData(). */

/*************************************************
 *
 * ReadInSchedule()
 *
 * Reads in a schedule;
 *
 *************************************************/

int CBudGraphClass::ReadInSchedule(int year,int data[NUM_TEAMS][NUM_WEEKS])

{
	FILE *fp;
	char filename[512];
	char string[256];
	int  i,j;
	char *dummy;
	char schedsdir[256];

	//Get the SCHEDSDIR env variable. If it doesn't exist, exit.
	if(getenv("SCHEDSDIR")==NULL)
	{
		printf("SCHEDSDIR environment variable not set.\r\n");
		return(0);
	}
	strcpy(schedsdir,getenv("SCHEDSDIR"));

	sprintf(filename,"%s/sched_%d.txt", schedsdir, year);

	fp = fopen(filename,"r");
	
	if(!fp)
	{
		printf("Cannot open scheds file.\r\n");
		return(0);
	}

	/* Read past the first line, scrap it. */
	fgets(string,256,fp);

	/* Read and parse. */
	for(i=0;i<NUM_TEAMS;i++)
	{
		fgets(string,256,fp);
		strncpy(m_sched_names[i],string,3);
		m_sched_names[i][3]=0;
	//	dummy=strtok(string+7," ,");	/* Skip this one. */
		j=0;
		dummy=strtok(string+7,","); /* Read in the first. */
		while(dummy!=NULL)
		{
			data[i][j]=atol(dummy);
			dummy=strtok(NULL," ,");
			j++;
		}
	}

	/* Close. */
	fclose(fp);

	return(1);


}/* End of ReadInSchedule(). */

/*************************************************
 *
 * GetWeekData()
 *
 * Puts into an array week data for a team.
 *
 *************************************************/

void CBudGraphClass::GetWeekData(int team_num,int var_num, float *data)

{
	int i;

	for(i=0;i<NUM_WEEKS;i++)
	{
		data[i]=m_w_vars[team_num-1][i][var_num-1];
	}
	
	

}/* End of GetWeekData(). */


/*************************************************
 *
 * VARW2()
 *
 * THIS PLOT COMPUTES WEEKLY STATS(V) FOR EXTENDED VAR. 
 * Converted from FORTRAN code by Pete Mundle.
 *
 *************************************************/
 void CBudGraphClass::VARW2(float VO[75],float VN[75],
							float V[201],int NVIN,int NV) 

 {


	 int	J;
	 int    A,B,C,D,G,MOLD,MNEW;
	 
	/****************************/
	/*                          */ 
	/*    A = GAMES WON         */ 
	/*    B = GAMES LOST        */ 
	/*    C = GAMES TIED        */ 
	/*    D = GAMES PLAYED      */ 
	/*                          */ 
	/****************************/ 
	
	A=(int)((VN[1-1]+0.01)/10000.0);                                                
    B=(int)((VN[1-1]-A*10000.0+0.01)/100.0);
    C=(int)(VN[1-1]-A*10000.0-B*100.0);                                           
    G=A+B+C;                                                            

	//POST GAME game 1 comment out, comment IN for regualr season
	
	/*
	for(J=0;J<=NVIN;J++)
	{
		V[J] = VN[J] - VO[J]; 
	}
	*/
	

	
	
	for(J=0;J<=NVIN;J++)
	{
		V[J] = VN[J]; 
	}
	
	  

	if(G==0)
	{
		printf("\nWAS 0!!!!!!!!!!\n");
		return;
	}
 
    A=(int)((V[1-1]+0.01)/10000.0);
    B=(int)((V[1-1]-A*10000.0+0.01)/100.0);
    C=(int)(V[1-1]-A*10000.-B*100.0);
    D=A+B+C;

	V[1-1]=(float)A;
    V[2-1]=(float)(((float)A+.5*(float)C)/(float)D);
    V[11-1] = V[8-1] / V[10-1];
    V[14-1] = (float)100.0 * V[13-1] / V[12-1];
	  
    V[15-1] = (float)100.0 * V[13-1] / (V[12-1]+V[17-1]);
    if(V[22-1]>0.0)  V[23-1] = (float)(
     ((int)(VN[22-1]*(VN[23-1]+.05)) - (int)(VO[22-1]*(VO[23-1]+.05))))/ V[22-1];
    if(V[24-1]>0.0)  V[25-1] = (float)(
     ((int)(VN[24-1]*(VN[25-1]+.05)) - (int)(VO[24-1]*(VO[25-1]+.05))))/ V[24-1];
    if(V[27-1]>0.0)  V[28-1] = (float)(
     ((int)(VN[27-1]*(VN[28-1]+.05)) - (int)(VO[27-1]*(VO[28-1]+.05))))/ V[27-1];
    V[53-1] = (float)0.0;
	  
    if(V[52-1]>0.0) V[53-1] = V[50-1] / V[52-1];
    V[54-1] = V[55-1];
    V[55-1] = V[56-1];
    V[56-1] = (float)100.0 * V[55-1] / V[54-1];
    V[57-1] = (float)100.0 * V[55-1] / (V[54-1]+V[58-1]);
    MNEW = (int)VN[71-1];
    MOLD = (int)VO[71-1];
    V[174-1] = (VO[71-1] - (float)(MOLD)) / (float).6 + (float)(MOLD);
    V[174-1]= G*((VN[71-1] - (float)(MNEW))/(float).6+(float)(MNEW)) - (G-(float)1.0)*V[174-1];
    if( V[63-1]>0.0)  V[64-1] = (float)(
     ((int)(VN[63-1]*(VN[64-1]+.05)) - (int)(VO[63-1]*(VO[64-1]+.05))))/ V[63-1];
    if( V[65-1]>0.0)  V[66-1] = (float)(
     ((int)(VN[65-1]*(VN[66-1]+.05)) - (int)(VO[65-1]*(VO[66-1]+.05))))/ V[65-1];
	 
    V[71-1] = (float)100.0 * V[19-1] / (V[12-1]+V[17-1]);
    V[72-1] = (float)100.0 * V[60-1] / (V[54-1]+V[58-1]);
    V[73-1] = (float)100.0 * V[38-1] / (V[12-1]+V[17-1]);
    V[74-1] = (float)100.0 * V[69-1] / (V[54-1]+V[58-1]);
    V[75-1] = V[10-1] + V[12-1] + V[17-1];
    V[76-1] = V[52-1] + V[54-1] + V[58-1];
    V[77-1] = V[7-1] / V[75-1];
    V[78-1] = V[49-1] / V[76-1];
    V[79-1] = V[23-1] - V[64-1];
    V[80-1] = (float)0.0;
    if( V[32-1] > 0.0 ) V[80-1] = (float)100.0 * V[33-1] / V[32-1];
    V[81-1] = 0.0;
    if( V[34-1] > 0.0 ) V[81-1] = (float)100.0 * V[35-1] / V[34-1];
    V[82-1] = V[9-1] / (V[12-1]+V[17-1]);
	if(V[9-1]==0)V[82-1] =0;
    V[83-1] = V[51-1] / (V[54-1]+V[58-1]);
    V[84-1] = V[43-1] / V[12-1];
    V[85-1] = V[44-1] / V[54-1];
    V[86-1] = 0.0;
    if( V[22-1] > 0.0 ) V[86-1] = V[3-1] / V[22-1];
    V[87-1] = (float)0.0;
    if( V[44-1] > 0.0 ) V[87-1] = V[43-1] / V[44-1];
    V[88-1] = (float)0.0;
    if( V[22-1] > 0.0 ) V[88-1] = V[63-1] * V[64-1] / V[22-1];
    V[89-1] = (V[44-1] - (float)7.0*V[67-1]) / (float)3.0;
    V[90-1] = V[60-1] - V[19-1];
    V[91-1] = V[10-1] / V[12-1];
    V[92-1] = V[52-1] / V[54-1];
    V[93-1] = V[18-1] + V[20-1];
    V[94-1] = V[10-1] - V[52-1];
    V[95-1] = V[82-1] - V[83-1];
    V[96-1] = V[58-1] - V[17-1];
    V[97-1] = V[38-1] - V[69-1];
    V[98-1] = V[8-1]  - V[50-1];
    V[99-1] = V[35-1] - V[33-1];
    V[100-1] = V[34-1] - V[32-1];
    V[101-1] = V[12-1] - V[54-1];
    V[102-1] = V[91-1] - V[92-1];
    V[103-1] = V[73-1] - V[74-1];
    V[104-1] = V[59-1] - V[18-1];
    V[105-1] = V[3-1] - V[45-1];
    V[106-1] = V[7-1] - V[49-1];
    V[107-1] = V[77-1] - V[78-1];
    V[108-1] = V[37-1] - V[68-1];
    V[109-1] = V[61-1] - V[20-1];
    V[110-1] = V[4-1] - V[46-1];
    V[111-1] = V[9-1] - V[51-1];
    V[112-1] = V[25-1] - V[64-1];
    V[113-1] = V[28-1] - V[66-1];
    V[114-1] = V[39-1] - V[70-1];
    V[115-1] = V[11-1] - V[53-1];
    V[116-1] = V[13-1] - V[55-1];
    V[117-1] = (float)100.0 * V[93-1] / V[9-1];
	//TRACE("\nV117=%f, %f %f",V[117-1],V[93-1],V[9-1]);
	if(V[9-1]==0)V[117-1] =0;
    V[118-1] = V[75-1] / V[3-1];
    V[119-1] = V[76-1] / V[45-1];
    V[120-1] = V[43-1] - (float)7.0 * V[39-1];
    V[121-1] = (float)0.0;
	 
      if(V[19-1] > 0.0 ) V[121-1] = V[38-1] / V[19-1];
      V[122-1] = (float)0.0;
      if(V[60-1] > 0.0 ) V[122-1] = V[69-1] / V[60-1];
      V[123-1] = V[43-1] - V[44-1];
      V[124-1] = V[35-1] + V[60-1] - V[33-1] - V[19-1];
      V[125-1] = (float)0.0;
      if( V[32-1]+V[34-1] > 0.0)
        V[125-1] = (float)100.0 * (V[32-1]-V[33-1]+V[35-1]) / (V[32-1]+V[34-1]);
      V[126-1] = (float)100.0 * V[5-1] / V[12-1];
      V[127-1] = (float)100.0 * V[47-1] / V[54-1];
      V[128-1] = (float)100.0 * V[17-1] / V[12-1];
      V[129-1] = (float)0.0;
      V[130-1] = (float)0.0;
      if(V[13-1] > 0.0) V[129-1] = V[16-1] / V[13-1];
      if(V[13-1] > 0.0) V[130-1] = V[9-1] / V[13-1];
      V[131-1] = V[71-1] - V[72-1];
      V[132-1] = V[75-1] - V[76-1];
      V[133-1] = ((V[61-1]+V[59-1]) / V[51-1] ) *(float)100.0;
      V[134-1] = V[84-1] - V[85-1];
      V[135-1] = V[21-1] - V[62-1];
      V[136-1] = V[24-1] - V[63-1];
      V[137-1] = V[27-1] - V[65-1];
      V[138-1] = V[36-1] - V[67-1];
      V[139-1] = V[5-1] - V[47-1];
      V[140-1] = V[126-1] - V[127-1];
      V[141-1] = V[118-1] - V[119-1];
      V[142-1] = V[121-1] - V[122-1];
      V[143-1] = (float)0.0;
	  
      if( V[37-1]+V[38-1] > 0.0 ) V[143-1] = V[7-1] / (V[37-1]+V[38-1]);
      V[144-1] = 0.0;
      if( V[68-1]+V[69-1] > 0.0 ) V[144-1] = V[49-1] / (V[68-1]+V[69-1]);
      V[145-1] = V[14-1] - V[56-1];
      V[146-1] = V[15-1] - V[57-1];
      V[147-1] = (float)100 * V[8-1] / V[7-1];
      V[148-1] = (float)100 * V[50-1] / V[49-1];
      V[149-1] = (float)0.0;
      if( V[37-1]+V[38-1] > 0.0 ) V[149-1] = V[75-1] / (V[37-1]+V[38-1]);
      V[150-1] = 0.0;
      if( V[68-1]+V[69-1] > 0.0 ) V[150-1] = V[76-1] / (V[68-1]+V[69-1]);
      V[151-1] = (float)100 * V[58-1] / V[54-1];
      V[152-1] = (float)0.0;
      if( V[17-1] > 0.0 ) V[152-1] = V[18-1] / V[17-1];
      V[153-1] = 0.0;
      if( V[58-1] > 0.0 ) V[153-1] = V[59-1] / V[58-1];
      V[154-1] = V[6-1] - V[48-1];
      V[155-1] = V[19-1] + V[33-1];
      V[156-1] = V[60-1] + V[35-1];
      V[157-1] = (float)100.0 * ((float)6.0*(V[37-1] + V[38-1]) + 
		(float)3.0*V[41-1] + V[40-1]) / V[7-1];
      V[158-1] = (float)100.0 * (V[44-1] - (float)7.0*V[70-1]) / V[49-1];
      V[159-1] = (float)0;
      V[160-1] = (float)0;
	  
      if(V[55-1]!=0.0) V[159-1] = (V[51-1] + V[59-1]) / V[55-1];
      if(V[55-1]!=0.0) V[160-1] = V[51-1] / V[55-1];
      V[161-1] = V[129-1] - V[159-1];
      V[162-1] = V[130-1] - V[160-1];
      V[163-1] = V[128-1] - V[151-1];
      V[164-1] = V[117-1] - V[133-1];
	  printf("\nV[163]=%f, %f, %f",V[164-1],V[117-1],V[133-1]);
      V[165-1] = V[59-1] + V[61-1];
      V[166-1] = V[93-1] - V[165-1];
      V[167-1] = V[39-1] * (float)7.0;
      V[168-1] = V[143-1] - V[144-1];
      V[169-1] = V[149-1] - V[150-1];
      V[170-1] = V[152-1] - V[153-1];
      V[171-1] = (float)100.0 * V[33-1] / V[75-1];
      V[172-1] = (float)100.0 * V[35-1] / V[76-1];
      V[173-1] = V[147-1] - V[148-1];
      V[175-1] = V[10-1] + V[13-1];
      V[176-1] = V[52-1] + V[55-1];
      V[177-1] = V[175-1] - V[176-1];
      V[178-1] = (float)60.0 - V[174-1];
      V[179-1] = V[174-1] - V[178-1];
      V[180-1] = V[12-1] - V[13-1];
      V[181-1] = V[54-1] - V[55-1];
      V[182-1] = V[181-1] - V[180-1];
	  
      if(V[75-1]!=0.0) V[183-1] = (float)100.0 * V[17-1] / V[75-1];
      if(V[76-1]!=0.0) V[184-1] = (float)100.0 * V[58-1] / V[76-1];
      V[185-1] = V[184-1] - V[183-1];
      if(V[75-1]!=0.0) V[186-1] = (float)100.0 * V[19-1] / V[75-1];
      if(V[76-1]!=0.0) V[187-1] = (float)100.0 * V[60-1] / V[76-1];
      V[188-1] = V[187-1] - V[186-1];
      if(V[43-1]!=0.0) V[189-1] = V[7-1] / V[43-1];
      if(V[44-1]!=0.0) V[190-1] = V[49-1] / V[44-1];
      V[191-1] = V[190-1] - V[189-1];
      V[192-1] = V[61-1]*V[60-1] + V[25-1]*V[24-1] + V[28-1]*V[27-1];
      V[193-1] = V[20-1]*V[19-1] + V[4-1]*V[63-1] + V[66-1]*V[65-1];
      V[194-1] = V[192-1] - V[191-1];
      if(V[43-1]!=0.0) V[195-1] = V[10-1] / V[43-1];
      if(V[44-1]!=0.0) V[196-1] = V[52-1] / V[44-1];
      V[197-1] = V[195-1] - V[196-1];
      if(V[12-1]!=0.0) V[198-1] = V[16-1] / V[12-1];
      if((V[54-1]+V[58-1])!=0.0) V[199-1] = (V[51-1]+V[59-1]) / (V[54-1]+V[58-1]);
      V[200-1] = V[198-1] - V[199-1];
	  

      return;                                                             
                                                                     

}

/*************************************************
 *
 * ReadWeekData()
 *
 * Reads in all the week data for a team.
 *
 *************************************************/
 int	CBudGraphClass::ReadWeekData()
{

	FILE	*fp;
	char	filename[1024];
	int		i,j,k;
	float	VO[75][NUM_TEAMS],VN[75],V[201];
	float	VOtemp[75],VOld[75][NUM_TEAMS];
	char    statsdir[256];
	
	/******************************/
	/* Read in this year to date. */
	/******************************/
	/* Init VO. */
	for(i=0;i<75;i++)
		for(j=0;j<NUM_TEAMS;j++)
			VO[i][j]=0.0;

	//Get the STATSDIR env variable. If it doesn't exist, exit.
	if(getenv("STATSDIR")==NULL)
	{
		printf("STATSDIR environment variable not set.\r\n");
		return(0);
	}
	strcpy(statsdir,getenv("STATSDIR"));
	sprintf(filename,"%s/%d/stats.txt", statsdir, m_this_year);
	fp = fopen(filename,"r");
	if(!fp)
	{
		printf("\nCannot open stats file\r\n");
		return(0);
	}

	//for(i=(NUM_WEEKS-m_this_week);i<NUM_WEEKS;i++)
	for(i=0;i<m_this_week;i++)
	{
		for(j=0;j<(NUM_TEAMS);j++)
		{
			ReadOneStatRecord(fp,VN);

			printf("\nT=%d, VN[6][7] = %f,%f", j+1, VN[6], VN[7]);

			if(j>=0) //Post season week=18, so j starts as -1
			{
				if(VN[0]<0.0001)
				{
					for(k=0;k<200;k++)
						m_w_vars[j][i][k]=0.0;
				}
				else if(VO[1][j]==VN[1])
				{
					for(k=0;k<200;k++)
						m_w_vars[j][i][k]=m_w_vars[j][i-1][k];
				}
				 
				else
				{
					for(k=0;k<75;k++)
						VOtemp[k]=VO[k][j];
					VARW2(VOtemp,VN,V,75,75);
					for(k=0;k<200;k++)
						m_w_vars[j][i][k]=V[k];
					
				}
				for(k=0;k<75;k++)
					VO[k][j]=VN[k];
			}
		}
	}

	fclose(fp);


	return(1);

}/* End of ReadWeekData(). */


/*************************************************
 *
 * ReadOneStatRecord()
 *
 * Reads in one record from the stat file.
 *
 *************************************************/
int	CBudGraphClass::ReadOneStatRecord(FILE *fp,float *VN)

{
	int		w,l,t;
	float	dummy;
	char	string[256];
	char	string2[32];

	VN[52]=0.0;
	fgets(string,256,fp);
	strncpy(string2,string,16);
	string2[2]=0;
	w = atol(string2);
	strncpy(string2,string+2,16);
	string2[2]=0;
	l = atol(string2);
	strncpy(string2,string+4,16);
	string2[2]=0;
	t = atol(string2);
	sscanf(string+6,"%f %f %f %f %f %f %f %f %f %f ",
		&VN[1],&VN[2],&VN[3],&VN[4],&VN[5],&VN[6],&dummy,&VN[7],&dummy,&VN[8]);
	fgets(string,256,fp);
	sscanf(string,"%f %f %f %f %f %f %f %f %f %f %f %f",
		&VN[9],&VN[10],&VN[11],&VN[12],&VN[13],&VN[14],&VN[15],&VN[16],&VN[17],&VN[18],&VN[19],&VN[20]);
	fgets(string,256,fp);
	sscanf(string,"%f %f %f %f %f %f %f %f %f %f %f %f",
		&VN[21],&VN[22],&VN[23],&VN[24],&VN[25],&VN[26],&VN[27],&VN[28],&VN[29],&VN[30],&VN[31],&VN[32]);
	//	&VN[21],&VN[22],&VN[23],&VN[24],&VN[25],&VN[26],&VN[27],&VN[28],&VN[28],&VN[30],&VN[31],&VN[32]);
	fgets(string,256,fp);
	sscanf(string,"%f %f %f %f %f %f %f %f %f %f %f %f",
		&VN[33],&VN[34],&VN[35],&VN[36],&VN[37],&VN[38],&VN[39],&VN[40],&VN[41],&VN[42],&VN[43],&VN[44]);
	fgets(string,256,fp);
	sscanf(string,"%f %f %f %f %f %f %f %f %f %f %f %f",
		&VN[45],&VN[46],&VN[47],&VN[48],&dummy,&VN[49],&dummy,&VN[50],&dummy,&VN[51],&VN[53],&VN[54]);
	fgets(string,256,fp);
	sscanf(string,"%f %f %f %f %f %f %f %f %f %f %f %f",
		&VN[55],&VN[56],&VN[57],&VN[58],&VN[59],&VN[60],&VN[61],&VN[62],&VN[63],&VN[64],&VN[65],&VN[66]);
	fgets(string,256,fp);
	sscanf(string,"%f %f %f %f",
		&VN[67],&VN[68],&VN[69],&VN[70]);
	VN[0]=(float)w*(float)10000.0+(float)l*(float)100.0+(float)t;

	/*
	int i;
	TRACE("\n***************\n");
	for(i=0;i<70;i++)
	{
		TRACE("%f ",VN[i]);
	}
	TRACE("\r\n");
	*/

	return(1);

}/* End of ReadOneStatRecord(). */

void CBudGraphClass::TraceOut()
{
	FILE *fp;
	int i,j,k;

	fp = fopen("c:\\bud.txt","w");

	for(i=0;i<NUM_TEAMS;i++)
	{
		fprintf(fp,"\r\nTeam %d",i+1);
		//TRACE("\r\nTeam %d",i);
		
		for(j=0;j<NUM_WEEKS;j++)
		{
			fprintf(fp,"\n%d ",j+1);
			for(k=0;k<200;k++)
				fprintf(fp,"%f\t",m_w_vars[i][j][k]);
		}
		
	}

	fclose(fp);
}