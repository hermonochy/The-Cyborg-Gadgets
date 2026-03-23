***Warning: A large portion of this manual and the shell itself are AI written.***

# Watch Shell Manual

## Overview

This manual documents the User Interface for the interactive shell on Watch 5.0. The shell provides direct access to GPIO control, variable management, timing functions, mathematical operations, loops, and display output.

## Command Reference

### 🔧 GPIO Control

Control digital and analog pins on the device.

| Command | Syntax | Description |
| ----- | ----- | ----- |
| Write Digital | `dw <pin> <state>` or `digitalWrite <pin> <state>` | Set pin HIGH (1) or LOW (0) |
| Read Digital | `dr <pin>` or `digitalRead <pin>` | Read pin state (0 or 1\) |
| Write PWM | `aw <pin> <val>` or `analogWrite <pin> <val>` | Set PWM duty cycle (0-255) |
| Read Analog | `ar <pin>` or `analogRead <pin>` | Read analog value (0-1023) |
| Set Mode | `pm <pin> <mode>` or `pinMode <pin> <mode>` | Set pin as `INPUT` or `OUTPUT` |

**Examples:**

```
dw 5 1               # Turn pin 5 HIGH
dr 1                 # Read pin 1 state
aw 0 2048            # Set pin 0 PWM to 50%
ar 2                 # Read analog value from pin 2
```

---

### 📊 Variables

Manage persistent and temporary variables.

| Command | Syntax | Description |
| ----- | ----- | ----- |
| Create/Set | `var <name> <value>` | Create or update a variable |
| Math Assignment | `let <var> = <expr>` | Evaluate math expression and assign |
| Increment | `inc <var>` | Add 1 to variable |
| Decrement | `dec <var>` | Subtract 1 from variable |
| List All | `vars` | Display all variables and values |
| Clear All | `clear` | Remove all variables |
| Save | `save <key>` | Persist variable to NVS storage |
| Load | `load <key>` | Restore variable from NVS storage |

**Supported Math Operators:** `+`, `-`, `*`, `/`

**Examples:**

```
var counter 0              # Initialize counter
let counter = counter + 1  # Increment via math
inc counter                # Alternative increment
vars                       # View all variables
save settings              # Persist to storage
load settings              # Restore from storage
```

---

### ⏱️ Timing

Delay and time-based operations.

| Command | Syntax | Description |
| ----- | ----- | ----- |
| Delay | `delay <ms>` | Pause execution (milliseconds) |
| Get Time | `millis` | Return elapsed milliseconds since boot |
| Blink | `blink <pin> <count> [<delay>]` | Blink LED `count` times |
| Pattern | `pattern <pin> <seq> [<delay>]` | Output binary pattern (e.g., `101010`) |
| Pulse | `pulse <pin> [<duration>]` | Generate single pulse |

**Examples:**

```
delay 1000               # Wait 1 second
millis                   # Check elapsed time
blink 5 10 200           # Blink pin 5, 10 times, 200ms apart
pattern 3 101010 100     # Output pattern on pin 3
pulse 7                  # Single pulse on pin 7
```

---

### 🧮 Math

Perform calculations.

| Command | Syntax | Description |
| ----- | ----- | ----- |
| Calculate | `calc <expr>` | Evaluate mathematical expression |
| Random | `random [<min> <max>]` | Generate random number |

**Examples:**

```
calc 10 + 5 * 2          # Returns 20
random 1 100             # Random number between 1-100
calc var1 + var2         # Use variables in expressions
```

---

### 🔄 Loops

Iterate through ranges.

| Command | Syntax | Description |
| ----- | ----- | ----- |
| For Loop | `for <var> <start> <end> [<step>]` | Loop through numeric range |

**Examples:**

```
for i 0 10               # Loop i from 0 to 10
for j 0 100 5            # Loop j from 0 to 100, step 5
```

---

### 📺 Display

Output to screen.

| Command | Syntax | Description |
| ----- | ----- | ----- |
| Print | `print <text>` | Display text on screen |
| Clear | `cls` | Clear the display |

**Examples:**

```
print Hello World        # Display text
cls                      # Clear screen
print Counter: 5         # Display variable value
```

---

### ⌨️ Help

Get assistance.

| Command | Syntax | Description |
| ----- | ----- | ----- |
| Help | `help` | Show complete command list |

---

## Quick Reference Card

```
GPIO:    dw/dr/aw/ar/pm  |  Vars:   var/let/inc/dec/save/load
Timing:  delay/millis/blink/pulse/pattern  |  Math:  calc/random
Loops:   for  |  Display:  print/cls  |  Help:  help
```

---

## Notes

* All pin numbers must be valid GPIO pins for your device  
* PWM values range from 0 (off) to 255 (full on)  
* Analog read returns 0-1023 (10-bit resolution)  
* NVS storage persists across device reboots  
* Commands are case-insensitive
