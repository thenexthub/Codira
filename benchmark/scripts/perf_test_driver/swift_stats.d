/*===--- language_stats.d ----------------------------------------------------===//
 *
 * Copyright (c) NeXTHub Corporation. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * Author(-s): Tunjay Akbarli
 *
 *===----------------------------------------------------------------------===*/

pid$target:*:language_retain:entry
{
        @counts["rr-opts"] = count();
}

pid$target:*:language_release:entry
{
        @counts["rr-opts"] = count();
}

pid$target:*:language_retain_n:entry
{
        @counts["rr-opts"] = count();
}

pid$target:*:language_release_n:entry
{
        @counts["rr-opts"] = count();
}

END
{
        printf("\nDTRACE RESULTS\n");
        printa("%s,%@u\n", @counts)
}