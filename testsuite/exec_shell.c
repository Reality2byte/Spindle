#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

typedef enum {
    run_pathsearch = 0,
    run_dot,
    run_relative,
    run_abs,
    run_done
} run_t;

typedef enum {
    shell_none_sh = 0,
    shell_none_tcsh,
    shell_sh,
    shell_bash,
    shell_tcsh,
    shell_env,
    shell_done
} shell_t;

typedef enum {
    arg_match = 0,
    arg_partial,
    arg_abs,
    arg_junk,
    arg_done
} arg_t;

typedef enum {
    exect_v,
    exect_vp,
    exect_done
} exec_type_t;

static char cwd[4097];
static int print_results = 0;

static int have_sh;
static int have_bash;
static int have_tcsh;
static int have_env;

/**
 * Initialization and setup
 **/
static int init()
{
    char *newpath;
    char *curpath;
    char *sresult;    
    size_t len;
    int result;
    struct stat buf;

    memset(cwd, 0, sizeof(cwd));
    sresult = getcwd(cwd, sizeof(cwd)-1);
    if (!sresult) { 
        perror("Could not getcwd");
        return -1;
    }

    sresult = realpath(cwd, NULL);
    if (!sresult) {
        perror("Could not realpath");
        return -1;
    }
    strncpy(cwd, sresult, sizeof(cwd)-1);
    free(sresult);
    sresult = NULL;

    result = chdir(cwd);
    if (result == -1) {
        perror("Could not chdir");
        return -1;
    }

    curpath = getenv("PATH");
    len = curpath ? strlen(curpath) : 0;
    len += strlen(cwd);
    len += 2;

    newpath = (char *) malloc(len);
    snprintf(newpath, len, "%s%s%s", cwd, curpath ? ":" : "", curpath ? curpath : "");
    setenv("PATH", newpath, 1);

    result = stat("/bin/sh", &buf);
    have_sh = (result == 0);
    result = stat("/bin/bash", &buf);
    have_bash = (result == 0);
    result = stat("/bin/tcsh", &buf);
    have_tcsh = (result == 0);
    result = stat("/bin/env", &buf);
    have_env = (result == 0);

    return 0;
}

/**
 * Run the appropriate script/test based on the inputs and return the scripts first output line if successful.
 **/
