// BudGraphClass.h: interface for the CBudGraphClass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUDGRAPHCLASS_H__7FA8DE20_9E3E_11D2_ACFF_6C2B08C10000__INCLUDED_)
#define AFX_BUDGRAPHCLASS_H__7FA8DE20_9E3E_11D2_ACFF_6C2B08C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <stdio.h>

#define NUM_WEEKS 18
#define NUM_YEARS 20
#define NUM_TEAMS 32
//#define NUM_TEAMS 31

class CBudGraphClass  
{
public:
	CBudGraphClass(int year, int week);
	virtual ~CBudGraphClass();
	void	ReadInData();
	int		ReadWeekData();
	void    GetWeekData(int team_num,int var_num, float *data);
	int		ReadInSchedule(int year,int data[NUM_TEAMS][NUM_WEEKS]);
	void	VARW2(float VOld[75],float VNew[75],
				  float Var[201],int NVIN,int NV);
	int		ReadOneStatRecord(FILE *fp,float *VN);
	void    TraceOut();

	float	m_w_vars[NUM_TEAMS][NUM_WEEKS][201];   /* 31 teams, 17 weeks, 200 vars. */
	float	m_y_vars[NUM_TEAMS][NUM_YEARS][201];   /* 31 teams, 20 years, 200 vars. */
	int		m_sched_this[NUM_TEAMS][NUM_WEEKS];	   /* 31 teams, 17 weeks.           */
	int		m_sched_last[NUM_TEAMS][NUM_WEEKS];	   /* 31 teams, 17 weeks.           */
	char	m_sched_names[NUM_TEAMS][4];

	char	m_status_string[1024];

	int		m_this_year;
	int		m_this_week;
	int		m_num_weeks_last_year;



};

#endif // !defined(AFX_BUDGRAPHCLASS_H__7FA8DE20_9E3E_11D2_ACFF_6C2B08C10000__INCLUDED_)
