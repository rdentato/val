//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "val.h"

#define N 1<<22

void generatemapping(val_t v)
{
  for (int i=00; i<N; i++) {
      vecmap(v,i,100+i);
  }
}

int checkmapping(val_t v) {
  for (int i=00; i<N; i++) {
    if (!valeq(vecget(v,i),100+i)) return 0;
  }
  return 1;
}

int main(int argc, char *argv[]) 
{
    val_t v = valnil;

    v = vecnew();
    vecsize(v,N);

    generatemapping(v);
    printf("Count %d\n",N);
    if (checkmapping(v))
      printf("Mapping OK\n");
    else
      printf("Mapping NOT OK\n");

    if (valisvec(v)) v=vecfree(v);
}