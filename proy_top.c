#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>

#define MAX_PROCESSES 500
#define MAX_LINE 1024
#define MAX_COMM 256

typedef struct {
    int pid;
    char comm[MAX_COMM];
    char state;
    long utime;
    long stime;
    long vsize;
    long rss;
    double cpu_percent;
    double mem_percent;
} ProcessInfo;

typedef struct {
    long total_mem;
    long free_mem;
    long available_mem;
    long buffers;
    long cached;
} MemInfo;

// Variables globales
ProcessInfo processes[MAX_PROCESSES];
int process_count = 0;
int running = 1;

// Función para manejar señales
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
    }
}

// Leer información de memoria del sistema
void read_meminfo(MemInfo *mem) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Error abriendo /proc/meminfo");
        return;
    }
    
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %ld kB", &mem->total_mem) == 1) continue;
        if (sscanf(line, "MemFree: %ld kB", &mem->free_mem) == 1) continue;
        if (sscanf(line, "MemAvailable: %ld kB", &mem->available_mem) == 1) continue;
        if (sscanf(line, "Buffers: %ld kB", &mem->buffers) == 1) continue;
        if (sscanf(line, "Cached: %ld kB", &mem->cached) == 1) continue;
    }
    fclose(fp);
}

// Leer información de CPU
long read_cpu_total() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return 0;
    
    char line[MAX_LINE];
    long user, nice, system, idle, iowait, irq, softirq, steal;
    
    if (fgets(line, sizeof(line), fp)) {
        sscanf(line, "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
    }
    fclose(fp);
    
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

// Leer información de un proceso específico
int read_process_info(int pid, ProcessInfo *proc) {
    char path[256];
    FILE *fp;
    
    // Leer /proc/[pid]/stat
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (!fp) return 0;
    
    char comm[MAX_COMM];
    int ppid, pgrp, session, tty_nr, tpgid;
    unsigned flags;
    long minflt, cminflt, majflt, cmajflt;
    long cutime, cstime, priority, nice, num_threads, itrealvalue;
    
    if (fscanf(fp, "%d %s %c %d %d %d %d %d %u %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
               &proc->pid, comm, &proc->state, &ppid, &pgrp, &session, &tty_nr, &tpgid,
               &flags, &minflt, &cminflt, &majflt, &cmajflt, &proc->utime, &proc->stime,
               &cutime, &cstime, &priority, &nice, &num_threads, &itrealvalue, &proc->vsize) != 22) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    
    // Limpiar el nombre del comando (quitar paréntesis)
    strncpy(proc->comm, comm + 1, sizeof(proc->comm) - 1);
    proc->comm[strlen(proc->comm) - 1] = '\0';
    
    // Leer RSS de /proc/[pid]/status
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    fp = fopen(path, "r");
    if (fp) {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "VmRSS: %ld kB", &proc->rss) == 1) break;
        }
        fclose(fp);
    }
    
    return 1;
}

// Escanear todos los procesos
void scan_processes() {
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Error abriendo /proc");
        return;
    }
    
    struct dirent *entry;
    process_count = 0;
    
    while ((entry = readdir(proc_dir)) && process_count < MAX_PROCESSES) {
        if (!isdigit(entry->d_name[0])) continue;
        
        int pid = atoi(entry->d_name);
        if (read_process_info(pid, &processes[process_count])) {
            process_count++;
        }
    }
    closedir(proc_dir);
}

// Calcular porcentajes de CPU y memoria
void calculate_percentages(MemInfo *mem) {
    for (int i = 0; i < process_count; i++) {
        // Cálculo simplificado del porcentaje de CPU
        processes[i].cpu_percent = (double)(processes[i].utime + processes[i].stime) / 100.0;
        
        // Porcentaje de memoria
        if (mem->total_mem > 0) {
            processes[i].mem_percent = ((double)processes[i].rss / mem->total_mem) * 100.0;
        } else {
            processes[i].mem_percent = 0.0;
        }
    }
}

// Comparar procesos por uso de CPU (para ordenar)
int compare_by_cpu(const void *a, const void *b) {
    ProcessInfo *p1 = (ProcessInfo *)a;
    ProcessInfo *p2 = (ProcessInfo *)b;
    return (p2->cpu_percent > p1->cpu_percent) ? 1 : -1;
}

