#include <iostream>

using namespace std;

#define Block_Num 32
#define Sector_Num 32
#define Log_Block_Ratio 0.5
#define Data_Block_Num (int)((1 - Log_Block_Ratio) * Block_Num)

int main() {
    int Log_Block_Num = Block_Num - Data_Block_Num;
    if (Log_Block_Num == 1 || Log_Block_Num == 0) { Log_Block_Num = 2; }
    std::cout << "Log_Block_Num : " << Log_Block_Num << "\n";
    int Data_set[Data_Block_Num * Sector_Num] = {};
    int wear_level[Data_Block_Num * Sector_Num] = {};

    char oper;
    int sec_num = -1;
    int idx = 0;
    int w_cnt = 0;
    int r_cnt = 0;
    int e_cnt = 0;
    int swmp[Sector_Num] = {};
    int* rwmp = new int[(Log_Block_Num - 1) * Sector_Num];
    int seq = 0;
    int cnt = 0;
    int SPBN = -1;
    int RPBN = -1;
    int free_block_cnt = Sector_Num;
    int seq_block_cnt = Sector_Num;

    fill_n(Data_set, Data_Block_Num * Sector_Num, -2);
    fill_n(swmp, Sector_Num, -1);
    fill_n(rwmp, (Log_Block_Num - 1) * Sector_Num, -1);

    while (true) {
        // Data Set input
        std::cout << "Data_Set 입력(나가기 -> x): "; cin >> oper >> sec_num;

        // Exit
        if (oper == 'x') { break; }

        // Flash_Memory
        else if (Data_set[sec_num] < 0) { Data_set[sec_num]++; w_cnt++; }
        
        // Overwrite
        if (Data_set[sec_num] == 0 && oper == 'w') {
            r_cnt++;
            int LSN = sec_num;
            int offset = LSN % Sector_Num;
            int LBN = LSN / Sector_Num;
            

            // 순차 시작
            if (offset == seq) {
                if (seq == 0) {
                    std::cout << "Seqeuence start" << "\n\n";
                    seq++;
                    w_cnt++;
                    swmp[offset] = LSN;
                    SPBN = swmp[0] / Sector_Num;
                    seq_block_cnt--;
                    continue;
                }
                else {
                    SPBN = swmp[0] / Sector_Num;

                    // random write
                    if (LBN != SPBN) { 
                        Data_set[sec_num] = idx; std::cout << "idx : " << idx << "\n";
                        //w_cnt++;
                        rwmp[Data_set[LSN]] = LSN;
                        cnt++;

                        // full merge
                        if (cnt == (Log_Block_Num - 1) * Sector_Num) {
                            std::cout << "Full Merge start" << "\n\n";
                            for (int b = 0; b < idx; b++) {
                                if (rwmp[b] == -1) { continue; }
                                Data_set[rwmp[b]] = -1;
                                free_block_cnt--;
                                w_cnt++;
                                RPBN = rwmp[b] / Sector_Num;
                                for (int c = b + 1; c < idx; c++) {
                                    if (RPBN == rwmp[c] / Sector_Num) {
                                        Data_set[rwmp[c]] = -1;
                                        free_block_cnt--;
                                        rwmp[c] = -1;
                                        r_cnt++;
                                        w_cnt++;
                                    }
                                    else if (free_block_cnt == 0) { break; }
                                    else if (Data_set[LBN * Sector_Num + offset] < -1){ free_block_cnt--; }
                                }
                                
                                rwmp[b] = -1;
                                r_cnt += free_block_cnt;
                                w_cnt += free_block_cnt;
                                e_cnt++;
                                wear_level[rwmp[b]]++;
                            }
                            cnt = 0;
                            idx = 0;
                            std::cout << "Merge finish" << "\n\n";
                            continue;
                        }
                        continue;
                    }
                    else {
                       // w_cnt++;
                        seq_block_cnt--;
                        swmp[offset] = LSN;
                        seq++;
                        std::cout << "seq :" << seq << "\n";

                        // Seq_write block이 꽉찼을때
                        if (seq == Sector_Num) {
                            std::cout << "Full seq_block" << "\n";
                            seq = 0;
                            e_cnt++;
                            wear_level[LBN * Sector_Num + offset]++;
                            for (int a = 0; a < Sector_Num; a++) { swmp[a] = -1; }
                            std::cout << "switch block" << "\n\n";
                        }
                        continue;
                    }
                }
            }

            // 순차성에 어긋나지만 순차블럭에 해당될때
            else if (offset != seq && LBN == SPBN && swmp[offset] == -1) {
                Data_set[LBN * Sector_Num + offset] = -1;
                w_cnt++;
                e_cnt++;
                wear_level[LBN * Sector_Num + offset]++;
                for (int a = 0; a < Sector_Num; a++) { 
                    if (swmp[a] > -1) { Data_set[swmp[a]] = -1; }
                    swmp[a] = -1; 
                }
                std::cout << "switch operation" << "\n\n";
            }

            // 순차성에 어긋나지만 순차블럭에 해당될때
            else if (offset != seq && LBN == SPBN && swmp[offset] != -1) {
                std::cout << "switch seq_block" << "\n\n";
                seq = 0;
                w_cnt += (seq_block_cnt - 1);
                r_cnt += (seq_block_cnt - 1);
                e_cnt++;
                wear_level[LBN*Sector_Num + offset]++;
                for (int a = 0; a < Sector_Num; a++) { swmp[a] = -1; }
                std::cout << "switch block" << "\n\n";
            }

            // Random 블럭에 해당될때
            else if (offset != seq && LBN != SPBN) {
                if (Data_set[sec_num] <= 0) { Data_set[sec_num] = idx; idx++; }
                std::cout << "idx(random_write) : " << idx-1 << "\n\n";
                //w_cnt++;
                rwmp[idx-1] = LSN;
                cnt++;

                // full merge
                if (cnt == (Log_Block_Num - 1) * Sector_Num) {
                    std::cout << "Full Merge start" << "\n";
                    for (int b = 0; b < idx; b++) {
                        if (rwmp[b] == -1) { continue; }
                        Data_set[rwmp[b]] = -1;
                        free_block_cnt--;
                        w_cnt++;
                        RPBN = rwmp[b] / Sector_Num;
                        for (int c = b + 1; c < idx; c++) {
                            if (RPBN == rwmp[c] / Sector_Num) {
                                Data_set[rwmp[c]] = -1;
                                free_block_cnt--;
                                rwmp[c] = -1;
                                r_cnt++;
                                w_cnt++;
                            }
                            if (free_block_cnt == 0) { break; }
                            if (Data_set[LBN * Sector_Num + offset] < -1) { free_block_cnt--; }
                        }
                        rwmp[b] = -1;
                        r_cnt += free_block_cnt;
                        w_cnt += free_block_cnt;
                        e_cnt++;
                        wear_level[rwmp[b]]++;
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
    std::cout << "\n" << "Flash_memory" << "\n";
    for (int l = 0; l < Data_Block_Num * Sector_Num; l++) {
        std::cout << Data_set[l] << " | ";
    }
    std::cout << "\n\n" << "sw" << "\n";
    for (int l = 0; l < Sector_Num; l++) {
        std::cout << swmp[l] << " | ";
    }
    std::cout <<"\n\n"<< "rw" << "\n";
    for (int l = 0; l < (Log_Block_Num-1) * Sector_Num; l++) {
        std::cout << rwmp[l] << " | ";
    }
    std::cout << "\n\n" << "wear_level" << "\n";
    for (int l = 0; l < Data_Block_Num * Sector_Num; l++) {
        std::cout << wear_level[l] << " | ";
    }

    std::cout << "\n" << "w_cnt : " << w_cnt << "\n" << "r_cnt : " << r_cnt << "\n" << "e_cnt : " << e_cnt;
    return 0;
}