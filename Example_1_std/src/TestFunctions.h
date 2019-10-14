#ifndef FUNCTIONS_H
#define FUNCTIONS_H

extern thread_local const char* threadId;

void print(const char* func);

void currentThreadTask();
void writerThreadTask1();
void writerThreadTask2();
bool networkThreadTask();
void uiThreadTask();
bool needNetwork();

#endif
