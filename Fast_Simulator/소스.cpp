#include <iostream>

#define Mega_Byte 11

#define Sector_Size 516
#define Sector_Cnt 32
#define Total_Sector_Cnt 1024*1024*Mega_Byte / Sector_Size
#define Block_Cnt Total_Sector_Cnt/Sector_Cnt
#define Log_Block_Ratio 0.4

using namespace std;

int main() {
    if (Log_Block_Ratio == 0.0) { cout << "비율을 다시 설정해주세요"; return 0; }
    int Data_Block_Cnt = (int)((1 - Log_Block_Ratio) * Block_Cnt);
    if (Block_Cnt - Data_Block_Cnt == 1) { cout << "최소 2개이상의 로그블록이 필요하므로 재설정합니다." << "\n"; Data_Block_Cnt--; }
    cout << "Data_Block_Cnt : " << Data_Block_Cnt << "\n";
    int Log_Block_Cnt = Block_Cnt - Data_Block_Cnt;
    if (Log_Block_Cnt == 1 || Log_Block_Cnt == 0) { Log_Block_Cnt = 2; }
    std::cout << "Log_Block_Cnt : " << Log_Block_Cnt << "\n";
    int* Data_set = new int[Data_Block_Cnt * Sector_Cnt];
    //int wear_level[Block_Cnt * Sector_Cnt] = {};

    char oper;
    int sec_num = -1;
    int idx = 0;
    int w_cnt = 0;
    int r_cnt = 0;
    int e_cnt = 0;
    int merge_cnt = 0;
    int switch_cnt = 0;
    int input_cnt = 0;
    int swmp[Sector_Cnt] = {};
    int* rwmp = new int[(Log_Block_Cnt - 1) * Sector_Cnt];
    int seq = 0;
    int cnt = 0;
    int SPBN = -1;
    int RPBN = -1;
    int free_block_cnt = Sector_Cnt;
    int seq_block_cnt = Sector_Cnt;

    fill_n(Data_set, Data_Block_Cnt * Sector_Cnt, -2);
    fill_n(swmp, Sector_Cnt, -1);
    fill_n(rwmp, (Log_Block_Cnt - 1) * Sector_Cnt, -1);

    while (true) {
        // Data Set input
        std::cout << "Data_Set 입력(나가기 -> x): "; cin >> oper >> sec_num;
        input_cnt++;
        if (sec_num >= Data_Block_Cnt * Sector_Cnt || sec_num < 0) { cout << "out of range" << "\n\n"; return 0; }
        // Exit
        if (oper == 'x') { break; }

        // Flash_Memory
        if (Data_set[sec_num] < 0) { Data_set[sec_num]++; r_cnt++; w_cnt++; }
        
        // Overwrite
        if (Data_set[sec_num] == 0 && oper == 'w') {
            int LSN = sec_num;
            int offset = LSN % Sector_Cnt;
            int LBN = LSN / Sector_Cnt;
            

            // 순차 시작
            if (offset == seq) {
                if (seq == 0) {
                    std::cout << "Seqeuence start" << "\n\n";
                    seq++;
                    swmp[offset] = LSN;
                    SPBN = swmp[0] / Sector_Cnt;
                    seq_block_cnt--;
                    continue;
                }
                else {
                    SPBN = swmp[0] / Sector_Cnt;

                    // random write
                    if (LBN != SPBN) { 
                        Data_set[sec_num] = idx; std::cout << "idx : " << idx << "\n";
                        rwmp[Data_set[LSN]] = LSN;
                        cnt++;

                        // full merge
                        if (cnt == (Log_Block_Cnt - 1) * Sector_Cnt) {
                            std::cout << "Full Merge start" << "\n\n";
                            for (int b = 0; b < idx; b++) {
                                if (rwmp[b] == -1) { continue; }
                                Data_set[rwmp[b]] = -1;
                                free_block_cnt--;
                                w_cnt++;
                                RPBN = rwmp[b] / Sector_Cnt;
                                for (int c = b + 1; c < idx; c++) {
                                    if (RPBN == rwmp[c] / Sector_Cnt) {
                                        Data_set[rwmp[c]] = -1;
                                        free_block_cnt--;
                                        rwmp[c] = -1;
                                        w_cnt++;
                                    }
                                    else if (free_block_cnt == 0) { break; }
                                    else if (Data_set[LBN * Sector_Cnt + offset] < -1){ free_block_cnt--; }
                                }
                                merge_cnt++;
                                rwmp[b] = -1;
                                r_cnt += free_block_cnt;
                                w_cnt += free_block_cnt;
                                e_cnt++;
                                free_block_cnt = Sector_Cnt;
                               // wear_level[rwmp[b]]++;
                            }
                            cnt = 0;
                            idx = 0;
                            std::cout << "Merge finish" << "\n\n";
                            continue;
                        }
                        continue;
                    }
                    else {
                        if (rwmp[Data_set[LSN]] != -1) { rwmp[Data_set[LSN]] = -1; }
                        seq_block_cnt--;
                        swmp[offset] = LSN;
                        seq++;
                        std::cout << "seq :" << seq << "\n";

                        // Seq_write block이 꽉찼을때
                        if (seq == Sector_Cnt) {
                            switch_cnt++;
                            std::cout << "Full seq_block" << "\n";
                            seq = 0;
                            e_cnt++;
                            //wear_level[LBN * Sector_Cnt + offset]++;
                            for (int a = 0; a < Sector_Cnt; a++) { swmp[a] = -1; }
                            std::cout << "switch block" << "\n\n";
                            seq_block_cnt = Sector_Cnt;
                        }
                        continue;
                    }
                }
            }

            else if (offset != seq && offset == 0) {
                if (Data_set[sec_num] == 0) {
                    std::cout << "idx(random_write) : " << idx << "\n\n";
                    Data_set[sec_num] = idx;
                    rwmp[idx] = LSN;
                    cnt++;
                    idx++;
                }
                seq = 0;
                r_cnt += (seq_block_cnt - 1);
                w_cnt += (seq_block_cnt - 1) + 1;
                e_cnt++;
                seq_block_cnt = Sector_Cnt;
                // wear_level[LBN*Sector_Cnt + offset]++;
                for (int a = 0; a < Sector_Cnt; a++) { swmp[a] = -1; }
                std::cout << "switch operation" << "\n\n";
            }

            // 순차성에 어긋나지만 순차블럭에 해당될때 또 해당 순차블록의 섹터에 데이터가 없을 때
            else if (offset != seq && LBN == SPBN && swmp[offset] == -1) {
                swmp[offset] = LSN;
                seq = 0;
                r_cnt++;
                w_cnt++;
                e_cnt++;
                seq_block_cnt = Sector_Cnt;
                //wear_level[LBN * Sector_Cnt + offset]++;
                for (int a = 0; a < Sector_Cnt; a++) { 
                    if (swmp[a] > -1) { Data_set[swmp[a]] = -1; }
                    swmp[a] = -1; 
                }
                std::cout << "switch operation" << "\n\n";
                switch_cnt++;
            }

            // 순차성에 어긋나지만 순차블럭에 해당될때 또 해당 순차블록의 섹터에 데이터가 있을 때
            else if (offset != seq && LBN == SPBN && swmp[offset] != -1) {
                if (Data_set[sec_num] == 0) {
                    std::cout << "idx(random_write) : " << idx << "\n\n";
                    Data_set[sec_num] = idx;
                    rwmp[idx] = LSN;
                    cnt++;
                    idx++;
                }
                seq = 0;
                r_cnt += (seq_block_cnt - 1);
                w_cnt += (seq_block_cnt - 1)+1;
                e_cnt++;
                seq_block_cnt = Sector_Cnt;
               // wear_level[LBN*Sector_Cnt + offset]++;
                for (int a = 0; a < Sector_Cnt; a++) { swmp[a] = -1; }
                std::cout << "switch operation" << "\n\n";
            }

            // Random 블럭에 해당될때
            else if (offset != seq && LBN != SPBN) {
                if (Data_set[sec_num] <= 0) { 
                    std::cout << "idx(random_write) : " << idx << "\n\n";
                    Data_set[sec_num] = idx; 
                    rwmp[idx] = LSN;
                    cnt++;
                    idx++;
                }
               

                // full merge
                if (cnt == (Log_Block_Cnt - 1) * Sector_Cnt) {
                    std::cout << "Full Merge start" << "\n";
                    for (int b = 0; b < idx; b++) {
                        if (rwmp[b] == -1) { continue; }
                        Data_set[rwmp[b]] = -1;
                        free_block_cnt--;
                        w_cnt++;
                        RPBN = rwmp[b] / Sector_Cnt;
                        for (int c = b + 1; c < idx; c++) {
                            if (RPBN == rwmp[c] / Sector_Cnt) {
                                Data_set[rwmp[c]] = -1;
                                free_block_cnt--;
                                rwmp[c] = -1;
                                w_cnt++;
                            }
                            if (free_block_cnt == 0) { break; }
                            if (Data_set[LBN * Sector_Cnt + offset] < -1) { free_block_cnt--; }
                        }
                        merge_cnt++;
                        rwmp[b] = -1;
                        r_cnt += free_block_cnt;
                        w_cnt += free_block_cnt;
                        e_cnt++;
                        free_block_cnt = Sector_Cnt;
                        //wear_level[rwmp[b]]++;
                    }
                    std::cout << "Full Merge finish" << "\n\n";
                    cnt = 0;
                    idx = 0;
                    continue;
                }
            }
        }
        else if (oper == 'r') { r_cnt++; }
    }
   /* std::cout << "\n" << "Flash_memory" << "\n";
    for (int l = 0; l < (Block_Cnt - Log_Block_Cnt)*Sector_Cnt; l++) {
        std::cout << Data_set[l] << " | ";
    }
    std::cout << "\n\n" << "sw" << "\n";
    for (int s = 0; s < Sector_Cnt; s++) {
        std::cout << swmp[s] << " | ";
    }
    std::cout <<"\n\n"<< "rw" << "\n";
    for (int r = 0; r < (Log_Block_Cnt-1) * Sector_Cnt; r++) {
        std::cout << rwmp[r] << " | ";
    }*/
    /*std::cout << "\n\n" << "wear_level" << "\n";
    for (int l = 0; l < Data_Block_Cnt * Sector_Cnt; l++) {
        std::cout << wear_level[l] << " | ";
    }*/

    std::cout << "\n" << "w_cnt : " << w_cnt << "\n";
    cout << "r_cnt : " << r_cnt << "\n";
    cout << "e_cnt : " << e_cnt << "\n";
    cout << "input_cnt : " << input_cnt << "\n";
    cout << "merge_cnt : " << merge_cnt << "\n";
    cout << "switch_cnt : " << switch_cnt << "\n";
    return 0;
}
