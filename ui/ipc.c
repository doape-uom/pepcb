#define _DEFAULT_SOURCE // For struct sockaddr_un sun_len if needed
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h> // For sig_atomic_t if used
#include <stdatomic.h> // Use atomic for keep_running
#include <stddef.h> // For offsetof

#define SOCKET_PATH "/tmp/ipc_local_socket.sock" // Using Unix Domain Socket path
#define BUFFER_SIZE 4096
#define MAX_MSG_SIZE (BUFFER_SIZE - 1) // Leave space for null terminator
#define LISTEN_BACKLOG 5

// --- Global Variables ---
static pthread_t g_ipc_thread;
static int g_listen_fd = -1;
static void (*g_message_callback)(const char *) = NULL;
// Use atomic_int for thread-safe stop signal
static atomic_int g_keep_running;
// Mutex to protect access to the callback pointer if needed (e.g., if set after start)
static pthread_mutex_t g_callback_mutex = PTHREAD_MUTEX_INITIALIZER;

// --- Forward Declarations ---
static void *ipc_thread_func(void *arg);

// --- Public API Functions ---

/**
 * @brief Sets the callback function to be invoked when a message arrives.
 *
 * @param callback_func Pointer to the function to call. The function should
 *                      accept a const char* (the message) and return void.
 * @return 0 on success, -1 on error (currently always returns 0).
 */
int ipcSetCallback(void (*callback_func)(const char *)) {
    pthread_mutex_lock(&g_callback_mutex);
    g_message_callback = callback_func;
    pthread_mutex_unlock(&g_callback_mutex);
    return 0;
}

/**
 * @brief Initializes the IPC system, creates a listening socket, and starts
 *        the listener thread.
 *
 * @return 0 on success, -1 on error.
 */
int ipcStart() {
    struct sockaddr_un server_addr;

    if (g_listen_fd != -1) {
        fprintf(stderr, "IPC Error: Server already started.\n");
        return -1;
    }

    atomic_store(&g_keep_running, 1); // Set running flag

    // 1. Create Socket
    g_listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_listen_fd < 0) {
        perror("IPC Error: socket create failed");
        return -1;
    }

    // 2. Prepare Address Structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    // strncpy is safer but calculate max length precisely
    size_t max_path_len = sizeof(server_addr.sun_path) - 1;
     if (strlen(SOCKET_PATH) > max_path_len) {
        fprintf(stderr, "IPC Error: Socket path '%s' is too long (max %zu chars).\n", SOCKET_PATH, max_path_len);
        close(g_listen_fd);
        g_listen_fd = -1;
        return -1;
    }
    strncpy(server_addr.sun_path, SOCKET_PATH, max_path_len);
    server_addr.sun_path[max_path_len] = '\0'; // Ensure null termination

    // 3. Unlink existing socket file (if any)
    // Ignore error if it doesn't exist
    unlink(SOCKET_PATH);

    // 4. Bind Socket
    // Use offsetof to calculate the actual size needed for bind with sun_path
    socklen_t addr_len = offsetof(struct sockaddr_un, sun_path) + strlen(server_addr.sun_path) + 1;
    if (bind(g_listen_fd, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("IPC Error: bind failed");
        close(g_listen_fd);
        g_listen_fd = -1;
        unlink(SOCKET_PATH); // Clean up socket file on bind error
        return -1;
    }

    // 5. Listen for Connections
    if (listen(g_listen_fd, LISTEN_BACKLOG) < 0) {
        perror("IPC Error: listen failed");
        close(g_listen_fd);
        g_listen_fd = -1;
        unlink(SOCKET_PATH);
        return -1;
    }

    // 6. Create Listener Thread
    if (pthread_create(&g_ipc_thread, NULL, ipc_thread_func, NULL) != 0) {
        perror("IPC Error: pthread_create failed");
        close(g_listen_fd);
        g_listen_fd = -1;
        unlink(SOCKET_PATH);
        return -1;
    }

    printf("IPC Info: Server started successfully on %s.\n", SOCKET_PATH);
    return 0; // Success
}

/**
 * @brief Stops the IPC listener thread, closes the socket, and cleans up resources.
 */
void ipcClose() {
    if (g_listen_fd == -1) {
        printf("IPC Info: Server not running or already closed.\n");
        return;
    }

    printf("IPC Info: Stopping server...\n");
    atomic_store(&g_keep_running, 0); // Signal thread to stop

    // Shutdown the listening socket to interrupt accept()
    // This helps unblock the accept call in the listener thread.
    if (shutdown(g_listen_fd, SHUT_RDWR) < 0) {
        // Ignore common errors like "not connected" or "bad file descriptor" if already closed
        if (errno != ENOTCONN && errno != EBADF) {
            perror("IPC Warning: shutdown listen socket failed");
        }
    }

    // Close the listening socket file descriptor
    // This will also cause accept() in the thread to return an error.
    if (close(g_listen_fd) < 0) {
         if (errno != EBADF) { // Ignore if already closed by shutdown/error
            perror("IPC Warning: close listen socket failed");
         }
    }
    int closed_fd = g_listen_fd; // Store fd for potential unlink check
    g_listen_fd = -1; // Mark as closed immediately

    // Wait for the listener thread to terminate
    if (pthread_join(g_ipc_thread, NULL) != 0) {
        perror("IPC Warning: pthread_join failed");
    }

    // Remove the socket file (best effort)
    if (closed_fd != -1) { // Only unlink if we had a valid fd initially
        if (unlink(SOCKET_PATH) < 0) {
            // ENOENT (No such file or directory) is expected if already removed
            if (errno != ENOENT) {
                perror("IPC Warning: unlink socket file failed");
            }
        }
    }


    // Reset callback (optional, good practice)
    pthread_mutex_lock(&g_callback_mutex);
    g_message_callback = NULL;
    pthread_mutex_unlock(&g_callback_mutex);

    // Optionally destroy mutex if IPC system is permanently shut down
    // pthread_mutex_destroy(&g_callback_mutex);

    printf("IPC Info: Server stopped.\n");
}


