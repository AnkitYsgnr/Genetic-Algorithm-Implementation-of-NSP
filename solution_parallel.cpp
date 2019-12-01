#include <iostream>
#include <algorithm>
#include <vector>
#include <time.h>
#include <omp.h>
#include <semaphore.h>
using namespace std;

const int no_of_days = 7;
const int no_of_nurses = 10;
const int population_size = 100;
const int penalty[] = {250, 200, 50, 150, 100, 300};
const int no_of_nurses_class_A = 5;

/*
    M : Morning Shift (8:00 am to 2:00 pm)
    E : Evening Shift (2:00 pm to 8:00 pm)
    N : Night Shift (8:00 pm to 8:00 am)
    H : Holiday

    Classes of Nurses:
    A : Most Skilled (4 Nurses)
    B : Intermediate Skill Level (3 Nurses)
    C : Least Skilled (3 Nurses)

    Constraints :
    1. There must be exactly one holiday in a week for every nurse (Penalty: 250)
    2. Certain shift-patterns are not allowed   (Penalty: 200)
        i. Morning shift after night shift
        ii. Evening shift after night shift
    3. There must be at least one and at most two night shifts in a week for every nurse (Penalty: 50)
    4. There must be at least one nurse of class A in every morning and evening shift (Penalty: 150)
    5. There must be exactly one nurse of class A in night shift (Penalty: 100)
    6. There must be at least one nurse allocated to every shift in every day (Penalty: 300)
*/


/*
    Valid Shifts
*/
const char valid_duty[4] = {'M', 'E', 'N', 'H'};


/*
    randomly generates a shift
*/
char generate_duty() {
    int r = rand()%4;
    return valid_duty[r];
}

/*
    generates a random schedule
*/
void generate_random_schedule(char (&arr) [no_of_nurses][no_of_days]) {
    for (int i=0; i<no_of_nurses; i++)
        for (int j=0; j<no_of_days; j++)
            arr[i][j] = generate_duty();
}


/*
    class to represent schedule and its associated functionalites
*/
class schedule {
    public:
        char arr[no_of_nurses][no_of_days];
        int fitness;
        schedule(char (&a)[no_of_nurses][no_of_days]);
        int calculate_fitness();
        schedule mate(schedule &parent2);
};


/*
    Creates object of the schedule from a given 2-D array
*/
schedule :: schedule(char (&a) [no_of_nurses][no_of_days])
{
    for (int i=0; i<no_of_nurses; i++)
        for (int j=0; j<no_of_days; j++)
            arr[i][j] = a[i][j];
    fitness = calculate_fitness();
}

/*
    Calculates fitness score for the corresponding schedule
*/
int schedule :: calculate_fitness()
{
    int fitness = 0;


    for (int i=0; i<no_of_nurses; i++) {

        int H = 0;                                                                          // Stores number of holidays for the ith nurse in a week
        int N = 0;                                                                          // Stores number of night shifts for the ith nurse in a week
        int false_patterns = 0;                                                             // Stores number of forbidden pattern in a week

        for (int j=0; j<no_of_days; j++) {
            if (arr[i][j] == 'H')
                H++;
            else if (arr[i][j] == 'N') {
                N++;
                if (arr[i][(j+1)%no_of_days] == 'M' || arr[i][(j+1)%no_of_days] == 'E')
                    false_patterns++;
            }
                
        }
        
        
        if (H!=1)
            fitness += abs(H-1)*penalty[0];
        
        fitness += false_patterns * penalty[1];

        if (N != 1 && N != 2) {
            if (N)
                fitness += (N-2)*penalty[2];
            else
                fitness += penalty[2];
        }

    }


    for (int j=0; j<no_of_days; j++) {
        int N = 0;                                                                               // Stores number of nurses having night shifts on jth day
        int M = 0;                                                                               // Stores number of nurses having morning shifts on jth day 
        int E = 0;                                                                               // Stores number of nurses having evening shifts on jth day
        int classA_in_M = 0;                                                                     // Stores number of nurses of class A having morning shifts on jth day
        int classA_in_E = 0;                                                                     // Stores number of nurses of class A having evening shifts on jth day
        int classA_in_N = 0;                                                                     // Stores number of nurses of class A having night shifts on jth day
        for (int i = 0; i<no_of_nurses; i++) {
           
            if (arr[i][j] == 'N') {
                N++;
                if (i<no_of_nurses_class_A)
                    classA_in_N++;
                
            } else if (arr[i][j] == 'M') {
                M++;
                if (i<no_of_nurses_class_A)
                    classA_in_M++;
            } else if (arr[i][j] == 'E'){
                E++;
                if (i<no_of_nurses_class_A)
                    classA_in_E++;
            }
        }
        if (M == 0)
            fitness += penalty[5];
        if (E == 0)
            fitness += penalty[5];
        if (N == 0)
            fitness += penalty[5];
        if (classA_in_M == 0)
            fitness += penalty[3];
        if (classA_in_E == 0)
            fitness += penalty[3];
        if (classA_in_N != 1)
            fitness += abs(classA_in_N-1)*penalty[4];
        
    }

    return fitness;
}