// Mostrar información del sistema
void display_system_info(MemInfo *mem) {
    time_t now;
    time(&now);
    
    printf("\033[2J\033[H"); // Limpiar pantalla y mover cursor al inicio
    printf("Monitor de Procesos - %s", ctime(&now));
    printf("========================================\n");
    
    // Información de memoria
    printf("Memoria Total: %ld MB, Libre: %ld MB, Disponible: %ld MB\n",
           mem->total_mem / 1024, mem->free_mem / 1024, mem->available_mem / 1024);
    printf("Buffers: %ld MB, Cache: %ld MB\n",
           mem->buffers / 1024, mem->cached / 1024);
    
    printf("\nProcesos activos: %d\n", process_count);
    printf("========================================\n");
    printf("%-8s %-20s %-8s %-8s %-8s %-8s\n",
           "PID", "COMANDO", "ESTADO", "CPU%", "MEM%", "RSS(KB)");
    printf("========================================\n");
}

// Mostrar lista de procesos
void display_processes() {
    int display_count = (process_count > 20) ? 20 : process_count;
    
    for (int i = 0; i < display_count; i++) {
        printf("%-8d %-20s %-8c %-8.1f %-8.1f %-8ld\n",
               processes[i].pid,
               processes[i].comm,
               processes[i].state,
               processes[i].cpu_percent,
               processes[i].mem_percent,
               processes[i].rss);
    }
}

// Enviar señal a un proceso
void send_signal_to_process() {
    int pid, sig;
    
    printf("\nIngrese PID del proceso: ");
    if (scanf("%d", &pid) != 1) {
        printf("PID inválido\n");
        return;
    }
    
    printf("Señales disponibles:\n");
    printf("1. SIGTERM (15) - Terminación normal\n");
    printf("2. SIGKILL (9) - Forzar terminación\n");
    printf("3. SIGSTOP (19) - Pausar proceso\n");
    printf("4. SIGCONT (18) - Continuar proceso\n");
    printf("Ingrese número de señal: ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Opción inválida\n");
        return;
    }
    
    switch (choice) {
        case 1: sig = SIGTERM; break;
        case 2: sig = SIGKILL; break;
        case 3: sig = SIGSTOP; break;
        case 4: sig = SIGCONT; break;
        default:
            printf("Opción inválida\n");
            return;
    }
    
    if (kill(pid, sig) == 0) {
        printf("Señal %d enviada al proceso %d\n", sig, pid);
    } else {
        perror("Error enviando señal");
    }
    
    sleep(2);
}

// Menú interactivo
void interactive_menu() {
    printf("\n========================================\n");
    printf("Opciones:\n");
    printf("r - Refrescar\n");
    printf("k - Enviar señal a proceso\n");
    printf("q - Salir\n");
    printf("Presione Enter para auto-refrescar...\n");
}

int main() {
    MemInfo mem;
    
    // Configurar manejo de señales
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("Iniciando Monitor de Procesos...\n");
    printf("Presione Ctrl+C para salir\n\n");
    
    while (running) {
        // Leer información del sistema
        read_meminfo(&mem);
        
        // Escanear procesos
        scan_processes();
        
        // Calcular porcentajes
        calculate_percentages(&mem);
        
        // Ordenar por uso de CPU
        qsort(processes, process_count, sizeof(ProcessInfo), compare_by_cpu);
        
        // Mostrar información
        display_system_info(&mem);
        display_processes();
        interactive_menu();
        
        // Esperar entrada del usuario o timeout
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        timeout.tv_sec = 3;  // Auto-refresh cada 3 segundos
        timeout.tv_usec = 0;
        
        int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
        
        if (result > 0) {
            char input = getchar();
            switch (input) {
                case 'r':
                case 'R':
                    // Refrescar (no hacer nada, el bucle se encarga)
                    break;
                case 'k':
                case 'K':
                    send_signal_to_process();
                    break;
                case 'q':
                case 'Q':
                    running = 0;
                    break;
            }
            // Limpiar buffer de entrada
            while (getchar() != '\n');
        }
    }
    
    printf("\nSaliendo del monitor de procesos...\n");
    return 0;
}