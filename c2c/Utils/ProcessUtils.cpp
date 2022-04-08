/* Copyright 2013-2022 Bas van den Berg
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>

#include "Utils/ProcessUtils.h"

#define MAX_ARG_LEN 512
#define MAX_ARGS 16

using namespace C2;
using namespace std;

static void child_error(int fd, const char* msg) {
    size_t len = strlen(msg);
    size_t written = write(fd, msg, len);
    if (written != len) perror("write");
    fsync(fd);
    fprintf(stderr, "[make] %s\n", msg);
    fflush(stderr);
    _exit(-1);      // don't call atexit functions
}

int ProcessUtils::run(const std::string& path, const std::string& cmd, const std::string& logfile) {
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

    pid_t child_pid = fork();
    if (child_pid == 0) {   // child
        char errmsg[256];

        if (close(error_pipe[0])) {
            perror("close(errpipe)");
        }

        // redirect output
        std::string output = path + logfile;
        fflush(stdout);
        close(STDOUT_FILENO);
        int fdout = open(output.c_str(), O_TRUNC | O_CREAT | O_WRONLY, 0644);
        if (fdout == -1) {
            // TODO extract
            sprintf(errmsg, "cannot open output '%s': %s", output.c_str(), strerror(errno));
            child_error(error_pipe[1], errmsg);
        }

        close(STDERR_FILENO);
        if (dup(STDOUT_FILENO) == -1) {
            sprintf(errmsg, "dup(): %s", strerror(errno));
            child_error(error_pipe[1], errmsg);
        }
        printf("current dir: %s\n", getcwd(0, 0));

        // working dir
        if (chdir(path.c_str()) != 0) {
            sprintf(errmsg, "cannot change to dir '%s': %s", path.c_str(), strerror(errno));
            child_error(error_pipe[1], errmsg);
        }
        printf("changing to dir: %s\n", path.c_str());

        // only 'self' argument, convert const char* to char*
        assert(cmd.size() <= MAX_ARG_LEN);
        char self[MAX_ARG_LEN+1];
        strcpy(self, cmd.c_str());
        char* const argv[2] = { self, 0 };

        printf("running command: %s\n", cmd.c_str());
        execv(cmd.c_str(), argv);
        int lasterr = errno;
        fprintf(stderr, "failed to start %s: %s\n", cmd.c_str(), strerror(lasterr));
        sprintf(errmsg, "error starting %s: %s", cmd.c_str(), strerror(lasterr));
        child_error(error_pipe[1], errmsg);
        _exit(-1);  // to satisfy compiler
    } else {    // parent
        if (close(error_pipe[1])) {}    // ignore errors
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
        } else {
            //fprintf(stderr, "child exited ABNORMALLY\n");
            return -1;
        }
    }
    return 0;
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


int ProcessUtils::run_args(const std::string& path, const std::string& cmd, const std::string& logfile, const char* args)
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

    pid_t child_pid = fork();
    if (child_pid == 0) {   // child
        char errmsg[256];

        if (close(error_pipe[0])) {
            perror("close(errpipe)");
        }

        // redirect output
        std::string output = path + logfile;
        fflush(stdout);
        close(STDOUT_FILENO);
        int fdout = open(output.c_str(), O_TRUNC | O_CREAT | O_WRONLY, 0644);
        if (fdout == -1) {
            // TODO extract
            sprintf(errmsg, "cannot open output '%s': %s", output.c_str(), strerror(errno));
            child_error(error_pipe[1], errmsg);
        }

        close(STDERR_FILENO);
        if (dup(STDOUT_FILENO) == -1) {
            sprintf(errmsg, "dup(): %s", strerror(errno));
            child_error(error_pipe[1], errmsg);
        }
        printf("current dir: %s\n", getcwd(0, 0));

        // working dir
        if (chdir(path.c_str()) != 0) {
            sprintf(errmsg, "cannot change to dir '%s': %s", path.c_str(), strerror(errno));
            child_error(error_pipe[1], errmsg);
        }
        printf("changing to dir: %s\n", path.c_str());
        printf("running command: %s %s\n", cmd.c_str(), args);

        // only 'self' argument, convert const char* to char*
        const char* self = find_bin(cmd.c_str());
        if (!self) {
            printf("command not found\n");
            _exit(EXIT_FAILURE);
        }
        char* argv[MAX_ARGS];
        parseArgs(self, args, argv, MAX_ARGS);

        execv(self, argv);
        int lasterr = errno;
        fprintf(stderr, "failed to start %s: %s\n", cmd.c_str(), strerror(lasterr));
        sprintf(errmsg, "error starting %s: %s", cmd.c_str(), strerror(lasterr));
        child_error(error_pipe[1], errmsg);
        _exit(EXIT_FAILURE);  // to satisfy compiler
    } else {    // parent
        if (close(error_pipe[1])) {}    // ignore errors
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
        } else {
            //fprintf(stderr, "child exited ABNORMALLY\n");
            return -1;
        }
    }
    return 0;
}

