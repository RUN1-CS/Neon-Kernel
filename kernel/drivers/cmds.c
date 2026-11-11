#ifndef CMDS_C
#define CMDS_C

#include "cmds.h"
#include "filesystem/files.h"

/*
 * Execution model
 * ---------------
 * Files in /bin are real files on SimpleFS. exec() reads the file and decides:
 *
 *   "BUILTIN:<name>"   stub -> dispatch to a C function registered with
 *                      cmd_register() (this is how ls, cat, which, file ...
 * live in /bin) KEXE header        flat binary -> loaded at EXEC_LOAD_ADDR and
 * called anything else      EXEC_ERR_NOEXEC
 *
 * SimpleFS is flat, so "/bin/ls" is simply a file whose name starts with
 * "/bin/".
 */

#define BUILTIN_PREFIX "BUILTIN:"
#define BUILTIN_PREFIX_LEN 8
#define HEAD_SIZE 64

typedef struct {
  char name[CMD_NAME_MAX];
  cmd_fn fn;
} cmd_entry_t;

static cmd_entry_t registry[CMDS_MAX];
static int registry_count = 0;
static char path_var[CMDS_PATH_MAX] = CMDS_DEFAULT_PATH;
static void (*out_fn)(const char *) = NULL;

// Freestanding helpers
static uint32_t c_strlen(const char *s) {
  uint32_t n = 0;
  while (s[n])
    n++;
  return n;
}

static int c_streq(const char *a, const char *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return *a == *b;
}

static void c_memcpy(void *dst, const void *src, uint32_t n) {
  uint8_t *d = (uint8_t *)dst;
  const uint8_t *s = (const uint8_t *)src;
  while (n--)
    *d++ = *s++;
}

static int c_has_prefix(const uint8_t *buf, uint32_t n, const char *prefix) {
  uint32_t i = 0;
  for (; prefix[i]; i++) {
    if (i >= n || buf[i] != (uint8_t)prefix[i])
      return 0;
  }
  return 1;
}

static int c_has_slash(const char *s) {
  for (; *s; s++)
    if (*s == '/')
      return 1;
  return 0;
}

