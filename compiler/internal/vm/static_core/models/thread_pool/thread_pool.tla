\***************************************************************************
\* Copyright (c) 2021-2024 Huawei Device Co., Ltd.
\* Licensed under the Apache License, Version 2.0 (the "License");
\* you may not use this file except in compliance with the License.
\* You may obtain a copy of the License at
\* 
\* http://www.apache.org/licenses/LICENSE-2.0
\* 
\* Unless required by applicable law or agreed to in writing, software
\* distributed under the License is distributed on an "AS IS" BASIS,
\* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\* See the License for the specific language governing permissions and
\* limitations under the License.
\***************************************************************************

---------------------------- MODULE thread_pool ----------------------------
EXTENDS Sequences, Integers, TLC

CONSTANTS max_workers, max_producers, max_controllers, queue_max_size

workers_set == 1..max_workers
producers_set == (max_workers + 1)..(max_workers + max_producers)
controllers_set == (max_workers + max_producers + 1)..(max_workers + max_producers + max_controllers)
workers_initial_val == [i \in 1 .. max_workers |-> FALSE]

(*--algorithm thread_pool

variables workers = workers_initial_val,
          workers_status = workers_initial_val,
          num_workers = 0,
          is_active = TRUE,
          queue_lock_ = 0,
          scale_lock_ = 0;
          cond_var_ = FALSE;
          queue_size_ = 0;

\* Methods with return value

define 
    is_full == queue_size_ >= queue_max_size
end define;


\* Synchronisation primitives
macro lock(lock_var)
begin
  await lock_var = 0;
  lock_var := self;
end macro;

macro unlock(lock_var)
begin
  assert (lock_var = self);
  lock_var := 0;
end macro;

macro signal(cond_var)
begin
  cond_var := TRUE;
end macro;

macro wait_signal(lock_var, cond_var)
begin
  await cond_var = TRUE /\ lock_var = 0;
  cond_var := FALSE || lock_var := self;
end macro;

macro join(worker_num)
begin
  await workers_status[worker_num] = FALSE;
end macro;

procedure timed_wait_signal()
begin
timed_wait_signal_entry:
  either goto timed_wait_signal_wait;
  or     goto timed_wait_signal_exit;
  end either;
timed_wait_signal_wait:
  unlock(queue_lock_);
timed_wait_signal_wait_1:
  if cond_var_ # TRUE \/ queue_lock_ # 0 then
    if is_active = FALSE then
      await queue_lock_ = 0;
      queue_lock_ := self;
      return;
    else
      goto timed_wait_signal_wait_1;
    end if;
  else
    cond_var_ := FALSE || queue_lock_ := self;
  end if;
timed_wait_signal_exit:
  return;
end procedure;

\* TaskQueueInterface methods

procedure get_task()
begin
get_task_entry:
  if queue_size_ = 0 then
    task := 0;
  else
    task := 1;
    queue_size_ := queue_size_ - 1;
  end if;
get_task_exit:
  return;
end procedure;

procedure add_task()
begin
add_task_increase_queue_size:
  queue_size_ := queue_size_ + 1;
add_task_exit:
  return;
end procedure;

procedure try_add_task()
begin
try_add_task_entry:
  if is_full then
    goto try_add_task_exit;
  else
try_add_task_increase_queue_size:
  queue_size_ := queue_size_ + 1;
    end if;
try_add_task_exit:
  return;
end procedure;

procedure finalize()
begin
finalize_entry:
    queue_size_ := 0;
finalize_exit:
  return;
end procedure;

\* ThreadPool methods

procedure stop_worker(worker_num)
begin
stop_worker_lock_and_signal:
  lock(queue_lock_);
stop_worker_finalize_queue:
  call finalize();
stop_worker_unlock:
  signal(cond_var_);
  unlock(queue_lock_);
stop_worker_join:  
  join(worker_num);
  return;
end procedure;

procedure create_new_thread(worker_num)
begin
create_new_thread_lock:
  lock(queue_lock_);
  workers[num_workers + 1] := TRUE;
create_new_thread_unlock:
  unlock(queue_lock_);
  return;
end procedure;

procedure scale(num)
begin
scale_lock:
  lock(scale_lock_);
  assert num \in 0..max_workers;
scale_check:
  if num > num_workers    then goto scale_add_workers;
  elsif num < num_workers then goto scale_del_workers;
  else goto scale_ret;
  end if;
scale_add_workers:
  while num_workers < num do
    call create_new_thread(num_workers);
scale_queue_inc_lock:
    lock(queue_lock_);
scale_add_workers_inc:
    num_workers := num_workers + 1;
scale_queue_inc_unlock:
    unlock(queue_lock_);
  end while;
scale_del_workers:
  while num_workers > num do
    workers[num_workers] := FALSE;
    call stop_worker(num_workers);
scale_queue_dec_lock:
    lock(queue_lock_);
scale_del_workers_dec_num_workers:
    num_workers := num_workers - 1;
scale_queue_dec_unlock:
    unlock(queue_lock_);
  end while;
scale_ret:
  unlock(scale_lock_);
  return
end procedure;

procedure enumerate_procs()
begin
enumerate_procs_lock:
  lock(scale_lock_);
enumerate_procs_unlock:
  unlock(scale_lock_);
  return
end procedure;


procedure shutdown()
variable n = 1;
begin
shutdown_lock:
  lock(scale_lock_);
shutdown_loop:
  while n <= num_workers do
    call stop_worker(n);
  shutdown_inc_n:
    n := n + 1;
  end while;
shutdown_unlock:
  unlock(scale_lock_);
shutdown_exit:
  return;
end procedure;

procedure wait_task()
begin
wait_task_entry:
  call timed_wait_signal();
wait_task_exit:
  return;
end procedure;

procedure put_task()
begin
put_task_lock:
  lock(queue_lock_);
put_task_check_active:
  if is_active = FALSE then
    goto put_task_exit;
  end if;
put_task_full_queue:
  if is_full then
    call wait_task();
    goto put_task_full_queue;
  end if;
put_task_increase_queue_size:
  call add_task();
put_task_signal:
  signal(cond_var_);
put_task_exit:
  unlock(queue_lock_);
  return;
end procedure;

procedure try_put_task()
begin
try_put_task_lock:
  lock(queue_lock_);
try_put_task_check_active:
  if is_active = FALSE then
    goto try_put_task_exit;
  end if;
try_put_task_check_full_queue:
  call try_add_task();
try_put_task_signal:
  signal(cond_var_);
try_put_task_exit:
  unlock(queue_lock_);
  return;
end procedure;

\* Processes

fair process worker \in workers_set
variable task = 0;
begin
worker_inactive:
  while workers[self] = FALSE do
    if is_active = FALSE then
      goto worker_exit;
    end if;
  end while;
worker_lock:
  workers_status[self] := TRUE;
  lock(queue_lock_);
worker_check_active:
  if workers[self] = FALSE then
    unlock(queue_lock_);
    goto worker_exit;
  end if;
worker_get_task:
  call get_task();
worker_check_task_empty:
  if task = 0 then
    call timed_wait_signal();
    goto worker_unlock;
  end if;
worker_signal_task:
  signal(cond_var_);
worker_unlock:
  unlock(queue_lock_);
  goto worker_lock;
worker_exit:
  workers_status[self] := FALSE;
end process;

fair process producer \in producers_set
begin
producer_entry:
  if is_active = FALSE then
    goto producer_exit;
  end if;
producer_loop:
  either goto producer_put_task;
  or goto producer_try_put_task;
  end either;
producer_put_task:
  call put_task();
producer_put_task_end:
  goto producer_entry;
producer_try_put_task:
  call try_put_task();
producer_try_put_task_end:
  goto producer_entry;
producer_exit:
  skip;
end process;

fair process controller \in controllers_set
begin
controller_loop:
  either goto controller_scale;
  or goto controller_shutdown;
  or goto controller_enumerate_procs;
  end either;
controller_scale:
  with n_ \in 0..max_workers do
    call scale(n_);
  end with;
controller_scale_end:
  goto controller_loop;
controller_enumerate_procs:
  call enumerate_procs();
controller_enumerate_procs_end:
  goto controller_loop;
controller_shutdown:
  call shutdown();
controller_exit:
  skip;
end process;

end algorithm

safety(<> Finish)

*)
\* BEGIN TRANSLATION (chksum(pcal) = "c1ecf13e" /\ chksum(tla) = "e6aba2c")
\* Parameter worker_num of procedure stop_worker at line 128 col 23 changed to worker_num_
CONSTANT defaultInitValue
VARIABLES workers, workers_status, num_workers, is_active, queue_lock_, 
          scale_lock_, cond_var_, queue_size_, pc, stack

