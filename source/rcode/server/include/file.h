#ifndef KD_FILE_H_
#define KD_FILE_H_

int kd_file_read_all(const char *file, char *buf, int bufsz);
void file_scan(const char *file);
#endif