Modular Protocol Stack

MPS is a framework to ease the implementation of communication protocols and
combining various protocols to form a stack.

Features:
- Standardized layer interface.
- Buffers designed for protocol packets:
	- Adding removing of headers, trailers and payload.
	- Serializing a packet into a buffer.
	- Deseriazing buffer into a packet.
- Packet queues.