(* define statement *)
is_full == queue_size_ >= queue_max_size

VARIABLES worker_num_, worker_num, num, n, task

vars == << workers, workers_status, num_workers, is_active, queue_lock_, 
           scale_lock_, cond_var_, queue_size_, pc, stack, worker_num_, 
           worker_num, num, n, task >>

ProcSet == (workers_set) \cup (producers_set) \cup (controllers_set)

Init == (* Global variables *)
        /\ workers = workers_initial_val
        /\ workers_status = workers_initial_val
        /\ num_workers = 0
        /\ is_active = TRUE
        /\ queue_lock_ = 0
        /\ scale_lock_ = 0
        /\ cond_var_ = FALSE
        /\ queue_size_ = 0
        (* Procedure stop_worker *)
        /\ worker_num_ = [ self \in ProcSet |-> defaultInitValue]
        (* Procedure create_new_thread *)
        /\ worker_num = [ self \in ProcSet |-> defaultInitValue]
        (* Procedure scale *)
        /\ num = [ self \in ProcSet |-> defaultInitValue]
        (* Procedure shutdown *)
        /\ n = [ self \in ProcSet |-> 1]
        (* Process worker *)
        /\ task = [self \in workers_set |-> 0]
        /\ stack = [self \in ProcSet |-> << >>]
        /\ pc = [self \in ProcSet |-> CASE self \in workers_set -> "worker_inactive"
                                        [] self \in producers_set -> "producer_entry"
                                        [] self \in controllers_set -> "controller_loop"]

timed_wait_signal_entry(self) == /\ pc[self] = "timed_wait_signal_entry"
                                 /\ \/ /\ pc' = [pc EXCEPT ![self] = "timed_wait_signal_wait"]
                                    \/ /\ pc' = [pc EXCEPT ![self] = "timed_wait_signal_exit"]
                                 /\ UNCHANGED << workers, workers_status, 
                                                 num_workers, is_active, 
                                                 queue_lock_, scale_lock_, 
                                                 cond_var_, queue_size_, stack, 
                                                 worker_num_, worker_num, num, 
                                                 n, task >>

timed_wait_signal_wait(self) == /\ pc[self] = "timed_wait_signal_wait"
                                /\ Assert((queue_lock_ = self), 
                                          "Failure of assertion at line 38, column 3 of macro called at line 65, column 3.")
                                /\ queue_lock_' = 0
                                /\ pc' = [pc EXCEPT ![self] = "timed_wait_signal_wait_1"]
                                /\ UNCHANGED << workers, workers_status, 
                                                num_workers, is_active, 
                                                scale_lock_, cond_var_, 
                                                queue_size_, stack, 
                                                worker_num_, worker_num, num, 
                                                n, task >>

timed_wait_signal_wait_1(self) == /\ pc[self] = "timed_wait_signal_wait_1"
                                  /\ IF cond_var_ # TRUE \/ queue_lock_ # 0
                                        THEN /\ IF is_active = FALSE
                                                   THEN /\ queue_lock_ = 0
                                                        /\ queue_lock_' = self
                                                        /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                                                        /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                                                   ELSE /\ pc' = [pc EXCEPT ![self] = "timed_wait_signal_wait_1"]
                                                        /\ UNCHANGED << queue_lock_, 
                                                                        stack >>
                                             /\ UNCHANGED cond_var_
                                        ELSE /\ /\ cond_var_' = FALSE
                                                /\ queue_lock_' = self
                                             /\ pc' = [pc EXCEPT ![self] = "timed_wait_signal_exit"]
                                             /\ stack' = stack
                                  /\ UNCHANGED << workers, workers_status, 
                                                  num_workers, is_active, 
                                                  scale_lock_, queue_size_, 
                                                  worker_num_, worker_num, num, 
                                                  n, task >>

timed_wait_signal_exit(self) == /\ pc[self] = "timed_wait_signal_exit"
                                /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                                /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                                /\ UNCHANGED << workers, workers_status, 
                                                num_workers, is_active, 
                                                queue_lock_, scale_lock_, 
                                                cond_var_, queue_size_, 
                                                worker_num_, worker_num, num, 
                                                n, task >>

timed_wait_signal(self) == timed_wait_signal_entry(self)
                              \/ timed_wait_signal_wait(self)
                              \/ timed_wait_signal_wait_1(self)
                              \/ timed_wait_signal_exit(self)

