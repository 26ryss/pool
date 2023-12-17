#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "gravity.h" // この中に構造体定義、関数プロトタイプがある

void my_plot_objects(Object objs[], const size_t numobj, const double t, const Condition cond);
void my_update_velocities(Object objs[], const size_t numobj, const Condition cond);
void my_update_positions(Object objs[], const size_t numobj, const Condition cond);
void collide(Object objs[], const size_t numobj, const Condition cond, int i, int j);

int main(int argc, char **argv) {
    const Condition cond =
        {
            .width  = 75,
            .height = 40,
            .G = 1.0,
            .dt = 1.0
        };

    size_t objnum = 5;
    Object objects[objnum];
    objects[0] = (Object){ .m = 5, .y = -15, .x = 0, .vy = 3, .vx = 3};
    objects[1] = (Object){ .m = 5, .y = 20, .x = 20, .vy = 0, .vx = 0};
    objects[2] = (Object){ .m = 5, .y = -10, .x = -13, .vy = 0, .vx = 0};  
    objects[3] = (Object){ .m = 5, .y = 10, .x = -10, .vy = 0, .vx = 0};
    objects[4] = (Object){ .m = 5, .y = -10, .x = 17, .vy = 0, .vx = 0};
    
    // シミュレーション. ループは整数で回しつつ、実数時間も更新する
    const double stop_time = 400;
    double t = 0;
    for (size_t i = 0 ; t <= stop_time ; i++){
        t = i * cond.dt;
        my_update_velocities(objects, objnum, cond);
        my_update_positions(objects, objnum, cond);

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

    printf(" ");
    for (int i = 0; i < cond.width; i++) {
        printf("-");
    }
    printf("\n");
    int flag = 0;
    for (int y = 0; y < cond.height; y++){
        printf("|");
        for (int x = 0; x < cond.width; x++) {
            for (int i = 0; i < numobj; i++) {
                flag = 0;
                if (y == cond.height/2 + arr[i][0] - 1 && x == cond.width/2 + arr[i][1]) {
                    printf("o");
                    flag = 1;
                    break;
                }
            }
            if (flag == 0) printf(" ");
        }
        printf("|\n");
    }

    printf(" ");
    for (int i = 0; i < cond.width; i++) {
        printf("-");
    }
    printf("\n");
    printf("time: %f\n", t);
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

        if (objs[i].y == cond.height/2 || objs[i].y == -cond.height/2 + 1) {
            objs[i].prev_vy = -objs[i].vy * 0.5;
            objs[i].vy = -objs[i].vy * 0.5;
        }

        if (objs[i].x == cond.width/2 || objs[i].x == -cond.width/2) {
            objs[i].prev_vx = -objs[i].vx * 0.5;
            objs[i].vx = -objs[i].vx * 0.5;
        }
    }
}

void my_update_positions(Object objs[], const size_t numobj, const Condition cond){
    for (int i = 0; i < numobj; i++) {
        objs[i].y += objs[i].prev_vy  * cond.dt;
        objs[i].x += objs[i].prev_vx  * cond.dt;
        if (objs[i].y >= cond.height/2) objs[i].y = cond.height/2;
        if (objs[i].y <= -cond.height/2 + 1) objs[i].y = -cond.height/2 + 1;
        if (objs[i].x >= cond.width/2) objs[i].x = cond.width/2;
        if (objs[i].x <= -cond.width/2) objs[i].x = -cond.width/2;
    }
}

void collide(Object objs[], const size_t numobj, const Condition cond, int i, int j) {
    double m = objs[i].m + objs[j].m;
    double vy = (objs[i].m * objs[i].vy + objs[j].m * objs[j].vy) / m;
    double vx = (objs[i].m * objs[i].vx + objs[j].m * objs[j].vx) / m;
    double y = (objs[i].m * objs[i].y + objs[j].m * objs[j].y) / m;
    double x = (objs[i].m * objs[i].x + objs[j].m * objs[j].x) / m;
    objs[i] = (Object){ .m = m, .y = y, .x = x, .vy = vy, .vx = vx};
    objs[j] = (Object){ .m = 0, .y = 0, .x = 0, .vy = 0, .vx = 0};
}