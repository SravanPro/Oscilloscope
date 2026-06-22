<img width="1596" height="886" alt="image" src="https://github.com/user-attachments/assets/f3e064a4-b9d7-423f-ab69-4a3a783d1b7b" />https://github.com/user-attachments/assets/7ff37b86-3856-41c6-b837-532cd06acfe7

<img width="1597" height="885" alt="Image" src="https://github.com/user-attachments/assets/734fb604-ee99-4c05-b822-f6d6d4a41412" />

<img width="1596" height="886" alt="Image" src="https://github.com/user-attachments/assets/68f7d983-3de1-4ec8-9b33-40d1b5a0b2bd" />

# Oscilloscope Architecture

## Problem 1: Capture Data Continuously

Every **50 μs** (**20 kSPS**), a new sample arrives.

The STM32 must save every one of them.

---

## Problem 2: Display Old Data

At the same time, the OLED refreshes only about **20 times/sec**.

Every **50 ms** it asks:

> "Can I see the last 0.2 seconds?"

That means it wants old samples.

---

## `adc_dma_buf`

- Fresh samples that just arrived.
- **12.8 ms** worth of data.
- It is **not** the oscilloscope memory.

## `history`

- Everything we've collected during the last **1 s**.

---

Suppose `adc_dma_buf` holds **256 samples**.

Initially:

```text
[] [] [] [] ..... [] [] [] []
```

Once sampling starts, DMA begins filling `adc_dma_buf`.

DMA raises an interrupt for:

- Every **128 samples** (Half Transfer)
- Every **256 samples** (Transfer Complete)

For every interrupt, the CPU copies the newly received half-buffer into `history`.

---

## Why not make the DMA buffer 20,000 samples?

We actually could, since they're both memory locations anyway.

We chose not to because it keeps **acquisition** and **storage** separate.

- DMA continuously writes into a small temporary buffer.
- CPU periodically copies completed blocks into the larger history buffer.
- This simplifies synchronization.

---

## Displaying the Waveform

Suppose the user selects **0.2 seconds**.

- **1 second** → **20,000 samples**
- **0.2 second** → **4,000 samples**

| Time Window | Samples |
|-------------|--------:|
| 0.2 s | 4,000 |
| 0.5 s | 10,000 |
| 1.0 s | 20,000 |

The display needs the latest **4,000 samples**, which we already have, so we display them.

---

# Circular Buffer (Concept)

`history` stores exactly **20,000 samples**.

`history_write` always points to the next write location.

When `history_write` reaches index **19999**, it wraps back to index **0**.

This allows continuous sampling without shifting memory.

The oldest samples are automatically overwritten by the newest samples.

Both `adc_dma_buf` and `history` are circular.

---

## DMA Buffer

The DMA buffer is configured in **DMA Circular Mode**.

```text
0 1 2 ... 127 128 ... 255
^                         |
|_________________________|
```

DMA writes new data into memory in this order:

```text
0 → 1 → 2 → ... → 255 → 0 → 1 → ...
```

---

## History Buffer

The history buffer is made circular in software.

```c
history_write = (history_write + 1) % HISTORY_SIZE;
```

```text
0 1 2 ... 19998 19999
^                  |
|__________________|
```

---

# Two Problems

## 1. Waveform Sliding

If you keep appending the latest sample at the right end,

the waveform keeps getting added and slides sideways.

### Solution: Triggering

Instead of saying:

> "Draw from the newest sample"

We say:

> "Find where the signal crosses **1.65 V** going upward and draw from there."

### More Detailed

Instead of displaying the history starting from the absolute latest sample,

search backwards from the newest sample for the most recent **rising-edge crossing**
(**2048 ≈ 1.65 V**, from below to above), then start the displayed window from that trigger.

The important word is **rising edge**.

Not every sample equal to **2048**.

The wave appears frozen.

---

## 2. Display Width Limitation

We have:

- **4000 samples**
- OLED width: **128 pixels**

### Solution: Min-Max Compression

Each OLED column now represents:

```text
4000 / 128 ≈ 31 samples
```

Each OLED column represents a time interval rather than a single sample.

How do we compress multiple values into one column without losing visual information?

### Min-Max Compression

Suppose you want to condense five heights into one OLED column:

```text
7 23 64 27 12
```

Pick the minimum and maximum:

```text
7, 64
```

On that OLED column, draw a vertical line from coordinate **7** to **64**.

The peaks are preserved, so the waveform remains faithful to the original signal.