get_task_entry(self) == /\ pc[self] = "get_task_entry"
                        /\ IF queue_size_ = 0
                              THEN /\ task' = [task EXCEPT ![self] = 0]
                                   /\ UNCHANGED queue_size_
                              ELSE /\ task' = [task EXCEPT ![self] = 1]
                                   /\ queue_size_' = queue_size_ - 1
                        /\ pc' = [pc EXCEPT ![self] = "get_task_exit"]
                        /\ UNCHANGED << workers, workers_status, num_workers, 
                                        is_active, queue_lock_, scale_lock_, 
                                        cond_var_, stack, worker_num_, 
                                        worker_num, num, n >>

get_task_exit(self) == /\ pc[self] = "get_task_exit"
                       /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                       /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, queue_lock_, scale_lock_, 
                                       cond_var_, queue_size_, worker_num_, 
                                       worker_num, num, n, task >>

get_task(self) == get_task_entry(self) \/ get_task_exit(self)

add_task_increase_queue_size(self) == /\ pc[self] = "add_task_increase_queue_size"
                                      /\ queue_size_' = queue_size_ + 1
                                      /\ pc' = [pc EXCEPT ![self] = "add_task_exit"]
                                      /\ UNCHANGED << workers, workers_status, 
                                                      num_workers, is_active, 
                                                      queue_lock_, scale_lock_, 
                                                      cond_var_, stack, 
                                                      worker_num_, worker_num, 
                                                      num, n, task >>

add_task_exit(self) == /\ pc[self] = "add_task_exit"
                       /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                       /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, queue_lock_, scale_lock_, 
                                       cond_var_, queue_size_, worker_num_, 
                                       worker_num, num, n, task >>

add_task(self) == add_task_increase_queue_size(self) \/ add_task_exit(self)

try_add_task_entry(self) == /\ pc[self] = "try_add_task_entry"
                            /\ IF is_full
                                  THEN /\ pc' = [pc EXCEPT ![self] = "try_add_task_exit"]
                                  ELSE /\ pc' = [pc EXCEPT ![self] = "try_add_task_increase_queue_size"]
                            /\ UNCHANGED << workers, workers_status, 
                                            num_workers, is_active, 
                                            queue_lock_, scale_lock_, 
                                            cond_var_, queue_size_, stack, 
                                            worker_num_, worker_num, num, n, 
                                            task >>

try_add_task_increase_queue_size(self) == /\ pc[self] = "try_add_task_increase_queue_size"
                                          /\ queue_size_' = queue_size_ + 1
                                          /\ pc' = [pc EXCEPT ![self] = "try_add_task_exit"]
                                          /\ UNCHANGED << workers, 
                                                          workers_status, 
                                                          num_workers, 
                                                          is_active, 
                                                          queue_lock_, 
                                                          scale_lock_, 
                                                          cond_var_, stack, 
                                                          worker_num_, 
                                                          worker_num, num, n, 
                                                          task >>

try_add_task_exit(self) == /\ pc[self] = "try_add_task_exit"
                           /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                           /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                           /\ UNCHANGED << workers, workers_status, 
                                           num_workers, is_active, queue_lock_, 
                                           scale_lock_, cond_var_, queue_size_, 
                                           worker_num_, worker_num, num, n, 
                                           task >>

try_add_task(self) == try_add_task_entry(self)
                         \/ try_add_task_increase_queue_size(self)
                         \/ try_add_task_exit(self)

finalize_entry(self) == /\ pc[self] = "finalize_entry"
                        /\ queue_size_' = 0
                        /\ pc' = [pc EXCEPT ![self] = "finalize_exit"]
                        /\ UNCHANGED << workers, workers_status, num_workers, 
                                        is_active, queue_lock_, scale_lock_, 
                                        cond_var_, stack, worker_num_, 
                                        worker_num, num, n, task >>

finalize_exit(self) == /\ pc[self] = "finalize_exit"
                       /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                       /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, queue_lock_, scale_lock_, 
                                       cond_var_, queue_size_, worker_num_, 
                                       worker_num, num, n, task >>

finalize(self) == finalize_entry(self) \/ finalize_exit(self)

stop_worker_lock_and_signal(self) == /\ pc[self] = "stop_worker_lock_and_signal"
                                     /\ queue_lock_ = 0
                                     /\ queue_lock_' = self
                                     /\ pc' = [pc EXCEPT ![self] = "stop_worker_finalize_queue"]
                                     /\ UNCHANGED << workers, workers_status, 
                                                     num_workers, is_active, 
                                                     scale_lock_, cond_var_, 
                                                     queue_size_, stack, 
                                                     worker_num_, worker_num, 
                                                     num, n, task >>

stop_worker_finalize_queue(self) == /\ pc[self] = "stop_worker_finalize_queue"
                                    /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "finalize",
                                                                             pc        |->  "stop_worker_unlock" ] >>
                                                                         \o stack[self]]
                                    /\ pc' = [pc EXCEPT ![self] = "finalize_entry"]
                                    /\ UNCHANGED << workers, workers_status, 
                                                    num_workers, is_active, 
                                                    queue_lock_, scale_lock_, 
                                                    cond_var_, queue_size_, 
                                                    worker_num_, worker_num, 
                                                    num, n, task >>

stop_worker_unlock(self) == /\ pc[self] = "stop_worker_unlock"
                            /\ cond_var_' = TRUE
                            /\ Assert((queue_lock_ = self), 
                                      "Failure of assertion at line 38, column 3 of macro called at line 136, column 3.")
                            /\ queue_lock_' = 0
                            /\ pc' = [pc EXCEPT ![self] = "stop_worker_join"]
                            /\ UNCHANGED << workers, workers_status, 
                                            num_workers, is_active, 
                                            scale_lock_, queue_size_, stack, 
                                            worker_num_, worker_num, num, n, 
                                            task >>

stop_worker_join(self) == /\ pc[self] = "stop_worker_join"
                          /\ workers_status[worker_num_[self]] = FALSE
                          /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                          /\ worker_num_' = [worker_num_ EXCEPT ![self] = Head(stack[self]).worker_num_]
                          /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                          /\ UNCHANGED << workers, workers_status, num_workers, 
                                          is_active, queue_lock_, scale_lock_, 
                                          cond_var_, queue_size_, worker_num, 
                                          num, n, task >>

stop_worker(self) == stop_worker_lock_and_signal(self)
                        \/ stop_worker_finalize_queue(self)
                        \/ stop_worker_unlock(self)
                        \/ stop_worker_join(self)

create_new_thread_lock(self) == /\ pc[self] = "create_new_thread_lock"
                                /\ queue_lock_ = 0
                                /\ queue_lock_' = self
                                /\ workers' = [workers EXCEPT ![num_workers + 1] = TRUE]
                                /\ pc' = [pc EXCEPT ![self] = "create_new_thread_unlock"]
                                /\ UNCHANGED << workers_status, num_workers, 
                                                is_active, scale_lock_, 
                                                cond_var_, queue_size_, stack, 
                                                worker_num_, worker_num, num, 
                                                n, task >>

