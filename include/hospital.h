#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ── 구조체 정의 ──
typedef struct {
    int id;
    int priority;       // 3순위(초응급), 2순위(응급), 1순위(일반)
    int surgery_time;   // 수술시간/진료시간
    int arrive_time;    // 도착(접수) 시간
} Patient;

typedef struct {
    int id;             // 방 번호
    int finish_time;    // 수술/진료가 끝나는 시간
} Room;

typedef struct Node {
    Patient data;       
    struct Node* next;  
} Node;

typedef struct {
    Node* front;
    Node* rear;
    int count;
} Queue;

typedef struct StackNode {
    Patient data;
    struct StackNode* next;
} StackNode;

typedef struct {
    StackNode* top;
} RecordStack;

// ── 전역 변수 (extern 선언) ──
extern Room rooms[8];
extern Queue nonEmergency_q;
extern Queue emergency_q;
extern RecordStack normal_stack;
extern RecordStack emerge_stack;
extern int p1_finish_time;
extern int current_time;

// ── 함수 원형 및 설명 ──

// 시스템 초기화 (큐, 스택, 수술실 방 세팅)
void init();

// 진료/수술이 끝난 환자를 기록용 스택에 추가
void push_stack(RecordStack* s, Patient p);

// 우선순위 큐 삽입 (3순위가 2순위보다 무조건 앞, 같은 순위면 뒤로)
void enqueue_priority(Queue* q, Patient p);

// 일반 큐 삽입 (1순위 전용 단순 FIFO)
void enqueue(Queue* q, Patient p);

// 큐의 맨 앞(front) 환자를 제거하고 반환 (수술실 입장용)
Patient dequeue(Queue* q);

// 큐의 맨 뒤(rear) 환자를 제거하고 반환 (2순위 환자 타병원 이송 퇴출용)
Patient dequeue_rear(Queue* q);

// 특정 번호대 방들의 최소 대기 시간을 계산하여 반환
int cal_wait_time(int start, int end);

// 1순위 환자 접수 처리 (일반 진료)
void avail_priority1(Patient p);

// 2순위 환자 접수 처리 (5~7번 방 30분 이하 대기 시 접수)
void avail_priority2(Patient p);

// 3순위 환자 접수 처리 (0~4번 방 우선, 꽉 차면 5~7번 방 침범 및 2순위 퇴출 로직 포함)
void avail_priority3(Patient p);

// 환자의 순위별 수술 시간을 세팅하고 각 접수(avail) 함수로 연결하는 메인 접수 함수
void request_admission(Patient p);

// 현재 시간을 기준으로 대기 중인 환자들을 빈 수술실/진료실에 입장시키고 큐에서 제거하는 함수
void process_rooms();

#endif

