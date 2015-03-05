#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#include "Utils/ProcessUtils.h"

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

void ProcessUtils::run(const std::string& path, const std::string& cmd) {
    int error_pipe[2];
    if (pipe(error_pipe)) {
        //errorMsg = "pipe() failed";
        return;
    }

    if (fcntl(error_pipe[0], F_SETFD, FD_CLOEXEC) != 0) {
        //errorMsg = "fcncl(FD_CLOEXEC) failed";
        return;
    }
    if (fcntl(error_pipe[1], F_SETFD, FD_CLOEXEC) != 0) {
        //errorMsg = "fcncl(FD_CLOEXEC) failed";
        return;
    }

    pid_t child_pid = fork();
    if (child_pid == 0) {   // child
        char errmsg[256];

        if (close(error_pipe[0])) {
            perror("close(errpipe)");
        }

#if 0
        char logfile[32];
        sprintf(logfile, "/tmp/luna-%d.log", T.tid);
        fflush(stdout);
        close(STDOUT_FILENO);
        int fdout = open(logfile, O_APPEND | O_CREAT | O_WRONLY, 0644);
        if (fdout == -1) {
            // TODO extract
            sprintf(errmsg, "cannot open logfile '%s': %s", logfile, strerror(errno));
            child_error(error_pipe[1], errmsg);
        }
        close(STDERR_FILENO);
        if (dup(STDOUT_FILENO) == -1) {
            sprintf(errmsg, "dup(): %s", strerror(errno));
            child_error(error_pipe[1], errmsg);
        }
#endif
        // redirect output
        std::string logfile = path + "build.log";
        fflush(stdout);
        close(STDOUT_FILENO);
        int fdout = open(logfile.c_str(), O_TRUNC | O_CREAT | O_WRONLY, 0644);
        if (fdout == -1) {
            // TODO extract
            sprintf(errmsg, "cannot open logfile '%s': %s", logfile.c_str(), strerror(errno));
            child_error(error_pipe[1], errmsg);
        }
        close(STDERR_FILENO);
        if (dup(STDOUT_FILENO) == -1) {
            sprintf(errmsg, "dup(): %s", strerror(errno));
            child_error(error_pipe[1], errmsg);
        }

        // working dir
        if (chdir(path.c_str()) != 0) {
            sprintf(errmsg, "cannot change to dir '%s': %s", path.c_str(), strerror(errno));
            child_error(error_pipe[1], errmsg);
        }

        // arguments
        char* argv[1] = { 0 };

        //Utils::parseArgs(T.cmd, T.args, argv, MAX_ARGUMENTS);

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
            return;
        }
        else error[numread] = 0;
        close(error_pipe[0]);

        if (numread != 0) {
            int state = 0;
            pid_t pid = waitpid(child_pid, &state, 0);
            if (pid == -1) {
                fprintf(stderr, "Error waiting for pid: %s\n", strerror(errno));
                return;
            }
            //errorMsg = error;
        }
    }
}