create_new_thread_unlock(self) == /\ pc[self] = "create_new_thread_unlock"
                                  /\ Assert((queue_lock_ = self), 
                                            "Failure of assertion at line 38, column 3 of macro called at line 148, column 3.")
                                  /\ queue_lock_' = 0
                                  /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                                  /\ worker_num' = [worker_num EXCEPT ![self] = Head(stack[self]).worker_num]
                                  /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                                  /\ UNCHANGED << workers, workers_status, 
                                                  num_workers, is_active, 
                                                  scale_lock_, cond_var_, 
                                                  queue_size_, worker_num_, 
                                                  num, n, task >>

create_new_thread(self) == create_new_thread_lock(self)
                              \/ create_new_thread_unlock(self)

scale_lock(self) == /\ pc[self] = "scale_lock"
                    /\ scale_lock_ = 0
                    /\ scale_lock_' = self
                    /\ Assert(num[self] \in 0..max_workers, 
                              "Failure of assertion at line 156, column 3.")
                    /\ pc' = [pc EXCEPT ![self] = "scale_check"]
                    /\ UNCHANGED << workers, workers_status, num_workers, 
                                    is_active, queue_lock_, cond_var_, 
                                    queue_size_, stack, worker_num_, 
                                    worker_num, num, n, task >>

scale_check(self) == /\ pc[self] = "scale_check"
                     /\ IF num[self] > num_workers
                           THEN /\ pc' = [pc EXCEPT ![self] = "scale_add_workers"]
                           ELSE /\ IF num[self] < num_workers
                                      THEN /\ pc' = [pc EXCEPT ![self] = "scale_del_workers"]
                                      ELSE /\ pc' = [pc EXCEPT ![self] = "scale_ret"]
                     /\ UNCHANGED << workers, workers_status, num_workers, 
                                     is_active, queue_lock_, scale_lock_, 
                                     cond_var_, queue_size_, stack, 
                                     worker_num_, worker_num, num, n, task >>

scale_add_workers(self) == /\ pc[self] = "scale_add_workers"
                           /\ IF num_workers < num[self]
                                 THEN /\ /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "create_new_thread",
                                                                                  pc        |->  "scale_queue_inc_lock",
                                                                                  worker_num |->  worker_num[self] ] >>
                                                                              \o stack[self]]
                                         /\ worker_num' = [worker_num EXCEPT ![self] = num_workers]
                                      /\ pc' = [pc EXCEPT ![self] = "create_new_thread_lock"]
                                 ELSE /\ pc' = [pc EXCEPT ![self] = "scale_del_workers"]
                                      /\ UNCHANGED << stack, worker_num >>
                           /\ UNCHANGED << workers, workers_status, 
                                           num_workers, is_active, queue_lock_, 
                                           scale_lock_, cond_var_, queue_size_, 
                                           worker_num_, num, n, task >>

scale_queue_inc_lock(self) == /\ pc[self] = "scale_queue_inc_lock"
                              /\ queue_lock_ = 0
                              /\ queue_lock_' = self
                              /\ pc' = [pc EXCEPT ![self] = "scale_add_workers_inc"]
                              /\ UNCHANGED << workers, workers_status, 
                                              num_workers, is_active, 
                                              scale_lock_, cond_var_, 
                                              queue_size_, stack, worker_num_, 
                                              worker_num, num, n, task >>

scale_add_workers_inc(self) == /\ pc[self] = "scale_add_workers_inc"
                               /\ num_workers' = num_workers + 1
                               /\ pc' = [pc EXCEPT ![self] = "scale_queue_inc_unlock"]
                               /\ UNCHANGED << workers, workers_status, 
                                               is_active, queue_lock_, 
                                               scale_lock_, cond_var_, 
                                               queue_size_, stack, worker_num_, 
                                               worker_num, num, n, task >>

scale_queue_inc_unlock(self) == /\ pc[self] = "scale_queue_inc_unlock"
                                /\ Assert((queue_lock_ = self), 
                                          "Failure of assertion at line 38, column 3 of macro called at line 170, column 5.")
                                /\ queue_lock_' = 0
                                /\ pc' = [pc EXCEPT ![self] = "scale_add_workers"]
                                /\ UNCHANGED << workers, workers_status, 
                                                num_workers, is_active, 
                                                scale_lock_, cond_var_, 
                                                queue_size_, stack, 
                                                worker_num_, worker_num, num, 
                                                n, task >>

scale_del_workers(self) == /\ pc[self] = "scale_del_workers"
                           /\ IF num_workers > num[self]
                                 THEN /\ workers' = [workers EXCEPT ![num_workers] = FALSE]
                                      /\ /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "stop_worker",
                                                                                  pc        |->  "scale_queue_dec_lock",
                                                                                  worker_num_ |->  worker_num_[self] ] >>
                                                                              \o stack[self]]
                                         /\ worker_num_' = [worker_num_ EXCEPT ![self] = num_workers]
                                      /\ pc' = [pc EXCEPT ![self] = "stop_worker_lock_and_signal"]
                                 ELSE /\ pc' = [pc EXCEPT ![self] = "scale_ret"]
                                      /\ UNCHANGED << workers, stack, 
                                                      worker_num_ >>
                           /\ UNCHANGED << workers_status, num_workers, 
                                           is_active, queue_lock_, scale_lock_, 
                                           cond_var_, queue_size_, worker_num, 
                                           num, n, task >>

scale_queue_dec_lock(self) == /\ pc[self] = "scale_queue_dec_lock"
                              /\ queue_lock_ = 0
                              /\ queue_lock_' = self
                              /\ pc' = [pc EXCEPT ![self] = "scale_del_workers_dec_num_workers"]
                              /\ UNCHANGED << workers, workers_status, 
                                              num_workers, is_active, 
                                              scale_lock_, cond_var_, 
                                              queue_size_, stack, worker_num_, 
                                              worker_num, num, n, task >>

scale_del_workers_dec_num_workers(self) == /\ pc[self] = "scale_del_workers_dec_num_workers"
                                           /\ num_workers' = num_workers - 1
                                           /\ pc' = [pc EXCEPT ![self] = "scale_queue_dec_unlock"]
                                           /\ UNCHANGED << workers, 
                                                           workers_status, 
                                                           is_active, 
                                                           queue_lock_, 
                                                           scale_lock_, 
                                                           cond_var_, 
                                                           queue_size_, stack, 
                                                           worker_num_, 
                                                           worker_num, num, n, 
                                                           task >>