/*
    Generates child schedule from tow parent schedules
*/
schedule schedule :: mate(schedule &parent2)
{
    char child[no_of_nurses][no_of_days];
    for (int i=0; i<no_of_nurses; i++) {
        for (int j=0; j<no_of_days; j++) {
            int r = rand()%100;
            if (r < 45)
                child[i][j] = arr[i][j];
            else if (r < 90)
                child[i][j] = parent2.arr[i][j];
            else
                child[i][j] = generate_duty();
        }
    }
    return schedule(child);
}

/*
    Overloads '<' operator as it will be further used in sorting
*/
bool operator< (const schedule &S1, const schedule &S2) {
    return S1.fitness < S2.fitness;
}


int main()
{
    clock_t t1 = clock();
    
    srand((unsigned)(time(0)));

    //bool found = false;
    vector <schedule> population;
    sem_t mutex;
    sem_init(&mutex, 1, 1);

    #pragma omp parallel for
    for (int i=0; i<population_size; i++) {
        char a[no_of_nurses][no_of_days];
        
        generate_random_schedule(a);
        sem_wait(&mutex);
        population.push_back(schedule(a));
        sem_post(&mutex);
    }

    for (int k=0; k<1; k++) {
        // cout << "--------Generation- Parent---------------Schedule------------------" << endl;

        // for (int i=0; i<no_of_nurses; i++) {
        //     for (int j=0; j<no_of_days; j++) {
        //     cout << population[k].arr[i][j] << "\t";
        //     }
        //     cout << endl;
        // }
        // cout << endl << endl;
        cout << "Fitness Value: " << population[k].fitness << endl;
    }
    // for (int k=3; k<population_size; k++) {
    //     cout << "Fitness Value: " << population[k].fitness << endl;
    // }
    
    int generation = 0;

    while(true) {
        sort(population.begin(), population.end());

        if (population[0].fitness < 1)
            break;
        
        vector <schedule> new_generation;

        int s = 10*population_size/100;
        for (int i=0; i<s; i++)
            new_generation.push_back(population[i]);
        
        s = 90*population_size/100;
        
        #pragma omp parallel for
        for (int i=0; i<s; i++) {
            int r1 = rand()%(population_size/2);
            int r2 = rand()%(population_size/2);
            schedule S = population[r1].mate(population[r2]);
            sem_wait(&mutex);
            new_generation.push_back(S);
            sem_post(&mutex);
        }

        #pragma omp parallel for
        for (int i=0; i<population_size; i++) {
            for (int j=0; j<no_of_nurses; j++) {
                for (int k = 0; k<no_of_days; k++)
                    population[i] = new_generation[i];
            }
        }

        cout << "--------Generation-" << generation << "---------------Schedule------------------" << endl;

        // for (int i=0; i<no_of_nurses; i++) {
        //     for (int j=0; j<no_of_days; j++) {
        //         cout << population[0].arr[i][j] << "\t";
        //     }
        //     cout << endl;
        // }
        cout << "Fitness Value: " << population[0].fitness << endl;
        cout << endl << endl;

        generation++;
    }

    cout << "------------------Optimized Schedule--------------------" << endl;
    for (int i=0; i<no_of_nurses; i++) {
        for (int j=0; j<no_of_days; j++) {
            cout << population[0].arr[i][j] << "\t";
        }

        cout << endl;
    }
    cout << "Optimal Fitness : " << population[0].fitness << endl;
    clock_t t2 = clock();
    cout << "Time Taken : " << (t2-t1)/CLOCKS_PER_SEC << "s" << endl;
    return 0;
}