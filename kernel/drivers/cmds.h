#ifndef CMDS_H
#define CMDS_H

#include "filesystem/files.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

// Config
#define CMDS_MAX 32      // max registered built-ins
#define CMD_NAME_MAX 11  // 10 chars + NUL ("/bin/" + 10 = 15,
                         // the longest name files.c can store)
#define CMDS_PATH_MAX 64 // max length of the PATH string
#define CMDS_DEFAULT_PATH "/bin"

#define EXEC_MAX_ARGS 16
#define EXEC_LOAD_ADDR 0x00200000u // where flat binaries are loaded.
#define EXEC_MAX_SIZE 0x00100000u  // CHANGE to free memory in your kernel

/* ---- Return codes (all negative; a successful exec returns the command's
 *      own exit status, normally >= 0) ---------------------------------- */
#define EXEC_ERR_NOT_FOUND -100
#define EXEC_ERR_NOEXEC -101 // file exists but isn't executable
#define EXEC_ERR_TOO_BIG -102
#define EXEC_ERR_BAD_ARGS -103
#define EXEC_ERR_TABLE_FULL -104

/* Flat kernel executable ("KEXE"): 8-byte header at the start of the image.
 *   u32 magic = EXEC_KEXE_MAGIC (bytes 'K','E','X','E')
 *   u32 entry = byte offset of entry point from the start of the file
 * The whole file is loaded at EXEC_LOAD_ADDR, so link it for that address.
 * Entry signature: int entry(int argc, char **argv). */
#define EXEC_KEXE_MAGIC 0x4558454Bu

typedef int (*cmd_fn)(int argc, char **argv);

/* Setup */
void cmds_set_output(void (*puts_fn)(const char *s)); // where which/file print
void cmds_set_path(const char *path);                 // e.g. "/bin:/usr/bin"
int cmd_register(const char *name, cmd_fn fn);        // add a built-in command
int cmds_install_bin(void); // create /bin/<name> stubs
int cmds_init(void);        // registers which+file, installs stubs

/* Execution (run synchronously and return the exit status, like
 * fork+exec+wait; there is no process replacement in the kernel) */
int exec(const char *path, char **argv);   // exact path
int execvp(const char *file, char **argv); // searches PATH
int cmds_run_line(char *line);             // tokenize + execvp (modifies line)

// Lookup helper used by execvp and `which`
int cmd_resolve(const char *file, char *out, uint32_t outsz); // 0 = found

// Built-in commands
int cmd_which(int argc, char **argv);
int cmd_file(int argc, char **argv);

#endif // CMDS_H