On Screen Debug Color Meanings



Blue: Informational / Passive State (“This is happening, but it’s not a decision or action.”)

* State presence (idle, ticking)
* Background logic
* Non-interactive updates
* Current active state
* Baseline evaluation
* Cached values / timers





Green: Successful, Expected Behavior

* Valid state transitions
* Input accepted \& executed
* Normal gameplay flow





Orange: Suspicious / Edge-Case / Warning (“This shouldn’t happen often — investigate.”)

* Unexpected paths
* Fallback logic
* Edge-case recovery





Purple: Debugging Focus / High-Level Flow (“This is important for architecture-level understanding.”)

* State machine routing
* Priority arbitration
* Layer hand-offs (Movement ↔ Action)





Red: Error / Invalid State

* Null pointers
* Invalid transitions
* Contract violations
* Missing owner / component
* State machine corruption
* Impossible state





Yellow: Conditional / Blocked / Ignored (“Something tried to happen, but conditions stopped it.”)

* Input rejected
* Transition blocked
* Cooldown / lock / priority denial
* Jump pressed but locked
* Attack ignored due to hit-react
* State refused transition
