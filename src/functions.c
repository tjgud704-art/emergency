#include "../include/hospital.h"

// ── 전역 변수 정의 ──
Room rooms[8];
Queue nonEmergency_q;       // 1순위 전용 일반 큐
Queue emergency_q;          // 3순위, 2순위 공용 우선순위 큐
RecordStack normal_stack;   // 1순위 진료 기록 스택
RecordStack emerge_stack;   // 2, 3순위 수술 기록 스택
int p1_finish_time = 0;     // 1순위 진료실(1개) 끝나는 시간
int current_time = 0;       // 시뮬레이션 상 현재 시각

// 스택에 기록을 추가하는 함수 (수술/진료 완료 시 호출)
void push_stack(RecordStack* s, Patient p) {
    StackNode* node = (StackNode*)malloc(sizeof(StackNode));
    node->data = p;
    node->next = s->top;
    s->top = node;
}

// 초기화 함수 (큐, 스택 비우고 방 시간 0으로 세팅)
void init() {
    nonEmergency_q.front = nonEmergency_q.rear = NULL;
    nonEmergency_q.count = 0;
    emergency_q.front = emergency_q.rear = NULL;
    emergency_q.count = 0;
    normal_stack.top = NULL;
    emerge_stack.top = NULL;
    for (int i = 0; i < 8; i++) {
        rooms[i].id = i;
        rooms[i].finish_time = 0;
    }
}

// 우선순위 큐 삽입 (3순위가 먼저 오도록 정렬)
void enqueue_priority(Queue* q, Patient p) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = p;
    new_node->next = NULL;
    q->count++;

    // 큐가 비어있으면 바로 삽입
    if (q->front == NULL) {
        q->front = q->rear = new_node;
        return;
    }

    // 3순위(초응급) 환자가 들어올 경우
    if (p.priority == 3) {
        Node* curr = q->front;
        Node* prev = NULL;
        // 기존에 있던 3순위 환자들 뒤로, 2순위보다는 앞으로 위치를 찾음
        while (curr != NULL && curr->data.priority == 3) {
            prev = curr;
            curr = curr->next;
        }
        new_node->next = curr;
        if (prev == NULL) q->front = new_node;
        else prev->next = new_node;
        if (curr == NULL) q->rear = new_node;
    }
    // 2순위(응급) 환자가 들어올 경우 (무조건 맨 뒤로 삽입)
    else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
}

// 일반 큐 삽입 (1순위 전용, 들어온 순서대로 처리)
void enqueue(Queue* q, Patient p) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = p;
    new_node->next = NULL;
    q->count++;
    if (q->rear == NULL) {
        q->front = q->rear = new_node;
    }
    else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
}

// 큐의 맨 앞(front) 환자를 빼내는 함수 (방에 들어갈 때 사용)
Patient dequeue(Queue* q) {
    if (q->front == NULL) { fprintf(stderr, "큐 비어있음\n"); exit(1); }
    Patient p = q->front->data;
    Node* temp = q->front;
    q->front = temp->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    q->count--;
    return p;
}

// 큐의 맨 뒤(rear) 환자를 빼내는 함수 (2순위 타병원 이송/퇴출 시 사용)
Patient dequeue_rear(Queue* q) {
    if (q->rear == NULL) { fprintf(stderr, "큐 비어있음\n"); exit(1); }
    Patient p = q->rear->data;
    q->count--;
    if (q->front == q->rear) {
        free(q->rear);
        q->front = q->rear = NULL;
        return p;
    }
    Node* curr = q->front;
    while (curr->next != q->rear) curr = curr->next;
    free(q->rear);
    q->rear = curr;
    q->rear->next = NULL;
    return p;
}

// 특정 방 구간(start~end)의 최소 대기 시간을 계산하는 함수
int cal_wait_time(int start, int end) {
    int min_wait = 10000000;
    for (int i = start; i <= end; i++) {
        int wait = rooms[i].finish_time - current_time;
        if (wait <= 0) wait = 0; // 이미 방이 비어있으면 대기시간 0
        if (wait < min_wait) min_wait = wait;
    }
    return min_wait;
}

// 1순위(일반) 접수 처리: 대기열에 순서대로 넣음
void avail_priority1(Patient p) {
    enqueue(&nonEmergency_q, p);
    printf("  → [현재 시각] : %d [접수 성공] 1순위 ID%d | 대기인원 : %d\n", current_time, p.id , nonEmergency_q.count);
}

// 2순위(응급) 접수 처리: 5~7번 방 대기가 30분 이하일 때만 접수
void avail_priority2(Patient p) {
    int wait = cal_wait_time(5, 7);
    if (wait <= 30) {
        enqueue_priority(&emergency_q, p);
        printf("  → [현재 시각] %d [접수 성공] 2순위 ID%d | 5~7번방 대기: %d분 \n", current_time, p.id, wait);
    }
    else {
        printf("  → [현재 시각] : %d [접수 실패] 2순위 ID%d | 타병원 이용바랍니다.\n", current_time, p.id);
    }
}

