#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "gravity.h" // この中に構造体定義、関数プロトタイプがある

void my_plot_objects(Object objs[], const size_t numobj, const double t, const Condition cond);
void my_update_velocities(Object objs[], const size_t numobj, const Condition cond);
void my_update_positions(Object objs[], const size_t numobj, const Condition cond);
void my_fusion(Object objs[], const size_t numobj, const Condition cond);

int main(int argc, char **argv) {
    const Condition cond =
        {
            .width  = 75,
            .height = 40,
            .G = 1.0,
            .dt = 1.0
        };

    // ファイルの読み込み
    FILE *fp;
    fp = fopen(argv[2], "r");
    if (fp == NULL) {
        printf("File open error\n");
        exit(EXIT_FAILURE);
    }
    char line[256];

    size_t objnum = atoi(argv[1]);

    fgets(line, sizeof(line), fp);
    if (line[0] != '#') {
        fseek(fp, 0, SEEK_SET);
    }

    Object objects[objnum];
    double m, y, x, vy, vx;
    for (int i = 0; i < objnum; i++) {
        fscanf(fp, "%lf %lf %lf %lf %lf", &m, &x, &y, &vx, &vy);
        objects[i] = (Object){ .m = m, .y = y, .x = x, .vy = vy, .vx = vx};
        char c = 0;
        for (int j = 0; j < 50; j++) {
            c = fgetc(fp);
            if (c == '\n' || c == EOF) break;
        }
        if (fgetc(fp) == EOF) {
            for (int k = i+1; k < objnum; k++) {
                objects[k] = (Object){ .m = 0, .y = k*10, .x = k*10, .vy = 0, .vx = 0};
            }
            break;
        } else {
            fseek(fp, -1, SEEK_CUR);
        }
    }

    fclose(fp);
    
    // シミュレーション. ループは整数で回しつつ、実数時間も更新する
    const double stop_time = 400;
    double t = 0;
    for (size_t i = 0 ; t <= stop_time ; i++){
        t = i * cond.dt;
        my_update_velocities(objects, objnum, cond);
        my_update_positions(objects, objnum, cond);
        my_fusion(objects, objnum, cond);
        
        // 表示の座標系は width/2, height/2 のピクセル位置が原点となるようにする
        my_plot_objects(objects, objnum, t, cond);
        
        usleep(500 * 1000); // 200 x 1000us = 200 ms ずつ停止
        printf("\e[%dA", cond.height+4);// 表示位置を巻き戻す。壁がないのでheight+2行（境界とパラメータ表示分）
    }
    return EXIT_SUCCESS;
}

// 実習: 以下に my_ で始まる関数を実装する
// 最終的には physics.h 内の、こちらが用意したプロトタイプをコメントアウト
void my_plot_objects(Object objs[], const size_t numobj, const double t, const Condition cond){
    printf("\n");
    int arr[numobj][2];
    for (int i = 0; i < numobj; i++) {
        arr[i][0] = (int)objs[i].y;
        arr[i][1] = (int)objs[i].x;
    }

    for(int y = 0; y < cond.height; y++){
        for (int x = 0; x < cond.width; x++) {
            for (int i = 0; i < numobj; i++) {
                if (y == cond.height/2 + arr[i][0] - 1 && x == cond.width/2 + arr[i][1]) {
                    printf("o");
                    break;
                }
            }
            printf(" ");
        }
        printf("\n");
    }

    printf("-----\n");
    printf("t = %f\n", t);

    printf("x座標 ");
    for (int i = 0; i < numobj; i++) {
        printf("%d: %f ", i, objs[i].x);
    }
    printf("\n");
    printf("y座標 ");
    for (int i = 0; i < numobj; i++) {
        printf("%d: %f ", i, objs[i].y);
    }
}


void my_update_velocities(Object objs[], const size_t numobj, const Condition cond){
    double distance;
    for (int i = 0; i < numobj; i++) {
        double acc_y = 0;
        for (int j = 0; j < numobj; j++) {
            if (i == j) continue;
            distance = sqrt(pow(objs[i].x - objs[j].x, 2) + pow(objs[i].y - objs[j].y, 2));
            acc_y += cond.G * objs[j].m * (objs[j].y - objs[i].y) / pow(distance, 3);
        }
        objs[i].prev_vy = objs[i].vy;
        objs[i].vy += acc_y * cond.dt; 

        double acc_x = 0;
        for (int j = 0; j < numobj; j++) {
            if (i == j) continue;
            distance = sqrt(pow(objs[i].x - objs[j].x, 2) + pow(objs[i].y - objs[j].y, 2));
            acc_x += cond.G * objs[j].m * (objs[j].x - objs[i].x) / pow(distance, 3);
        }
        objs[i].prev_vx = objs[i].vx;
        objs[i].vx += acc_x * cond.dt; 
    }
}

void my_update_positions(Object objs[], const size_t numobj, const Condition cond){
    for (int i = 0; i < numobj; i++) {
        objs[i].y += objs[i].prev_vy  * cond.dt;
        objs[i].x += objs[i].prev_vx  * cond.dt;
    } 
}

void my_fusion(Object objs[], const size_t numobj, const Condition cond){
    double distance;
    for (int i = 0; i < numobj; i++) {
        for (int j = 0; j < numobj; j++) {
            if (i == j) continue;
            distance = sqrt(pow(objs[i].x - objs[j].x, 2) + pow(objs[i].y - objs[j].y, 2));
            if (distance < 2) {
                objs[i].m += objs[j].m;
                objs[i].vx = (objs[i].m * objs[i].vx + objs[j].m * objs[j].vx) / (objs[i].m + objs[j].m);
                objs[i].vy = (objs[i].m * objs[i].vy + objs[j].m * objs[j].vy) / (objs[i].m + objs[j].m);
                objs[j].m = 0;
                objs[j].x = 100000000000;
                objs[j].y = 100000000000;
            }
        }
    }
}
