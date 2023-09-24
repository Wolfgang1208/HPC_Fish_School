#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <omp.h>
#include <math.h>

// this is about 10^7 fishes
// #define MAX_FISH_10_7 10000000
#define MAX_FISH 4096
#define MAX_FISH_INIT_WEIGHT 10
#define MAX_FISH_X 100
#define MAX_FISH_Y 100
#define MIN_FISH_WEIGHT 1
#define MIN_FISH_X -100
#define MIN_FISH_Y -100

#define GEN_NUM 10

#define MAX_THREAD 16

struct fish
{
    float x;
    float y;
    float w;
    float max_w;
    int isSwam;
    float ED;
    float lastOBJ;
};

float currED[MAX_FISH];

float maxGlobalOBJ = -1.0;

int countFatFishes = 0;

void fishInit(struct fish *fishes, int fishNum)
{
// fish parameters should have one decimal fraction
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < fishNum; i++)
    {
        fishes[i].x = (rand() % (MAX_FISH_X * 10 - MIN_FISH_X * 10 + 1) + MIN_FISH_X * 10) / 10.0;
        fishes[i].y = (rand() % (MAX_FISH_Y * 10 - MIN_FISH_Y * 10 + 1) + MIN_FISH_Y * 10) / 10.0;
        fishes[i].w = (rand() % (MAX_FISH_INIT_WEIGHT * 10 - MIN_FISH_WEIGHT * 10 + 1) + MIN_FISH_WEIGHT * 10) / 10.0;
        fishes[i].max_w = fishes[i].w * 2;
        fishes[i].ED = 0.0;
        fishes[i].isSwam = 0;
        fishes[i].lastOBJ = 0.0;
        currED[i] = sqrt(pow(fishes[i].x, 2) + pow(fishes[i].y, 2));
    }
}

void fishEat(struct fish *fishes, int currFish, int fishNum)
{
// calculate curr fish's ED
#pragma omp parallel for schedule(dynamic)
#pragma omp reduction(+ : currED)
    for (int i = 0; i < fishNum; i++)
    {
        if (fishes[i].isSwam == 1)
        {
            currED[i] = sqrt(pow(fishes[i].x, 2) + pow(fishes[i].y, 2));
        }
    }

    // curr fish's obj
    float currOBJ = 0.0;
    for (int i = 0; i < fishNum; i++)
    {
        currOBJ += currED[i];
    }

    // curr fish's delta obj
    float deltaOBJ = currOBJ - fishes[currFish].lastOBJ;

    fishes[currFish].lastOBJ = currOBJ;

    // change global max obj if needed
    if (deltaOBJ >= maxGlobalOBJ)
    {
        maxGlobalOBJ = deltaOBJ;
    }

    // set new weight
    float newWeight = fishes[currFish].w + deltaOBJ / maxGlobalOBJ;
    if (newWeight > fishes[currFish].max_w)
    {
        fishes[currFish].w = fishes[currFish].max_w;
        countFatFishes += 1;
    }
    else if (newWeight < MIN_FISH_WEIGHT)
    {
        fishes[currFish].w = MIN_FISH_WEIGHT;
    }
    else
    {
        fishes[currFish].w = newWeight;
    }
}

void fishSwim(struct fish *fishes, int currFish, int fishNum)
{

    float currFishX = fishes[currFish].x;
    float currFishY = fishes[currFish].y;
    // get a value between -0.1 and 0.1 for x changes
    float xChange = (rand() % 21 - 10) / 100.0;

    if (fishes[currFish].w < fishes[currFish].max_w)
    {
        // update fish x
        if (currFishX + xChange > MAX_FISH_X)
        {
            fishes[currFish].x = MAX_FISH_X;
        }
        else if (currFishX + xChange < MIN_FISH_X)
        {
            fishes[currFish].x = MIN_FISH_X;
        }
        else
        {
            fishes[currFish].x += xChange;
        }

        // get a value between -0.1 and 0.1 for y changes
        float yChange = (rand() % 21 - 10) / 100.0;

        // update fish y
        if (currFishY + yChange > MAX_FISH_Y)
        {
            fishes[currFish].y = MAX_FISH_Y;
        }
        else if (currFishY + yChange < MIN_FISH_Y)
        {
            fishes[currFish].y = MIN_FISH_Y;
        }
        else
        {
            fishes[currFish].y += yChange;
        }
    }

    if (currFishX != fishes[currFish].x || currFishY != fishes[currFish].y)
    {
        fishes[currFish].isSwam = 1;
    }
    else
    {
        fishes[currFish].isSwam = 0;
    }
}

// void fishEat(struct fish *fishes, int fishNum)
// {
// #pragma omp parallel
// #pragma omp for schedule(dynamic, 100000)
//     for (int i = 0; i < fishNum; i++)
//     {
//         float f = 0.0;
// #pragma omp schedule(dynamic)
//         for (int j = 0; j <= i; j++)
//         {
//             f += sqrt(pow(fishes[j].x, 2) + pow(fishes[j].y, 2));
//         }
//         fishes[i].lastDelta = f - fishes[i].lastDelta;
//         // printf("Fish %d: %.5f\n", i, fishes[i].lastDelta);

//         float max_df = 0.0;
//         for (int m = 0; m < fishNum; m++)
//         {
//             if (fishes[m].lastDelta >= max_df)
//             {
//                 max_df = fishes[m].lastDelta;
//             }
//         }
//         printf("Fish %d max_df: %.5f\n", i, max_df);
//         printf("Fish %d: %.5f\n", i, (fishes[i].lastDelta) / (max_df));
//         fishes[i].w += (fishes[i].lastDelta) / (max_df);
//     }
// }

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

    float Bari[GEN_NUM];

    // init currED
    for (int i = 0; i < fishNum; i++)
    {
        currED[i] = 0.0;
    }

    fishInit(fishes, fishNum);

    // Iterate for GEN_NUM times
    for (int i = 0; i < GEN_NUM; i++)
    {
        double start = omp_get_wtime();
        // reset status all of the fishes
#pragma parallel for schedule(dynamic)
        for (int j = 0; j < fishNum; j++)
        {
            fishes[j].isSwam = 0;
        }

        // parallel for each chrunk of fishes
#pragma parallel
#pragma for schedule(dynamic)
#pragma reduction(+ : currED, maxGlobalOBJ, countFatFishes)
        for (int currFish = 0; currFish < MAX_FISH; currFish++)
        {
            fishEat(fishes, currFish, fishNum);
            fishSwim(fishes, currFish, fishNum);
        }
#pragma parallel barrier

        printf("There are %d fat fishes.\n", countFatFishes);

        countFatFishes = 0;

        double end = omp_get_wtime();
        printf("Time: %f seconds. \n", end - start);
        Bari[i] = bariUpdate(fishes, fishNum);
        printf("Bari: %.5f\n", Bari[i]);
    }
}