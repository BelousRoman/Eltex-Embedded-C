#include "../include/functions.h"

void first_task()
{
    printf("\nFirst task\n\n");

    int elem_count = FIRST_TASK_N * FIRST_TASK_N;
    int arr[elem_count];
    
    for (int i = 0; i < elem_count; i++) arr[i] = i+1;

    for (int j = 0; j < FIRST_TASK_N; j++)
    {
        for (int k = 0; k < FIRST_TASK_N; k++)
        {
            printf("%d ", arr[(j*FIRST_TASK_N)+k]);
        }
        printf("\n");
    }
}

void second_task()
{
    printf("\nSecond task\n\n");
    
    int buf;
    int arr[] = SECOND_TASK_ARR;

    for (int i = 0; i < SECOND_TASK_N; i++) printf("%d ", arr[i]);
    printf("\n");

    for (int i = 0; i < (int)(SECOND_TASK_N / 2); i++)
    {
        buf = arr[i];
        arr[i] = arr[(SECOND_TASK_N-1) - i];
        arr[(SECOND_TASK_N-1) - i] = buf;
    }

    for (int i = 0; i < SECOND_TASK_N; i++) printf("%d ", arr[i]);
    printf("\n");
}

void third_task_ltr()
{
    printf("\nThird task (left-to-right diagonal)\n\n");

    int arr[THIRD_TASK_N * THIRD_TASK_N];

    for (int j = 0; j < THIRD_TASK_N; j++)
    {
        for (int k = 0; k < THIRD_TASK_N; k++)
        {
            if (k > j) arr[(j*THIRD_TASK_N)+k] = 1;
            else arr[(j*THIRD_TASK_N)+(k*1)] = 0;
        }
    }

    for (int j = 0; j < THIRD_TASK_N; j++)
    {
        for (int k = 0; k < THIRD_TASK_N; k++)
        {
            printf("%d ", arr[(j*THIRD_TASK_N)+k]);
        }
        printf("\n");
    }
}

void third_task_rtl()
{
    printf("\nThird task (right-to-left diagonal)\n\n");

    int arr[THIRD_TASK_N * THIRD_TASK_N];
    int arr_marker = THIRD_TASK_N * THIRD_TASK_N;

    for (int i = 0; i < THIRD_TASK_N * THIRD_TASK_N; i++)
    {
        arr_marker = arr_marker - 1;
        if ((i % THIRD_TASK_N) >= ((arr_marker / THIRD_TASK_N)))
        {
            arr[i] = 1;
        }
        else 
            arr[i] = 0;
    }

    for (int j = 0; j < THIRD_TASK_N; j++)
    {
        for (int k = 0; k < THIRD_TASK_N; k++)
        {
            printf("%d ", arr[(j*THIRD_TASK_N)+k]);
        }
        printf("\n");
    }
}

void fourth_task()
{
    printf("\nFourth task\n\n");

    int arr[FOURTH_TASK_N * FOURTH_TASK_N];
    int cur_number = 1;
    int last_index = 0;
    int coef = 1;

    for (float j = (float) FOURTH_TASK_N; j > 0.5; j=j-0.5)
    {
        for (int k = 0; k < (int)j; k++)
        {
            arr[last_index + k * coef] = cur_number;
            cur_number++;
        }

        last_index = last_index + ((int)j-1) * coef;

        if (fmodf(j, 1.0f) == 0.0)
            coef = FOURTH_TASK_N;
        else
            coef = 1;

        if (((int)j % 2) != ((int)FOURTH_TASK_N % 2))
            coef = coef * -1;
        last_index += coef;
    }

    for (int j = 0; j < FOURTH_TASK_N; j++)
    {
        for (int k = 0; k < FOURTH_TASK_N; k++)
        {
            printf("%d\t", arr[(j*FOURTH_TASK_N)+k]);
        }
        printf("\n");
    }
}