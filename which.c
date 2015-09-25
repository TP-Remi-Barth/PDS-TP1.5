#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FILLDIRS_CHUNKSIZE 16
#define PATH_SEPARATOR ':'

static char *progname = NULL;

/* extend the size of the dirs array */
static char **extend_dirs(char **dirs, size_t *dirs_size){

  dirs = realloc(dirs, (*dirs_size + FILLDIRS_CHUNKSIZE) * sizeof(char *));
  *dirs_size += FILLDIRS_CHUNKSIZE;

  if (dirs == NULL){
    perror("realloc");
    return NULL;
  }

  return dirs;
}

/* create an null-terminated array of string containing the differents
  directory of the environnement variable PATH */
static char **filldirs(void){

  char *path;
  char **dirs;
  size_t dirs_i, dirs_size;

  if ((path = getenv("PATH")) == NULL){
    fprintf(stderr, "error: 'PATH' variable isn't in the environnement");
    return NULL;
  }

  if ((dirs = malloc(FILLDIRS_CHUNKSIZE * sizeof(char *))) == NULL){
    perror("malloc");
    return NULL;
  }
  dirs_size = FILLDIRS_CHUNKSIZE;
  dirs_i = 0;
  dirs[0] = NULL;

  while (*path){
    char *next_sep;
    size_t next_dir_size;
    char *new_dir;

    /* realloc dirs if necessary */
    if (dirs_i + 1 == dirs_size){
      if ((dirs = extend_dirs(dirs, &dirs_size)) == NULL){
        return NULL;
      }
    }

    /* search the next separator */
    if ((next_sep = strchr(path, PATH_SEPARATOR)) == NULL){
      next_dir_size = strlen(path);
    }
    else {
      next_dir_size = (size_t)next_sep - (size_t)path;
    }

    /* malloc a string to store the new dir */
    if ((new_dir = malloc((next_dir_size + 1) * sizeof(char))) == NULL){
      perror("malloc");
      return NULL;
    }

    /* copy the dir from the PATH to the new_dir string */
    strncpy(new_dir, path, next_dir_size)[next_dir_size] = '\0';

    /* put the new_dir string at the end of the dirs array */
    dirs[dirs_i] = new_dir;
    dirs_i += 1;
    dirs[dirs_i] = NULL;

    /* increment the path pointer */
    path += next_dir_size + (next_sep ? 1 : 0);
  }

  return dirs;
}

static void freedirs(char **dirs){
  size_t i;
  for (i = 0; dirs[i]; i += 1){
    free(dirs[i]);
  }
  free(dirs);
}

static int which(char *cmd, char **dirs){

  char *cmd_path = NULL;
  int ret = EXIT_FAILURE;
  size_t cmd_length = strlen(cmd);
  size_t cmd_path_length;

  while (*dirs){

    cmd_path_length = cmd_length + strlen(*dirs) + 1;
    cmd_path = realloc(cmd_path, (cmd_path_length + 1) * sizeof(char));
    if (cmd_path == NULL){
      perror("realloc");
      exit(EXIT_FAILURE);
    }

    strcpy(cmd_path, *dirs);
    strcat(cmd_path, "/");
    strcat(cmd_path, cmd);

    if (access(cmd_path, X_OK) == 0){
      printf("%s\n", cmd_path);
      ret = EXIT_SUCCESS;
      break;
    }
    dirs += 1;
  }

  free(cmd_path);
  if (ret == EXIT_FAILURE){
    printf("which: no %s in $PATH\n", cmd);
  }

  return ret;
}


int main(int ac, char **av){

  char **dirs;
  size_t ai;
  int ret = EXIT_SUCCESS;

  progname = av[0];

  if ((dirs = filldirs()) == NULL){
    exit(EXIT_FAILURE);
  }

  for (ai = 1; ai < ac; ai += 1){
    if (which(av[ai], dirs) == EXIT_FAILURE){
      ret = EXIT_FAILURE;
    }
  }

  freedirs(dirs);
  return ret;
}
