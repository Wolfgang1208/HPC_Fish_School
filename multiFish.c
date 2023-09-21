#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <omp.h>
#include <math.h>

// this is about 10^7 fishes
// #define MAX_FISH_10_7 10000000
#define MAX_FISH 10000000
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
    float lastDelta;
};

void fishInit(struct fish *fishes, int fishNum)
{
    // fish parameters should have one decimal fraction
#pragma omp parallel
#pragma omp for schedule(dynamic, 100000)
    for (int i = 0; i < fishNum; i++)
    {
        fishes[i].x = (rand() % (MAX_FISH_X * 10 - MIN_FISH_X * 10 + 1) + MIN_FISH_X * 10) / 10.0;
        fishes[i].y = (rand() % (MAX_FISH_Y * 10 - MIN_FISH_Y * 10 + 1) + MIN_FISH_Y * 10) / 10.0;
        fishes[i].w = (rand() % (MAX_FISH_INIT_WEIGHT * 10 - MIN_FISH_WEIGHT * 10 + 1) + MIN_FISH_WEIGHT * 10) / 10.0;
        fishes[i].max_w = fishes[i].w * 2;
        fishes[i].lastDelta = 0.0;
    }
}

void fishesSwim(struct fish *fishes, int fishNum)
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

void fishesEat(struct fish *fishes, int fishNum)
{
    // table based optimization
    float *fishesDistance = (float *)malloc(sizeof(float) * fishNum);
    float *fishesWeight = (float *)malloc(sizeof(float) * fishNum);
    float *fishesDelta = (float *)malloc(sizeof(float) * fishNum);

// calculate fishesDistance
#pragma omp parallel
#pragma omp for schedule(dynamic, 100000)
    for (int i = 0; i < fishNum; i++)
    {
        fishesDistance[i] = sqrt(pow(fishes[i].x, 2) + pow(fishes[i].y, 2));
    }

// calculate fishesWeight
#pragma omp parallel
#pragma omp for schedule(dynamic, 100000)
    for (int i = 0; i < fishNum; i++)
    {
        fishesWeight[i] = fishes[i].w;
    }

// calculate fishesDelta
#pragma omp parallel
#pragma omp for schedule(dynamic, 100000)
    for (int i = 0; i < fishNum; i++)
    {
        fishesDelta[i] = fishesDistance[i] - fishes[i].lastDelta;
    }

    // find maximum delta
    float maxDelta = 0.0;

    for (int i = 0; i < fishNum; i++)
    {
        if (fishesDelta[i] > maxDelta)
        {
            maxDelta = fishesDelta[i];
        }
    }

// calculate fishesWeight
#pragma omp parallel
#pragma omp for schedule(dynamic, 100000)
    for (int i = 0; i < fishNum; i++)
    {
        fishes[i].w = fishesWeight[i] + fishesDelta[i] / maxDelta;
    }
}

float bariUpdate(struct fish *fishes, int fishNum)
{
    float upper = 0.0;
    float lower = 0.0;
    for (int i = 0; i < fishNum; i++)
    {
        upper += sqrt(pow(fishes[i].x, 2) + pow(fishes[i].y, 2)) * fishes[i].w;
        lower += sqrt(pow(fishes[i].x, 2) + pow(fishes[i].y, 2));
    }
    return upper / lower;
}

void main()
{
    omp_set_num_threads(MAX_THREAD);

    int fishNum = MAX_FISH;

    struct fish *fishes;

    fishes = (struct fish *)malloc(sizeof(struct fish) * fishNum);

    fishInit(fishes, fishNum);

    float Bari[GEN_NUM];

    // Swim for 10 times
    for (int i = 0; i < GEN_NUM; i++)
    {
        fishesSwim(fishes, fishNum);
        fishesEat(fishes, fishNum);
        Bari[i] = bariUpdate(fishes, fishNum);
        printf("Bari %i: %.5f\n", i, Bari[i]);
    }
}