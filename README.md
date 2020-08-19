Demonstrating multithreading programming in C. Development was done on a Raspberry Pi 3B running Raspbian GNU/Linux 10 (buster). Linux kernel version 4.19.118-v7+

  1) An Arduino Uno reads data from a potentiometer and sends it over a serial connection using USB
  2) A Raspberry Pi running a C program receives serial data. It has two concurernt threads running
    a) Serial Thread - Opens a serial port connection, receives data and puts it in a FIFO
    b) Receiver Thread - Extracts data from FIFO to be used
  3) The FIFO is self-developed and thread safe. 
  4) Mutexes are used to avoid race conditions with the FIFO
  
Work in progress - Uploading data extracted from FIFO to AWS IoT cloud platform
