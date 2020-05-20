#include "types.h"
#include "stat.h"
#include "user.h"
#define SIG_TEST 5
#define SIG_TEST2 2
volatile int got_signal = 0;

void
user_handle_func(int signum){
  got_signal += 1;
}

int
test1(void) {

    struct sigaction sigaction1 = {
      .sa_handler = user_handle_func,
      .sigmask = 0
    };

    struct sigaction sigaction2 = {
      .sa_handler = SIG_DFL,
      .sigmask = 0
    };

    struct sigaction old_sigaction;

    int success = 1;
    printf(1, "Starting test 1!\n");
    if( sigaction( SIG_TEST, &sigaction1, 0 ) < 0){
        success = 0;
    }
    if( sigaction( SIG_TEST, &sigaction2, &old_sigaction ) < 0){
        success = 0;
    }

    if (!(old_sigaction.sa_handler ==  sigaction1.sa_handler && old_sigaction.sigmask ==  sigaction1.sigmask)) {
        success = 0;
    }
    return success;
}

void
wait_loop(int wait_amount) {
    for (int i = 0; i < wait_amount; i++) {
        printf(1,"");
    }
}

void
print(char * text) {
    printf(1,text);
    printf(1,"\n");
}

void
print_failed(int test_num) {
    printf(1,"Test %d failed\n", test_num);
}

void
test2(void) {
    struct sigaction sigaction_user = {
      .sa_handler = user_handle_func,
      .sigmask = 0
    };

    struct sigaction sigaction_ignore = {
      .sa_handler = (void *)SIG_IGN,
      .sigmask = 0
    };

    got_signal = 0;
    printf(1, "Starting test 2!\n");


    int pid = fork();
    // Child
    if (pid < 0) {
        print_failed(2);
    }
    else if (pid == 0) {
        if( sigaction( SIG_TEST2, &sigaction_user, 0 ) < 0) {
            print_failed(2);
        }

        int timeout = 10000;
        while (got_signal == 0){
            wait_loop(500);
            timeout--;
            if (timeout == 0) {
                print_failed(2);
            }
        }
        
        if( sigaction( SIG_TEST2, &sigaction_ignore, 0 ) < 0) {
            print_failed(2);
        }
        
        wait_loop(5000);
        
        if (got_signal == 1) {
            printf(1,"Test 2 passed\n");
        } else {
            print_failed(2);
        }
        exit();
    }
    // Father
    else {        
        sleep(1);
        kill(pid, SIG_TEST2);
        wait_loop(1000);
        kill(pid, SIG_TEST2);
        wait();
    }
}

void
test3(void) {
    struct sigaction sigaction1 = {
      .sa_handler = user_handle_func,
      .sigmask = 0
    };

    got_signal = 0;
    printf(1, "Starting test 3!\n");
    sigprocmask(1<<SIG_TEST);
    // Send signal
    if( sigaction( SIG_TEST, &sigaction1, 0 ) < 0) {
        exit();
    }
    int pid = fork();
    // Child
    if (pid == 0) {
        wait_loop(100);
        // Signal is masked - got signal should be 0.
        if (got_signal != 0) {
            print_failed(3);
        } else {
            printf(1,"Test 3 passsed\n");
        }
        exit();
    }
    // Father
    else {
        wait_loop(1000);
        kill(pid, SIG_TEST);
        wait();
    }
}

void
test4(void) {

    int pid =  fork();
    if (pid < 0) {
        print_failed(4);
    }
    // Child
    else if (pid == 0) {
        while (1) {
            printf(1, "Child is running\n");
        }
    }
    // Father
    else {
        wait_loop(1000);
        kill(pid,SIGSTOP);
        printf(1, "Sent sig stop\n");
        wait_loop(20000);
        kill(pid,SIGCONT);
        wait_loop(2000);
        kill(pid,SIGKILL);
        wait();
        printf(1, "Test 4 passed\n");
    }
}

void
test5(void) {
    int pid1 = fork();
    if (pid1<0) {
        print_failed(5);

    // Child 1
    } else if (pid1 == 0){
        int sum = 0;
        for (int i = 0; i < 100000; i++) {
            sum +=sum;
        }
        exit();
    }
    // Father
    else {
        int pid2 = fork();
        if (pid2 < 0) {
            print_failed(5);
        }
        // Child 2
        else if (pid2 == 0){
            int sum = 0;
            for (int i = 0; i < 100000; i++) {
                sum +=sum;
            }
            exit();
        }
        // Father
        else {
            sleep(10);
            int p1 = wait();
            int p2 = wait();

            if ((pid1 != p1 && pid1 != p2) || (pid2 != p1 && pid2 != p2)) {
                print_failed(5);
            }

            printf(1, "Test 5 passed\n");
        }
    }
}

int
main(int argc, char *argv[])
{
    
    // struct sigaction sigaction1 = {
    //   .sa_handler = user_handle_func,
    //   .sigmask = 0
    // };

    //struct sigaction old_sigaction;

    // Test 1 //
    if (test1() == 0) {
        printf(1, "Test 1 failed!\n");
    } else {
        printf(1, "Test 1 passed!\n");
    }
    // end of test 1 //

    test2();

    test3();

    test4();

    test5();

    exit();
}