scale_queue_dec_unlock(self) == /\ pc[self] = "scale_queue_dec_unlock"
                                /\ Assert((queue_lock_ = self), 
                                          "Failure of assertion at line 38, column 3 of macro called at line 181, column 5.")
                                /\ queue_lock_' = 0
                                /\ pc' = [pc EXCEPT ![self] = "scale_del_workers"]
                                /\ UNCHANGED << workers, workers_status, 
                                                num_workers, is_active, 
                                                scale_lock_, cond_var_, 
                                                queue_size_, stack, 
                                                worker_num_, worker_num, num, 
                                                n, task >>

scale_ret(self) == /\ pc[self] = "scale_ret"
                   /\ Assert((scale_lock_ = self), 
                             "Failure of assertion at line 38, column 3 of macro called at line 184, column 3.")
                   /\ scale_lock_' = 0
                   /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                   /\ num' = [num EXCEPT ![self] = Head(stack[self]).num]
                   /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                   /\ UNCHANGED << workers, workers_status, num_workers, 
                                   is_active, queue_lock_, cond_var_, 
                                   queue_size_, worker_num_, worker_num, n, 
                                   task >>

scale(self) == scale_lock(self) \/ scale_check(self)
                  \/ scale_add_workers(self) \/ scale_queue_inc_lock(self)
                  \/ scale_add_workers_inc(self)
                  \/ scale_queue_inc_unlock(self)
                  \/ scale_del_workers(self) \/ scale_queue_dec_lock(self)
                  \/ scale_del_workers_dec_num_workers(self)
                  \/ scale_queue_dec_unlock(self) \/ scale_ret(self)

enumerate_procs_lock(self) == /\ pc[self] = "enumerate_procs_lock"
                              /\ scale_lock_ = 0
                              /\ scale_lock_' = self
                              /\ pc' = [pc EXCEPT ![self] = "enumerate_procs_unlock"]
                              /\ UNCHANGED << workers, workers_status, 
                                              num_workers, is_active, 
                                              queue_lock_, cond_var_, 
                                              queue_size_, stack, worker_num_, 
                                              worker_num, num, n, task >>

enumerate_procs_unlock(self) == /\ pc[self] = "enumerate_procs_unlock"
                                /\ Assert((scale_lock_ = self), 
                                          "Failure of assertion at line 38, column 3 of macro called at line 193, column 3.")
                                /\ scale_lock_' = 0
                                /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                                /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                                /\ UNCHANGED << workers, workers_status, 
                                                num_workers, is_active, 
                                                queue_lock_, cond_var_, 
                                                queue_size_, worker_num_, 
                                                worker_num, num, n, task >>

enumerate_procs(self) == enumerate_procs_lock(self)
                            \/ enumerate_procs_unlock(self)

shutdown_lock(self) == /\ pc[self] = "shutdown_lock"
                       /\ scale_lock_ = 0
                       /\ scale_lock_' = self
                       /\ pc' = [pc EXCEPT ![self] = "shutdown_loop"]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, queue_lock_, cond_var_, 
                                       queue_size_, stack, worker_num_, 
                                       worker_num, num, n, task >>

shutdown_loop(self) == /\ pc[self] = "shutdown_loop"
                       /\ IF n[self] <= num_workers
                             THEN /\ /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "stop_worker",
                                                                              pc        |->  "shutdown_inc_n",
                                                                              worker_num_ |->  worker_num_[self] ] >>
                                                                          \o stack[self]]
                                     /\ worker_num_' = [worker_num_ EXCEPT ![self] = n[self]]
                                  /\ pc' = [pc EXCEPT ![self] = "stop_worker_lock_and_signal"]
                             ELSE /\ pc' = [pc EXCEPT ![self] = "shutdown_unlock"]
                                  /\ UNCHANGED << stack, worker_num_ >>
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, queue_lock_, scale_lock_, 
                                       cond_var_, queue_size_, worker_num, num, 
                                       n, task >>

shutdown_inc_n(self) == /\ pc[self] = "shutdown_inc_n"
                        /\ n' = [n EXCEPT ![self] = n[self] + 1]
                        /\ pc' = [pc EXCEPT ![self] = "shutdown_loop"]
                        /\ UNCHANGED << workers, workers_status, num_workers, 
                                        is_active, queue_lock_, scale_lock_, 
                                        cond_var_, queue_size_, stack, 
                                        worker_num_, worker_num, num, task >>

shutdown_unlock(self) == /\ pc[self] = "shutdown_unlock"
                         /\ Assert((scale_lock_ = self), 
                                   "Failure of assertion at line 38, column 3 of macro called at line 210, column 3.")
                         /\ scale_lock_' = 0
                         /\ pc' = [pc EXCEPT ![self] = "shutdown_exit"]
                         /\ UNCHANGED << workers, workers_status, num_workers, 
                                         is_active, queue_lock_, cond_var_, 
                                         queue_size_, stack, worker_num_, 
                                         worker_num, num, n, task >>

shutdown_exit(self) == /\ pc[self] = "shutdown_exit"
                       /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                       /\ n' = [n EXCEPT ![self] = Head(stack[self]).n]
                       /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, queue_lock_, scale_lock_, 
                                       cond_var_, queue_size_, worker_num_, 
                                       worker_num, num, task >>

shutdown(self) == shutdown_lock(self) \/ shutdown_loop(self)
                     \/ shutdown_inc_n(self) \/ shutdown_unlock(self)
                     \/ shutdown_exit(self)

wait_task_entry(self) == /\ pc[self] = "wait_task_entry"
                         /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "timed_wait_signal",
                                                                  pc        |->  "wait_task_exit" ] >>
                                                              \o stack[self]]
                         /\ pc' = [pc EXCEPT ![self] = "timed_wait_signal_entry"]
                         /\ UNCHANGED << workers, workers_status, num_workers, 
                                         is_active, queue_lock_, scale_lock_, 
                                         cond_var_, queue_size_, worker_num_, 
                                         worker_num, num, n, task >>

wait_task_exit(self) == /\ pc[self] = "wait_task_exit"
                        /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                        /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                        /\ UNCHANGED << workers, workers_status, num_workers, 
                                        is_active, queue_lock_, scale_lock_, 
                                        cond_var_, queue_size_, worker_num_, 
                                        worker_num, num, n, task >>

wait_task(self) == wait_task_entry(self) \/ wait_task_exit(self)

put_task_lock(self) == /\ pc[self] = "put_task_lock"
                       /\ queue_lock_ = 0
                       /\ queue_lock_' = self
                       /\ pc' = [pc EXCEPT ![self] = "put_task_check_active"]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, scale_lock_, cond_var_, 
                                       queue_size_, stack, worker_num_, 
                                       worker_num, num, n, task >>

