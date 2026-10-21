On Screen Debug Color Meanings



🔵: Informational / Passive State (“This is happening, but it’s not a decision or action.”)

* State presence (idle, ticking)
* Background logic
* Non-interactive updates
* Current active state
* Baseline evaluation
* Cached values / timers





🟢: Successful, Expected Behavior

* Valid state transitions
* Input accepted \& executed
* Normal gameplay flow





🟠: Suspicious / Edge-Case / Warning (“This shouldn’t happen often — investigate.”)

* Unexpected paths
* Fallback logic
* Edge-case recovery





🟣: Debugging Focus / High-Level Flow (“This is important for architecture-level understanding.”)

* State machine routing
* Priority arbitration
* Layer hand-offs (Movement ↔ Action)





🔴: Error / Invalid State

* Null pointers
* Invalid transitions
* Contract violations
* Missing owner / component
* State machine corruption
* Impossible state





🟡: Conditional / Blocked / Ignored (“Something tried to happen, but conditions stopped it.”)

* Input rejected
* Transition blocked
* Cooldown / lock / priority denial
* Jump pressed but locked
* Attack ignored due to hit-react
* State refused transition
