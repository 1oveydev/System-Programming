// [시스템 프로그래밍 HW03]
// 2020115532 컴퓨터학부 정서현
// curses 라이브러리를 사용한 snake game

#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define SNAKE_CHAR 'o'
#define FRUIT_CHAR '*'
#define WALL_CHAR '#'
#define MAX_SNAKE_LENGTH 100

int WIDTH, HEIGHT;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position snake[MAX_SNAKE_LENGTH];
    Position fruit;
    int length;
    int score;
} SharedData;

void setup() {
    initscr();            // curses 모드 시작
    clear();              // 화면 초기화
    noecho();             // 키 입력이 화면에 나타나지 않게 함
    curs_set(0);          // 커서를 숨김
    keypad(stdscr, TRUE); // 특수 키 사용을 가능하게 함
    timeout(0);         // 입력 대기 시간을 0으로 주어 키를 누르는 즉시 반응하도록 함

    // 터미널 크기 얻어오기
    getmaxyx(stdscr, HEIGHT, WIDTH);

    // 게임 화면 크기 설정
    HEIGHT -= 2; // 상하에 경계를 그리기 위해 높이를 2 감소
    WIDTH -= 2;  // 좌우에 경계를 그리기 위해 너비를 2 감소
}

void draw_snake(const SharedData *sharedData) {
    for (int i = 0; i < sharedData->length; ++i) {
        mvaddch(sharedData->snake[i].y, sharedData->snake[i].x, SNAKE_CHAR);
    }
}

void draw_fruit(int x, int y) {
    mvaddch(y, x, FRUIT_CHAR);
}

void draw_wall() {
    for (int i = 0; i < COLS; ++i) {
        mvaddch(0, i, WALL_CHAR);
        mvaddch(LINES - 1, i, WALL_CHAR);
    }

    for (int i = 0; i < LINES; ++i) {
        mvaddch(i, 0, WALL_CHAR);
        mvaddch(i, COLS - 1, WALL_CHAR);
    }
}

void move_snake(SharedData *sharedData, char direction) {
    for (int i = sharedData->length - 1; i > 0; --i) {
        sharedData->snake[i] = sharedData->snake[i - 1];
    }

    switch (direction) {
        case 'U':
            sharedData->snake[0].y--;
            break;
        case 'D':
            sharedData->snake[0].y++;
            break;
        case 'L':
            sharedData->snake[0].x--;
            break;
        case 'R':
            sharedData->snake[0].x++;
            break;
    }
}

void generate_fruit(Position *fruit) {
    fruit->x = (rand() % (COLS-2)) + 1;
    fruit->y = (rand() % (LINES-2)) + 1;
}

void update_ui(const SharedData *sharedData) {
    clear();
    draw_wall();
    draw_snake(sharedData);
    draw_fruit(sharedData->fruit.x, sharedData->fruit.y);
    mvprintw(LINES - 2, 2, "Score: %d", sharedData->score);
    refresh();
}

void grow_snake(SharedData *sharedData) {
    if (sharedData->length < MAX_SNAKE_LENGTH) {
        sharedData->length++;
    }
}

int check_collision(const SharedData *sharedData) {
    if (sharedData->snake[0].x == 0 || sharedData->snake[0].x == COLS - 1 ||
        sharedData->snake[0].y == 0 || sharedData->snake[0].y == LINES - 1) {
        return 1; 
    }
    return 0;
}

void child_process(SharedData *sharedData) {
    while (1) {        
        int key = getch();

        switch (key) {
            case 'w':
                move_snake(sharedData, 'U');
                break;
            case 's':
                move_snake(sharedData, 'D');
                break;
            case 'a':
                move_snake(sharedData, 'L');
                break;
            case 'd':
                move_snake(sharedData, 'R');
                break;
            case 'q':
                endwin();
                exit(0);
        }

        if (sharedData->snake[0].x == sharedData->fruit.x && sharedData->snake[0].y == sharedData->fruit.y) {
            generate_fruit(&sharedData->fruit);
            sharedData->score++;
            grow_snake(sharedData);
        }

        if (check_collision(sharedData)){
            clear();
            mvprintw(HEIGHT / 2, WIDTH / 2 - 5, "Game Over!");
            refresh(); // 화면 갱신
            usleep(5000000); // 5초 대기
            endwin();
            exit(0);
        }

        update_ui(sharedData);

        usleep(100000); 
    }
}

int main() {
    setup();

    SharedData sharedData = {
        .length = 1,
        .snake[0] = {COLS / 2, LINES / 2},
        .score = 0
    };

    generate_fruit(&sharedData.fruit);

    pid_t pid = fork();

    if (pid == 0) {
        // 자식 프로세스
        child_process(&sharedData);
    }
    else {
        // 부모 프로세스
        while (1) {
            usleep(100000); // 일시 정지

            // 자식 프로세스가 종료되었는지? 
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == -1) {
                perror("waitpid");
                endwin();
                exit(1);
            } 
            // 자식 프로세스 종료
            else if (result > 0)
                break;
        }

        endwin();
    }

    return 0;
}

