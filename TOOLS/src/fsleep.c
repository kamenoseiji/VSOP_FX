/*************************************************
**	FSLEEP.C : SLEEP FOR THE GIVEN DURATION in	**
**				UNIT of millisecond.			**
**												**
**	AUTHOR	:  KAMENO Seiji						**
**	CREATED	:  1996/6/4							**
**	CAUTION:	This module is available only on**
**				SystemV UNIX.					**
/*************************************************/

#include <stdio.h>
#include <sys/time.h>
#include <signal.h>

static	int		alarm_flag;


fsleep( sleep_counter )
	unsigned int		sleep_counter;		/* SLEEP DURATION in [msec] */
{
	void				turn_flag();		/* Signal Action Funcrion */
	int					sig_mask;			/* Signal Mask */
	struct	sigaction	current_act;		/* Signal Action */
	struct	sigaction	prev_act;			/* Previous Signal Action */
	struct	itimerval	current_itv;		/* Interval Timer */
	struct	itimerval	prev_itv;			/* Previous Interval Timer */
	register struct	itimerval	*itv_ptr = &current_itv;

	/*-------- CHECK FOR SLEEP COUNTER --------*/
	if( sleep_counter == 0){	return(0);	}

	/*-------- SET CURRENT TIME to INTERVAL TIMER --------*/
	timerclear( &itv_ptr->it_interval );
	timerclear( &itv_ptr->it_value );
	if(setitimer( ITIMER_REAL, itv_ptr, &prev_itv) < 0){ return(0);	}

	memset( &prev_act, 0, sizeof(prev_act) );
	prev_act.sa_handler	= SIG_DFL;

	itv_ptr->it_value.tv_sec	= sleep_counter/1000;
	itv_ptr->it_value.tv_usec	= (sleep_counter%1000)*1000;

	if(timerisset(&prev_itv.it_value)){
		if( prev_itv.it_value.tv_sec >= itv_ptr->it_value.tv_sec ){

			if( (prev_itv.it_value.tv_sec == itv_ptr->it_value.tv_sec)
			 &&	(prev_itv.it_value.tv_usec > itv_ptr->it_value.tv_usec)){
				prev_itv.it_value.tv_usec -= itv_ptr->it_value.tv_usec;
			}
			prev_itv.it_value.tv_sec -= itv_ptr->it_value.tv_sec;

		} else {

			itv_ptr->it_value	= prev_itv.it_value;
			prev_itv.it_value.tv_sec	= 1;
			prev_itv.it_value.tv_usec	= 0;
		}
	}

	memset( &current_act, 0, sizeof(current_act) );
	current_act.sa_handler = turn_flag;
	alarm_flag	= 0;

	sigaction( SIGALRM, &current_act, &prev_act );
	setitimer(ITIMER_REAL, itv_ptr, (struct itimerval *)0 );

	while(alarm_flag == 0) {
		sigpause( SIGALRM );
	}

	sigaction( SIGALRM, &prev_act, (struct sigaction *)0 );
	setitimer(ITIMER_REAL, &prev_itv, (struct itimerval *)0 );
	sighold(sig_mask);

	return(0);
}


static void	turn_flag()
{
	alarm_flag	= 1;
}
