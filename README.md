oscilloscope architecture:

Problem 1: Capture data continuously
every 50 μs (20 kSPS), a new sample arrives.
The STM32 must save every one of them.


Problem 2: Display old data
At the same time, the OLED refreshes only about 20 times/sec.
Every 50 ms it asks:
"Can I see the last 0.2 seconds?"
That means it wants old samples.

adc_dma_buf: 
	Fresh samples that just arrived
	12.8 ms worth of data.
	It is not the oscilloscope memory.

history:
	Everything we've collected during the last 1s
	

Suppose adc_dma_buf holds 256 samples
	initially empty [] [] [] [] ..... [] [] [] []
	once sampling starts: DMA starts filling adc_dma_buf

	DMA raises an interrupt for:
		every 128 samples (half-transfer) and 
		every 256 samples (transfer complete). 
		
	The CPU, for every interrupt, then copies the newly received half-buffer into history.


So why not just make DMA buffer 20,000 samples?
	We actually could, since they're both memory locations anyway.
	we did it coz it keeps acquisition and storage seperate

	DMA continuously writes into a small temporary buffer
	CPU periodically copies completed blocks into the larger history buffer.
	This simplifies synchronization


Now the display wants to draw
	Suppose the user selects 0.2 seconds
	1 second: 20,000 samples
	so 0.2 second: 4,000 samples

	0.2 s → 4000 samples
	0.5 s → 10000 samples
	1.0 s → 20000 samples

	The display needs latest 4000 samples, which we already have. so we display it



Circular Buffer (concept):
	history stores exactly 20,000 samples.
	history_write always points to the next write location.
	When history_write reaches index 19999,
	it wraps back to index 0.
	This allows continuous sampling without shifting memory.
	The oldest samples are automatically overwritten by the newest samples.

	Both adc_dma_buf & history are circular.

	The DMA buffer is configured in DMA Circular Mode.
		
		0 1 2 ... 127 128 ... 255
		^                         |
		|_________________________|

		DMA writes new data into mem locations in this order:
		0 → 1 → 2 → ... → 255 → 0 → 1 → ...

	The history buffer is made circular in software.
	
		history_write = (history_write + 1) % HISTORY_SIZE;
		0 1 2 ... 19998 19999
		^                  |
		|__________________|


		



2 Problems:
	1. Waveform sliding
		If you keep appending the latest sample at the right end, 
		waveform keeps getting added, and slides sideways. 

		Solution: Triggering

			Instead of saying: "Draw from the newest sample"
			We say: "Find where the signal crosses 1.65 V going upward & draw from there"

			More detailed:
			Insted of displaying the history starting from the absolute latest sample,
			Search backwards from the newest sample for the most recent rising-edge crossing 
			(2048 ~1.65V from below to above), then start the displayed window from that trigger.

			The important word is rising edge. Not every sample equal to 2048.

			The wave appears frozen.

	2. Display Width limitation
		We have 4000 samples
		OLED width: 128 pixels

		Solution: Min Max Compression

			Each OLED column now represents: 4000 / 128 ≈ 31 samples
			Each OLED column represents a time interval rather than a single sample.
			
			How to compress multiple values into 1 without losing visial info?

			MinMax Compression:
				lets say you wanna condense 5 heights into 1 column on the oled.
				7 23 64 27 12
				pick min,max: 7,64
				on that oled column: draw a line from coordinates 7 to 64.
				when done, the peaks are preserved, the waveform looks faithul on the display.
