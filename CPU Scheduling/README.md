## Goals of this assignment

Write a C program (`feedbackq.c`) implementing a simulation of
round-robin CPU scheduling that also makes use of a multi-level
feedback queue and an anti-starvation task boosting mechanism.

---

In effect you will be implementing a tick-by-tick simulation of this
single-core CPU scheduler for a set of CPU-bound and IO-bound tasks.
We will use abstract ticks for time rather than milliseconds or
microseconds.
 
### Overview of the input into, and output from, the simulator
`assign3/feedbackq.c`

Your implementation of `assign3/feedbackq.c` will accept a single argument
consisting of a text file with lines, the lines together containing
information about tasks for a particular simulation case. For example,
to run the first test case the following would be entered at the
command line:

`./feedbackq cases/test1.txt`

Therefore an input file is a representation of a simulation case,
where each line represents one of three possible facts. For example,
the following line contained within such a file:

```
13,3,0
```

indicates that at tick 13, task 3 has been created (i.e. the 0 means creation). A line in the file such as:

```
14,3,6
```

indicates that at tick 14, task 3 initiates an action that will require 6 ticks of CPU time. There may be many such CPU-tick lines for a task contained in the simulation case.

Lastly, there will appear a line in the file such as:

```
21,3,-1
```

here indicating that at tick 21, task 3 will terminate once its
remaining burst’s CPU ticks are scheduled. (If no such CPU ticks are
remaining, then the task would be said to terminate immediately.)

* The tick at which a task enters the system is shown (i.e., each
simulated tick appears on its own line).

* The currently-running task at the given tick is displayed on a line,
with some statistics about that task at the start of the tick. The
meaning of these statistics will be explained later in this
document.

* If there is no task to run at a tick, then the simulator outputs
IDLE for that tick. (A note on what IDLE means is given later in
this assignment description.)

* When a boost occurs, a message indicating the tick number and boost
message is displayed.


### Nature of the scheduling queue used by `feedbackq.c` ###

You are to implement a multi-level feedback queue, or MLFQ. An MLFQ is
designed to ensure the tasks requiring quick responses (i.e. which are
often characterized as having short CPU bursts) are scheduled before
tasks which are more compute bound (i.e. which are characterized as
having long – or at least longer – CPU bursts).

Your simulation will be of an MLFQ with three different queues: one
with a quantum of 2, one with a quantum of 4, and one with a quantum
of 8.

* When there is a CPU scheduling event, the queue corresponding to q=2
is examined. If it is not empty, then the task at the front is
selected to run and given a quantum of 2. Otherwise the queue
corresponding to q=4 is examined, and if it is not empty, then the
task at the front of this queue selected to run and given a quantum of
4. Otherwise the queue corresponding to q=8 is examined, with the task
at the front of this queue selected to run and given a quantum of 8.

* If a task selected from either the q=2 or q=4 queues finishes its
current burst within quantum provided to it, then it is placed at
the back of the queue from which it was taken.

* However if a task selected from either the q=2 or q=4 queues has a
burst that exceeds its quantum, then it is interrupted (as would be
the case for any round-robin algorithm), and placed into the next
queue with a larger quantum. That is, a task taken from q=2 would be
placed at the end of the q=4 queue; a task taken from q=4 would be
placed at the end of the q=8 queue.

* Without some sort of mechanism for giving tasks a chance to leave
the q=8 queue, a task might starve - for example, a task on the q=8
queue might never get a chance to resume if it is pre-empted by a
series of new tasks on a queue with a higher priority.  To prevent
this, the CPU must boost tasks from the q=8 queue up to the q=2 queue
periodically, to give them a chance to compete for CPU time with other
tasks.


### A word about `IDLE`

It may be the case that at some tick, there are either no tasks in the
task table, or the only tasks that are not yet terminated have
`remaining_burst_time` equal to zero. By definition this would mean
there are no tasks in the MLFQ, and therefore no tasks that can be
dispatched to the CPU. For that tick, therefore, the CPU is said to be
`IDLE`.


### Boosting

The program must prevent tasks from being starved for CPU time.
Starvation could occur if a task is relegated to the bottom queue
(q=8) and then a large number of new, short tasks are added.  These
new tasks would occupy the upper queues and run one after another,
with the CPU never getting a chance to resume the task on the bottom
queue.

To prevent this, you must implement a boost mechanic.  The `boost()`
function will be called just before the scheduler, and will be
responsible for moving all the tasks from the bottom queue (q=8) to
the top0 queue, and then for moving all the tasks from the middle
queue (q=4) to the top queue.  If there is a current task, it should
continue to be the current task, but its remaining time allocated
should be reduced to 2 if it is currently greater than 2.  Boosting,
like adding or exiting tasks, takes no CPU time.  This boost function
should run periodically -- if `BOOST_INTERVAL` is set to 25, a boost
should occur on tick 25, tick 50, and so on.