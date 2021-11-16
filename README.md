
# API Project 2020

Project for the Algorithms and Principles of Computer Science course.

The project consist in a simple **text editor** implementation with specific commands. For more details, see the [specifications](https://github.com/lorecampa/ProjectTextEditor/blob/main/deliverables/specifications.pdf).

>Final Grade: 30L

## Commands
| Command      | Meaning | Usage|
| ----------- | ----------- | -------|
| `c` | change | (addr1, addr2)c |
| `d` | delete | (addr1, addr2)d |
| `p` | print | (addr1, addr2)p|
| `u` | undo | (number)u |
| `r` | redo | (number)r |
| `q` | quit | q |

## Example

**Input**
```
1,2c
first row
second row
.
2,3c
new second row
third row
.
1,3p
1,1c
new first row
.
1,2p
2,2d
4,5d
1,4p
3u
1,6p
1r
1,3p
q

```

**Output**
```
first row
new second row
third row
new first row
new second row
.
.
new first row
third row
.
new first row
new second row
third row
.
.
.
new first row
new second row
third row
```



## Tests
| Name      | Result |
| ----------- | ----------- |
| WriteOnly | 5/5 |
| BulkReads | 5/5 |
| TimeForAChange | 5/5 |
| RollingBack | 5/5 |
| AlteringHistory | 5/5 |
| Rollercoaster | 5/5 |
| Lode | 1/1 |

