#include "stdafx.h" // <- MS

//////////////////////////////////////////////////////////////
//localtime() is marked unsafe by the Microsft Visual Studio
//This code use localtime c-function to get date. 
//Remove MS enoing warning with following pragma
//#pragma warning(disable : 4996) 
//////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <set>
#include <ctime>
#include <algorithm>

using namespace std;


struct Time   // (Helper class, 150 Lines, not part of solution)
{
	enum day {sunday, monday, tuesday, wednesday, thursday, friday, saturday, days};

    Time() {}
    
	Time(int dd, int mm, int yy, int h=0, int m=0, int s=0) : dd(dd), mm(mm), yy(yy), h(h), m(m), s(s) {}
	
	operator time_t () const { return getTime(); }

	Time& operator=(const Time& copy)		{ return  Time::copy(copy);		}
	bool operator==(const Time& test) const { return  Time::equal(test);	}
	bool operator< (const Time& test) const { return  Time::less	(test); }

	void clear() { copy(Time()); }

	int getDay		() const	{ return dd;				}
	int getMonth	() const	{ return mm;				}
	int getYear		() const	{ return yy;				}
	int getHour		() const	{ return h ;				}
	int getMinute	() const	{ return m ;				}
	int getSecond	() const	{ return s ;				}

	void addDate(int dd, int mm, int yy)
	{ 
		this->dd += dd;
		this->mm += mm;
		this->yy += yy;	
	
		update();	
	}

	void addClock(int h, int m, int s)
	{ 
		this->h += h;
		this->m += m;
		this->s += s;	
	
		update();	
	}

	void addSeconds(long seconds)
	{
		int m = seconds / 60;
		int s = seconds % 60;
		int h = m / 60;
		
		m = m % 60;

		addClock(h, m , s);
	}

	void addSeconds(size_t seconds, bool positive)
	{
		int m = (int)seconds / 60;
		int s = (int)seconds % 60;
		int h = m / 60;
		
		m = m % 60;

		positive ? addClock(h, m , s) : addClock(-h, -m , -s);
	}

	time_t getTime() const
	{
		struct tm date;

		date.tm_mday	= dd;
		date.tm_mon		= mm-1;		//months since January range : 0-11
		date.tm_year	= yy-1900;	//years since 1900
		date.tm_hour	= h;
		date.tm_min		= m;
		date.tm_sec		= s;

		return mktime(&date);
	}

	void update()
	{
		time_t t = getTime();

		if( t > -1 )
		{
			struct tm time = *localtime(&t); //C-function

			dd	= time.tm_mday; 
			mm	= time.tm_mon+1;
			yy	= time.tm_year+1900;
			h	= time.tm_hour;
			m	= time.tm_min;	
			s	= time.tm_sec;	
		}
		else
			clear();
	}

	int getDayOfWeek()
	{ 
		time_t t = getTime();

		if( t > -1 )
		{
			struct tm time = *localtime(&t);

			return time.tm_wday; 
		}
		else
			return saturday; //Happy day
	}

	void clearClock() { h = m = s = 0; }

	Time& copy(const Time& copy)
	{
		dd	= copy.dd;	
		mm	= copy.mm;	
		yy	= copy.yy;
		h	= copy.h ;	
		m	= copy.m ;	
		s	= copy.s ;

		return *this;
	}

	bool equal(const Time& test) const
	{
		if(  dd	!= test.dd ) return false;
		if(  mm	!= test.mm ) return false;
		if(  yy	!= test.yy ) return false;
		if(  h	!= test.h  ) return false;
		if(  m	!= test.m  ) return false;
		if(  s	!= test.s  ) return false; 

		return true;
	}

	bool less(const Time& test) const
	{
		return less(*this, test); 
	}

	bool less(const Time& a, const Time& b) const
	{
		return a.getTime() < b.getTime();
	}

private:

	int dd, mm, yy, h, m, s;	
};

//-----------------------------------------------------------------------------------------

struct SimpleDateFormat // (Helper class, 33 Lines, not part of solution)
{
	SimpleDateFormat(string dateFormat) : dateFormat(dateFormat) {}

	virtual void set(const std::string& set) { dateFormat = set; }

	string format(const Time& time) //http://www.cplusplus.com/reference/ctime/strftime/
	{
		if( dateFormat.empty() )
			return "<Data format not defined>";
		else
		{
			char buf[80];

			time_t t = time;

			if( t > -1 )
			{
				struct tm time = *localtime(&t);

				strftime(buf,sizeof(buf), dateFormat.c_str(), &time);

				return buf;
			}
			else
				return "<Date out of range>";
		}
	}

private:

	string dateFormat;
};

//-----------------------------------------------------------------------------------------------------------------

class WorkdayCalendar // (226 lines with code incl. space)
{
public:

	WorkdayCalendar() : headTime(0), workTime(0), tailTime(0) {	clear(); }
	