put_task_check_active(self) == /\ pc[self] = "put_task_check_active"
                               /\ IF is_active = FALSE
                                     THEN /\ pc' = [pc EXCEPT ![self] = "put_task_exit"]
                                     ELSE /\ pc' = [pc EXCEPT ![self] = "put_task_full_queue"]
                               /\ UNCHANGED << workers, workers_status, 
                                               num_workers, is_active, 
                                               queue_lock_, scale_lock_, 
                                               cond_var_, queue_size_, stack, 
                                               worker_num_, worker_num, num, n, 
                                               task >>

put_task_full_queue(self) == /\ pc[self] = "put_task_full_queue"
                             /\ IF is_full
                                   THEN /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "wait_task",
                                                                                 pc        |->  "put_task_full_queue" ] >>
                                                                             \o stack[self]]
                                        /\ pc' = [pc EXCEPT ![self] = "wait_task_entry"]
                                   ELSE /\ pc' = [pc EXCEPT ![self] = "put_task_increase_queue_size"]
                                        /\ stack' = stack
                             /\ UNCHANGED << workers, workers_status, 
                                             num_workers, is_active, 
                                             queue_lock_, scale_lock_, 
                                             cond_var_, queue_size_, 
                                             worker_num_, worker_num, num, n, 
                                             task >>

put_task_increase_queue_size(self) == /\ pc[self] = "put_task_increase_queue_size"
                                      /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "add_task",
                                                                               pc        |->  "put_task_signal" ] >>
                                                                           \o stack[self]]
                                      /\ pc' = [pc EXCEPT ![self] = "add_task_increase_queue_size"]
                                      /\ UNCHANGED << workers, workers_status, 
                                                      num_workers, is_active, 
                                                      queue_lock_, scale_lock_, 
                                                      cond_var_, queue_size_, 
                                                      worker_num_, worker_num, 
                                                      num, n, task >>

put_task_signal(self) == /\ pc[self] = "put_task_signal"
                         /\ cond_var_' = TRUE
                         /\ pc' = [pc EXCEPT ![self] = "put_task_exit"]
                         /\ UNCHANGED << workers, workers_status, num_workers, 
                                         is_active, queue_lock_, scale_lock_, 
                                         queue_size_, stack, worker_num_, 
                                         worker_num, num, n, task >>

put_task_exit(self) == /\ pc[self] = "put_task_exit"
                       /\ Assert((queue_lock_ = self), 
                                 "Failure of assertion at line 38, column 3 of macro called at line 241, column 3.")
                       /\ queue_lock_' = 0
                       /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                       /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, scale_lock_, cond_var_, 
                                       queue_size_, worker_num_, worker_num, 
                                       num, n, task >>

put_task(self) == put_task_lock(self) \/ put_task_check_active(self)
                     \/ put_task_full_queue(self)
                     \/ put_task_increase_queue_size(self)
                     \/ put_task_signal(self) \/ put_task_exit(self)

try_put_task_lock(self) == /\ pc[self] = "try_put_task_lock"
                           /\ queue_lock_ = 0
                           /\ queue_lock_' = self
                           /\ pc' = [pc EXCEPT ![self] = "try_put_task_check_active"]
                           /\ UNCHANGED << workers, workers_status, 
                                           num_workers, is_active, scale_lock_, 
                                           cond_var_, queue_size_, stack, 
                                           worker_num_, worker_num, num, n, 
                                           task >>

try_put_task_check_active(self) == /\ pc[self] = "try_put_task_check_active"
                                   /\ IF is_active = FALSE
                                         THEN /\ pc' = [pc EXCEPT ![self] = "try_put_task_exit"]
                                         ELSE /\ pc' = [pc EXCEPT ![self] = "try_put_task_check_full_queue"]
                                   /\ UNCHANGED << workers, workers_status, 
                                                   num_workers, is_active, 
                                                   queue_lock_, scale_lock_, 
                                                   cond_var_, queue_size_, 
                                                   stack, worker_num_, 
                                                   worker_num, num, n, task >>

try_put_task_check_full_queue(self) == /\ pc[self] = "try_put_task_check_full_queue"
                                       /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "try_add_task",
                                                                                pc        |->  "try_put_task_signal" ] >>
                                                                            \o stack[self]]
                                       /\ pc' = [pc EXCEPT ![self] = "try_add_task_entry"]
                                       /\ UNCHANGED << workers, workers_status, 
                                                       num_workers, is_active, 
                                                       queue_lock_, 
                                                       scale_lock_, cond_var_, 
                                                       queue_size_, 
                                                       worker_num_, worker_num, 
                                                       num, n, task >>

try_put_task_signal(self) == /\ pc[self] = "try_put_task_signal"
                             /\ cond_var_' = TRUE
                             /\ pc' = [pc EXCEPT ![self] = "try_put_task_exit"]
                             /\ UNCHANGED << workers, workers_status, 
                                             num_workers, is_active, 
                                             queue_lock_, scale_lock_, 
                                             queue_size_, stack, worker_num_, 
                                             worker_num, num, n, task >>

try_put_task_exit(self) == /\ pc[self] = "try_put_task_exit"
                           /\ Assert((queue_lock_ = self), 
                                     "Failure of assertion at line 38, column 3 of macro called at line 258, column 3.")
                           /\ queue_lock_' = 0
                           /\ pc' = [pc EXCEPT ![self] = Head(stack[self]).pc]
                           /\ stack' = [stack EXCEPT ![self] = Tail(stack[self])]
                           /\ UNCHANGED << workers, workers_status, 
                                           num_workers, is_active, scale_lock_, 
                                           cond_var_, queue_size_, worker_num_, 
                                           worker_num, num, n, task >>

try_put_task(self) == try_put_task_lock(self)
                         \/ try_put_task_check_active(self)
                         \/ try_put_task_check_full_queue(self)
                         \/ try_put_task_signal(self)
                         \/ try_put_task_exit(self)

worker_inactive(self) == /\ pc[self] = "worker_inactive"
                         /\ IF workers[self] = FALSE
                               THEN /\ IF is_active = FALSE
                                          THEN /\ pc' = [pc EXCEPT ![self] = "worker_exit"]
                                          ELSE /\ pc' = [pc EXCEPT ![self] = "worker_inactive"]
                               ELSE /\ pc' = [pc EXCEPT ![self] = "worker_lock"]
                         /\ UNCHANGED << workers, workers_status, num_workers, 
                                         is_active, queue_lock_, scale_lock_, 
                                         cond_var_, queue_size_, stack, 
                                         worker_num_, worker_num, num, n, task >>

worker_lock(self) == /\ pc[self] = "worker_lock"
                     /\ workers_status' = [workers_status EXCEPT ![self] = TRUE]
                     /\ queue_lock_ = 0
                     /\ queue_lock_' = self
                     /\ pc' = [pc EXCEPT ![self] = "worker_check_active"]
                     /\ UNCHANGED << workers, num_workers, is_active, 
                                     scale_lock_, cond_var_, queue_size_, 
                                     stack, worker_num_, worker_num, num, n, 
                                     task >>