static char* run(run_t rt, shell_t st, arg_t at, exec_type_t et, int *skip)
{
    const char* argv[8];
    const char *launcher = NULL, *exe, *ext;
    char abspath[4200];
    char script_name[4200];    
    char *s, *line;
    int i = 0, result, status;
    int fds[2];
    FILE *f;
    pid_t pid;

    memset(abspath, 0, sizeof(abspath));
    snprintf(abspath, sizeof(abspath)-1, "%s/exec_shellXXXXXXXXX", cwd);
    memset(script_name, 0, sizeof(script_name));

    if (et == exect_v && rt == run_pathsearch) {
        *skip = 1;
        return NULL;
    }
    
    *skip = 0;
    switch (st) {
        case shell_none_sh:
            if (!have_sh) {
                *skip = 1;
                return NULL;
            }
            ext = ".sh";
            break;
        case shell_none_tcsh:
            if (!have_tcsh) {
                *skip = 1;
                return NULL;
            }
            ext = ".tcsh";
            break;
        case shell_sh:
            if (!have_sh) {
                *skip = 1;
                return NULL;
            }
            ext = ".sh";
            launcher = "/bin/sh";
            break;
        case shell_bash:
            if (!have_bash) {
                *skip = 1;
                return NULL;
            }
            ext = ".sh";
            launcher = "/bin/bash";
            break;
        case shell_tcsh:
            if (!have_tcsh) {
                *skip = 1;
                return NULL;
            }
            ext = ".tcsh";
            launcher = "/bin/tcsh";
            break;
        case shell_env:
            if (!have_env || !have_sh) {
                *skip = 1;
                return NULL;
            }
            ext = "_env.sh";
            break;            
        case shell_done:
            launcher = NULL;
            return NULL;
    }

    memset(script_name, 0, sizeof(script_name));
    switch (rt) {
        case run_pathsearch:
            strncpy(script_name, "exec_shellXXXXXXXXX", sizeof(script_name)-1);
            break;
        case run_dot:
            strncpy(script_name, "./exec_shellXXXXXXXXX", sizeof(script_name)-1);
            break;
        case run_relative:
            strncpy(script_name, "../testsuite/exec_shellXXXXXXXXX", sizeof(script_name)-1);
            break;
        case run_abs:
            strncpy(script_name, abspath, sizeof(script_name));
            break;
        case run_done:
            return NULL;
    }
    s = strstr(script_name, "XXXXXXXXX");
    memcpy(s, ext, strlen(ext)+1);
    s = strstr(abspath, "XXXXXXXXX");
    if (s)
        memcpy(s, ext, strlen(ext)+1);

    exe = launcher ? launcher : script_name;
    if (launcher)
        argv[i++] = launcher;
    argv[i++] = script_name;
        
    //Muck with argv[0] and make it different than expected.
    switch (at) {
        case arg_match:
            break;
        case arg_partial:
            argv[0] = "exe_target.sh";
            break;
        case arg_abs:
            argv[0] = abspath;
            break;
        case arg_junk:
            argv[0] = "junk";
            break;
        case arg_done:
            return NULL;
    }

    argv[i++] = "--arg1";
    argv[i++] = "--arg2";

    argv[i++] = NULL;
    
    result = pipe(fds);
    if (result == -1) {
        perror("Could not pipe");
        return NULL;
    }

    pid = fork();
    if (pid == -1) {
        perror("Could not fork");
        close(fds[0]);
        close(fds[1]);
        return NULL;
    }
    else if (pid == 0) {
        dup2(fds[1], 1);
        close(fds[0]);
        if (et == exect_v) {
            execv(exe, (char* const*) argv);
        }
        else if (et == exect_vp) {
            execvp(exe, (char* const*) argv);
        }
        perror("Failed to exe");
        return NULL;
    }

    close(fds[1]);
    line = (char *) malloc(4096);
    f = fdopen(fds[0], "r");
    s = fgets(line, 4096, f);
    if (s != line) {
        free(line);
        fprintf(stderr, "Could not read input from script\n");
        line = NULL;
    }
    fclose(f);
    
    do {
        result = waitpid(pid, &status, 0);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        perror("waitpid error");
        return NULL;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Child process returned unexpected value %d from waitpid (exited = %d, exitstatus = %d ; signaled = %d, termsig = %d)\n",
                status, (int) WIFEXITED(status), (int) WEXITSTATUS(status), (int) WIFSIGNALED(status), (int) WTERMSIG(status));
        fprintf(stderr, "exec = %s; arg[0] = %s; argv[1] = %s; argv[2] = %s; argv[3] = %s\n", 
            exe, argv[0] ? argv[0] : "[NULL]", argv[1] ? argv[1] : "[NULL]", argv[2] ? argv[2] : "[NULL]", argv[3] ? argv[3] : "[NULL]");
        return NULL;
    }
      
    return line;
}

/**
 * Read one line from the file containing the expected output
 **/
static FILE* result_file = NULL;
static char *read_result()
{
    char *s, *t;
    if (!result_file) {
        result_file = fopen("./exec_shell_expected_output", "r");
        if (!result_file) {
            perror("Could not open result file ./exec_shell_expected_output");
            exit(-1);
        }
    }
    s = (char *) malloc(4096);
    t = fgets(s, 4096, result_file);
    if (s != t) {
        fprintf(stderr, "Could not read from ./exec_shell_expected_output");
        return NULL;
    }
    return s;
}

/**
 * Debugging assist method to print out what combo of paramters we were running
 **/
