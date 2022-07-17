/* Copyright 2022 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common/process_utils.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define MAX_ARG_LEN 512
#define MAX_ARGS 16

static void child_error(int fd, const char* msg) {
    size_t len = strlen(msg);
    size_t written = write(fd, msg, len);
    if (written != len) perror("write");
    fsync(fd);
    fprintf(stderr, "[make] %s\n", msg);
    fflush(stderr);
    _exit(-1);      // don't call atexit functions
}

static void parseArgs(const char* cmd, const char* args, char* argv[], unsigned maxargs) {
    static char tmp[MAX_ARG_LEN];
    unsigned argc = 0;
    argv[argc++] = (char*)cmd;

    size_t len = strlen(args) + 1;
    assert(len < MAX_ARG_LEN);
    memcpy(tmp, args, len);

    char* token = strtok(tmp, " ");
    while (token) {
        argv[argc] = token;
        argc++;
        token = strtok(NULL, " ");
    }
    argv[argc] = 0;
}

static const char* find_bin(const char* name) {
    static char result[512];
    struct stat statbuf;

    char *dup = strdup(getenv("PATH"));
    char *s = dup;
    char *p = NULL;
    do {
        p = strchr(s, ':');
        if (p != NULL) p[0] = 0;
        sprintf(result, "%s/%s", s, name);
        if (stat(result, &statbuf) == 0) {
            free(dup);
            return result;
        }
        s = p + 1;
    } while (p != NULL);

    free(dup);
    return NULL;
}

#define LF 10
#define CR 13
static void stripNewline(char* buf) {
    int len = strlen(buf);
    char* cp = buf + len -1;
    while (cp != buf) {
        switch (*cp) {
        case LF: *cp = 0; break;
        case CR: *cp = 0; break;
        default: return;
        }
        cp--;
    }
}

int process_run(const char* path, const char* cmd, const char* args, char* out)
{
    int error_pipe[2];
    if (pipe(error_pipe)) {
        fprintf(stderr, "pipe() failed: %s\n", strerror(errno));
        return -1;
    }

    if (fcntl(error_pipe[0], F_SETFD, FD_CLOEXEC) != 0) {
        fprintf(stderr, "fcncl(FD_CLOEXEC() failed: %s\n", strerror(errno));
        return -1;
    }
    if (fcntl(error_pipe[1], F_SETFD, FD_CLOEXEC) != 0) {
        fprintf(stderr, "fcncl(FD_CLOEXEC) failed: %s\n", strerror(errno));
        return -1;
    }

    int output_pipe[2];
    if (pipe(output_pipe)) {
        fprintf(stderr, "pipe() failed: %s\n", strerror(errno));
        return -1;
    }

    if (fcntl(output_pipe[0], F_SETFD, FD_CLOEXEC) != 0) {
        fprintf(stderr, "fcncl(FD_CLOEXEC() failed: %s\n", strerror(errno));
        return -1;
    }
    if (fcntl(output_pipe[1], F_SETFD, FD_CLOEXEC) != 0) {
        fprintf(stderr, "fcncl(FD_CLOEXEC) failed: %s\n", strerror(errno));
        return -1;
    }

    pid_t child_pid = fork();
    if (child_pid == 0) {   // child
        char errmsg[256];

        if (close(output_pipe[0])) {
            perror("close(output_pipe)");
        }

        if (close(error_pipe[0])) {
            perror("close(errpipe)");
        }

        // redirect output
        fflush(stdout);
        close(STDOUT_FILENO);
        if (dup(output_pipe[1]) == -1) {
            sprintf(errmsg, "dup(): %s", strerror(errno));
            child_error(error_pipe[1], errmsg);
        }

        close(STDERR_FILENO);
        if (dup(STDOUT_FILENO) == -1) {
            sprintf(errmsg, "dup(): %s", strerror(errno));
            child_error(error_pipe[1], errmsg);
        }

        // working dir
        if (chdir(path) != 0) {
            sprintf(errmsg, "cannot change to dir '%s': %s", path, strerror(errno));
            child_error(error_pipe[1], errmsg);
        }

        // only 'self' argument, convert const char* to char*
        const char* self = find_bin(cmd);
        if (!self) {
            printf("command not found\n");
            _exit(EXIT_FAILURE);
        }
        char* argv[MAX_ARGS];
        parseArgs(self, args, argv, MAX_ARGS);

        execv(self, argv);
        int lasterr = errno;
        fprintf(stderr, "failed to start %s: %s\n", cmd, strerror(lasterr));
        sprintf(errmsg, "error starting %s: %s", cmd, strerror(lasterr));
        child_error(error_pipe[1], errmsg);
        _exit(EXIT_FAILURE);  // to satisfy compiler
    } else {    // parent
        if (close(error_pipe[1])) {}    // ignore errors
        if (close(output_pipe[1])) {}   // ignore errors
        char error[256];
        memset(error, 0, sizeof(error));
        ssize_t numread = read(error_pipe[0], error, sizeof(error)-1);
        if (numread < 0) {
            fprintf(stderr, "Error reading pipe\n");
            return -1;
        }
        else error[numread] = 0;
        close(error_pipe[0]);

        if (numread != 0) {
            printf("ERROR %s\n", error);
            //errorMsg = error;
            // always give error
            return -1;
        }

        int state = 0;
        pid_t pid = waitpid(child_pid, &state, 0);
        if (pid == -1) {
            fprintf(stderr, "Error waiting for pid: %s\n", strerror(errno));
            return -1;
        }
        if (WIFSIGNALED(state)) {
            //bool termsig = WTERMSIG(state);
            //bool coredump = WCOREDUMP(state);
            //fprintf(stderr, "child was SIGNALED: term=%d core=%d\n", termsig, coredump);
            return -1;
        }
        if (WIFEXITED(state)) { // normal termination)
            char exitcode = (char)WEXITSTATUS(state);
            //fprintf(stderr, "child exited NORMALLY, exitcode=%d\n", exitcode);
            if (exitcode != 0) return -1;
            numread = read(output_pipe[0], error, sizeof(error)-1);
            error[numread] = 0;
            char* cp = error;
            while (*cp == ' ' || *cp == '\t') cp++;
            stripNewline(cp);
            strcpy(out, cp);
        } else {
            fprintf(stderr, "child exited ABNORMALLY\n");
            return -1;
        }
    }
    return 0;
}

void process_split_cmd(const char* full, char* cmd, char* args) {
    // find first ' ', copy head to cmd, rest to args
    const char* delim = strstr(full, " ");
    if (delim) {
        unsigned len = (unsigned)(delim - full);
        memcpy(cmd, full, len);
        cmd[len] = 0;
        strcpy(args, delim+1);
    } else { // no args
        strcpy(cmd, full);
        args[0] = 0;
    }
}

