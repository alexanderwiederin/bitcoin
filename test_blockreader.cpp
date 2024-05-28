
#include "kernel/bitcoinkernel.h"
#include "kernel/blockreader/blockreader.h"
#include <cstdio>
#include <cstring>

void log_callback(void* user_data, const char* message, size_t message_len) {
    printf("LOG: %.*s\n", (int)message_len, message);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <bitcoin_data_directory>\n", argv[0]);
        fprintf(stderr, "Example: %s ~/.bitcoin\n", argv[0]);
        return 1;
    }

    const char* data_dir = argv[1];
    printf("Testing Blockreader with data directory: %s\n", data_dir);

    kernel_LoggingOptions log_opts = {
        .log_timestamps = true,
        .log_time_micros = false,
        .log_threadnames = false,
        .log_sourcelocations = false,
        .always_print_category_levels = true
    };

    kernel_LoggingConnection* log_conn = kernel_logging_connection_create(
            log_callback, NULL, log_opts);

    if (!log_conn) {
        fprintf(stderr, "Failed to create logging connection\n");
        return 1;
    }

    kernel_ChainParameters* chain_params = kernel_chain_parameters_create(kernel_CHAIN_TYPE_SIGNET);
    if (!chain_params) {
        fprintf(stderr, "Failed to create chain parameters\n");
        kernel_logging_connection_destroy(log_conn);
        return 1;
    }

    printf("Creating BlockReader...\n");
    kernel_blockreader_Reader* reader = kernel_blockreader_create(chain_params, data_dir, strlen(data_dir));

    if (!reader) {
        fprintf(stderr, "Failed to create BlockReader\n");
        kernel_chain_parameters_destroy(chain_params);
        kernel_logging_connection_destroy(log_conn);
        return 1;
    }

    printf("BlockReader created successfully\n");
    return 0;
}
