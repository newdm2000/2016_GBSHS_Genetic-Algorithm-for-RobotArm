/* Replace "dll.h" with the name of your header */
#include "main.h"
#include <time.h>

#define COUNT 10 /// COUNT per generation
#define TARGET_DEFAULT 10
#define MUTANT 5

using namespace std;

typedef struct S_ACTION {
    int w1, w2, w3, t2, t3, t4;
} ACTION;

int target = TARGET_DEFAULT;    /// state that we are targetting

ACTION now[COUNT];
int    eva[COUNT]; /// evaluating action
int    rnk[COUNT]; /// rank of action

int willAct = 0;


int generation = 1;

int isinit = 0; // 초기화가 진행되었는지 아닌지를 검증합니다.

void init();  /** 함수 init()는 DLL 호출 시 단 한번만 호출되며, 초기화 역할을 수행합니다. */
void initAction();

void logTime();
void nextGen();
int evaluate(int state); /// 현재 상태 평가 함수. 값이 클수록 유리함.
ACTION crossbreed(ACTION a, ACTION b);

void printActions();
ACTION randomAction();                /// randomAction();
int random(int mn, int mx);           /// random(최소 정수, 최대 정수)
int boundary(int x, int mn, int mx);  /// boundary(값, 최솟값, 최댓값)
int abs(int x);

bool forsort(int a, int b) { return eva[a] > eva[b]; }


/** 함수 study()는 매 학습 과정마다 한 번씩 호출됩니다.
  int state           : 입력받는 값. 이전의 결과에 대한 피드백을 제공합니다.
  int* w1, w2, w3, t2, t3, t4 : 출력값으로, LabVIEW에서 해당 값으로 로봇 팔을 제어합니다.
 */

DLLIMPORT void study(int state, int* w1, int* w2, int* w3, int* t2, int* t3, int* t4) {
    ACTION result; // 모든 작업이 끝나면 result에 있는 행동을 LabVIEW에 반환합니다.

    if (!isinit) init();
    if (state == 0) { /// 0을 입력받으면 현재 Action 정보를 날립니다.
        initAction();
    }

    result = now[willAct];
    *w1 = result.w1; *w2 = result.w2; *w3 = result.w3;
    *t2 = result.t2; *t3 = result.t3; *t4 = result.t4;
    printf("study() --> %d %d %d %d %d %d\n", *w1, *w2, *w3, *t2, *t3, *t4);
}

DLLIMPORT void feedback(int value, int debug) {
    printf("feedback() = %d %d\n", value, debug);
    eva[willAct] = evaluate(value);
    willAct++;

    if (willAct == COUNT) {
        printActions();
        nextGen();
        willAct = 0;
    }
}

DLLIMPORT void setTarget(int t) {
    target = t;
}

DLLIMPORT int getTarget() {
    return target;
}

DLLIMPORT int getGeneInfo(int num, int attr) {
    switch(attr) {
        case 0: return now[num].w1;
        case 1: return now[num].w2;
        case 2: return now[num].w3;
        case 3: return now[num].t2;
        case 4: return now[num].t3;
        case 5: return now[num].t4;
    }
    return -1;
}

DLLIMPORT int getGeneration() {
    return generation;
}

DLLIMPORT int getGeneNum() {
    return willAct;
}

DLLIMPORT void logVoltage(float voltage, int value) {
    printf("volt = %f  ", voltage);
    return;

}

///////////////////////////////////////////////////////////////
//                  학습 흐름과 관련된 함수들                //
///////////////////////////////////////////////////////////////

void init() {
    srand(time(NULL));

    freopen("log.txt", "a", stdout); // 이 코드 이후로 printf()는 로깅 작업을 수행합니다.
    printf("\n\nLogging Start.\n");

    // 초기화 작업은 이 곳에서 진행합니다.
    for (int i=0;i<COUNT;i++)
        rnk[i] = i;

    FILE *in = fopen("GAInput.txt", "r");
    if (in != NULL) {
        int t, r, a, b, c, d, e, f;
        fscanf(in, "%d %d", &t, &r); //t는 target 지정 여부, r은 action 설정 여부를 의미한다.
        setTarget(t);
        printf("target: %d\n", target);
        if (r==1) {
            for (int i=0;i<10;i++) {
                fscanf(in, "%d %d %d %d %d %d", &a, &b, &c, &d, &e, &f);
                now[i].w1=a; now[i].w2=b; now[i].w3=c; now[i].t2=d; now[i].t3=e; now[i].t4=f;
            }

            printActions();
            nextGen();
        } else initAction();
    } else initAction();

    logTime();
    isinit = 1;
}

