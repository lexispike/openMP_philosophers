/*
 * Alyxandra Spikerman
 * High Perfomance Computing
 * Homework 3 - Question 3
 *
 */

#include <iostream>
#include <omp.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdint>

// global variables
int PHILOSOPHERS;
omp_lock_t *locks;
omp_lock_t print_lock;
int *p_eaten;   // keep track of which philosophers have eaten
int *p_thought; // keep track of which philosophers have thought

using namespace std;

/*
 * get_left_fork:
 * - left fork = thread_num - 1
 *
 * @p_num: thread number (philosopher number)
 *
 */
int get_left_fork(int p_num) {
    int left;
    if (p_num == 0) {
        left = PHILOSOPHERS - 1; // the first philosopher's left fork is the last fork
    } else {
        left = p_num - 1;
    }
    return left;
}

/*
 * get_right_fork:
 * - right fork = current thread num
 *
 * @p_num: thread number (philosopher number)
 *
 */
int get_right_fork(int p_num) {
    return p_num;
}

/*
 * all_philosophers_done:
 * - checks if all the philosophers have both eaten and thought
 *
 */
bool all_philosophers_done() {
    // initialize have_eaten and have_thought to true, set them later to false if one philosopher has not done it
    bool have_eaten = true;
    bool have_thought = true;
    int eaten;
    int thought;
    for (int i = 0; i < PHILOSOPHERS; i++) {
        #pragma omp atomic read
        eaten = p_eaten[i];
        if (!eaten) {
            have_eaten = false;
        }
        #pragma omp atomic read
        thought = p_thought[i];
        if (!thought) {
            have_thought = false;
        }
    }
    return have_eaten && have_thought;
}

/*
 * print_state:
 * - prints the state of the current philosopher by waiting for the the print lock
 *
 * @thread_num: integer value for thread/philosopher number
 * @state: state of the philosopher, either 'thinking' or 'eating'
 * @action: action of the philosopher, either 'has' or 'waiting for'
 *
 */
void print_state(int thread_num, string state, string action) {
    // if (!all_philosophers_done()) { // check if all the philosophers are done or not, or else we output bad data
        omp_set_lock(&print_lock);
        cout << "Philosopher " << thread_num << " " << state << endl;
        cout << "- " << action << " left fork: " << get_left_fork(thread_num) << endl;
        cout << "- " << action << " right fork: " << get_right_fork(thread_num) << endl << endl;
        omp_unset_lock(&print_lock);
    // }
}

/*
 * let_them_eat:
 * - tries to get a left and right fork to eat otherwise philosopher thinks
 * - the philosopher can only have either 2 forks in hand or no forks in hand
 *   (they do not hold a fork if they can't get the second one)
 *
 */
void let_them_eat() {
    int thread_num = omp_get_thread_num();

    while (!all_philosophers_done()) {
        // try to get the left and right fork to eat, else think
        if (omp_test_lock(&locks[get_left_fork(thread_num)]) != 0) {
            if (omp_test_lock(&locks[get_right_fork(thread_num)]) != 0) {
                print_state(thread_num, "eating", "has");
                p_eaten[thread_num] = 1;

                sleep(1); // simulate some eating time
                omp_unset_lock(&locks[get_right_fork(thread_num)]);
            } else {
                print_state(thread_num, "thinking", "waiting for");
                p_thought[thread_num] = 1;
            }
            omp_unset_lock(&locks[get_left_fork(thread_num)]);
        } else {
            print_state(thread_num, "thinking", "waiting for");
            p_thought[thread_num] = 1;
        }
        sleep(1); // let the thread pause so all the threads aren't pushing for the same locks at the same moment
    }
}

int main(int argc, char *argv[]) {
    // get the command line arguments and error out if there aren't enough
    if (argc == 2) {
        PHILOSOPHERS = atoi(argv[1]);
    } else {
        cout << "Error: must input 1 argument, <number of philosophers>" << endl;
        return 1;
    }

    // initialize global arrays
    locks = new omp_lock_t[PHILOSOPHERS];
    p_eaten = new int[PHILOSOPHERS];
    p_thought = new int[PHILOSOPHERS];

    // initialize locks and values for p_eaten and p_thought
    for (int i = 0; i < PHILOSOPHERS; i++) {
        omp_init_lock(&locks[i]);
        p_eaten[i] = 0;
        p_thought[i] = 0;
    }
    omp_init_lock(&print_lock); // initialize print lock

    // run let them eat with num_threads = PHILOSOPHERS
    #pragma omp parallel for num_threads(PHILOSOPHERS)
    for (int i = 0; i < PHILOSOPHERS; i++) {
        let_them_eat();
    }

    // destroy locks
    for (int i = 0; i < PHILOSOPHERS; i++) {
        omp_destroy_lock(&locks[i]);
    }
    omp_destroy_lock(&print_lock);

    return 0;
}
