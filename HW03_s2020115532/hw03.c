// [시스템 프로그래밍 HW03]
// 2020115532 컴퓨터학부 정서현
// curses 라이브러리를 사용한 snake game

#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define SNAKE_CHAR 'o'
#define FRUIT_CHAR '*'
#define WALL_CHAR '#'
#define MAX_SNAKE_LENGTH 100

int WIDTH, HEIGHT;
int GAME_SPEED; // 게임 속도로 난이도 구분

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position snake[MAX_SNAKE_LENGTH];
    Position fruit;
    int length;
    int score;
    char direction; // 뱀이 현재 향하는 방향
} SharedData;

int showDifficultyMenu();
void setup();
void draw_snake(const SharedData *sharedData);
void draw_fruit(int x, int y);
void draw_wall();
void move_snake(SharedData *sharedData, char direction);
void generate_fruit(Position *fruit);
void update_ui(const SharedData *sharedData);
void grow_snake(SharedData *sharedData);
int check_collision(const SharedData *sharedData);
void child_process(SharedData *sharedData);

int showDifficultyMenu() {
    clear();
    int choice;

    // 터미널 크기 얻어오기
    getmaxyx(stdscr, HEIGHT, WIDTH);

    int boxWidth = WIDTH / 2;
    int boxHeight = HEIGHT / 2;

    int boxStartX = (WIDTH - boxWidth) / 2;
    int boxStartY = (HEIGHT - boxHeight) / 2;

    for (int i = 0; i < boxHeight; ++i) {
        for (int j = 0; j < boxWidth; ++j) {
            if (i == 0 || i == boxHeight - 1 || j == 0 || j == boxWidth - 1) {
                mvaddch(boxStartY + i, boxStartX + j, '*');
            }
        }
    }

    int textStartX = boxStartX + (boxWidth - strlen("Select [Snake Game] Difficulty :")) / 2;
    mvprintw(boxStartY + boxHeight / 2 - 3, textStartX, "Select [Snake Game] Difficulty :");
    mvprintw(boxStartY + boxHeight / 2 - 1, boxStartX + (boxWidth - 10) / 2, "1. Easy");
    mvprintw(boxStartY + boxHeight / 2 , boxStartX + (boxWidth - 10) / 2, "2. Normal");
    mvprintw(boxStartY + boxHeight / 2 + 1, boxStartX + (boxWidth - 10) / 2, "3. Hard");

    refresh(); // 변경된 내용을 화면에 반영

    // 입력이 있을 때까지 대기
    while ((choice = getch()) == ERR) {
        usleep(1000); 
    }

    return choice;
}

void setup() {
    signal(SIGINT, SIG_IGN);
    initscr();            
    clear();              
    noecho();             // 키 입력이 화면에 나타나지 않게 함
    curs_set(0);          // 커서를 숨김
    keypad(stdscr, TRUE); // 특수 키 사용을 가능하게 함
    timeout(0);         // 입력 대기 시간을 0으로 주어 키를 누르는 즉시 반응하도록 함

    int difficultyChoice;

    do {
        difficultyChoice = showDifficultyMenu();
    } while (difficultyChoice < '1' || difficultyChoice > '3');

    switch (difficultyChoice) {
        case '1':
            GAME_SPEED = 200000; // Easy
            break;
        case '2':
            GAME_SPEED = 100000; // Medium
            break;
        case '3':
            GAME_SPEED = 50000; // Hard
            break;
        default:
            GAME_SPEED = 200000; 
            break;
    }

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
    // 현재 방향이 입력으로 변경되지 않았다면, 기존 방향을 유지하며 이동
    if (direction == 0) {
        direction = sharedData->direction;
    }

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

    // 뱀이 움직이는 방향 갱신
    sharedData->direction = direction;
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
    // 뱀의 머리가 벽과 부딪혔는지 확인
    if (sharedData->snake[0].x == 0 || sharedData->snake[0].x == COLS - 1 ||
        sharedData->snake[0].y == 0 || sharedData->snake[0].y == LINES - 1) {
        return 1; 
    }

    // 뱀의 머리가 자기 몸과 부딪혔는지 확인
    for (int i = 1; i < sharedData->length; ++i) {
        if (sharedData->snake[0].x == sharedData->snake[i].x &&
            sharedData->snake[0].y == sharedData->snake[i].y) {
            return 1; // 자기 몸에 부딪힘
        }
    }

    return 0;
}

void child_process(SharedData *sharedData) {
    while (1) {        
        int key = getch();

        char direction = 0; // 키 입력이 없을 때는 방향을 변경하지 않음

        switch (key) {
            case 'w':
                direction = 'U';
                break;
            case 's':
                direction = 'D';
                break;
            case 'a':
                direction = 'L';
                break;
            case 'd':
                direction = 'R';
                break;
            case 'q':
                endwin();
                exit(0);
            default:
                break;
        }

        move_snake(sharedData, direction);

        if (sharedData->snake[0].x == sharedData->fruit.x && sharedData->snake[0].y == sharedData->fruit.y) {
            generate_fruit(&sharedData->fruit);
            sharedData->score++;
            grow_snake(sharedData);
        }

        if (check_collision(sharedData)){
            clear();
            mvprintw(HEIGHT / 2, WIDTH / 2 - 5, "Game Over!");
            refresh(); // 화면 갱신
            sleep(3); // 3초 대기
            endwin();
            exit(0);
        }

        update_ui(sharedData);

        usleep(GAME_SPEED); // 난이도에 따라 이동 속도 조절 
    }
}

int main() {
    setup();

    SharedData sharedData = {
        .length = 1,
        .snake[0] = {COLS / 2, LINES / 2},
        .score = 0,
        .direction = 'R' // 초기 방향 설정
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
            usleep(1000); // 1밀리초 대기

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


