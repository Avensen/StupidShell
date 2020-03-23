// This is StupidShell, we call it that way 'cause is a shell and also smart.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <regex.h>

#define TRUE 1
#define FALSE 0
#define BUFSIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
#define cls() printf("\033[H\033[J")

void main_loop(void);

char *read_line(void);

char *scan_line(void);

char **parse(char *);

int launch(char **);

int execute(char **);

void prompt(void);

// Built-in StupidShell commands
int __cd(char **);

int __help(char **);

int __exit(char **);

int __donate(char **);

// Built-ins
char *builtin_names[] = {"cd", "help", "exit", "donate"};

int (*builtin_funcs[])(char **) = {&__cd, &__help, &__exit, &__donate};

int builtin_num() { return sizeof(builtin_names) / sizeof(char *); }

int main(int argc, char **argv) {
    // Presentation
    char *username = getenv("USER");
    printf("    ----- StupidShell -----\n");
    printf("The smarter shell in the market\n");
    printf("Welcome %s\n", username);
    sleep(1);
    cls();

    // StupidShell main loop
    main_loop();

    return EXIT_SUCCESS;
}

void main_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        prompt();
        line = scan_line();
        args = parse(line);
        status = execute(args);

        free(line);
        free(args);
    } while (status);
}

char *read_line(void) {
    int bufsize = BUFSIZE,
            pos = 0,
            c;
    char *buffer = malloc(sizeof(char) * bufsize);

    if (!buffer) {
        fprintf(stderr, "stupidshell: -allerr- allocation error\n");
        exit(EXIT_FAILURE);
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (TRUE) {
        // Read a character.
        c = getchar();

        // If EOF or EOL, replace with nullchar and return.
        if (c == EOF || c == '\n') {
            buffer[pos] = '\0';
            return buffer;
        }
        buffer[pos++] = (char) c;

        // If buffer exceeded, reallocate.
        if (pos >= bufsize) {
            bufsize += BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "stupidshell: -allerr- allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
#pragma clang diagnostic pop
}

char *scan_line(void) {
    char *line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

char **parse(char *line) {
    int bufsize = TOK_BUFSIZE,
            pos = 0;
    char **tokens = malloc(sizeof(char *) * bufsize);
    char *token;

    if (!tokens) {
        printf("stupidshell: -allerr- allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token) {
        tokens[pos++] = token;

        if (pos >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, sizeof(char *) * bufsize);
            if (!tokens) {
                printf("stupidshell: -allerr- allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

int launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (!pid) {
        // Child process
        if (execvp(args[0], args) == -1)
            printf("stupidshell: -cmderr- do not know how to run that\n");
        exit(EXIT_FAILURE);
    } else if (pid < 0)
        printf("stupidshell: -frkerr- forking process failed");
    else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!(WIFEXITED((status)) && !WIFSIGNALED((status)))); // NOLINT(hicpp-signed-bitwise)
    }
    return 1;
}

int execute(char **args) {
    int i;

    if (!args[0])
        return -1;

    for (i = 0; i < builtin_num(); ++i) {
        if (!strcmp(args[0], builtin_names[i]))
            return (*builtin_funcs[i])(args);
    }

    return launch(args);
}

int __cd(char **args) {
    if (!args[1])
        printf("stupidshell: -argerr- expected argument to \"cd\"\n");
    else if (chdir(args[1]))
        printf("stupidshell: -argerr- what that file is that\n");

    return 1;
}

int __help(char **args) {
    int i;
    printf("----- StupidShell -----\n");
    printf("An over 200 IQ shell\n");
    printf("Type what you want to know, then hit \\n :).\n");
    printf("Some features are built-in, otherwise they wouldn't work\n");

    for (i = 0; i < builtin_num(); ++i)
        printf("  %s\n", builtin_names[i]);

    printf("For more info, ask Google, I mean, run the man command.\n");
    return 1;
}

int __exit(char **args) {
    printf("Goodbye, don't come back soon...");
    return 0;
}

int __donate(char **args) {
    printf("Just so you know, if you want to contribute, this is our BTC address\n");
    printf("129GxaGyphjvAa3koPkFktQsmY3jQ6fsrd\nThank you... moron.\n");
    return 1;
}

void prompt() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd) * 1);
    printf("%s >>> ", cwd);
}