#define STR(X) STR2(X)
#define STR2(X) #X
#define STRCASE(X) case X: s = STR(X); break
char *test_string(run_t rt, shell_t st, arg_t at, exec_type_t et)
{
    const char *rs, *ss, *as, *es, *s = NULL;
    char buffer[4096];

    switch (rt) {
        STRCASE(run_pathsearch);
        STRCASE(run_dot);
        STRCASE(run_relative);
        STRCASE(run_abs);
        STRCASE(run_done);
    }
    rs = s;
    switch (st) {
        STRCASE(shell_none_sh);
        STRCASE(shell_none_tcsh);
        STRCASE(shell_sh);
        STRCASE(shell_bash);
        STRCASE(shell_tcsh);
        STRCASE(shell_env);
        STRCASE(shell_done);
    }
    ss = s;
    switch (at) {
        STRCASE(arg_match);
        STRCASE(arg_partial);
        STRCASE(arg_abs);
        STRCASE(arg_junk);
        STRCASE(arg_done);
    }
    as = s;
    switch (et) {
        STRCASE(exect_v);
        STRCASE(exect_vp);
        STRCASE(exect_done);
    }
    es = s;

    snprintf(buffer, sizeof(buffer), "run_t = '%s', shell_t = '%s', arg_t = '%s', exect = '%s'", rs, ss, as, es);
    return strdup(buffer);
}

/**
 * Easier debug printing by removing newlines
 **/
static void strip_newline(char *s) {
    if (!s)
        return;
    for (; *s != '\0'; s++)
        if (*s == '\n') {
            *s = '\0';
            break;
        }
}

/**
 * Run a test by exec'ing a script that prints its argv[]. We'll invoke that script a bunch of different ways,
 * based on parameters, and read a line from the expected results output file. If the two lines match, that
 * subtest passes.
 */
static int run_test(run_t rt, shell_t st, arg_t at, exec_type_t et) {
    char *test_line = NULL, *result_line = NULL, *errstr = NULL;
    int skip = 0;
    int result = -1;

    test_line = run(rt, st, at, et, &skip);
    if (print_results) {
        if (skip) {
            printf("Skipped\n");
            return 0;
        }            
        assert(test_line);
        printf("%s", test_line);
        free(test_line);
        return 0;
    }
    result_line = read_result();
    strip_newline(test_line);
    strip_newline(result_line);

    if (skip) {
        result = 0;
        goto done;
    }
    
    if (!test_line) {
        result = -1;
        goto done;
    }
    if (!result_line) {
        fprintf(stderr, "Internal test error. No results to compare to\n");
        result = -1;
        goto done;
    }

    if (strcmp(test_line, result_line) != 0) {
        fprintf(stderr, "Error: Test line '%s' and result line '%s' do not match\n", test_line, result_line);
        result = -1;
        goto done;
    }

    result = 0;
    
    done:
    if (result != 0) {
        errstr = test_string(rt,  st, at, et);
        fprintf(stderr, "Test failure while running: %s\n", errstr);
    }
    if (test_line)
        free(test_line);
    if (result_line)
        free(result_line);
    if (errstr)
        free(errstr);
    return result;
}

int main(int argc, char *argv[])
{
    int r, s, a, e;
    int result;
    int had_error = 0;

    if (argc >= 2 && strcmp(argv[1], "--print") == 0) {
        print_results = 1;
    }
    result = init();
    if (result == -1)
        return -1;

    for (r = 0; r < (int) run_done; r++) {
        for (s = 0; s < (int) shell_done; s++) {
            for (a = 0; a < (int) arg_done; a++) {
                for (e = 0; e < (int) exect_done; e++) {
                    result = run_test((run_t) r, (shell_t) s, (arg_t) a, (exec_type_t) e);
                    if (result == -1) {
                        had_error = 1;
                    }
                }
            }
        }
    }
    if (result_file)
        fclose(result_file);
    if (had_error) {
        fprintf(stderr, "Failure\n");
        return -1;
    }
    printf("PASSED.\n");
    return 0;
}