	void setHoliday(Time::day set)	{ free[set] = true;	}
															 
	void setHoliday(const Time&	set)
	{ 
		Time date(set); 
		date.clearClock(); //!
		time.push_back( date );
	}

	void delHoliday(const Time&	set)
	{ 
		Time date(set); 
		date.clearClock(); //!
		time.erase(remove(time.begin(), time.end(), date), time.end());
	}

	bool hasHoliday() const { return time.empty(); }

	void setRecurringHoliday(const Time& set) { date.insert(set.getDay()); }

	void setWorkdayStartAndStop(const Time& start, const Time& stop) { startWork = start; stopWork  = stop; } 

	Time getWorkdayIncrement(const Time& start, float workingDays) { return calulate(start, workingDays); }

	void clear()
	{
		time.clear();
		date.clear();

		startWork = startWork = Time();

		headTime = workTime = tailTime = 0;
				
		for(int n=0; n<Time::days; n++) free[n] = false;
	}

protected: //--------------------------------------------------

	bool holiday(Time& t)
	{
		int n = t.getDayOfWeek();	
		int d = t.getDay();

		if( n>=0 && n<7 && free[n] ) return true; //Saturday, Sunday, ...
				
		if( find(time.begin(), time.end(), t) != time.end() ) return true; // 27.05.2004

		if( find(date.begin(), date.end(), d) != date.end() ) return true; // 17.nn.nnnn

		return false;
	}
	
	void getSeconds(const Time& clock, int& time, int& rest)
	{
		int hh = clock.getHour	();
		int mm = clock.getMinute(); 
		int ss = clock.getSecond(); 

		time = (hh * 60 * 60) + (mm * 60) + ss;
		rest = (24 * 60 * 60) - time;
	}

	int getSeconds(const Time& clock, bool rest = false)
	{
		int t, r;

		getSeconds(clock, t, r);
		
		return rest ? r : t;
	}

	void initializeList()
	{ 
		setHoliday(Time::saturday	);
		setHoliday(Time::sunday		);

		sort(time.begin(), time.end()); //Sort ascending

		time.erase(unique (time.begin(), time.end()), time.end()); //Remove duplicate values
	}

	Time calulate(const Time& start, float days)
	{
		int step = days >= 0 ? 1 : -1;

		long work(0), free(0);
		
		int tempTime(0);

		size_t clock(0);

		getSeconds(startWork, headTime, tempTime);			// headTime = (00:00 -> 08:00) //Free
		getSeconds(stopWork,  tempTime, tailTime);			// tailTime = (16:00 -> 23:59) //Free
		workTime = (24 * 60 * 60) - (headTime + tailTime);	// workTime = (08:00 -> 16:00) //Work

		size_t pool = (int)( days * ((float)step) * ((float)workTime) );

		if( 0 == pool ) return Time();

		initializeList();

		Time date(start);

		first(date, work, free, step);

		pool  -= work;

		clock += (work + free);

		date.clearClock(); //!

		SimpleDateFormat f("%d-%m-%Y");

		while( pool > 0 )
		{
			date.addDate(step, 0, 0);

			if( holiday(date) )
				iter(date, work, free, step);
			else if( pool < (size_t)workTime )
				last(pool, work, free, step);
			else
				iter(date, work, free, step);

			pool -= work;

			clock += (work + free);
		}

		date = start;
			
		date.addSeconds(clock, (1 == step) ? true : false);

		return date;
	}

	void first(Time& time, long& work, long& free, int step)
	{
		(1 == step) ? next(time, work, free) : prev(time, work, free);
	}

	void next(Time& time, long& work, long& free)
	{
		work = 0;

		if( holiday(time) )
			free = getSeconds(time, false);
		else if( time > stopWork )  //After worktime
			free = getSeconds(time, true);
		else if( time < startWork ) //Before worktime
		{
			free  = headTime - getSeconds(time);				//(start -> 08:00)
			work  = workTime;									//(08:00 -> 16:00)
			free += tailTime;									//(16:00 -> 23:59)
		}
		else //Office time
		{
			work  = getSeconds(stopWork) - getSeconds(time);	//(start -> 16:00)
			free  = tailTime;									//(16:00 -> 23:59)
		}
	}

	void prev(Time& time, long& work, long& free)
	{
		work = 0;

		if( holiday(time) )
			free = getSeconds(time, true);
		else if( time > stopWork ) //After worktime
		{
			free  = getSeconds(time) - getSeconds(stopWork);	//(16:00 <- start)
			work  = workTime;									//(08:00 <- 16:00)
			free += headTime;									//(00:00 <- 08:00)
		}
		else if( time < startWork )	//Before worktime
			free  = getSeconds(time);							//(start <- start)
		else //Office time
		{
			work  = getSeconds(time) - getSeconds(startWork);	//(08:00 <- start)
			free  = headTime; //(00:00 -> 08:00)
		}
	}

