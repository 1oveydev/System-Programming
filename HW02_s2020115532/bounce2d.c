// 2020115532 컴퓨터학부 정서현 시스템 프로그래밍 HW2
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#define TOP_ROW 5
#define BOT_ROW 20
#define LEFT_EDGE 10
#define RIGHT_EDGE 70
#define TICKS_PER_SEC 50
#define MESSAGE "o"
#define BLANK "   "

int row;
int col;
double x_dir;
double y_dir;

void setup();
void move_ball(int);
int set_ticker(int);

int main() {
    int c;
    setup();

    while ((c = getch()) != 'Q') {
        if (c == 's' && x_dir > 1)
            x_dir /= 2;
        else if (c == 'S' && y_dir > 1)
            y_dir /= 2;
        else if (c == 'f')
            x_dir *= 2;
        else if (c == 'F')
            y_dir *= 2;
    }

    endwin();
    return 0;
}

void move_ball(int signum) {
    signal(SIGALRM, move_ball);
    move(row, col);
    addstr(BLANK);
    col += x_dir;
    row += y_dir;

    if(x_dir < 0 && col<= LEFT_EDGE)
    {
        col = LEFT_EDGE;
        x_dir = -x_dir;
    }
    else if(x_dir > 0 && col+strlen(MESSAGE) >= RIGHT_EDGE)
    {
        col = RIGHT_EDGE;
        x_dir = -x_dir;
    }
    if(y_dir < 0 && row <= TOP_ROW)
    {
        row = TOP_ROW;
        y_dir = -y_dir;
    }
    else if(y_dir > 0 && row+strlen(MESSAGE) >= BOT_ROW)
    {
        row = BOT_ROW;
        y_dir = -y_dir;
    }
    if (row <= BOT_ROW && row >= TOP_ROW && col <= RIGHT_EDGE && col >= LEFT_EDGE)
    {  
        move(row, col); 
        addstr(MESSAGE);
    }
    refresh();
}

void setup() {
    initscr();
    crmode();
    noecho();
    curs_set(0);
    clear();
	
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    // row = (TOP_ROW + BOT_ROW) / 2;
    // col = (LEFT_EDGE + RIGHT_EDGE) / 2;
    x_dir = 1;
    y_dir = 1;

    signal(SIGALRM, move_ball);
    set_ticker(TICKS_PER_SEC);
}

int set_ticker(int n_msecs) {
    struct itimerval new_timeset;
    long n_sec, n_usecs;

    n_sec = n_msecs / 1000;
    n_usecs = (n_msecs % 1000) * 1000L;

    new_timeset.it_interval.tv_sec = n_sec;
    new_timeset.it_interval.tv_usec = n_usecs;
    new_timeset.it_value.tv_sec = n_sec;
    new_timeset.it_value.tv_usec = n_usecs;

    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}


