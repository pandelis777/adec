#pragma once

#include<stdio.h>
#include<errno.h>

#define fatal(msg, ...) do {fprintf(stderr, "%s:%i: fatal, " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(-1);} while (0);

#define warning(msg, ...) do {fprintf(stderr, "%s:%i: warning, " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__);} while (0);

#define expect(comparison, msg, ...) do { if (!(comparison)) {fprintf(stderr, "%s:%i: %s failed, " msg "\n", __FILE__, __LINE__, #comparison, ##__VA_ARGS__); exit(1);}} while(0);