// 3순위(초응급) 접수 처리 및 2순위 퇴출 로직
void avail_priority3(Patient p) {
    int wait = cal_wait_time(0, 4); // 우선 0~4번 방 확인
    
    // 조건 1. 0~4번 방 대기가 5분 이하면 정상 접수
    if (wait <= 5) {
        enqueue_priority(&emergency_q, p);
        printf("  → [현재 시각] %d [접수 성공] 3순위 ID%d | 0-4번방 대기: %d분 \n", current_time, p.id, wait);
        return; 
    }

    printf("  → 0-4번방 대기 %d분 초과. 5-7번방 확인 중...\n", wait);   
    int wait57 = cal_wait_time(5, 7); // 5~7번 방 확인

    // 대기 중인 환자가 아예 없을 경우 (5~7번 방만 확인)
    if (emergency_q.rear == NULL) {
        if (wait57 <= 5) {
            enqueue_priority(&emergency_q, p);
            printf("  → [현재 시각] %d [접수 성공] 3순위 ID%d | 5-7번방 대기 %d분 (대기열 없음)\n", current_time, p.id, wait57);
        }
        else {
            printf("  → [현재 시각] %d [접수 실패] 3순위 ID%d | 모든 방 대기 5분 초과 접수 거부\n", current_time, p.id);
        }
        return;
    }

    // 큐에 대기 환자가 있을 경우, 타 병원 이송(퇴출) 조건 확인
    int rear_arrive = emergency_q.rear->data.arrive_time;
    int rear_priority = emergency_q.rear->data.priority;
    int rear_id = emergency_q.rear->data.id;
    int golden_time = current_time - rear_arrive; // 마지막 환자가 대기한 시간

    // 조건 2. 5~7방 대기 5분 이하 & 큐 마지막 환자가 2순위 & 골든타임 임박 전(15분 이하 대기)
    if (wait57 <= 5 && rear_priority == 2 && golden_time <= 15) {
        printf("  → [현재 시각] %d 2순위 ID%d 타병원 이송 (대기 %d분)\n", current_time, rear_id, golden_time);
        dequeue_rear(&emergency_q);         // 2순위 퇴출
        enqueue_priority(&emergency_q, p);  // 3순위 침범 접수
        printf("  → [현재 시각] %d [접수 성공] 3순위 ID%d | 5-7번방 침범 접수\n", current_time, p.id);
    }
    // 조건 불충족 시 접수 실패
    else {
        printf("  → [현재 시각] %d [접수 실패] 3순위 ID%d | ", current_time , p.id);
        if (golden_time > 15)
            printf("2순위 ID%d 골든타임 임박(%d분 대기) → 퇴출 불가\n", rear_id, golden_time);
        else
            printf("5-7번방 대기 %d분 초과\n", wait57);        
    }
}

// 환자 순위별로 수술 시간을 세팅하고 접수 로직으로 넘기는 분배 함수
void request_admission(Patient p) {
    p.arrive_time = current_time;
    p.surgery_time = (p.priority == 2) ? 90 : (p.priority == 3) ? 180 : 10;

    switch (p.priority) {
    case 1: avail_priority1(p); break;
    case 2: avail_priority2(p); break;
    case 3: avail_priority3(p); break;
    }
}

// 큐에 있는 환자들을 실제 빈 방으로 밀어 넣는 처리 로직
void process_rooms() {     
    int changed = 1;
    // 응급 큐 처리 (방 상태가 업데이트되는 동안 계속 반복)
    while (changed && emergency_q.count > 0) {
        changed = 0;
        Patient p = emergency_q.front->data; 

        // 3순위 환자는 0~4번 방을 우선 확인하고, 없으면 5~7번 방 확인
        if (p.priority == 3) {
            for (int i = 0; i <= 4; i++) { 
                if (rooms[i].finish_time <= current_time) { // 방이 비었으면
                    rooms[i].finish_time = current_time + p.surgery_time;
                    dequeue(&emergency_q);
                    push_stack(&emerge_stack, p);
                    printf("  >> 현재 시각 [%d분] 3순위 ID%d → %d번방 입장 (종료:%d분)\n", 
                        current_time, p.id, i, rooms[i].finish_time);          
                    changed = 1; break;  
                }
            }
            if (!changed) { // 0~4번이 꽉 찼다면 5~7번 확인
                for (int i = 5; i <= 7; i++) {
                    if (rooms[i].finish_time <= current_time) {
                        rooms[i].finish_time = current_time + p.surgery_time;
                        dequeue(&emergency_q);
                        push_stack(&emerge_stack, p);
                        printf("  >> 현재 시각 [%d분] 3순위 ID%d → %d번방(침범) 입장 (종료:%d분)\n",
                            current_time, p.id, i, rooms[i].finish_time);
                        changed = 1; break;
                    }
                }
            }
        }
        // 2순위 환자는 5~7번 방만 확인
        else if (p.priority == 2) {
            for (int i = 5; i <= 7; i++) {
                if (rooms[i].finish_time <= current_time) {
                    rooms[i].finish_time = current_time + p.surgery_time;
                    dequeue(&emergency_q);
                    push_stack(&emerge_stack, p);
                    printf("  >> 현재 시각 [%d분] 2순위 ID%d → %d번방 입장 (종료:%d분)\n",
                        current_time, p.id, i, rooms[i].finish_time);
                    changed = 1; break;
                }
            }
        }
    } 

    // 일반 큐 처리 (1순위 전용 진료실 1개)
    while (nonEmergency_q.count > 0 && p1_finish_time <= current_time) {
        Patient p = dequeue(&nonEmergency_q);
        p1_finish_time = current_time + p.surgery_time;
        push_stack(&normal_stack, p);
        printf("  >> 현재 시각 [%d분] 1순위 ID%d → 진료실 입장 (종료:%d분)\n",
            current_time, p.id, p1_finish_time);
    }
}
