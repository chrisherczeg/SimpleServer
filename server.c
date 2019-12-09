#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>

#define NUM_THREADS (10)
#define PROCESS_POOL (3)

void run(int slave_socket);
void *run_proxy(void *slave_socket);
void *run_pool_proxy (void *master_socket);

pthread_mutex_t g_mutex = { 0 };

int accept_helper(int master_socket) {
    struct sockaddr_in addr = {0};
    socklen_t length = sizeof(addr);

    int slave_socket = accept(master_socket, (struct sockaddr *) &addr, &length);

    char inet_pres[INET_ADDRSTRLEN];
    inet_ntop(addr.sin_family, &(addr.sin_addr) , inet_pres, INET_ADDRSTRLEN);

    return slave_socket;
}
void run_linear_server(int master_socket) {
    while (1) {
        int slave_socket = accept_helper(master_socket);
        run(slave_socket);
    }
}
void run_forking_server(int master_socket) {
    while (1) {
        int slave_socket = accept_helper(master_socket);

        int pid = fork();
        if (pid == 0) {
            run(slave_socket);
            exit(0);
        }
        close(slave_socket);
    }
}

void run_threaded_server(int master_socket) {
    while(1) {
        int slave_socket = accept_helper(master_socket);
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_t thread_id;
        pthread_create(&thread_id, &attr, run_proxy, (void *)(&slave_socket));
    }
}

void run_thread_pool_server (int master_socket) {
    pthread_t thread_ids[NUM_THREADS];
    pthread_mutex_init(&g_mutex, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&thread_ids[i], NULL, run_pool_proxy, (void *)(&master_socket));
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    run_pool_proxy((void *)(&master_socket));
    pthread_mutex_destroy(&g_mutex);
}

void run_process_pool(int master_socket) {
    for (int i = 0; i < PROCESS_POOL; i++) {
        int slave_socket = accept_helper(master_socket);
        int pid = fork();
        if (pid == 0) {
            run(slave_socket);
            exit(0);
        }
        close(slave_socket);
    }
    wait(NULL);
}

void *run_pool_proxy (void *master_socket) {

    int *master = (int *)master_socket;

    while (1) {
        pthread_mutex_lock(&g_mutex);
        int slave_socket = accept_helper(*master);
        pthread_mutex_unlock(&g_mutex);
        run(slave_socket);
    }
    return NULL;
}
void *run_proxy(void *slave_socket) {
    int *slave = (int *)slave_socket;
    run(*slave);
    return NULL;
}
void run(int slave_socket) {
    sleep(5);
    send(slave_socket, "Hello from Simple Server\n", sizeof("Hello from Simple Server\n"), 0);
    close(slave_socket);
}