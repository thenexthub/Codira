/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

int add(int a, float b);

//float multiply(int a, float b = 1.0f);

int legacy_func();

int (*noProtoPtr)() = &legacy_func;

void cfoo1(int *a);

void cfoo2(int a[3]);

int main(int argc, char *argv[]);

typedef void (*callback)(int);

void set_callback(callback cb);

int add2(int a, int b, int (*func)(int, int));

typedef struct {
    long long x;
    long long y;
    long long z;
} Point3D;

Point3D addPoint(Point3D p1, Point3D p2);
