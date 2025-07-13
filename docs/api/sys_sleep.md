sys_sleep
=========

**Prototype:**

```c
int sys_sleep(int seconds);
```

**Type:** `System Call`

Description
-----------

Suspends the execution of the current process for the specified number of seconds. This system call causes the current process to yield CPU time and resume execution after the specified delay period. The actual sleep time may be longer than requested due to system scheduling.

Parameters
----------

*   `int seconds` — Number of seconds to sleep (must be non-negative)

Returns
-------

Returns 0 on success, or -1 if the seconds parameter is negative.

Notes
-----

- System call number: `SYSTEM_COMMAND9_SLEEP` (9)
- Negative values are rejected and return an error
- The actual sleep time may be longer than requested due to system scheduling
- During sleep, the process does not consume CPU time
- Other processes can continue executing while this process sleeps
- The sleep is not interruptible by signals in this implementation
