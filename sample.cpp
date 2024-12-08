#include <pthread.h>

int Global;

void *ThreadFunc(void *x) {
    Global++;
    return NULL;
}

int main() {
    pthread_t t;
    pthread_create(&t, NULL, ThreadFunc, NULL);
    Global++;
    pthread_join(t, NULL);
    return 0;
}
