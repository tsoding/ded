#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "lsp.h"
#include <json-c/json.h>


int to_ccls[2];  // Pipe for sending data to ccls
int from_ccls[2];  // Pipe for receiving data from ccls
pthread_t receive_thread;

const char* project_root = "/home/l/Desktop/test/ded";


#include "file_browser.h"

void get_current_file_uri(Editor *e, char *file_uri, size_t uri_size) {
    // Assuming that `e->file_path` is a String_Builder containing the full file path
    // and `expand_path` is a function that normalizes or expands the path to a full path.
    char expanded_path[256];
    expand_path(e->file_path.items, expanded_path, sizeof(expanded_path));

    // Convert the file path to a URI format. This typically involves prefixing with "file://"
    // and ensuring the path is correctly encoded for a URI (e.g., spaces are encoded, etc.)
    // Here, for simplicity, we're just prefixing with "file://".
    snprintf(file_uri, uri_size, "file://%s", expanded_path);
}


/* void start_ccls(const char* project_root) { */
/*     if (pipe(to_ccls) == -1 || pipe(from_ccls) == -1) { */
/*         perror("Failed to create pipes"); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     pid_t pid = fork(); */
/*     if (pid == -1) { */
/*         perror("Failed to fork"); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     if (pid == 0) { */
/*         dup2(to_ccls[0], STDIN_FILENO); */
/*         dup2(from_ccls[1], STDOUT_FILENO); */
/*         close(to_ccls[0]); */
/*         close(to_ccls[1]); */
/*         close(from_ccls[0]); */
/*         close(from_ccls[1]); */
/*         execlp("ccls", "ccls", NULL); */
/*         perror("Failed to start ccls"); */
/*         exit(EXIT_FAILURE); */
/*     } else { */
/*         close(to_ccls[0]); */
/*         close(from_ccls[1]); */
/*     } */

/*     // After starting ccls, send initialize message with the project root */
/*     char init_params[1024]; */
/*     snprintf(init_params, sizeof(init_params), */
/*              "{\"processId\": null, \"rootUri\": \"file://%s\", \"capabilities\": {}}", */
/*              project_root); */
/*     send_json_rpc("initialize", init_params); */
/*     pthread_create(&receive_thread, NULL, receive_json_rpc, NULL); */
/* } */

void start_ccls() {
    char expanded_root[PATH_MAX];
    // Assuming expand_path is a function that expands tildes and relative paths
    expand_path(project_root, expanded_root, sizeof(expanded_root));

    if (pipe(to_ccls) == -1 || pipe(from_ccls) == -1) {
        perror("Failed to create pipes");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: Setup pipes and start ccls
        dup2(to_ccls[0], STDIN_FILENO);
        dup2(from_ccls[1], STDOUT_FILENO);
        close(to_ccls[0]);
        close(to_ccls[1]);
        close(from_ccls[0]);
        close(from_ccls[1]);

        execlp("ccls", "ccls", "--log-file=/dev/stderr", NULL);
        perror("Failed to start ccls");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: Close unused pipe ends
        close(to_ccls[0]);
        close(from_ccls[1]);
    }

    // Send initialize message with the expanded project root
    char init_params[1024];
    snprintf(init_params, sizeof(init_params),
             "{\"processId\": null, \"rootUri\": \"file://%s\", \"capabilities\": {}}",
             expanded_root);
    send_json_rpc("initialize", init_params);
    pthread_create(&receive_thread, NULL, receive_json_rpc, NULL);
}



void goto_definition(Editor *e) {
    printf("Debug: Entering goto_definition\n");

    char file_uri[1024];
    int line, character;
    get_current_file_uri(e, file_uri, sizeof(file_uri));
    get_cursor_position(e, &line, &character);

    printf("Debug: file_uri = %s, line = %d, character = %d\n", file_uri, line, character);

    char params[1024];
    snprintf(params, sizeof(params),
             "{\"textDocument\": {\"uri\": \"%s\"}, \"position\": {\"line\": %d, \"character\": %d}}",
             file_uri, line, character);

    send_json_rpc("textDocument/definition", params);
    printf("Debug: JSON-RPC request sent\n");
}

void send_json_rpc(const char* method, const char* params) {
    char message[4096];
    snprintf(message, sizeof(message), "{\"jsonrpc\": \"2.0\", \"method\": \"%s\", \"params\": %s}\n", method, params);
    printf("Debug: Sending JSON-RPC: %s\n", message);

    if (write(to_ccls[1], message, strlen(message)) == -1) {
        perror("Error sending JSON-RPC");
    }
}

void* receive_json_rpc(void* arg) {
    char buffer[4096];
    ssize_t nbytes;

    while ((nbytes = read(from_ccls[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[nbytes] = '\0';
        printf("Received from ccls: %s\n", buffer);  // Print the raw response

        struct json_object *parsed_json = json_tokener_parse(buffer);
        if (!parsed_json) {
            printf("Failed to parse JSON response: %s\n", buffer);
            continue;
        }

        // Log the entire JSON object for debugging
        printf("Parsed JSON response: %s\n", json_object_to_json_string(parsed_json));
        json_object_put(parsed_json);  // Free the JSON object
    }

    if (nbytes == -1) {
        perror("Error reading from ccls");
    }

    return NULL;
}

void initialize_lsp() {
    send_json_rpc("initialize", "{\"capabilities\": {}}");
    pthread_create(&receive_thread, NULL, receive_json_rpc, NULL);
}

void shutdown_lsp() {
    send_json_rpc("shutdown", "{}");
    send_json_rpc("exit", "{}");
    close(to_ccls[1]);  // Close the write-end of the pipe

    pthread_join(receive_thread, NULL);  // Wait for the receiving thread to finish

    int status;
    waitpid(-1, &status, 0);  // Wait for the ccls process to terminate
}

void handle_signal(int sig) {
    shutdown_lsp();
    exit(0);  // Exit the program
}
