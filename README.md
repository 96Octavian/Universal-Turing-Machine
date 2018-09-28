# Universal-Turing-Machine
C implementation of a single tape non deterministic universal turing machine\
Implementazione in C di una macchina di Turing universale non deterministica a nastro singolo

## Input
Input via stdin
```
tr
[state_number read_char write_char head_move next_state]
...
acc
[acceptance_state_number]
...
max
[max_steps]
run
[input strings]
...
```

## Output
1: the machine entered an acceptance state
0: the machine halted for in an invalid state
U: the machine did not halt before the specified number of steps