#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <stdatomic.h> // Use atomic for keep_running
#include <stddef.h> // For offsetof
#include <signal.h> // For sig_atomic_t if used
#include <unistd.h> // For sleep
#include <signal.h> // For signal handling example

#define SOCKET_PATH "/tmp/ipc_local_socket.sock" // Using Unix Domain Socket path
#define BUFFER_SIZE 4096
#define MAX_MSG_SIZE (BUFFER_SIZE - 1) // Leave space for null terminator
#define LISTEN_BACKLOG 5

volatile sig_atomic_t g_stop_signal = 0;

void handle_sigint(int sig) {
    g_stop_signal = 1;
    printf("\nCaught SIGINT, shutting down...\n");
}

void my_message_handler(const char *message) {
    printf("Callback received: [%s]\n", message);
    // Example: Echo back to client (requires client implementation)
}

int main() {
    // Setup signal handler for graceful shutdown (Ctrl+C)
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint); // Handle termination signal too

    // Set the callback function
    if (ipcSetCallback(my_message_handler) != 0) {
        fprintf(stderr, "Fatal: Failed to set IPC callback.\n");
        return EXIT_FAILURE;
    }

    // Start the IPC server
    if (ipcStart() != 0) {
        fprintf(stderr, "Fatal: Failed to start IPC server.\n");
        return EXIT_FAILURE;
    }

    printf("IPC Server running. Send messages (newline-terminated) to %s.\n", SOCKET_PATH);
    printf("Press Ctrl+C to stop.\n");

    // Keep the main thread alive while the server runs
    // Or do other application work here.
    while (!g_stop_signal) {
        // Sleep or perform other tasks to avoid busy-waiting
        sleep(1);
    }

    // Stop the IPC server
    ipcClose();

    printf("Main application finished.\n");
    return EXIT_SUCCESS;
}

// To test, you can use netcat (nc) or socat:
// socat - UNIX-CONNECT:/tmp/ipc_local_socket.sock
// or
// nc -U /tmp/ipc_local_socket.sock
// Then type messages followed by Enter.