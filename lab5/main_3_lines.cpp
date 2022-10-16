#include <bits/stdc++.h>
void S(int s){perror((s==SIGFPE?"Division by zero!":"Segmentation violation"));exit((s==SIGFPE?1:2));}
int main(int a,char* v[]){signal((a>1?atoi(v[1]):64),S);((a>1?atoi(v[1]):32)==SIGFPE?a/=0:((a>1?atoi(v[1]):32)==SIGSEGV?printf("Getting data from NULL =) %c",*(v[5051])):*v[0]=0));return 0;}