worker_check_active(self) == /\ pc[self] = "worker_check_active"
                             /\ IF workers[self] = FALSE
                                   THEN /\ Assert((queue_lock_ = self), 
                                                  "Failure of assertion at line 38, column 3 of macro called at line 278, column 5.")
                                        /\ queue_lock_' = 0
                                        /\ pc' = [pc EXCEPT ![self] = "worker_exit"]
                                   ELSE /\ pc' = [pc EXCEPT ![self] = "worker_get_task"]
                                        /\ UNCHANGED queue_lock_
                             /\ UNCHANGED << workers, workers_status, 
                                             num_workers, is_active, 
                                             scale_lock_, cond_var_, 
                                             queue_size_, stack, worker_num_, 
                                             worker_num, num, n, task >>

worker_get_task(self) == /\ pc[self] = "worker_get_task"
                         /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "get_task",
                                                                  pc        |->  "worker_check_task_empty" ] >>
                                                              \o stack[self]]
                         /\ pc' = [pc EXCEPT ![self] = "get_task_entry"]
                         /\ UNCHANGED << workers, workers_status, num_workers, 
                                         is_active, queue_lock_, scale_lock_, 
                                         cond_var_, queue_size_, worker_num_, 
                                         worker_num, num, n, task >>

worker_check_task_empty(self) == /\ pc[self] = "worker_check_task_empty"
                                 /\ IF task[self] = 0
                                       THEN /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "timed_wait_signal",
                                                                                     pc        |->  "worker_unlock" ] >>
                                                                                 \o stack[self]]
                                            /\ pc' = [pc EXCEPT ![self] = "timed_wait_signal_entry"]
                                       ELSE /\ pc' = [pc EXCEPT ![self] = "worker_signal_task"]
                                            /\ stack' = stack
                                 /\ UNCHANGED << workers, workers_status, 
                                                 num_workers, is_active, 
                                                 queue_lock_, scale_lock_, 
                                                 cond_var_, queue_size_, 
                                                 worker_num_, worker_num, num, 
                                                 n, task >>

worker_signal_task(self) == /\ pc[self] = "worker_signal_task"
                            /\ cond_var_' = TRUE
                            /\ pc' = [pc EXCEPT ![self] = "worker_unlock"]
                            /\ UNCHANGED << workers, workers_status, 
                                            num_workers, is_active, 
                                            queue_lock_, scale_lock_, 
                                            queue_size_, stack, worker_num_, 
                                            worker_num, num, n, task >>

worker_unlock(self) == /\ pc[self] = "worker_unlock"
                       /\ Assert((queue_lock_ = self), 
                                 "Failure of assertion at line 38, column 3 of macro called at line 291, column 3.")
                       /\ queue_lock_' = 0
                       /\ pc' = [pc EXCEPT ![self] = "worker_lock"]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, scale_lock_, cond_var_, 
                                       queue_size_, stack, worker_num_, 
                                       worker_num, num, n, task >>

worker_exit(self) == /\ pc[self] = "worker_exit"
                     /\ workers_status' = [workers_status EXCEPT ![self] = FALSE]
                     /\ pc' = [pc EXCEPT ![self] = "Done"]
                     /\ UNCHANGED << workers, num_workers, is_active, 
                                     queue_lock_, scale_lock_, cond_var_, 
                                     queue_size_, stack, worker_num_, 
                                     worker_num, num, n, task >>

worker(self) == worker_inactive(self) \/ worker_lock(self)
                   \/ worker_check_active(self) \/ worker_get_task(self)
                   \/ worker_check_task_empty(self)
                   \/ worker_signal_task(self) \/ worker_unlock(self)
                   \/ worker_exit(self)

producer_entry(self) == /\ pc[self] = "producer_entry"
                        /\ IF is_active = FALSE
                              THEN /\ pc' = [pc EXCEPT ![self] = "producer_exit"]
                              ELSE /\ pc' = [pc EXCEPT ![self] = "producer_loop"]
                        /\ UNCHANGED << workers, workers_status, num_workers, 
                                        is_active, queue_lock_, scale_lock_, 
                                        cond_var_, queue_size_, stack, 
                                        worker_num_, worker_num, num, n, task >>

producer_loop(self) == /\ pc[self] = "producer_loop"
                       /\ \/ /\ pc' = [pc EXCEPT ![self] = "producer_put_task"]
                          \/ /\ pc' = [pc EXCEPT ![self] = "producer_try_put_task"]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, queue_lock_, scale_lock_, 
                                       cond_var_, queue_size_, stack, 
                                       worker_num_, worker_num, num, n, task >>

producer_put_task(self) == /\ pc[self] = "producer_put_task"
                           /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "put_task",
                                                                    pc        |->  "producer_put_task_end" ] >>
                                                                \o stack[self]]
                           /\ pc' = [pc EXCEPT ![self] = "put_task_lock"]
                           /\ UNCHANGED << workers, workers_status, 
                                           num_workers, is_active, queue_lock_, 
                                           scale_lock_, cond_var_, queue_size_, 
                                           worker_num_, worker_num, num, n, 
                                           task >>

producer_put_task_end(self) == /\ pc[self] = "producer_put_task_end"
                               /\ pc' = [pc EXCEPT ![self] = "producer_entry"]
                               /\ UNCHANGED << workers, workers_status, 
                                               num_workers, is_active, 
                                               queue_lock_, scale_lock_, 
                                               cond_var_, queue_size_, stack, 
                                               worker_num_, worker_num, num, n, 
                                               task >>

producer_try_put_task(self) == /\ pc[self] = "producer_try_put_task"
                               /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "try_put_task",
                                                                        pc        |->  "producer_try_put_task_end" ] >>
                                                                    \o stack[self]]
                               /\ pc' = [pc EXCEPT ![self] = "try_put_task_lock"]
                               /\ UNCHANGED << workers, workers_status, 
                                               num_workers, is_active, 
                                               queue_lock_, scale_lock_, 
                                               cond_var_, queue_size_, 
                                               worker_num_, worker_num, num, n, 
                                               task >>

producer_try_put_task_end(self) == /\ pc[self] = "producer_try_put_task_end"
                                   /\ pc' = [pc EXCEPT ![self] = "producer_entry"]
                                   /\ UNCHANGED << workers, workers_status, 
                                                   num_workers, is_active, 
                                                   queue_lock_, scale_lock_, 
                                                   cond_var_, queue_size_, 
                                                   stack, worker_num_, 
                                                   worker_num, num, n, task >>