void logTime() {
    time_t timer;
    struct tm *t;

    timer = time(NULL);
    t = localtime(&timer);

    printf("Now : %d.%02d.%02d %02d:%02d:%02d\n",
           t->tm_year+1900, t->tm_mon+1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
}

void initAction() {
    for (int i=0; i<COUNT; i++) now[i] = randomAction();
    willAct = 0;
}

void nextGen() {
    ACTION tmp[COUNT];
    int idx = 0;

    logTime();
    printf("\n\n* GENERATION %5d\n", ++generation);

    /// Sorting (& Select)
    sort(rnk, rnk+COUNT, forsort);


    /// now : A0  A1  A2  A3
    /// eva :  4   2   7   4
    /// rnk :  2   0   3   1

    /// now[rnk[0]] == now[2] == A1
    /// now[rnk[k]]

    /// Make new actions : 이 값은 개체수 10에 최적화됨
    for (int i=0;i<5;i++) {
        for (int j=i+1;j<5;j++) {
            if (eva[rnk[i]]==-1 || eva[rnk[j]]==-1) continue;
            tmp[idx++] = crossbreed(now[rnk[i]], now[rnk[j]]);

            if (idx >= COUNT) break;
        }
        if (idx >= COUNT) break;
    }

    while (idx < COUNT) {
        tmp[idx++] = randomAction();
    }

    /// 대입하기
    for (int i=0;i<COUNT;i++) {
        now[i] = tmp[i];
    }
}

int evaluate(int state) {
    if (state==99) return -1;
    return 100-abs(target-state);
}

ACTION crossbreed(ACTION a, ACTION b) {
    ACTION result;

    result.w1 = boundary(random(1, 100) > MUTANT ? (random(0, 1) ? a.w1 : b.w1)+random(-5, 5) : random(W1_MIN, W1_MAX), W1_MIN, W1_MAX);
    result.w2 = boundary(random(1, 100) > MUTANT ? (random(0, 1) ? a.w2 : b.w2)+random(-5, 5) : random(W2_MIN, W2_MAX), W2_MIN, W2_MAX);
    result.w3 = boundary(random(1, 100) > MUTANT ? (random(0, 1) ? a.w3 : b.w3)+random(-5, 5) : random(W2_MIN, W2_MAX), W3_MIN, W3_MAX);
    result.t2 = boundary(random(1, 100) > MUTANT ? (random(0, 1) ? a.t2 : b.t2)+random(-5, 5) : random(T2_MIN, T2_MAX), T2_MIN, T2_MAX);
    result.t3 = boundary(random(1, 100) > MUTANT ? (random(0, 1) ? a.t3 : b.t3)+random(-5, 5) : random(T3_MIN, T3_MAX), T3_MIN, T3_MAX);
    result.t4 = boundary(random(1, 100) > MUTANT ? (random(0, 1) ? a.t4 : b.t4)+random(-5, 5) : random(T3_MIN, T3_MAX), T4_MIN, T4_MAX);

    return result;
}


///////////////////////////////////////////////////////////////
//                간단한 역할을 수행하는 함수들              //
///////////////////////////////////////////////////////////////1

void printActions() {
    for (int i=0;i<COUNT;i++) {
        ACTION a = now[i];
        printf("  %3d %3d %3d %3d %3d %3d : %3d\n", a.w1, a.w2, a.w3, a.t2, a.t3, a.t4, eva[i]);
    }
}

ACTION randomAction() {
    ACTION act;
    act.w1 = random(W1_MIN, W1_MAX);
    act.w2 = random(W2_MIN, W2_MAX);
    act.w3 = random(W3_MIN, W3_MAX);
    act.t2 = random(T2_MIN, T2_MAX);
    act.t3 = random(T3_MIN, T3_MAX);
    act.t4 = random(T4_MIN, T4_MAX);

    return act;
}


int random(int mn, int mx) {
    return rand()%(mx-mn+1) + mn;
}

int boundary(int x, int mn, int mx) {
    if (x<mn) return mn;
    if (x>mx) return mx;
    return x;
}

int abs(int x) {
    return (x>0)?x:-x;
}



BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason, LPVOID reserved) { return TRUE; }