// --- Internal Helper Functions ---

/**
 * @brief The main function for the IPC listener thread.
 *        Accepts connections, reads messages (newline-delimited), and
 *        invokes the registered callback.
 *
 * @param arg Thread arguments (unused).
 * @return NULL.
 */
static void *ipc_thread_func(void *arg) {
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    int conn_fd = -1;
    char buffer[BUFFER_SIZE]; // Buffer for reading data
    char message_buffer[BUFFER_SIZE] = {0}; // Buffer to assemble complete messages
    size_t message_len = 0; // Current length of assembled message

    printf("IPC Info: Listener thread started (TID: %lu).\n", (unsigned long)pthread_self());

    while (atomic_load(&g_keep_running)) {
        conn_fd = accept(g_listen_fd, (struct sockaddr *)&client_addr, &client_len);

        if (conn_fd < 0) {
            // Check if accept failed because we are shutting down
            if (!atomic_load(&g_keep_running) || errno == EINVAL || errno == EBADF) {
                // EINVAL/EBADF likely means the listening socket was closed by ipcClose()
                break; // Exit loop cleanly on shutdown
            } else if (errno == EINTR) {
                continue; // Interrupted by signal, try again
            } else {
                perror("IPC Warning: accept failed");
                // Consider adding a small delay here to prevent busy-looping on persistent errors
                sleep(1);
                continue; // Try to continue accepting
            }
        }

        printf("IPC Info: Client connected (FD: %d).\n", conn_fd);
        message_len = 0; // Reset message buffer for new connection

        // Loop to read data from the connected client
        ssize_t bytes_read;
        while ((bytes_read = read(conn_fd, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the read chunk

            char *buf_ptr = buffer;
            char *buf_end = buffer + bytes_read;

            // Process the received chunk for newline delimiters
            while (buf_ptr < buf_end) {
                char *newline = memchr(buf_ptr, '\n', buf_end - buf_ptr);

                if (newline) {
                    // Found a newline - complete message detected
                    size_t chunk_len = newline - buf_ptr;

                    if (message_len + chunk_len < MAX_MSG_SIZE) {
                        // Append chunk before newline to message buffer
                        memcpy(message_buffer + message_len, buf_ptr, chunk_len);
                        message_buffer[message_len + chunk_len] = '\0'; // Null-terminate

                        // --- Invoke Callback ---
                        pthread_mutex_lock(&g_callback_mutex);
                        if (g_message_callback) {
                            // Make a copy in case callback modifies the input
                            // char msg_copy[BUFFER_SIZE];
                            // strncpy(msg_copy, message_buffer, BUFFER_SIZE-1);
                            // msg_copy[BUFFER_SIZE-1] = '\0';
                            // g_message_callback(msg_copy);
                            // Or just pass directly if callback is trusted not to modify
                             g_message_callback(message_buffer);
                        } else {
                            printf("IPC Warning: Message received but no callback set.\n");
                        }
                        pthread_mutex_unlock(&g_callback_mutex);
                        // -----------------------

                        // Reset message buffer for the next message
                        message_len = 0;
                        message_buffer[0] = '\0';
                    } else {
                        fprintf(stderr, "IPC Error: Message too long, discarding.\n");
                        // Discard current partial message and start fresh after newline
                        message_len = 0;
                        message_buffer[0] = '\0';
                    }
                    // Move pointer past the newline for next iteration
                    buf_ptr = newline + 1;
                } else {
                    // No newline in the rest of the buffer, append to message buffer
                    size_t remaining_len = buf_end - buf_ptr;
                    if (message_len + remaining_len < MAX_MSG_SIZE) {
                        memcpy(message_buffer + message_len, buf_ptr, remaining_len);
                        message_len += remaining_len;
                        message_buffer[message_len] = '\0'; // Keep it null-terminated
                    } else {
                         fprintf(stderr, "IPC Error: Message buffer overflow, discarding.\n");
                         message_len = 0; // Discard message
                         message_buffer[0] = '\0';
                    }
                    // Break inner loop, need more data
                    break;
                }
            } // End processing buffer chunk
        } // End read loop

        // Handle read() return value
        if (bytes_read == 0) {
            printf("IPC Info: Client disconnected (FD: %d).\n", conn_fd);
            // Handle any remaining data in message_buffer if needed (partial message)
            if (message_len > 0) {
                fprintf(stderr, "IPC Warning: Partial message received before disconnect: %s\n", message_buffer);
                // Optionally call callback with partial message or just log/discard
            }
        } else if (bytes_read < 0) {
            // Read error (client might have crashed or network issue)
            // Ignore EBADF if the socket was closed concurrently
            if (errno != EBADF) {
                 perror("IPC Warning: read error from client");
            }
        }

        // Close the client connection socket
        close(conn_fd);
        conn_fd = -1; // Reset connection fd
    } // End main accept loop

    printf("IPC Info: Listener thread exiting.\n");
    // Ensure any active connection is closed if loop exited unexpectedly
    if (conn_fd >= 0) {
        close(conn_fd);
    }
    // g_listen_fd is closed in ipcClose() which signals this thread to stop
    return NULL;
}

/*
// --- Example Usage ---
// (Should be in a separate file like main.c)

#include <signal.h> // For signal handling example

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
*/