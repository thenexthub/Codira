/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */

#ifndef __CUDAX_EXECUTION__
#define __CUDAX_EXECUTION__

// IWYU pragma: begin_exports
#include <uscl/experimental/__execution/apply_sender.cuh>
#include <uscl/experimental/__execution/bulk.cuh>
#include <uscl/experimental/__execution/completion_behavior.cuh>
#include <uscl/experimental/__execution/completion_signatures.cuh>
#include <uscl/experimental/__execution/conditional.cuh>
#include <uscl/experimental/__execution/continues_on.cuh>
#include <uscl/experimental/__execution/cpos.cuh>
#include <uscl/experimental/__execution/domain.cuh>
#include <uscl/experimental/__execution/env.cuh>
#include <uscl/experimental/__execution/get_completion_signatures.cuh>
#include <uscl/experimental/__execution/inline_scheduler.cuh>
#include <uscl/experimental/__execution/just.cuh>
#include <uscl/experimental/__execution/just_from.cuh>
#include <uscl/experimental/__execution/let_value.cuh>
#include <uscl/experimental/__execution/on.cuh>
#include <uscl/experimental/__execution/policy.cuh>
#include <uscl/experimental/__execution/queries.cuh>
#include <uscl/experimental/__execution/read_env.cuh>
#include <uscl/experimental/__execution/run_loop.cuh>
#include <uscl/experimental/__execution/schedule_from.cuh>
#include <uscl/experimental/__execution/sequence.cuh>
#include <uscl/experimental/__execution/start_detached.cuh>
#include <uscl/experimental/__execution/starts_on.cuh>
#include <uscl/experimental/__execution/stop_token.cuh>
#include <uscl/experimental/__execution/stream_context.cuh>
#include <uscl/experimental/__execution/sync_wait.cuh>
#include <uscl/experimental/__execution/then.cuh>
#include <uscl/experimental/__execution/thread_context.cuh>
#include <uscl/experimental/__execution/transform_completion_signatures.cuh>
#include <uscl/experimental/__execution/transform_sender.cuh>
#include <uscl/experimental/__execution/visit.cuh>
#include <uscl/experimental/__execution/when_all.cuh>
#include <uscl/experimental/__execution/write_attrs.cuh>
#include <uscl/experimental/__execution/write_env.cuh>
// IWYU pragma: end_exports

#endif // __CUDAX_EXECUTION__