static uint32_t read_le32(const uint8_t *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

static void out(const char *s) {
  if (out_fn)
    out_fn(s);
}

// Set-up
void cmds_set_output(void (*puts_fn)(const char *s)) { out_fn = puts_fn; }

void cmds_set_path(const char *path) {
  uint32_t n = c_strlen(path);
  if (n >= CMDS_PATH_MAX)
    n = CMDS_PATH_MAX - 1;
  c_memcpy(path_var, path, n);
  path_var[n] = 0;
}

int cmd_register(const char *name, cmd_fn fn) {
  if (!name || !fn || !*name)
    return EXEC_ERR_BAD_ARGS;
  uint32_t len = c_strlen(name);
  if (len >= CMD_NAME_MAX)
    return EXEC_ERR_BAD_ARGS;

  for (int i = 0; i < registry_count; i++) { // re-register = replace
    if (c_streq(registry[i].name, name)) {
      registry[i].fn = fn;
      return 0;
    }
  }
  if (registry_count >= CMDS_MAX)
    return EXEC_ERR_TABLE_FULL;

  c_memcpy(registry[registry_count].name, name, len + 1);
  registry[registry_count].fn = fn;
  registry_count++;
  return 0;
}

static cmd_fn find_builtin(const char *name) {
  for (int i = 0; i < registry_count; i++) {
    if (c_streq(registry[i].name, name))
      return registry[i].fn;
  }
  return NULL;
}

// Create /bin/<name> files for all registered built-ins. Returns the number of
int cmds_install_bin(void) {
  int installed = 0;
  for (int i = 0; i < registry_count; i++) {
    char path[FS_NAME_MAX];
    char content[BUILTIN_PREFIX_LEN + CMD_NAME_MAX + 2];
    uint32_t nlen = c_strlen(registry[i].name);

    c_memcpy(path, "/bin/", 5);
    c_memcpy(path + 5, registry[i].name, nlen + 1);

    if (fs_exists(path))
      continue;

    c_memcpy(content, BUILTIN_PREFIX, BUILTIN_PREFIX_LEN);
    c_memcpy(content + BUILTIN_PREFIX_LEN, registry[i].name, nlen);
    content[BUILTIN_PREFIX_LEN + nlen] = '\n';

    if (fs_write(path, content, BUILTIN_PREFIX_LEN + nlen + 1) == FS_OK)
      installed++;
  }
  return installed;
}

int cmds_init(void) {
  cmd_register("which", cmd_which);
  cmd_register("file", cmd_file);
  return cmds_install_bin();
}

// Path resolution
int cmd_resolve(const char *file, char *out_path, uint32_t outsz) {
  if (!file || !*file || !out_path)
    return -1;
  uint32_t fl = c_strlen(file);

  if (c_has_slash(file)) { // explicit path: no PATH search
    if (fl >= outsz || !fs_exists(file))
      return -1;
    c_memcpy(out_path, file, fl + 1);
    return 0;
  }

  const char *p = path_var;
  while (*p) {
    const char *end = p;
    while (*end && *end != ':')
      end++;
    uint32_t dl = (uint32_t)(end - p);

    if (dl > 0 && dl + 1 + fl < FS_NAME_MAX && dl + 1 + fl < outsz) {
      c_memcpy(out_path, p, dl);
      out_path[dl] = '/';
      c_memcpy(out_path + dl + 1, file, fl + 1);
      if (fs_exists(out_path))
        return 0;
    }
    p = *end ? end + 1 : end;
  }
  return -1;
}

// Exec
int exec(const char *path, char **argv) {
  if (!path || !argv || !argv[0])
    return EXEC_ERR_BAD_ARGS;

  int argc = 0;
  while (argv[argc]) {
    if (++argc > EXEC_MAX_ARGS)
      return EXEC_ERR_BAD_ARGS;
  }

  int size = fs_size(path);
  if (size < 0)
    return EXEC_ERR_NOT_FOUND;

  uint8_t head[HEAD_SIZE + 1];
  int n = fs_read(path, head, HEAD_SIZE);
  if (n < 0)
    return EXEC_ERR_NOT_FOUND;
  head[n] = 0;

  // 1) built-in command stub
  if (c_has_prefix(head, (uint32_t)n, BUILTIN_PREFIX)) {
    char name[CMD_NAME_MAX];
    uint32_t i = 0;
    const uint8_t *s = head + BUILTIN_PREFIX_LEN;
    while (*s && *s != '\n' && *s != '\r' && *s != ' ' && i < CMD_NAME_MAX - 1)
      name[i++] = (char)*s++;
    name[i] = 0;

    cmd_fn fn = find_builtin(name);
    if (!fn)
      return EXEC_ERR_NOEXEC;
    return fn(argc, argv);
  }

  // 2) flat binary with KEXE header
  if (n >= 8 && read_le32(head) == EXEC_KEXE_MAGIC) {
    uint32_t entry_off = read_le32(head + 4);

    if ((uint32_t)size > EXEC_MAX_SIZE)
      return EXEC_ERR_TOO_BIG;
    if (entry_off >= (uint32_t)size)
      return EXEC_ERR_NOEXEC;

    if (fs_read(path, (void *)EXEC_LOAD_ADDR, (uint32_t)size) != size)
      return EXEC_ERR_NOT_FOUND;

    cmd_fn entry = (cmd_fn)(EXEC_LOAD_ADDR + entry_off);
    return entry(argc, argv);
  }

  return EXEC_ERR_NOEXEC;
}

int execvp(const char *file, char **argv) {
  char path[FS_NAME_MAX];
  if (cmd_resolve(file, path, sizeof(path)) != 0)
    return EXEC_ERR_NOT_FOUND;
  return exec(path, argv);
}

// Split a line into argv[] and call execvp(). Modifies the line in-place.
int cmds_run_line(char *line) {
  char *argv[EXEC_MAX_ARGS + 1];
  int argc = 0;
  char *p = line;

  while (*p) {
    while (*p == ' ' || *p == '\t')
      *p++ = 0;
    if (!*p)
      break;
    if (argc >= EXEC_MAX_ARGS)
      return EXEC_ERR_BAD_ARGS;
    argv[argc++] = p;
    while (*p && *p != ' ' && *p != '\t')
      p++;
  }
  if (argc == 0)
    return 0;
  argv[argc] = NULL;
  return execvp(argv[0], argv);
}

// Built-in: which <command>...
int cmd_which(int argc, char **argv) {
  if (argc < 2) {
    out("usage: which <command>...\n");
    return 1;
  }

  int status = 0;
  for (int i = 1; i < argc; i++) {
    char path[FS_NAME_MAX];
    if (cmd_resolve(argv[i], path, sizeof(path)) == 0) {
      out(path);
      out("\n");
    } else {
      out(argv[i]);
      out(" not found\n");
      status = 1;
    }
  }
  return status;
}

// Built-in: file <path>...
static int looks_like_text(const uint8_t *buf, int n) {
  for (int i = 0; i < n; i++) {
    uint8_t c = buf[i];
    if (c == '\n' || c == '\r' || c == '\t')
      continue;
    if (c < 32 || c > 126)
      return 0;
  }
  return 1;
}

int cmd_file(int argc, char **argv) {
  if (argc < 2) {
    out("usage: file <path>...\n");
    return 1;
  }

  int status = 0;
  for (int i = 1; i < argc; i++) {
    uint8_t head[HEAD_SIZE + 1];
    int n = fs_read(argv[i], head, HEAD_SIZE);

    out(argv[i]);
    out(": ");

    if (n < 0) {
      out("cannot open (no such file)\n");
      status = 1;
    } else if (n == 0) {
      out("empty\n");
    } else if (c_has_prefix(head, (uint32_t)n, BUILTIN_PREFIX)) {
      head[n] = 0;
      char name[CMD_NAME_MAX];
      uint32_t k = 0;
      const uint8_t *s = head + BUILTIN_PREFIX_LEN;
      while (*s && *s != '\n' && *s != '\r' && *s != ' ' &&
             k < CMD_NAME_MAX - 1)
        name[k++] = (char)*s++;
      name[k] = 0;
      out("built-in command (");
      out(name);
      out(")\n");
    } else if (n >= 8 && read_le32(head) == EXEC_KEXE_MAGIC) {
      out("kernel flat executable\n");
    } else if (n >= 4 && head[0] == 0x7F && head[1] == 'E' && head[2] == 'L' &&
               head[3] == 'F') {
      out("ELF executable (not supported by exec)\n");
    } else if (looks_like_text(head, n)) {
      out("ASCII text\n");
    } else {
      out("data\n");
    }
  }
  return status;
}

#endif // CMDS_C