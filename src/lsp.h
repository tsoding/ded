#ifndef LSP_H_
#define LSP_H_

#include "editor.h" 
#include <pthread.h>

extern pthread_t receive_thread;
extern const char *project_root;

/* void start_ccls(const char* project_root); */
void start_ccls();
void initialize_lsp();
void handle_signal(int sig);
void send_json_rpc(const char* method, const char* params);
void* receive_json_rpc(void* arg);
void shutdown_lsp();

void get_current_file_uri(Editor *e, char *file_uri, size_t uri_size);
void goto_definition(Editor *e);

#endif // LPS_H_
