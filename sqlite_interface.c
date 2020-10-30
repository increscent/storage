#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "sqlite3.h"

#define MAX_PARAMS  4096

const char QUIT = '0';
const char *NULL_VALUE = "(null)";

void failure(char *msg);
void query(char *sql);
void quit(int ret);
void replace_char(char *str, char seek, char replace);
void sig_int_handler(int signal);

sqlite3 *db;

int main(int argc, char **argv)
{
    char *line;
    size_t n;

    db = NULL;

    signal(SIGINT, sig_int_handler);

    if (argc != 2) {
        failure("Usage: sqlite_interface [DB_FILENAME]");
        quit(EXIT_FAILURE);
    }

    if (sqlite3_open(argv[1], &db)) {
        failure((char*) sqlite3_errmsg(db));
        quit(EXIT_FAILURE);
    }

    while (1) {
        line = NULL;
        n = 0;
        if (getline(&line, &n, stdin) < 0)
            quit(EXIT_SUCCESS);

        replace_char(line, '\n', '\0');

        if (!line[0]) {
            // Empty line
            free(line);
            continue;
        }
        
        if (line[0] == QUIT)
            quit(EXIT_SUCCESS);

        query(line);

        free(line);
    }
}

void query(char *sql)
{
    char *params[MAX_PARAMS];
    char *line;
    const char *value;
    const char *column;
    sqlite3_stmt *statement;
    int i;
    int params_count;
    int columns_count;
    size_t n;

    // Read an extra line for each param ('?')
    params_count = 0;
    for (i = 0; sql[i]; i++) {
        if (sql[i] == '?') {
            n = 0;
            line = NULL;
            if (getline(&line, &n, stdin) < 0)
                quit(EXIT_FAILURE);

            replace_char(line, '\n', '\0');

            params[params_count++] = line;
        }
    }

    if (sqlite3_prepare_v2(db, sql, -1, &statement, NULL) != SQLITE_OK) {
        failure((char*) sqlite3_errmsg(db));

        for (i = 0; i < params_count; i++)
            free(params[i]);

        printf("QUERY\n");
        fflush(stdout);
        return;
    }

    for (i = 0; i < params_count; i++)
        sqlite3_bind_text(statement, i + 1, params[i], -1, free);

    while (sqlite3_step(statement) == SQLITE_ROW) {
        columns_count = sqlite3_column_count(statement);

        for (i = 0; i < columns_count; i++) {
            column = sqlite3_column_name(statement, i);
            printf("%s\n", column ? column : NULL_VALUE);

            value = sqlite3_column_text(statement, i);
            printf("%s\n", value ? value : NULL_VALUE);
        }

        printf("ROW\n");
    }

    sqlite3_finalize(statement);

    printf("QUERY\n");
    fflush(stdout);
}

void replace_char(char *str, char seek, char replace)
{
    int i;

    for (i = 0; str[i]; i++)
        if (str[i] == seek)
            str[i] = replace;
}

void failure(char *errmsg)
{
    printf("FAIL\n%s\n", errmsg);
}

void quit(int ret)
{
    printf("END\n");

    if (db)
        sqlite3_close(db);

    exit(ret);
}

void sig_int_handler(int signal)
{
    quit(EXIT_SUCCESS);
}
