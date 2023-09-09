#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <omp.h>
#include <math.h>

// #define MAX_FISH_10_7 10000000 // this is about 10^7 fishes
#define MAX_FISH 1000
#define MAX_FISH_INIT_WEIGHT 10
#define MAX_FISH_X 100
#define MAX_FISH_Y 100
#define MIN_FISH_WEIGHT 1
#define MIN_FISH_X -100
#define MIN_FISH_Y -100

#define GEN_NUM 10

#define MAX_THREAD 20

struct fish
{
    float x;
    float y;
    float w;
    float max_w;
    float lastDistance;
    float maxDelta;
};

void fishInit(struct fish *fishes, int fishNum)
{
#pragma omp parallel
#pragma omp for schedule(dynamic, 100000)
    // fish parameters should have one decimal fraction
    for (int i = 0; i < fishNum; i++)
    {
        fishes[i].x = (rand() % (MAX_FISH_X * 10 - MIN_FISH_X * 10 + 1) + MIN_FISH_X * 10) / 10.0;
        fishes[i].y = (rand() % (MAX_FISH_Y * 10 - MIN_FISH_Y * 10 + 1) + MIN_FISH_Y * 10) / 10.0;
        fishes[i].w = (rand() % (MAX_FISH_INIT_WEIGHT * 10 - MIN_FISH_WEIGHT * 10 + 1) + MIN_FISH_WEIGHT * 10) / 10.0;
        fishes[i].max_w = fishes[i].w * 2;
        fishes[i].lastDistance = 0.0;
        fishes[i].maxDelta = 0.0;
    }
}

void fishSwim(struct fish *fishes, int fishNum)
{
#pragma omp parallel
#pragma omp for schedule(dynamic, 100000)
    for (int i = 0; i < fishNum; i++)
    {
        float curFishX = fishes[i].x;
        float curFishY = fishes[i].y;
        // get a value between -0.1 and 0.1 for x changes
        float xChange = (rand() % 21 - 10) / 100.0;

        // update fish x
        if (curFishX + xChange > MAX_FISH_X)
        {
            fishes[i].x = MAX_FISH_X;
        }
        else if (curFishX + xChange < MIN_FISH_X)
        {
            fishes[i].x = MIN_FISH_X;
        }
        else
        {
            fishes[i].x += xChange;
        }

        // get a value between -0.1 and 0.1 for y changes
        float yChange = (rand() % 21 - 10) / 100.0;

        // update fish y
        if (curFishY + yChange > MAX_FISH_Y)
        {
            fishes[i].y = MAX_FISH_Y;
        }
        else if (curFishY + yChange < MIN_FISH_Y)
        {
            fishes[i].y = MIN_FISH_Y;
        }
        else
        {
            fishes[i].y += yChange;
        }
    }
}

void fishEat(struct fish *fishes, int fishNum)
{
    for (int i = 0; i < fishNum; i++)
    {
        float f = 0.0;
        for (int j = 0; j <= i; j++)
        {
            f+=sqrt(pow(fishes[j].x,2)+pow(fishes[j].y,2));
        }
    }
}

float bariUpdate(struct fish *fishes, int fishNum)
{
}

void main()
{
    omp_set_num_threads(MAX_THREAD);

    int fishNum = MAX_FISH;

    struct fish *fishes;

    fishes = (struct fish *)malloc(sizeof(struct fish) * fishNum);

    fishInit(fishes, fishNum);

    // for (int i = 0; i < fishNum; i++)
    // {
    //     printf("%.1f %.1f %.1f\n", fishes[i].x, fishes[i].y, fishes[i].w);
    // }

    float Bari[GEN_NUM];

    // Swim for 10 times
    for (int i = 0; i < GEN_NUM; i++)
    {
        fishSwim(fishes, fishNum);
        fishEat(fishes, fishNum);
        // printf("After swim:\n");
        // for (int i = 0; i < fishNum; i++)
        // {
        //     printf("Fish No.%d: %.1f %.1f %.1f\n", i, fishes[i].x, fishes[i].y, fishes[i].w);
        // }
        Bari[i] = bariUpdate(fishes, fishNum);
        printf("Bari: %.2f\n", Bari[i]);
    }
}