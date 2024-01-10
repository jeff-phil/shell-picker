/* MIT License

Copyright (c) 2024 Jeffrey Phillips

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/utsname.h>
#include <string.h>
#ifdef __APPLE__
#include <getopt.h>
#endif // __APPLE__
#include <unistd.h>
#include <wordexp.h>

#define MAX_LINE_LENGTH 10
#define MAX_PATH_LENGTH 262144

int main(int argc, char *argv[]) {
    int opt;
    bool verbose = false;

    // Define long options, just verbose, which will be passed to the shell as well
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    // Use getopt to parse options
    while ((opt = getopt_long(argc, argv, "v", long_options, NULL)) != -1) {
        switch (opt) {
        case 'v': // Handle -v or --verbose
            verbose = true;
            break;
        default:
            // Handle other options
            break;
        }
    }

    struct utsname u;
    if (uname(&u) < 0) {
        fprintf(stderr, "Could not get system information\n");
        return 1;
    }

    if (verbose) {
        // A console messes with lldb-argdumper not able to process launch with
        // `r' shortcut in lldb just print this if verbose is passed, which also
        // causes zsh to be verbose.
        // Run the command as: `./shell-picker -v 2>/dev/null' to see the arch and
        // not the rest from zsh verbose.
        printf("System architecture: %s\n", u.machine);
    }

    char args[MAX_PATH_LENGTH] = "";

    for (int i = 1; i < argc; i++) {
        strcat(args, argv[i]);
        strcat(args, " ");
    }

    wordexp_t p;
    char configFilePath[MAX_PATH_LENGTH];

    // Expand ~/ to the user's home directory
    if (wordexp("~", &p, 0) == 0) {
        snprintf(configFilePath, sizeof(configFilePath),
                 "%s/.config/shell-picker.conf", p.we_wordv[0]);
        wordfree(&p);  // Free memory allocated by wordexp
    } else {
        fprintf(stderr, "Failed to expand path\n");
    }

    char line[MAX_LINE_LENGTH];
    char shell_x86_64[MAX_PATH_LENGTH] = "/usr/local/bin/zsh";
    char shell_arm64[MAX_PATH_LENGTH] = "/opt/homebrew/bin/zsh";

    FILE *file = fopen(configFilePath, "r");
    if (file == NULL) {
        fprintf(stderr, "%s file not found, using default shells.\n", configFilePath);
    } else {
        while (fgets(line, sizeof(line), file)) {
            char *key = strtok(line, "=");
            char *value = strtok(NULL, "\n");

            if (strcmp(key, "shell_x86_64") == 0) {
                strcpy(shell_x86_64, value);
            } else if (strcmp(key, "shell_arm64") == 0) {
                strcpy(shell_arm64, value);
            }
        }
        fclose(file);
    }

    char fullCmd[MAX_PATH_LENGTH] = {0};
    snprintf(fullCmd, sizeof(fullCmd), "%s %s",
             u.machine[0] == 'x' ? shell_x86_64 : shell_arm64,
             args);

    system(fullCmd);

    return 0;
}

