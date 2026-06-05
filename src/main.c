#include "../include/hospital.h"

int main() {
    // 1. 시스템 초기화
    init();
    
    // 2. 환자 ID 부여용 변수
    int id = 1;  
    int id2 = 1; 
    int id3 = 1; 

    // 3. 테스트 시나리오 시작 시간 및 방 초기 상태 강제 세팅
    current_time = 50;
    rooms[0].finish_time = 300; rooms[1].finish_time = 310; rooms[2].finish_time = 320; 
    rooms[3].finish_time = 330; rooms[4].finish_time = 340;
    rooms[5].finish_time = 180; rooms[6].finish_time = 180; rooms[7].finish_time = 180;
    
    // 4. 환자 템플릿 생성
    Patient p1 = { id, 1, 0, current_time };
    Patient p2 = { id2, 2, 0, current_time };
    Patient p3 = { id3, 3, 0, current_time };

    // 5. 5분 단위로 시간이 흐르는 시뮬레이션 시작
    for (current_time = 50; current_time <= 400; current_time += 5) {
        
        // 시나리오 1: 50분부터 150분까지 1순위 환자가 5분마다 계속 도착
        if (current_time >= 50 && current_time <= 150) { 
            p1.id = id++;
            request_admission(p1);
            printf("\n");
        }
        
        // 시나리오 2: 150분에 2순위 환자 2명 동시 도착
        if (current_time == 150) {
            p2.id = id2++; request_admission(p2); printf("\n");
            p2.id = id2++; request_admission(p2); printf("\n");
        }
        
        // 시나리오 3: 170분에 2순위 환자 1명 도착
        if (current_time == 170) {
            p2.id = id2++; request_admission(p2); printf("\n");
        }
        
        // 시나리오 4: 175분에 3순위 환자 2명 동시 도착
        if (current_time == 175) {
            p3.id = id3++; request_admission(p3); printf("\n");
            p3.id = id3++; request_admission(p3); printf("\n");
        }

        // 시간이 지날 때마다 빈 방이 있는지 확인하고 대기열 환자 입장 처리
        process_rooms();
    }

    return 0;
}
