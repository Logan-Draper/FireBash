#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dir_info {
  char *path;
  char *file_names[100];
  int file_sizes[100];
  int num_files;

  struct dir_info *sub_dirs[100];
  int num_sub_dirs;
} dir_info;

int sum_ints_in_file(const char *filepath) {
  FILE *fp = fopen(filepath, "r");
  if (!fp) {
    printf("Cannot open file %s\n", filepath);
    return 0;
  }

  int total = 0;
  char line[1024];

  while (fgets(line, sizeof(line), fp)) {
    char *comma = strchr(line, ',');
    if (!comma)
      continue;

    comma++;
    while (*comma && isspace(*comma))
      comma++;

    int sign = 1;
    if (*comma == '-') {
      sign = -1;
      comma++;
    } else if (*comma == '+') {
      comma++;
    }

    if (!isdigit(*comma))
      continue;

    int value = 0;
    while (*comma && isdigit(*comma)) {
      value = value * 10 + (*comma - '0');
      comma++;
    }

    total += sign * value;
  }

  fclose(fp);
  return total;
}

dir_info *get_dir(const char *directory) {
  DIR *dir = opendir(directory);
  if (!dir) {
    printf("Cannot open directory %s\n", directory);
    return NULL;
  }

  dir_info *info = calloc(1, sizeof(dir_info));
  info->path = strdup(directory);

  struct dirent *ep;
  while ((ep = readdir(dir)) != NULL) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;

    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", directory, ep->d_name);

    if (ep->d_type == DT_DIR) {
      dir_info *child = get_dir(full_path);
      if (child) {
        info->sub_dirs[info->num_sub_dirs++] = child;
      }
      continue;
    }

    size_t len = strlen(ep->d_name);
    const char *suffix = "_output.txt";
    size_t suf_len = strlen(suffix);

    if (len < suf_len)
      continue;
    if (strcmp(ep->d_name + (len - suf_len), suffix) != 0)
      continue;

    int total = sum_ints_in_file(full_path);
    info->file_names[info->num_files] = strdup(ep->d_name);
    info->file_sizes[info->num_files] = total;
    info->num_files++;
  }

  closedir(dir);
  return info;
}

void print_tree(dir_info *info, int depth) {
  if (!info)
    return;

  for (int i = 0; i < info->num_files; i++) {
    for (int j = 0; j < depth; j++)
      printf("  ");
    printf("%s : %d\n", info->file_names[i], info->file_sizes[i]);
  }

  for (int i = 0; i < info->num_sub_dirs; i++) {
    print_tree(info->sub_dirs[i], depth + 1);
  }
}

void free_tree(dir_info *info) {
  if (!info)
    return;

  for (int i = 0; i < info->num_files; i++)
    free(info->file_names[i]);
  free(info->path);

  for (int i = 0; i < info->num_sub_dirs; i++)
    free_tree(info->sub_dirs[i]);

  free(info);
}

int main() {
  int result = system("bash ./src/search.sh");
  if (result != 0) {
    printf("Bash script failed.\n");
    return 1;
  }

  dir_info *root = get_dir(".");
  printf("\n--- OUTPUT FILES SUMS ---\n\n");
  print_tree(root, 0);

  free_tree(root);
  return 0;
}