producer_exit(self) == /\ pc[self] = "producer_exit"
                       /\ TRUE
                       /\ pc' = [pc EXCEPT ![self] = "Done"]
                       /\ UNCHANGED << workers, workers_status, num_workers, 
                                       is_active, queue_lock_, scale_lock_, 
                                       cond_var_, queue_size_, stack, 
                                       worker_num_, worker_num, num, n, task >>

producer(self) == producer_entry(self) \/ producer_loop(self)
                     \/ producer_put_task(self)
                     \/ producer_put_task_end(self)
                     \/ producer_try_put_task(self)
                     \/ producer_try_put_task_end(self)
                     \/ producer_exit(self)

controller_loop(self) == /\ pc[self] = "controller_loop"
                         /\ \/ /\ pc' = [pc EXCEPT ![self] = "controller_scale"]
                            \/ /\ pc' = [pc EXCEPT ![self] = "controller_shutdown"]
                            \/ /\ pc' = [pc EXCEPT ![self] = "controller_enumerate_procs"]
                         /\ UNCHANGED << workers, workers_status, num_workers, 
                                         is_active, queue_lock_, scale_lock_, 
                                         cond_var_, queue_size_, stack, 
                                         worker_num_, worker_num, num, n, task >>

controller_scale(self) == /\ pc[self] = "controller_scale"
                          /\ \E n_ \in 0..max_workers:
                               /\ /\ num' = [num EXCEPT ![self] = n_]
                                  /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "scale",
                                                                           pc        |->  "controller_scale_end",
                                                                           num       |->  num[self] ] >>
                                                                       \o stack[self]]
                               /\ pc' = [pc EXCEPT ![self] = "scale_lock"]
                          /\ UNCHANGED << workers, workers_status, num_workers, 
                                          is_active, queue_lock_, scale_lock_, 
                                          cond_var_, queue_size_, worker_num_, 
                                          worker_num, n, task >>

controller_scale_end(self) == /\ pc[self] = "controller_scale_end"
                              /\ pc' = [pc EXCEPT ![self] = "controller_loop"]
                              /\ UNCHANGED << workers, workers_status, 
                                              num_workers, is_active, 
                                              queue_lock_, scale_lock_, 
                                              cond_var_, queue_size_, stack, 
                                              worker_num_, worker_num, num, n, 
                                              task >>

controller_enumerate_procs(self) == /\ pc[self] = "controller_enumerate_procs"
                                    /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "enumerate_procs",
                                                                             pc        |->  "controller_enumerate_procs_end" ] >>
                                                                         \o stack[self]]
                                    /\ pc' = [pc EXCEPT ![self] = "enumerate_procs_lock"]
                                    /\ UNCHANGED << workers, workers_status, 
                                                    num_workers, is_active, 
                                                    queue_lock_, scale_lock_, 
                                                    cond_var_, queue_size_, 
                                                    worker_num_, worker_num, 
                                                    num, n, task >>

controller_enumerate_procs_end(self) == /\ pc[self] = "controller_enumerate_procs_end"
                                        /\ pc' = [pc EXCEPT ![self] = "controller_loop"]
                                        /\ UNCHANGED << workers, 
                                                        workers_status, 
                                                        num_workers, is_active, 
                                                        queue_lock_, 
                                                        scale_lock_, cond_var_, 
                                                        queue_size_, stack, 
                                                        worker_num_, 
                                                        worker_num, num, n, 
                                                        task >>

controller_shutdown(self) == /\ pc[self] = "controller_shutdown"
                             /\ stack' = [stack EXCEPT ![self] = << [ procedure |->  "shutdown",
                                                                      pc        |->  "controller_exit",
                                                                      n         |->  n[self] ] >>
                                                                  \o stack[self]]
                             /\ n' = [n EXCEPT ![self] = 1]
                             /\ pc' = [pc EXCEPT ![self] = "shutdown_lock"]
                             /\ UNCHANGED << workers, workers_status, 
                                             num_workers, is_active, 
                                             queue_lock_, scale_lock_, 
                                             cond_var_, queue_size_, 
                                             worker_num_, worker_num, num, 
                                             task >>

controller_exit(self) == /\ pc[self] = "controller_exit"
                         /\ TRUE
                         /\ pc' = [pc EXCEPT ![self] = "Done"]
                         /\ UNCHANGED << workers, workers_status, num_workers, 
                                         is_active, queue_lock_, scale_lock_, 
                                         cond_var_, queue_size_, stack, 
                                         worker_num_, worker_num, num, n, task >>

controller(self) == controller_loop(self) \/ controller_scale(self)
                       \/ controller_scale_end(self)
                       \/ controller_enumerate_procs(self)
                       \/ controller_enumerate_procs_end(self)
                       \/ controller_shutdown(self)
                       \/ controller_exit(self)

(* Allow infinite stuttering to prevent deadlock on termination. *)
Terminating == /\ \A self \in ProcSet: pc[self] = "Done"
               /\ UNCHANGED vars

Next == (\E self \in ProcSet:  \/ timed_wait_signal(self) \/ get_task(self)
                               \/ add_task(self) \/ try_add_task(self)
                               \/ finalize(self) \/ stop_worker(self)
                               \/ create_new_thread(self) \/ scale(self)
                               \/ enumerate_procs(self) \/ shutdown(self)
                               \/ wait_task(self) \/ put_task(self)
                               \/ try_put_task(self))
           \/ (\E self \in workers_set: worker(self))
           \/ (\E self \in producers_set: producer(self))
           \/ (\E self \in controllers_set: controller(self))
           \/ Terminating

Spec == /\ Init /\ [][Next]_vars
        /\ \A self \in workers_set : /\ WF_vars(worker(self))
                                     /\ WF_vars(get_task(self))
                                     /\ WF_vars(timed_wait_signal(self))
        /\ \A self \in producers_set : /\ WF_vars(producer(self))
                                       /\ WF_vars(put_task(self))
                                       /\ WF_vars(try_put_task(self))
                                       /\ WF_vars(timed_wait_signal(self))
                                       /\ WF_vars(add_task(self))
                                       /\ WF_vars(wait_task(self))
                                       /\ WF_vars(try_add_task(self))
        /\ \A self \in controllers_set : /\ WF_vars(controller(self))
                                         /\ WF_vars(scale(self))
                                         /\ WF_vars(enumerate_procs(self))
                                         /\ WF_vars(shutdown(self))
                                         /\ WF_vars(finalize(self))
                                         /\ WF_vars(stop_worker(self))
                                         /\ WF_vars(create_new_thread(self))

Termination == <>(\A self \in ProcSet: pc[self] = "Done")

\* END TRANSLATION 

=============================================================================