	void iter(Time& time, long& work, long& free, long)
	{
		if( holiday(time) )
		{
			work = 0; //NO work ;-)

			free  = headTime;	//(00:00 <-> 08:00)
			free += workTime;	//(08:00 <-> 16:00)
			free += tailTime;	//(16:00 <-> 23:59)
		}
		else
		{
			free  = headTime;	//(00:00 <-> 08:00)
			work  = workTime;	//(08:00 <-> 16:00)
			free += tailTime;	//(16:00 <-> 23:59)
		}
	}

	void last(const unsigned long& pool, long& work, long& free, int step)
	{
		work = pool;
		free = (1 == step) ? headTime : tailTime; 
	}

private: //--------------------------------------------------

	Time				startWork;			//08:00
	Time				stopWork;			//16:00

	int					headTime;			// = seconds befor workTime	(00:00 -> 08:00)
	int					workTime;			// = seconds workTime		(08:00 -> 16:00)
	int					tailTime;			// = seconds after workTime	(16:00 -> 23:59)

	vector<Time>		time;				//Exact date (05.01.2020)
	set<int>			date;				//Exact day  (05)
	bool				free[Time::days];	//struct tm::tm_wday { 0 = Sunday, 1 = Monday, ... , 7 = Saturday }
};

void streamWorkdayIncrement(int n, WorkdayCalendar& calender, SimpleDateFormat& format, const Time& start, float increment)
{
	cout << fixed << setprecision(7);

	cout << " " << n << ") " << format.format(start) << " with the addition of ";
	
	cout << right << setw(10) << increment;
		
	cout << " working days is ";
	
	cout << format.format(calender.getWorkdayIncrement(start, increment));

	cout << std::string(10, ' ') << "Holiday = ";

	cout << (calender.hasHoliday() ? "ON" : "OFF") << endl;
}

int main()
{
	//------------------------------------------------------------------------------------------------------------
	
	SimpleDateFormat format("%d-%m-%Y %H:%M:%S"); //http://www.cplusplus.com/reference/ctime/strftime/

	//------------------------------------------------------------------------------------------------------------

	Time start; float increment;

//	1) Time(24, 5, 2004, 18,  5); incr = -5.5f			result -> 14-05-2004 12:00
//	2) Time(24, 5, 2004, 19,  3); incr = 44.723656f		result -> 27-07-2004 13:47
//	3) Time(24, 5, 2004, 18,  3); incr = -6.7470217f	result -> 13-05-2004 10:01 (13-05-2004 10:02 diff 00:01)
//	4) Time(24, 5, 2004,  8,  3); incr = 12.782709f		result -> 10-06-2004 14:18 (10-06-2004 14:18 diff 00:03)
//	5) Time(24, 5, 2004,  7,  3); incr =  8.276628f		result -> 04-06-2004 10:12
	
	//------------------------------------------------------------------------------------------------------------

	WorkdayCalendar workdayCalendar;

	workdayCalendar.setWorkdayStartAndStop( Time(1, 1, 2004, 8), Time(1, 1, 2004, 16) );

	workdayCalendar.setRecurringHoliday( Time(17, 5, 2004) );

	cout << endl;

	//------------------------------------------------------------------------------------------------------------
	//1
	
	start = Time(24, 5, 2004, 18, 5); increment = -5.5f;

	workdayCalendar.setHoliday( Time(27, 5, 2004) );

	streamWorkdayIncrement(1, workdayCalendar, format, start, increment);

	//------------------------------------------------------------------------------------------------------------
	//2

	start = Time(24, 5, 2004, 19, 3); increment = 44.723656f;

	workdayCalendar.delHoliday( Time(27, 5, 2004) );

	streamWorkdayIncrement(2, workdayCalendar, format, start, increment);

	//------------------------------------------------------------------------------------------------------------
	//3

	start = Time(24, 5, 2004, 18,  3); increment = -6.7470217f;

	workdayCalendar.setHoliday( Time(27, 5, 2004) );

	streamWorkdayIncrement(3, workdayCalendar, format, start, increment);

	//------------------------------------------------------------------------------------------------------------
	//4

	start = Time(24, 5, 2004,  8,  3); increment = 12.782709f;

	workdayCalendar.delHoliday( Time(27, 5, 2004) );

	streamWorkdayIncrement(4, workdayCalendar, format, start, increment);

	//------------------------------------------------------------------------------------------------------------
	//5

	start = Time(24, 5, 2004,  7,  3); increment = 8.276628f;

	workdayCalendar.delHoliday( Time(27, 5, 2004) );

	streamWorkdayIncrement(5, workdayCalendar, format, start, increment);

	//------------------------------------------------------------------------------------------------------------

	cout << endl;

	getchar();

	return 0;
}
